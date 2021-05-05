/* -------------------------------------------------------------------------------
* Copyright (c) 2018-2021, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the OLogN Technologies AG nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL OLogN Technologies AG BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/


#ifndef Q_BASED_INFRASTRUCTURE_H
#define Q_BASED_INFRASTRUCTURE_H

#include "../../src/clustering_impl/clustering_impl.h"
#include "../../src/clustering_impl/interthread_comm.h"
#include "../../src/clustering_impl/interthread_comm_impl.h"

#include "../include/nodecpp/common.h"
#include "../include/nodecpp/common_structs.h"
#include "../include/nodecpp/nls.h"

#include "ev_queue.h"

#include "../include/nodecpp/timers.h"
#include <functional>

#ifdef NODECPP_RECORD_AND_REPLAY
#include "tcp_socket/tcp_socket_replaying_loop.h"
#endif // NODECPP_RECORD_AND_REPLAY

#include "timeout_manager.h"

#include "clustering_impl/interthread_comm.h"

extern InterThreadMessagePostmanBase* useQueuePostman();

int getPollTimeout(uint64_t nextTimeoutAt, uint64_t now);
uint64_t infraGetCurrentTime();

extern thread_local TimeoutManager* timeoutManager;
extern thread_local EvQueue* inmediateQueue;

#ifndef NODECPP_NO_COROUTINES
inline
auto a_timeout_impl(uint32_t ms) { 

    struct timeout_awaiter {

        std::experimental::coroutine_handle<> who_is_awaiting;
		uint32_t duration = 0;
		nodecpp::Timeout to;

        timeout_awaiter(uint32_t ms) {duration = ms;}

        timeout_awaiter(const timeout_awaiter &) = delete;
        timeout_awaiter &operator = (const timeout_awaiter &) = delete;

        timeout_awaiter(timeout_awaiter &&) = delete;
        timeout_awaiter &operator = (timeout_awaiter &&) = delete;

        ~timeout_awaiter() {}

        bool await_ready() {
            return false;
        }

        void await_suspend(std::experimental::coroutine_handle<> awaiting) {
			nodecpp::initCoroData(awaiting);
            who_is_awaiting = awaiting;
			to = timeoutManager->appSetTimeout(awaiting, duration, infraGetCurrentTime());
        }

		auto await_resume() {
			if ( nodecpp::isCoroException(who_is_awaiting) )
				throw nodecpp::getCoroException(who_is_awaiting);
		}
    };
    return timeout_awaiter(ms);
}
#endif // NODECPP_NO_COROUTINES

template<typename NodeT, typename NodeAddressT, typename MessageT> concept has_global_mq_message_handler_call = requires { { std::declval<NodeT>().onGlobalMQMessage(std::declval<NodeAddressT>(),std::declval<MessageT&>()) }; };
template<typename NodeT, typename NodeAddressT, typename MessageT> concept has_infrastructure_message_handler_call = requires { { std::declval<NodeT>().onInfrastructureMessage(std::declval<NodeAddressT>(),std::declval<MessageT&>()) }; };

template<class Node>
struct InfraNodeWrapper
{
	using NlsT = nodecpp::NLS;
	using NodeT = Node;
	static constexpr bool has_global_mq_message_handler = has_global_mq_message_handler_call<NodeT, NodeAddress, nodecpp::platform::internal_msg::InternalMsg>;
	static constexpr bool has_infrastructure_message_handler = has_infrastructure_message_handler_call<NodeT, NodeAddress, nodecpp::platform::internal_msg::InternalMsg>;
	NlsT nls;
#ifdef NODECPP_USE_IIBMALLOC
		nodecpp::iibmalloc::ThreadLocalAllocatorT allocManager;
#endif
	safememory::owning_ptr<NodeT> node;
};

template<class UserNodeT>
class NodeProcessor
{
	using NodeType = InfraNodeWrapper<UserNodeT>;
	template<class InfraT, class Node> 
	friend class Runnable;
	
	TimeoutManager timeout;
	EvQueue immediateEvQueue;

	NodeType node;
public:
	NodeProcessor() {}
	~NodeProcessor() { deinit(); }

	safememory::soft_ptr<UserNodeT> getNode() { return node.node; }

	int init() {
#ifdef NODECPP_USE_IIBMALLOC
		node.allocManager.initialize();
		::nodecpp::iibmalloc::ThreadLocalAllocatorT* formerAlloc = ::nodecpp::iibmalloc::setCurrneAllocator( &(node.allocManager) );
#endif
		timeoutManager = &(getTimeout());
		inmediateQueue = &(getInmediateQueue());

		node.node = safememory::make_owning<typename NodeType::NodeT>();
#ifdef NODECPP_RECORD_AND_REPLAY
		if ( replayMode == nodecpp::record_and_replay_impl::BinaryLog::Mode::recording )
			node.node->binLog.initForRecording( 26 );
		::nodecpp::threadLocalData.binaryLog = &(node.node->binLog);
#endif // NODECPP_RECORD_AND_REPLAY
		// NOTE!!! 
		// By coincidence it so happened that both void Node::main() and nodecpp::handler_ret_type Node::main() are currently treated in the same way.
		// If, for any reason, treatment should be different, to check exactly which one is present, see, for instance
		// http://www.gotw.ca/gotw/071.htm and 
		// https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
#ifdef NODECPP_RECORD_AND_REPLAY
		if ( replayMode == nodecpp::record_and_replay_impl::BinaryLog::Mode::recording )
			::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::node_main_call, nullptr, 0 );
#endif // NODECPP_RECORD_AND_REPLAY
		node.node->main();

		uint64_t nextTimeoutAt = nextTimeout();
		uint64_t now = infraGetCurrentTime();
		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

		timeoutManager = nullptr;
		inmediateQueue = nullptr;
		safememory::detail::killAllZombies();

#ifdef NODECPP_USE_IIBMALLOC
		::nodecpp::iibmalloc::setCurrneAllocator( formerAlloc );
#endif
		return timeoutToUse;
	}

	void deinit()
	{
#ifdef NODECPP_USE_IIBMALLOC
		::nodecpp::iibmalloc::ThreadLocalAllocatorT* formerAlloc = ::nodecpp::iibmalloc::setCurrneAllocator( &(node.allocManager) );
#endif
		node.node = nullptr;
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
		nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		safememory::detail::killAllZombies();
#ifdef NODECPP_USE_IIBMALLOC
		::nodecpp::iibmalloc::setCurrneAllocator( formerAlloc );
#endif
	}

public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	TimeoutManager& getTimeout() { return timeout; }
	EvQueue& getInmediateQueue() { return immediateEvQueue; }
	void emitInmediates() { immediateEvQueue.emit(); }

	bool refedTimeout() const noexcept
	{
		return !immediateEvQueue.empty() || timeout.infraRefedTimeout();
	}

	uint64_t nextTimeout() const noexcept
	{
		return immediateEvQueue.empty() ? timeout.infraNextTimeout() : 0;
	}

	int processMessagesAndOrTimeout( InterThreadMsg* thq )
	{
		nodecpp::nodeLocalData = &(node.nls);

#ifdef NODECPP_USE_IIBMALLOC
		::nodecpp::iibmalloc::ThreadLocalAllocatorT* formerAlloc = ::nodecpp::iibmalloc::setCurrneAllocator( &(node.allocManager) );
#endif
		timeoutManager = &(getTimeout());
		inmediateQueue = &(getInmediateQueue());

		EvQueue queue;
		uint64_t now = infraGetCurrentTime();
		timeout.infraTimeoutEvents(now, queue);
		queue.emit();

		if ( thq )
		{
			switch( thq->msgType )
			{
				case InterThreadMsgType::Infrastructural:
				{
					if constexpr ( NodeType::has_infrastructure_message_handler )
						node.node->onInfrastructureMessage( thq->sourceThreadID, thq->msg );
					else
						throw std::exception(); // unexpected / unhandled message type
					break;
				}
				case InterThreadMsgType::GlobalMQ:
				{
					if constexpr ( NodeType::has_global_mq_message_handler )
						node.node->onGlobalMQMessage( thq->sourceThreadID, thq->msg );
					else
						throw std::exception(); // unexpected / unhandled message type
					break;
				}
				default:
					throw std::exception(); // unexpected
			}
		}

		emitInmediates();

		bool refed = refedTimeout();
		uint64_t nextTimeoutAt = nextTimeout();
		now = infraGetCurrentTime();
		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

		timeoutManager = nullptr;
		inmediateQueue = nullptr;
		safememory::detail::killAllZombies();
#ifdef NODECPP_USE_IIBMALLOC
		::nodecpp::iibmalloc::setCurrneAllocator( formerAlloc );
#endif

		nodecpp::nodeLocalData = nullptr;

		return timeoutToUse;
	}

	void run() {}
};

template<class NodeT>
class QueueBasedInfrastructure : public NodeProcessor<NodeT>
{
	bool running = true;
	int timeoutToUse = 0;
public:
	QueueBasedInfrastructure() : NodeProcessor<NodeT>() {}

	void init()
	{
		timeoutToUse = NodeProcessor<NodeT>::init();
	}

	void run()
	{
		while (running) // TODO: exit condition
		{
			static constexpr size_t maxMsgCnt = 8;
			ThreadQueueItem thq[maxMsgCnt];
			size_t actualFromQueue = popFrontFromThisThreadQueue( thq, maxMsgCnt, timeoutToUse );

			if ( actualFromQueue )
				for ( size_t i=0; i<actualFromQueue; ++i )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thq[i].invalidRecipientID == 0 ); // current version limitation: just a single node per thread
					timeoutToUse = NodeProcessor<NodeT>::processMessagesAndOrTimeout( &(thq[i].msg) );
				}
			else
				timeoutToUse = NodeProcessor<NodeT>::processMessagesAndOrTimeout( nullptr );
		}
	}
};


template<class NodeT>
class NodeLoopBase
{
public:
	class Initializer
	{
		ThreadStartupData data;
		friend class NodeLoopBase;
		void acquire(InterThreadMessagePostmanBase* postman) {
			preinitThreadStartupData( data, postman );
		}
	public:
		Initializer() {}
		Initializer( const Initializer& ) = default;
		Initializer& operator = ( const Initializer& ) = default;
		Initializer( Initializer&& ) = default;
		Initializer& operator = ( Initializer&& ) = default;
	};

private:
	ThreadStartupData loopStartupData;
	bool initialized = false;
	bool entered = false;

public:
	NodeLoopBase() {}
	NodeLoopBase( Initializer i ) { 
		loopStartupData = i.data;
		setThisThreadDescriptor( i.data ); 
		nodecpp::logging_impl::currentLog = i.data.defaultLog;
		nodecpp::logging_impl::instanceId = i.data.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", i.data.threadCommID.slotId );
		initialized = true;
	}

	NodeAddress getAddress() { return NodeAddress( loopStartupData.threadCommID, loopStartupData.IdWithinGroup ); } // TODO: details of addressing

	static std::pair<Initializer, NodeAddress> getInitializer(InterThreadMessagePostmanBase* postman)
	{
		Initializer i;
		i.acquire(postman);
		return std::make_pair(i, NodeAddress(i.data.threadCommID, 0));
	}

protected:
	template<class InfraT>
	int init( InfraT& infra, uint64_t nodeID, InterThreadMessagePostmanBase* postman = nullptr )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !entered ); 
		entered = true;
		// note: startup data must be allocated using std allocator (reason: freeing memory will happen at a new thread)
		if ( !initialized )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, postman != nullptr ); 
			preinitThreadStartupData( loopStartupData, postman );
			loopStartupData.IdWithinGroup = nodeID;
			initialized = true;
		}
		size_t threadIdx = loopStartupData.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, loopStartupData.threadCommID.slotId != 0 ); 
		setThisThreadDescriptor( loopStartupData );
		nodecpp::logging_impl::currentLog = loopStartupData.defaultLog;
		nodecpp::logging_impl::instanceId = loopStartupData.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", loopStartupData.threadCommID.slotId );

		return infra.init();
	}

	template<class InfraT>
	void run( InfraT& infra)
	{
		infra.run();
	}
};


template<class NodeT>
class NoNodeLoop : public NodeLoopBase<NodeT>
{
	NodeProcessor<NodeT> infra;
public:
	NoNodeLoop() {}
	NoNodeLoop( typename NodeLoopBase<NodeT>::Initializer i ) : NodeLoopBase<NodeT>( i ) {}

	safememory::soft_ptr<NodeT> getNode() { return infra.getNode(); }
	
	int init( uint64_t nodeID, InterThreadMessagePostmanBase* postman = nullptr )
	{
		return NodeLoopBase<NodeT>::template init<NodeProcessor<NodeT>>(infra, nodeID, postman);
	}

	int onInfrastructureMessage( InterThreadMsg&& thq )
	{
		return infra.processMessagesAndOrTimeout( &thq );
	}

	int onTimeout()
	{
		return infra.processMessagesAndOrTimeout( nullptr );
	}
};

template<class NodeT>
class QueueBasedNodeLoop : public NodeLoopBase<NodeT>
{
	QueueBasedInfrastructure<NodeT> infra;
public:
	QueueBasedNodeLoop() {}
	QueueBasedNodeLoop( typename NodeLoopBase<NodeT>::Initializer i ) : NodeLoopBase<NodeT>( i ) {}

	safememory::soft_ptr<NodeT> getNode() { return infra.getNode(); }
	
	int init( uint64_t nodeID, InterThreadMessagePostmanBase* postman = nullptr )
	{
		return NodeLoopBase<NodeT>::template init<NodeProcessor<NodeT>>(infra, nodeID, postman);
	}
	
	int init( InterThreadMessagePostmanBase* postman = nullptr )
	{
		return NodeLoopBase<NodeT>::template init<NodeProcessor<NodeT>>(infra, (uint64_t)(-1), postman);
	}

	void run()
	{
		NodeLoopBase<NodeT>::template run<QueueBasedInfrastructure<NodeT>>(infra);
	}
};

#endif // Q_BASED_INFRASTRUCTURE_H
