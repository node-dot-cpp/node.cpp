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

#include "ev_queue.h"
#include "tcp_socket/tcp_socket.h"

#include "../include/nodecpp/timers.h"
#include <functional>

/*
	'appSetTimeout()' will return a 'Timeout' object, that user may or may not store.
	If the user doesn't store it, timeout will fire normally, and after that all
	resources will be discarded.
	But if user keeps a living reference, then she may call 'refresh' to reschedule,
	so resources may not be discarded until user discards her reference.


*/

static constexpr uint64_t TimeOutNever = std::numeric_limits<uint64_t>::max();

struct TimeoutEntryHandlerData
{
	std::function<void()> cb = nullptr; // is assumed to be self-contained in terms of required action
//	nodecpp::awaitable_handle_data::handler_fn_type h = nullptr;
	awaitable_handle_t h = nullptr;
	bool setExceptionWhenDone = false; // is used to indicate that a handle is released because of timeout and not because of its "regular" event
};

struct TimeoutEntry : public TimeoutEntryHandlerData
{
	uint64_t id;
	uint64_t lastSchedule;
	uint64_t delay;
	uint64_t nextTimeout;
	bool handleDestroyed = false;
	bool active = false;
};

class TimeoutManager
{
	uint64_t lastId = 0;
	std::unordered_map<uint64_t, TimeoutEntry> timers;
	std::multimap<uint64_t, uint64_t> nextTimeouts;
	template<class H>
	nodecpp::Timeout appSetTimeoutImpl(H h, bool indicateThrowing, int32_t ms)
	{
		if (ms == 0) ms = 1;
		else if (ms < 0) ms = std::numeric_limits<int32_t>::max();

		uint64_t id = ++lastId;

		TimeoutEntry entry;
		entry.setExceptionWhenDone = indicateThrowing;
		entry.id = id;
		static_assert( !std::is_same<std::function<void()>, awaitable_handle_t>::value ); // we're in trouble anyway and not only here :)
		static_assert( std::is_same<H, std::function<void()>>::value || std::is_same<H, awaitable_handle_t>::value );
		if constexpr ( std::is_same<H, std::function<void()>>::value )
		{
			entry.cb = h;
			entry.h = nullptr;
		}
		else
		{
			static_assert( std::is_same<H, awaitable_handle_t>::value );
			entry.cb = nullptr;
			entry.h = h;
		}
		entry.delay = ms * 1000;

		auto res = timers.insert(std::make_pair(id, std::move(entry)));
		if (res.second)
		{
			appSetTimeout(res.first->second);

			return Timeout(id);
		}
		else
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Failed to insert Timeout {}", id);
			return Timeout(0);
		}
	}

public:
	void appSetTimeout(TimeoutEntry& entry);
	void appClearTimeout(TimeoutEntry& entry);

	nodecpp::Timeout appSetTimeout(std::function<void()> cb, int32_t ms) { return appSetTimeoutImpl( cb, false, ms ); }
	void appClearTimeout(const nodecpp::Timeout& to);
	void appRefresh(uint64_t id);
#ifndef NODECPP_NO_COROUTINES
//	nodecpp::Timeout appSetTimeout(std::experimental::coroutine_handle<> h, int32_t ms) { return appSetTimeoutImpl( h, false, ms ); }
	nodecpp::Timeout appSetTimeout(awaitable_handle_t ahd, int32_t ms) { return appSetTimeoutImpl( ahd, false, ms ); }
	nodecpp::Timeout appSetTimeoutForAction(awaitable_handle_t ahd, int32_t ms) { return appSetTimeoutImpl( ahd, true, ms ); }
#endif
	void appTimeoutDestructor(uint64_t id);

	void infraTimeoutEvents(uint64_t now, EvQueue& evs);
	uint64_t infraNextTimeout() const noexcept
	{
		auto it = nextTimeouts.begin();
		return it != nextTimeouts.end() ? it->first : TimeOutNever;
	}

	bool infraRefedTimeout() const noexcept
	{
		return !nextTimeouts.empty();
	}
};


int getPollTimeout(uint64_t nextTimeoutAt, uint64_t now);
uint64_t infraGetCurrentTime();


#include "clustering_impl/interthread_comm.h"

class Infrastructure
{
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

	bool pollPhase2(bool refed, uint64_t nextTimeoutAt, uint64_t now)
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
extern thread_local int sessionCnt;
extern thread_local size_t sessionCreationtime;
extern thread_local size_t waitTime;
++pollCnt;
pollRetCnt += retval;
if ( pollRetMax < retval ) pollRetMax = retval;
if ( (pollCnt &0xFFFF) == 0 )
#ifdef NODECPP_ENABLE_CLUSTERING
printf( "[thread %zd] pollCnt = %d, pollRetCnt = %d, pollRetMax = %d, ioSockets.size() = %zd, sessionCnt = %d, sessionCreationtime = %zd, waitTime = %zd\n", getCluster().isMaster() ? 0 : getCluster().worker().id(), pollCnt, pollRetCnt, pollRetMax, ioSockets.size(), sessionCnt, sessionCreationtime, waitTime );
#else
printf( "pollCnt = %d, pollRetCnt = %d, pollRetMax = %d, ioSockets.size() = %zd, sessionCnt = %d, sessionCreationtime = %zd, waitTime = %zd\n", pollCnt, pollRetCnt, pollRetMax, ioSockets.size(), sessionCnt, sessionCreationtime, waitTime );
#endif // NODECPP_ENABLE_CLUSTERING
#endif // USE_TEMP_PERF_CTRS
			int processed = 0;
			for ( size_t i=0; processed<retval; ++i)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, i<ioSockets.size() );
				short revents = ioSockets.reventsAt( 1 + i );
