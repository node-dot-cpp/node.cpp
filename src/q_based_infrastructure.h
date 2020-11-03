/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
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

#include <clustering_impl/clustering_impl.h>
#include <clustering_impl/interthread_comm.h>
#include <clustering_impl/interthread_comm_impl.h>

#include "../include/nodecpp/common.h"
#include "../include/nodecpp/nls.h"

#include "ev_queue.h"

#include "../include/nodecpp/timers.h"
#include <functional>

#ifdef NODECPP_RECORD_AND_REPLAY
#include "tcp_socket/tcp_socket_replaying_loop.h"
#endif // NODECPP_RECORD_AND_REPLAY

#include "timeout_manager.h"

int getPollTimeout(uint64_t nextTimeoutAt, uint64_t now);
uint64_t infraGetCurrentTime();


#include "clustering_impl/interthread_comm.h"

class Infrastructure
{
	template<class Node> 
	friend class Runnable;

	TimeoutManager timeout;
	EvQueue inmediateQueue;

public:
	Infrastructure() {}

public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	TimeoutManager& getTimeout() { return timeout; }
	EvQueue& getInmediateQueue() { return inmediateQueue; }
	void emitInmediates() { inmediateQueue.emit(); }

	bool refedTimeout() const noexcept
	{
		return !inmediateQueue.empty() || timeout.infraRefedTimeout();
	}

	uint64_t nextTimeout() const noexcept
	{
		return inmediateQueue.empty() ? timeout.infraNextTimeout() : 0;
	}

	template<class NodeT>
	int processMessagesAndOrTimeout( NodeT& node, InterThreadMsg* thq, size_t msgCnt )
	{
		EvQueue queue;
		uint64_t now = infraGetCurrentTime();
		timeout.infraTimeoutEvents(now, queue);
		queue.emit();

		for ( size_t i=0; i<msgCnt; ++i )
			if ( thq[i].msgType == InterThreadMsgType::Infrastructural )
			{
				nodecpp::platform::internal_msg::InternalMsg::ReadIter riter = thq[i].msg.getReadIter();
				node.onInfrastructureMessage( thq[i].sourceThreadID, riter );
			}
			else
			{
				// TODO: ...
				;
			}

		emitInmediates();

		bool refed = refedTimeout();
		uint64_t nextTimeoutAt = nextTimeout();
		now = infraGetCurrentTime();
		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

		return timeoutToUse;
	}

	template<class NodeT>
	void runStandardLoop( NodeT& node )
	{
		int timeoutToUse = 0;
		while (running)
		{
			static constexpr size_t maxMsgCnt = 8;
			InterThreadMsg thq[maxMsgCnt];
			size_t actualFromQueue = popFrontFromThisThreadQueue( thq, maxMsgCnt, timeoutToUse );

			timeoutToUse = processMessagesAndOrTimeout( node, thq, actualFromQueue );
		}
	}
};


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

extern thread_local NodeBase* thisThreadNode;
template<class Node>
class Runnable : public RunnableBase
{
	nodecpp::safememory::owning_ptr<Node> node;

	void internalRun( bool isMaster, ThreadStartupData* startupData )
	{
		nodecpp::safememory::interceptNewDeleteOperators(true);
		{
			Infrastructure infra;
			timeoutManager = &infra.getTimeout();
			inmediateQueue = &infra.getInmediateQueue();

			node = nodecpp::safememory::make_owning<Node>();
			thisThreadNode = &(*node); 
#ifdef NODECPP_RECORD_AND_REPLAY
			if ( replayMode == nodecpp::record_and_replay_impl::BinaryLog::Mode::recording )
				node->binLog.initForRecording( 26 );
			::nodecpp::threadLocalData.binaryLog = &(node->binLog);
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
			node->main();
			infra.runStandardLoop(*node);
			node = nullptr;

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		}
		nodecpp::safememory::killAllZombies();
		nodecpp::safememory::interceptNewDeleteOperators(false);
	}

public:
	using NodeType = Node;
	Runnable() {}
	void run(bool isMaster, ThreadStartupData* startupData) override
	{
		return internalRun(isMaster, startupData);
	}
};


template<class NodeT>
class SimplePollLoop
{
public:
	class Initializer
	{
		ThreadStartupData data;
		friend class SimplePollLoop;
		void acquire() {
			preinitThreadStartupData( data );
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
	SimplePollLoop() {}
	SimplePollLoop( Initializer i ) { 
		loopStartupData = i.data;
		setThisThreadDescriptor( i.data ); 
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
		nodecpp::logging_impl::currentLog = i.data.defaultLog;
		nodecpp::logging_impl::instanceId = i.data.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", i.data.threadCommID.slotId );
		initialized = true;
	}

	static std::pair<Initializer, ThreadID> getInitializer()
	{
		Initializer i;
		i.acquire();
		return std::make_pair(i, i.data.threadCommID);
	}
	
	void run()
	{
		// note: startup data must be allocated using std allocator (reason: freeing memory will happen at a new thread)
		if ( !initialized )
		{
			preinitThreadStartupData( loopStartupData );
			initialized = true;
		}
		size_t threadIdx = loopStartupData.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, loopStartupData.threadCommID.slotId != 0 ); 
		setThisThreadDescriptor( loopStartupData );
#ifdef NODECPP_USE_IIBMALLOC
		g_AllocManager.initialize();
#endif
		nodecpp::logging_impl::currentLog = loopStartupData.defaultLog;
		nodecpp::logging_impl::instanceId = loopStartupData.threadCommID.slotId;
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "starting Node thread with threadID = {}", loopStartupData.threadCommID.slotId );
		Runnable<NodeT> r;
		r.run( false, &loopStartupData );


		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
	}

	ThreadID getAddress() { return loopStartupData.threadCommID; }
};

#endif // Q_BASED_INFRASTRUCTURE_H
