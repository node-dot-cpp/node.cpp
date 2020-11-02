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

	NetSockets ioSockets;
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
	bool pollPhase2( NodeT& node, bool refed, uint64_t nextTimeoutAt, uint64_t now )
	{
		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);
		auto ret = ioSockets.wait( timeoutToUse );

		if ( !ret.first )
		{
			return refed;
		}

		int retval = ret.second;

		if (retval < 0)
		{
			return false;
		}
		else if (retval == 0)
		{
			//timeout, just return with empty queue
			return true; 
		}
		else //if(retval)
		{
			static constexpr size_t maxMsgCnt = 8;
			InterThreadMsg thq[maxMsgCnt];
			size_t actaulFromSock = 8;
			size_t actualFromQueue = popFrontFromThisThreadQueue( thq, actaulFromSock, timeoutToUse );
//			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actualFromQueue == actaulFromSock, "{} vs. {}", actualFromQueue, actaulFromSock );
			for ( size_t i=0; i<actualFromQueue; ++i )
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
			return true;
		}
	}

	void doBasicInitialization()
	{
//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,isNetInitialized());
	}

	template<class NodeT>
	void runStandardLoop( NodeT& node )
	{
		while (running)
		{

			EvQueue queue;

			queue.emit();

			uint64_t now = infraGetCurrentTime();
			timeout.infraTimeoutEvents(now, queue);
			queue.emit();

			now = infraGetCurrentTime();
			bool refed = pollPhase2( node, refedTimeout(), nextTimeout(), now );
			if(!refed)
				return;

			queue.emit();
			emitInmediates();

			queue.emit();
		}
	}
};


/*#ifdef NODECPP_ENABLE_CLUSTERING
inline
void registerAgentServer(soft_ptr<Cluster::AgentServer> t)
{
	return netServerManagerBase->appAddAgentServer(t);
}
inline
SOCKET acquireSocketAndLetInterThreadCommServerListening(nodecpp::Ip4 ip, uint16_t& port, int backlog)
{
	return netServerManagerBase->acquireSocketAndLetInterThreadCommServerListening( ip, port, backlog );
}
inline
std::pair<std::pair<SOCKET, uint16_t>, std::pair<SOCKET, uint16_t>> acquireAndConnectSocketForInterThreadComm( SOCKET interThreadCommServerSock, const char* ip, uint16_t destinationPort )
{
	return netServerManagerBase->acquireAndConnectSocketForInterThreadComm( interThreadCommServerSock, ip, destinationPort );
}
#endif // NODECPP_ENABLE_CLUSTERING*/

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
			infra.doBasicInitialization();
#ifdef NODECPP_ENABLE_CLUSTERING
			/*if ( isMaster )
			{
				uintptr_t readHandle = initInterThreadCommSystemAndGetReadHandleForMainThread();
				infra.ioSockets.setAwakerSocket( readHandle );
			}
			else
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, startupData != nullptr );
				infra.ioSockets.setAwakerSocket( startupData->readHandle );
			}
#endif
			// from now on all internal structures are ready to use; let's run their "users"
#ifdef NODECPP_ENABLE_CLUSTERING
			nodecpp::postinitThreadClusterObject();*/
			// TODO_XXX: initializing (see above)
			/*if ( isMaster )
			{
				size_t listenerCnt = 1;
				auto argv = getArgv();
				for ( size_t i=1; i<argv.size(); ++i )
				{
					if ( argv[i].size() > 13 && argv[i].substr(0,13) == "numlisteners=" )
						listenerCnt = atol(argv[i].c_str() + 13);
				}
				for ( size_t i=0; i<listenerCnt; ++i )
					createListenerThread();
			}*/
#endif // NODECPP_ENABLE_CLUSTERING

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

#endif // Q_BASED_INFRASTRUCTURE_H