#ifdef NODECPP_LINUX
				if ( revents )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, (int64_t)(ioSockets.socketsAt(i + 1)) > 0, "indeed: {}", (int64_t)(ioSockets.socketsAt(i + 1)) );
#else
				if ( revents && (int64_t)(ioSockets.socketsAt(i + 1)) > 0 ) // on Windows WSAPoll() may set revents to a non-zero value despite the socket is invalid
				{
#endif
					++processed;
					if ( 1 + i >= ioSockets.reserved_capacity )
					{
						NetSocketEntry& current = ioSockets.at( 1 + i );
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
					else if ( i == ioSockets.awakerSockIdx )
					{
						// TODO: ...
					}
					else if ( i == ioSockets.interThreadCommServerIdx )
					{
						netServer.infraCheckPollFdSetOnInterThreadServerCommInitSocket(revents);
					}
				}
			}
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

#ifdef NODECPP_ENABLE_CLUSTERING
	void runInterthreadCommInitLoop()
	{
		activateInterThreadCommSystem();

		while (!isInterThreadCommSystemInitialized())
		{
			uint64_t now = infraGetCurrentTime();
			bool refed = pollPhase2(refedTimeout(), nextTimeout(), now);
			if(!refed)
				return;
			ioSockets.reworkIfNecessary();
		}
	}
#endif // NODECPP_ENABLE_CLUSTERING

	void runStandardLoop()
	{
		while (running)
		{

			EvQueue queue;

			netServer.infraGetPendingEvents(queue);
			netServer. infraEmitListeningEvents();
			queue.emit();

			uint64_t now = infraGetCurrentTime();
			timeout.infraTimeoutEvents(now, queue);
			queue.emit();

			now = infraGetCurrentTime();
			bool refed = pollPhase2(refedTimeout(), nextTimeout(), now/*, queue*/);
			if(!refed)
				return;

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
void registerWithInfraAndAcquireSocket(nodecpp::safememory::soft_ptr<net::SocketBase> t)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
	netSocketManagerBase->appAcquireSocket(t);
}

inline
void registerWithInfraAndAssignSocket(nodecpp::safememory::soft_ptr<net::SocketBase> t, OpaqueSocketData& sdata)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
	netSocketManagerBase->appAssignSocket(t, sdata);
}

inline
void connectSocket(net::SocketBase* s, const char* ip, uint16_t port)
{
	netSocketManagerBase->appConnectSocket(s, ip, port);
}

inline
void registerServer(soft_ptr<net::ServerBase> t)
{
	return netServerManagerBase->appAddServer(t);
}

#ifdef NODECPP_ENABLE_CLUSTERING
inline
void registerAgentServer(soft_ptr<Cluster::AgentServer> t)
{
	return netServerManagerBase->appAddAgentServer(t);
}
inline
void acquireSocketAndLetInterThreadCommServerListening(nodecpp::Ip4 ip, uint16_t& port, int backlog)
{
	return netServerManagerBase->acquireSocketAndLetInterThreadCommServerListening( ip, port, backlog );
}
inline
SOCKET acquireAndConnectSocketForInterThreadComm( const char* ip, uint16_t destinationPort, uint16_t& sourcePort )
{
	return netServerManagerBase->acquireAndConnectSocketForInterThreadComm( ip, destinationPort, sourcePort );
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
			nodecpp::setNoException(awaiting);
            who_is_awaiting = awaiting;
			to = timeoutManager->appSetTimeout(awaiting, duration);
        }

		auto await_resume() {
			if ( nodecpp::isException(who_is_awaiting) )
				throw nodecpp::getException(who_is_awaiting);
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
	void internalRun( bool isMaster )
#else
	void internalRun()
#endif
	{
		interceptNewDeleteOperators(true);
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
				infra.runInterthreadCommInitLoop();
#endif
			// from now on all internal structures are ready to use; let's run their "users"
#ifdef NODECPP_ENABLE_CLUSTERING
			nodecpp::postinitThreadClusterObject();
//			interThreadComm.init( nodecpp::cluster.worker().id(), 0 );
#endif // NODECPP_ENABLE_CLUSTERING
			node = make_owning<Node>();
			thisThreadNode = &(*node); 
			// NOTE!!! 
			// By coincidence it so happened that both void Node::main() and nodecpp::handler_ret_type Node::main() are currently treated in the same way.
			// If, for any reason, treatment should be different, to check exactly which one is present, see, for instance
			// http://www.gotw.ca/gotw/071.htm and 
			// https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
			node->main();
			infra.runStandardLoop();
			node = nullptr;

#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
#endif // NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
		}
		killAllZombies();
		interceptNewDeleteOperators(false);
	}
public:
	using NodeType = Node;
	Runnable() {}
#ifdef NODECPP_ENABLE_CLUSTERING
	void run(bool isMaster) override
	{
		return internalRun(isMaster);
	}
#else
	void run() override
	{
		return internalRun();
	}
#endif // NODECPP_ENABLE_CLUSTERING
};

#endif //INFRASTRUCTURE_H
