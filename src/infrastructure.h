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


#ifndef INFRASTRUCTURE_H
#define INFRASTRUCTURE_H

#include "../include/nodecpp/common.h"
#include "../include/nodecpp/nls.h"

#include "ev_queue.h"
#include "tcp_socket/tcp_socket.h"

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

	//nodecpp::vector<NetSocketEntry> ioSockets;
	NetSockets ioSockets;
	NetSocketManager netSocket;
	NetServerManager netServer;
	TimeoutManager timeout;
	EvQueue inmediateQueue;

public:
	Infrastructure() : netSocket(ioSockets), netServer(ioSockets) {}

public:
	NetSocketManagerBase& getNetSocketBase() { return netSocket; }
public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	NetSocketManager& getNetSocket() { return netSocket; }
	NetServerManager& getNetServer() { return netServer; }
	TimeoutManager& getTimeout() { return timeout; }
	EvQueue& getInmediateQueue() { return inmediateQueue; }
//	void setInmediate(std::function<void()> cb) { inmediateQueue.add(std::move(cb)); }
	void emitInmediates() { inmediateQueue.emit(); }

	bool refedTimeout() const noexcept
	{
		return !inmediateQueue.empty() || timeout.infraRefedTimeout();
//		return timeout.infraRefedTimeout();
	}

	uint64_t nextTimeout() const noexcept
	{
		return inmediateQueue.empty() ? timeout.infraNextTimeout() : 0;
//		return timeout.infraNextTimeout();
	}

	template<class NodeT>
	bool pollPhase2( NodeT& node, bool refed, uint64_t nextTimeoutAt, uint64_t now )
	{
/*		size_t fds_sz;
		pollfd* fds_begin;
		auto pollfdRet = ioSockets.getPollfd();
		fds_sz = pollfdRet.second;
		fds_begin = pollfdRet.first;
		if ( fds_sz == 0 ) // if (refed == false && refedSocket == false) return false; //stop here'
			return false;

		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

#ifdef _MSC_VER
		int retval = WSAPoll(fds_begin, static_cast<ULONG>(fds_sz), timeoutToUse);
#else
		int retval = poll(fds_begin, fds_sz, timeoutToUse);
#endif
*/
		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);
#ifdef USE_TEMP_PERF_CTRS
extern thread_local size_t waitTime;
size_t now1 = infraGetCurrentTime();
		auto ret = ioSockets.wait( timeoutToUse );
waitTime += infraGetCurrentTime() - now1;
#else
		auto ret = ioSockets.wait( timeoutToUse );
#endif

		if ( !ret.first )
		{
			return refed;
		}

		int retval = ret.second;

		if (retval < 0)
		{
#ifdef _MSC_VER
			int error = WSAGetLastError();
			//		if ( error == WSAEWOULDBLOCK )
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"error {}", error);
#else
			perror("select()");
			//		int error = errno;
			//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
			/*        return WAIT_RESULTED_IN_TIMEOUT;*/
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,false);
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"COMMLAYER_RET_FAILED");
			return false;
		}
		else if (retval == 0)
		{
			//timeout, just return with empty queue
			return true; 
		}
		else //if(retval)
		{
#ifdef USE_TEMP_PERF_CTRS
extern thread_local int pollCnt;
extern thread_local int pollRetCnt;	
extern thread_local int pollRetMax;	
extern thread_local int pollRetCnt;
extern thread_local int zeroSockCnt;
extern thread_local uint64_t eventProcTime;
extern thread_local int eventCnt;

++pollCnt;
pollRetCnt += retval;
if ( pollRetMax < retval ) 
	pollRetMax = retval;

#endif // USE_TEMP_PERF_CTRS
			int processed = 0;
#ifdef NODECPP_ENABLE_CLUSTERING
			short revents = ioSockets.reventsAt(ioSockets.awakerSockIdx);
			if ( revents && (int64_t)(ioSockets.socketsAt(ioSockets.awakerSockIdx)) > 0 )
			{
#ifdef USE_TEMP_PERF_CTRS
++zeroSockCnt;
#endif // USE_TEMP_PERF_CTRS
				++processed;
				// TODO: see infraCheckPollFdSet() for more details to be implemented
				if ( clusterIsMaster() )
				{
					if ((revents & POLLIN) != 0)
					{
						static constexpr size_t maxMsgCnt = 8;
						uint8_t recvBuffer[maxMsgCnt];
						size_t actaulFromSock = 0;
						bool res = OSLayer::infraGetPacketBytes(recvBuffer, maxMsgCnt, actaulFromSock, ioSockets.getAwakerSockSocket());
						if (res)
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actaulFromSock <= maxMsgCnt, "{} vs. {}", actaulFromSock, maxMsgCnt );
							InterThreadMsg thq[maxMsgCnt];
							size_t actualFromQueue = popFrontFromThisThreadQueue( thq, actaulFromSock );
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actualFromQueue == actaulFromSock, "{} vs. {}", actualFromQueue, actaulFromSock );
							for ( size_t i=0; i<actualFromQueue; ++i )
								getCluster().onInterthreadMessage( thq[i] );
						}
						else
						{
							internal_usage_only::internal_getsockopt_so_error(ioSockets.getAwakerSockSocket());
							// TODO: process error
						}
					}
				}
				else
				{
					if ((revents & POLLIN) != 0)
					{
						static constexpr size_t maxMsgCnt = 8;
						uint8_t recvBuffer[maxMsgCnt];
						size_t actaulFromSock = 0;
						bool res = OSLayer::infraGetPacketBytes(recvBuffer, maxMsgCnt, actaulFromSock, ioSockets.getAwakerSockSocket());
						if (res)
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actaulFromSock <= maxMsgCnt, "{} vs. {}", actaulFromSock, maxMsgCnt );
							InterThreadMsg thq[maxMsgCnt];
							size_t actualFromQueue = popFrontFromThisThreadQueue( thq, actaulFromSock );
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, actualFromQueue == actaulFromSock, "{} vs. {}", actualFromQueue, actaulFromSock );
							for ( size_t i=0; i<actualFromQueue; ++i )
								getCluster().slaveProcessor.onInterthreadMessage( thq[i] );
						}
						else
						{
							internal_usage_only::internal_getsockopt_so_error(ioSockets.getAwakerSockSocket());
							// TODO: process error
						}
					}
				}
			}
