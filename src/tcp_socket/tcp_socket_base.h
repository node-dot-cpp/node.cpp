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


#ifndef TCP_SOCKET_BASE_H
#define TCP_SOCKET_BASE_H

#include "../../include/nodecpp/common.h"
#include "../../include/nodecpp/socket_common.h"
#include "../../include/nodecpp/server_common.h"
#include "../../include/nodecpp/cluster.h"
#include "../../include/nodecpp/ip_and_port.h"
#include "../ev_queue.h"

#ifdef _MSC_VER
#include <winsock2.h>
using socklen_t = int;
#else
#include <sys/poll.h> // for pollfd
using SOCKET = int;
const SOCKET INVALID_SOCKET = -1;
struct pollfd;
#endif



namespace nodecpp {

	class OpaqueEmitter
	{
		using UnderlyingType = void;
		nodecpp::safememory::soft_ptr<UnderlyingType> ptr;
	public:
		enum ObjectType { Undefined, ClientSocket, ServerSocket, AgentServer };
		ObjectType objectType;
		int type = -1;
		OpaqueEmitter() : objectType(ObjectType::Undefined), type(-1) {}
		OpaqueEmitter( ObjectType ot, nodecpp::safememory::soft_ptr<net::SocketBase> ptr_, int type_ ) : ptr( ptr_), objectType(ot), type(type_) {}
		OpaqueEmitter( ObjectType ot, nodecpp::safememory::soft_ptr<net::ServerBase> ptr_, int type_ ) : ptr( ptr_), objectType(ot), type(type_) {}
		OpaqueEmitter( ObjectType ot, nodecpp::safememory::soft_ptr<Cluster::AgentServer> ptr_, int type_ ) : ptr( ptr_), objectType(ot), type(type_) {}
		bool isValid() const { return (bool)ptr; }
		nodecpp::safememory::soft_ptr<net::SocketBase> getClientSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ClientSocket ); return nodecpp::safememory::soft_ptr_static_cast<net::SocketBase>(ptr); }
		nodecpp::safememory::soft_ptr<net::ServerBase> getServerSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ServerSocket ); return nodecpp::safememory::soft_ptr_static_cast<net::ServerBase>(ptr); }
		nodecpp::safememory::soft_ptr<Cluster::AgentServer> getAgentServerPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::AgentServer ); return nodecpp::safememory::soft_ptr_static_cast<Cluster::AgentServer>(ptr); }
	};
} // namespace nodecpp

using namespace nodecpp;

/*
	Pending events need some special treatment,
	If a socket is closed, we must discard any pending event
	for such socket
*/
class PendingEvQueue
{
	std::list<std::pair<size_t, std::function<void()>>> evQueue;
public:
	template<class M, class T, class... Args>
	void add(size_t ix, M T::* pm, T* inst, Args... args)
	{
		//code to call events async
		auto b = std::bind(pm, inst, args...);
		evQueue.emplace_back(ix, std::move(b));
	}

	void toQueue(EvQueue& ev)
	{
		for (auto& current : evQueue)
		{
			ev.add(std::move(current.second));
		}
		evQueue.clear();
	}

	void remove(size_t ix)
	{
		auto it = evQueue.begin();
		auto itEnd = evQueue.end();
		while (it != itEnd)
		{
			if (ix == it->first)
				it = evQueue.erase(it);
			else
				++it;
		}
	}
};


class SocketRiia // moved to .h
{
	SOCKET s;
public:
	SocketRiia(SOCKET s) :s(s) {}

	SocketRiia(const SocketRiia&) = delete;
	SocketRiia& operator=(const SocketRiia&) = delete;

	SocketRiia(SocketRiia&&other) 
	{
		s = other.s; 
		other.s = INVALID_SOCKET; }
	SocketRiia& operator=(SocketRiia&&other) {s = other.s; other.s = INVALID_SOCKET; return *this; }

	~SocketRiia();// { if (s != INVALID_SOCKET) internal_close(s); }

	SOCKET get() const noexcept { return s; }
	SOCKET release() noexcept { SOCKET tmp = s; s = INVALID_SOCKET; return tmp; }
	explicit operator bool() const noexcept { return s != INVALID_SOCKET; }

};

class NetSocketManagerBase; // forward declaration

class OpaqueSocketData
{
	friend class NetSocketManagerBase;
	SocketRiia s;
	OpaqueSocketData( SOCKET s_ ) : s(s_) {};
	OpaqueSocketData( SocketRiia s_ ) : s(s_.release()) {};
public:
	OpaqueSocketData( bool ) : s(INVALID_SOCKET) {}
	OpaqueSocketData( OpaqueSocketData& other ) = delete;
	OpaqueSocketData& operator=(const OpaqueSocketData&) = delete;
	OpaqueSocketData( OpaqueSocketData&& other ) : s( std::move( other.s) ) {}
	OpaqueSocketData& operator=(OpaqueSocketData&&other) {s = std::move( other.s); return *this; }
};



namespace nodecpp
{
	namespace internal_usage_only
	{
		SOCKET internal_make_tcp_socket();
		bool internal_bind_socket(SOCKET sock, struct sockaddr_in& sa_self);
		bool internal_bind_socket(SOCKET sock, Ip4 ip, Port port);
		uint16_t internal_port_of_tcp_socket(SOCKET sock);
		bool internal_listen_tcp_socket(SOCKET sock, int backlog);
		bool internal_getsockopt_so_error(SOCKET sock);
		void internal_shutdown_send(SOCKET sock);
		bool internal_linger_zero_socket(SOCKET sock);
		void internal_close(SOCKET sock);

		SOCKET internal_tcp_accept(Ip4& ip, Port& port, SOCKET sock);
	} // internal_usage_only
} // nodecpp


bool isNetInitialized();

/* 
	NetSocketManager is accesed from two 'sides'
	Methods prepended with 'app' are to be used from user code.
	They can throw Error back to user code, but events and callback may
	only be issued delayed
	
	Methods prepended with infra are to called by the event loop itself.
	They can't throw. 
	Events can be pushed onto the evQueue or delayed

	Each method 'kind' must be isolated and can't call the other.
*/

//extern thread_local std::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;

class OSLayer {
protected:
	IPFAMILY family = "IPv4";

public:
	static SocketRiia appAcquireSocket();
	static void appConnectSocket(SOCKET s, const char* ip, uint16_t port);

public:
	static void appDestroy(net::SocketBase::DataForCommandProcessing& sockData);
	static void appEnd(net::SocketBase::DataForCommandProcessing& sockData);
	//static bool appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size);
	static void appSetKeepAlive(net::SocketBase::DataForCommandProcessing& sockData, bool enable);
	static void appSetNoDelay(net::SocketBase::DataForCommandProcessing& sockData, bool noDelay);

	static bool infraGetPacketBytes(Buffer& buff, SOCKET sock);
	static bool infraGetPacketBytes2(CircularByteBuffer& buff, SOCKET sock, size_t target_sz);

	//enum ShouldEmit { EmitNone, EmitConnect, EmitDrain };
	//static ShouldEmit infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData);

public:

	static void closeSocket(net::SocketBase::DataForCommandProcessing& sockData);//app-infra neutral
	static void errorCloseSocket(net::SocketBase::DataForCommandProcessing& sockData, Error& err);//app-infra neutral
};


#endif // TCP_SOCKET_BASE_H

