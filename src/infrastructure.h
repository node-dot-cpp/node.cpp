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


struct TimeoutEntry
{
	uint64_t id;
	std::function<void()> cb;
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
public:
	void appSetTimeout(TimeoutEntry& entry);
	void appClearTimeout(TimeoutEntry& entry);

	nodecpp::Timeout appSetTimeout(std::function<void()> cb, int32_t ms);
	void appClearTimeout(const nodecpp::Timeout& to);
	void appRefresh(uint64_t id);
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


template<class EmitterType, class ServerEmitterType>
class Infrastructure
{
	//std::vector<NetSocketEntry> ioSockets;
	NetSockets ioSockets;
	NetSocketManager<EmitterType> netSocket;
	NetServerManager<ServerEmitterType> netServer;
	TimeoutManager timeout;
//	EvQueue inmediateQueue;

public:
	using EmitterTypeT = EmitterType;
	using ServerEmitterTypeT = ServerEmitterType;

	Infrastructure() : netSocket(ioSockets), netServer(ioSockets) {}

public:
	NetSocketManagerBase& getNetSocketBase() { return netSocket; }
public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	NetSocketManager<EmitterType>& getNetSocket() { return netSocket; }
	NetServerManager<ServerEmitterType>& getNetServer() { return netServer; }
	TimeoutManager& getTimeout() { return timeout; }
//	void setInmediate(std::function<void()> cb) { inmediateQueue.add(std::move(cb)); }
//	void emitInmediates() { inmediateQueue.emit(); }

	bool refedTimeout() const noexcept
	{
//		return !inmediateQueue.empty() || timeout.infraRefedTimeout();
		return timeout.infraRefedTimeout();
	}

	uint64_t nextTimeout() const noexcept
	{
//		return inmediateQueue.empty() ? timeout.infraNextTimeout() : 0;
		return timeout.infraNextTimeout();
	}

	template<class Node>
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
		auto ret = ioSockets.wait( timeoutToUse );

		if ( !ret.first )
			return false;

		int retval = ret.second;

		if (retval < 0)
		{
#ifdef _MSC_VER
			int error = WSAGetLastError();
			//		if ( error == WSAEWOULDBLOCK )
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("error {}", error);
#else
			perror("select()");
			//		int error = errno;
			//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
			/*        return WAIT_RESULTED_IN_TIMEOUT;*/
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,false);
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("COMMLAYER_RET_FAILED");
			return false;
		}
		else if (retval == 0)
		{
			//timeout, just return with empty queue
			return true; 
		}
		else //if(retval)
		{
			for ( size_t i=0; i<ioSockets.size(); ++i)
			{
				if ( (int64_t)(ioSockets.socketsAt(i + 1)) > 0 )
				{
					NetSocketEntry& current = ioSockets.at( 1 + i );
					short revents = ioSockets.reventsAt( 1 + i );
					switch ( current.emitter.objectType )
					{
						case OpaqueEmitter::ObjectType::ClientSocket:
							netSocket.template infraCheckPollFdSet<Node>(current, revents);
							break;
						case OpaqueEmitter::ObjectType::ServerSocket:
							if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
							{
								netServer.template infraCheckPollFdSet<Node>(current, revents);
								break;
							}
							else
							{
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false );
								break;
							}
						default:
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected value {}", (int)(current.emitter.objectType) );
							break;
					}
				}
			}
			return true;
		}
	}

	template<class Node>
	void runInfraLoop2()
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,isNetInitialized());

		while (running)
		{

			EvQueue queue;

			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.infraGetPendingEvents(queue);
				netServer.template infraEmitListeningEvents<Node>();
				queue.emit();
			}

			uint64_t now = infraGetCurrentTime();
			timeout.infraTimeoutEvents(now, queue);
			queue.emit();

			now = infraGetCurrentTime();
			bool refed = pollPhase2<Node>(refedTimeout(), nextTimeout(), now/*, queue*/);
			if(!refed)
				return;

			queue.emit();
	//		emitInmediates();

			netSocket.template infraGetCloseEvent<Node>(/*queue*/);
			netSocket.template infraProcessSockAcceptedEvents<Node>();
			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.template infraGetCloseEvents<Node>(/*queue*/);
			}
			queue.emit();

//			netSocket.infraClearStores();
			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.infraClearStores();
			}
		}
	}
};

inline
size_t registerWithInfraAndAcquireSocket(/*NodeBase* node,*/ nodecpp::safememory::soft_ptr<net::SocketBase> t, int typeId)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
	return netSocketManagerBase->appAcquireSocket(/*node, */t, typeId);
}

inline
size_t registerWithInfraAndAssignSocket(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::SocketBase> t, int typeId, OpaqueSocketData& sdata)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, t );
	return netSocketManagerBase->appAssignSocket(/*node, */t, typeId, sdata);
}

inline
void connectSocket(net::SocketBase* s, const char* ip, uint16_t port)
{
	netSocketManagerBase->appConnectSocket(s, ip, port);
}

inline
void registerServer(/*NodeBase* node, */soft_ptr<net::ServerBase> t, int typeId)
{
	return netServerManagerBase->appAddServer(/*node, */t, typeId);
}


extern thread_local NodeBase* thisThreadNode;
template<class Node>
class Runnable : public RunnableBase
{
	owning_ptr<Node> node;
	template<class ClientSocketEmitter, class ServerSocketEmitter>
	void internalRun()
	{
		nodecpp::log::touch_log();
		interceptNewDeleteOperators(true);
		{
			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.init();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.init();

			Infrastructure<ClientSocketEmitter, ServerSocketEmitter> infra;
			netSocketManagerBase = reinterpret_cast<NetSocketManagerBase*>(&infra.getNetSocket());
			if constexpr (!std::is_same< ServerSocketEmitter, void >::value)
			{
				netServerManagerBase = reinterpret_cast<NetServerManagerBase*>(&infra.getNetServer());
			}
			node = make_owning<Node>();
			thisThreadNode = &(*node);
			node->main();
			infra.template runInfraLoop2<Node>();
			node = nullptr;

			nodecpp::net::SocketBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
			nodecpp::net::ServerBase::DataForCommandProcessing::userHandlerClassPattern.destroy();
		}
		killAllZombies();
		interceptNewDeleteOperators(false);
	}
public:
	using NodeType = Node;
	Runnable() {}
	void run() override
	{
		return internalRun<typename Node::EmitterType, typename Node::EmitterTypeForServer>();
	}
};

#endif //INFRASTRUCTURE_H