#endif // NODECPP_ENABLE_CLUSTERING
				
#ifdef USE_TEMP_PERF_CTRS
size_t now2 = infraGetCurrentTime();

#endif // #ifdef NODECPP_ENABLE_CLUSTERING
			for ( size_t i=ioSockets.reserved_capacity; processed<retval; ++i)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, i<=ioSockets.size(), "i={}, processed={}, retval={}, ioSockets.size()={}", i, processed, retval, ioSockets.size());
				short revents = ioSockets.reventsAt( i );
#ifdef NODECPP_LINUX
				if ( revents )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, (int64_t)(ioSockets.socketsAt(i)) > 0, "indeed: {}", (int64_t)(ioSockets.socketsAt(i)) );
#else
				if ( revents && (int64_t)(ioSockets.socketsAt(i)) > 0 ) // on Windows WSAPoll() may set revents to a non-zero value despite the socket is invalid
				{
#endif
#ifdef USE_TEMP_PERF_CTRS
++eventCnt;
#endif // USE_TEMP_PERF_CTRS
					++processed;
					NetSocketEntry& current = ioSockets.at( i );
					if ( current.isAssociated() )
					{
						switch ( current.emitter.objectType )
						{
							case OpaqueEmitter::ObjectType::ClientSocket:
								netSocket. infraCheckPollFdSet(current, revents);
								break;
							case OpaqueEmitter::ObjectType::ServerSocket:
							case OpaqueEmitter::ObjectType::AgentServer:
								netServer. infraCheckPollFdSet(current, revents);
								break;
							default:
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected value {}", (int)(current.emitter.objectType) );
								break;
						}
					}
				}
			}
#ifdef USE_TEMP_PERF_CTRS
eventProcTime += infraGetCurrentTime() - now2;
#endif

#ifdef NODECPP_ENABLE_CLUSTERING
			if ( getCluster().isWorker() )
				netServer. infraEmitAcceptedSocketEventsReceivedfromMaster();
