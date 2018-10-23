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
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/


#ifndef INFRASTRUCTURE_H
#define INFRASTRUCTURE_H

#include "ev_queue.h"
#include "tcp_socket/tcp_socket.h"

#include "../include/nodecpp/timers.h"
#include <functional>

#include "../include/nodecpp/loop.h"
#include "../include/nodecpp/socket_o.h"
#include "../include/nodecpp/socket_l.h"
#include "../include/nodecpp/server_o.h"
#include "../include/nodecpp/server_l.h"

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

inline
bool infraSetPollFdSet___Client(const NetSockets& ioSockets, pollfd* begin, const pollfd* end)
{
	size_t sz = end - begin;
	assert(sz >= ioSockets.size());
	bool anyRefed = false;
	
	for (size_t i = 0; i != sz; ++i)
	{
//			if(i < ioSockets.size() && ioSockets[i].isValid())
		if(i < ioSockets.size() && ioSockets.at(i).isAssociated())
		{
			const auto& current = ioSockets.at(i);
			NODECPP_ASSERT(current.getClientSocketData()->osSocket != INVALID_SOCKET);

//				anyRefed = anyRefed || current.getClientSocketData()->refed;
			anyRefed = anyRefed || current.refed;

			begin[i].fd = current.getClientSocketData()->osSocket;
			begin[i].events = 0;

			// TODOY: revide!
			if(!current.getClientSocketData()->remoteEnded && !current.getClientSocketData()->paused)
				begin[i].events |= POLLIN;
			if (current.getClientSocketData()->state == net::SocketBase::DataForCommandProcessing::Connecting || !current.getClientSocketData()->writeBuffer.empty())
				begin[i].events |= POLLOUT;
		}
		else
			begin[i].fd = INVALID_SOCKET;
	}
	return anyRefed;
}

inline
bool infraSetPollFdSet___Server(const NetSockets& ioSockets, pollfd* begin, const pollfd* end)
{ 
	size_t sz = end - begin;
	assert(sz >= ioSockets.size());
	bool anyRefed = false;

	for (size_t i = 0; i != sz; ++i)
	{
//			if (i < ioSockets.size() && ioSockets[i].isValid())
		if (i < ioSockets.size() && ioSockets.at(i).isAssociated())
		{
			const auto& current = ioSockets.at(i);
			NODECPP_ASSERT(current.getServerSocketData()->osSocket != INVALID_SOCKET);

			anyRefed = anyRefed || current.getServerSocketData()->refed;

			begin[i].fd = current.getServerSocketData()->osSocket;
			begin[i].events = 0;

			bool f2 = true;
			if (f2)
				begin[i].events |= POLLIN;
		}
		else
			begin[i].fd = INVALID_SOCKET;
	}

	return anyRefed;
}



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

	bool pollPhase2(bool refed, uint64_t nextTimeoutAt, uint64_t now)
	{
#if 0 // old version
		size_t fds_sz;
		if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			fds_sz = NetSocketManagerBase::MAX_SOCKETS + NetServerManagerBase::MAX_SOCKETS;
		else
			fds_sz = NetSocketManagerBase::MAX_SOCKETS;
		std::unique_ptr<pollfd[]> fds(new pollfd[fds_sz]);

	
		pollfd* fds_begin = fds.get();
		pollfd* fds_end = fds_begin + NetSocketManagerBase::MAX_SOCKETS;
	//	bool refedSocket = netSocket.infraSetPollFdSet(fds_begin, fds_end);
		bool refedSocket = infraSetPollFdSet___Client(ioSockets, fds_begin, fds_end);

		fds_begin = fds_end;

		if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
		{
			fds_end += NetServerManagerBase::MAX_SOCKETS;
		//	pollfdsz = NetServerManager::MAX_SOCKETS;
	//		bool refedServer = netServer.infraSetPollFdSet(fds_begin, fds_end);
			bool refedServer = infraSetPollFdSet___Server(ioSockets, fds_begin, fds_end);
		//	bool refedServer = netServer.infraSetPollFdSet(fds_begin, pollfdsz);
			if (refed == false && refedSocket == false && refedServer == false) return false; //stop here
		}
		else
		{
			if (refed == false && refedSocket == false) return false; //stop here
		}

#else
		// TODOY: '			if (refed == false && refedSocket == false) return false; //stop here'
		size_t fds_sz;
		pollfd* fds_begin;
		auto pollfdRet = ioSockets.getPollfd();
		fds_sz = pollfdRet.second;
		fds_begin = pollfdRet.first;
#endif

		int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

#if 0 // old version
	#ifdef _MSC_VER
		int retval = WSAPoll(fds.get(), static_cast<ULONG>(fds_sz), timeoutToUse);
	#else
		int retval = poll(fds.get(), fds_sz, timeoutToUse);
	#endif
#else
	#ifdef _MSC_VER
		int retval = WSAPoll(fds_begin, static_cast<ULONG>(fds_sz), timeoutToUse);
	#else
		int retval = poll(fds_begin, fds_sz, timeoutToUse);
	#endif
#endif // 0


		if (retval < 0)
		{
	#ifdef _MSC_VER
			int error = WSAGetLastError();
			//		if ( error == WSAEWOULDBLOCK )
			NODECPP_TRACE("error {}", error);
	#else
			perror("select()");
			//		int error = errno;
			//		if ( error == EAGAIN || error == EWOULDBLOCK )
	#endif
			/*        return WAIT_RESULTED_IN_TIMEOUT;*/
			NODECPP_ASSERT(false);
			NODECPP_TRACE0("COMMLAYER_RET_FAILED");
			return false;
		}
		else if (retval == 0)
		{
			//timeout, just return with empty queue
			return true; 
		}
		else //if(retval)
		{
#if 0 // old version
			fds_begin = fds.get();
			fds_end = fds_begin + NetSocketManagerBase::MAX_SOCKETS;
	//		pollfdsz = NetSocketManagerBase::MAX_SOCKETS;
			netSocket.infraCheckPollFdSet(fds_begin, fds_end/*, evs*/);
	//		netSocket.infraCheckPollFdSet(fds_begin, pollfdsz, evs);

			fds_begin = fds_end;
	//		fds_begin += pollfdsz;
			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				fds_end += NetServerManagerBase::MAX_SOCKETS;
				netServer.infraCheckPollFdSet(fds_begin, fds_end/*, evs*/);
		//		pollfdsz = NetServerManager::MAX_SOCKETS;
		//		netServer.infraCheckPollFdSet(fds_begin, pollfdsz, evs);
			}

			//if (queue.empty())
			//{
			//	NODECPP_TRACE("No event generated from poll wake up (non timeout)");
			//	for (size_t i = 0; i != fds_sz; ++i)
			//	{
			//		if (fds[i].fd >= 0 && fds[i].revents != 0)
			//		{
			//			NODECPP_TRACE("At id {}, socket {}, revent {:x}", i, fds[i].fd, fds[i].revents);
			//		}
			//	}
			//}
#else
			for ( size_t i=0; i<fds_sz; ++i)
			{
				if ( fds_begin[i].fd > 0 )
				{
					NetSocketEntry& current = ioSockets.at( i );
					switch ( current.emitter.objectType )
					{
						case OpaqueEmitter::ObjectType::ClientSocket:
							netSocket.infraCheckPollFdSet(current, fds_begin[i]);
							break;
						case OpaqueEmitter::ObjectType::ServerSocket:
							if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
							{
								netServer.infraCheckPollFdSet(current, fds_begin[i]);
								break;
							}
							else
							{
								NODECPP_ASSERT( false );
								break;
							}
						default:
							NODECPP_ASSERT( false );
							break;
					}
				}
			}
#endif
			return true;
		}
	}

	void runInfraLoop2()
	{
		NODECPP_ASSERT(isNetInitialized());

		while (running)
		{

			EvQueue queue;

			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.infraGetPendingEvents(queue);
				queue.emit();
			}

			uint64_t now = infraGetCurrentTime();
			timeout.infraTimeoutEvents(now, queue);
			queue.emit();

			now = infraGetCurrentTime();
			bool refed = pollPhase2(refedTimeout(), nextTimeout(), now/*, queue*/);
			if(!refed)
				return;

			queue.emit();
	//		emitInmediates();

			netSocket.infraGetCloseEvent(/*queue*/);
			netSocket.infraProcessSockAcceptedEvents();
			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.infraGetCloseEvents(/*queue*/);
			}
			queue.emit();

			netSocket.infraClearStores();
			if constexpr ( !std::is_same< ServerEmitterTypeT, void >::value )
			{
				netServer.infraClearStores();
			}
		}
	}
};