#endif // NODECPP_ENABLE_CLUSTERING
			return true;
		}
	}

	void doBasicInitialization()
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,isNetInitialized());

		ioSockets.reworkIfNecessary();
	}

	template<class NodeT>
	void runStandardLoop( NodeT& node )
	{
extern thread_local uint64_t eventProcTime;
#ifdef USE_TEMP_PERF_CTRS
size_t now2 = infraGetCurrentTime();
#endif
		while (running)
		{

			EvQueue queue;

			netServer.infraGetPendingEvents(queue);
			netServer. infraEmitListeningEvents();
			queue.emit();

			uint64_t now = infraGetCurrentTime();
#ifdef USE_TEMP_PERF_CTRS
			reportTimes( now );
#endif
			timeout.infraTimeoutEvents(now, queue);
			queue.emit();

#ifdef USE_TEMP_PERF_CTRS
eventProcTime += infraGetCurrentTime() - now2;
#endif
			now = infraGetCurrentTime();
			bool refed = pollPhase2( node, refedTimeout(), nextTimeout(), now );
			if(!refed)
				return;

#ifdef USE_TEMP_PERF_CTRS
now2 = infraGetCurrentTime();
#endif
			queue.emit();
			emitInmediates();

			netSocket. infraGetCloseEvent(/*queue*/);
			netSocket. infraProcessSockAcceptedEvents();
			netServer. infraGetCloseEvents(/*queue*/);
			queue.emit();

//			netSocket.infraClearStores();
			netServer.infraClearStores();

			ioSockets.reworkIfNecessary();
		}
	}
};

inline
void registerWithInfraAndAcquireSocket(nodecpp::soft_ptr<net::SocketBase> t)
{
#ifdef NODECPP_RECORD_AND_REPLAY
	if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
		NodeReplayer::registerAndAssignSocket(t);
	}
	else
#endif // NODECPP_RECORD_AND_REPLAY
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
		netSocketManagerBase->appAcquireSocket(t);
	}
}

inline
void registerWithInfraAndAssignSocket(nodecpp::soft_ptr<net::SocketBase> t, OpaqueSocketData& sdata)
{
#ifdef NODECPP_RECORD_AND_REPLAY
	if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
		NodeReplayer::registerAndAssignSocket(t);
	}
	else
#endif // NODECPP_RECORD_AND_REPLAY
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
		netSocketManagerBase->appAssignSocket(t, sdata);
	}
}

inline
void connectSocket(net::SocketBase* s, const char* ip, uint16_t port)
{
#ifdef NODECPP_RECORD_AND_REPLAY
	if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
	{
		return NodeReplayer::appConnectSocket(s, ip, port);
	}
	else
#endif // NODECPP_RECORD_AND_REPLAY
	{
		netSocketManagerBase->appConnectSocket(s, ip, port);
	}
}

inline
void registerServer(soft_ptr<net::ServerBase> t)
{
#ifdef NODECPP_RECORD_AND_REPLAY
	if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
	{
		return NodeReplayer::appAddServer(t);
	}
	else
#endif // NODECPP_RECORD_AND_REPLAY
	{
		return netServerManagerBase->appAddServer(t);
	}
}

#ifdef NODECPP_ENABLE_CLUSTERING
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
#endif // NODECPP_ENABLE_CLUSTERING

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
	owning_ptr<Node> node;

#ifdef NODECPP_ENABLE_CLUSTERING
	void internalRun( bool isMaster, ThreadStartupData* startupData )
#else
	void internalRun()
#endif
	{
		safememory::detail::interceptNewDeleteOperators(true);
		{
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.init();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.init();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702

			Infrastructure infra;
			netSocketManagerBase = reinterpret_cast<NetSocketManagerBase*>(&infra.getNetSocket());
			timeoutManager = &infra.getTimeout();
			inmediateQueue = &infra.getInmediateQueue();
			netServerManagerBase = reinterpret_cast<NetServerManagerBase*>(&infra.getNetServer());
			infra.doBasicInitialization();
#ifdef NODECPP_ENABLE_CLUSTERING
			if ( isMaster )
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
			nodecpp::postinitThreadClusterObject();
			if ( isMaster )
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
			}
#endif // NODECPP_ENABLE_CLUSTERING

			node = make_owning<Node>();
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
		safememory::detail::killAllZombies();
		safememory::detail::interceptNewDeleteOperators(false);
	}

public:
	using NodeType = Node;
	Runnable() {}
#ifdef NODECPP_ENABLE_CLUSTERING
	void run(bool isMaster, ThreadStartupData* startupData) override
	{
		return internalRun(isMaster, startupData);
	}
#else
	void run() override
	{
#ifndef NODECPP_RECORD_AND_REPLAY
			internalRun();
#else
		if ( replayMode != nodecpp::record_and_replay_impl::BinaryLog::Mode::replaying )
			internalRun();
		else
			NodeReplayer::run<Node>();
#endif // NODECPP_RECORD_AND_REPLAY
	}
#endif // NODECPP_ENABLE_CLUSTERING
};

#endif //INFRASTRUCTURE_H