#ifdef USING_T_SOCKETS
inline
size_t registerWithInfraAndAcquireSocket(NodeBase* node, net::SocketTBase* t, int typeId)
{
	NODECPP_ASSERT( t != nullptr );
	return netSocketManagerBase->appAcquireSocket(node, t, typeId);
}

inline
size_t registerWithInfraAndAssignSocket(NodeBase* node, net::SocketTBase* t, int typeId, OpaqueSocketData& sdata)
{
	NODECPP_ASSERT( t != nullptr );
	return netSocketManagerBase->appAssignSocket(node, t, typeId, sdata);
}

inline
void connectSocket(net::SocketTBase* s, const char* ip, uint16_t port)
{
	netSocketManagerBase->appConnectSocket(s, ip, port);
}

inline
void registerServer(NodeBase* node, net::ServerTBase* t, int typeId)
{
	return netServerManagerBase->appAddServer(node, t, typeId);
}

template<class Node>
class Runnable : public RunnableBase
{
	Node* node;
	template<class ClientSocketEmitter, class ServerSocketEmitter>
	void internalRun()
	{
		Infrastructure<ClientSocketEmitter, ServerSocketEmitter> infra;
		netSocketManagerBase = reinterpret_cast<NetSocketManagerBase*>(&infra.getNetSocket());
		netSocketManagerBase->typeIndexOfSocketO = ClientSocketEmitter::template softGetTypeIndexIfTypeExists<net::SocketO>();
		netSocketManagerBase->typeIndexOfSocketL = ClientSocketEmitter::template softGetTypeIndexIfTypeExists<net::Socket>();
		if constexpr( !std::is_same< ServerSocketEmitter, void >::value )
		{
			netServerManagerBase = reinterpret_cast<NetServerManagerBase*>(&infra.getNetServer());
			netServerManagerBase->typeIndexOfServerO = ServerSocketEmitter::template softGetTypeIndexIfTypeExists<net::ServerO>();
			netServerManagerBase->typeIndexOfServerL = ServerSocketEmitter::template softGetTypeIndexIfTypeExists<net::Server>();
		}
		node = new Node;
		node->main();
		infra.runInfraLoop2();
	}
public:
	using NodeType = Node;
	Runnable() {}
	void run() override
	{
		return internalRun<typename Node::EmitterType, typename Node::EmitterTypeForServer>();
	}
};

#else
extern thread_local Infrastructure<net::SocketEmitter> infra;
inline
NetSocketManagerBase& getNetSocket() { return infra.getNetSocket(); }
#endif // USING_T_SOCKETS

#endif //INFRASTRUCTURE_H
