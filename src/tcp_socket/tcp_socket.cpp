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

#include "tcp_socket.h"
#include "../../include/nodecpp/common.h"
#include "../../include/nodecpp/error.h"
#include "../infrastructure.h"


#ifdef _MSC_VER

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <time.h> //TIME_UTC
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define CLOSE_SOCKET( x ) closesocket( x )
typedef int ssize_t;

#else // _MSC_VER

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h> // for appClose() for socket
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/tcp.h>

#define CLOSE_SOCKET( x ) close( x )

#endif // _MSC_VER

//mb: TODO make enum
#define COMMLAYER_RET_FAILED 0
#define COMMLAYER_RET_OK 1
#define COMMLAYER_RET_PENDING 2

using namespace std;

/////////////////////////////////////////////     COMMUNICATION     ///////////////////////////////////////////


const int tcpListenBacklogSize = 3;
static bool netInitialize();

static bool netInitialized = netInitialize();

bool isNetInitialized() { return netInitialized; }

inline bool isErrorWouldBlock(int error)
{
#if defined _MSC_VER || defined __MINGW32__
	return WSAEWOULDBLOCK == error;
#else
	return errno == EINPROGRESS || error == EAGAIN || error == EWOULDBLOCK;
#endif
}



inline int getSockError()
{
#if defined _MSC_VER || defined __MINGW32__
	return WSAGetLastError();
#else
	return errno;
#endif
}

bool netInitialize()
{
#ifdef _MSC_VER
	// do Windows magic
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		NODECPP_TRACE("WSAStartup failed with error: {}", iResult);
		return false;
	}
	NODECPP_TRACE0("WSAStartup success");
#endif
	return true;
}


namespace nodecpp
{
	namespace internal_usage_only
	{
		//static
		void internal_close(SOCKET sock)
		{
			NODECPP_TRACE("internal_close() on sock {}", sock);
			CLOSE_SOCKET(sock);
		}

		//static
		void internal_shutdown_send(SOCKET sock)
		{
		#ifdef _MSC_VER
			int how = SD_SEND;
		#else
			int how = SHUT_WR;
		#endif

			NODECPP_TRACE("internal_shutdown_send() on sock {}", sock);

			int res = shutdown(sock, how);
			if (0 != res)
			{
				int error = getSockError();
				NODECPP_TRACE("shutdown on sock {} failed; error {}", sock, error);
			}
		}


		static
		bool internal_async_socket(SOCKET sock)
		{
		#if defined _MSC_VER || defined __MINGW32__
			unsigned long one = 1;
			int res2 = ioctlsocket(sock, FIONBIO, &one);
		#else
			int one = 1;
			int res2 = ioctl(sock, FIONBIO, &one);
		#endif
			if (0 != res2)
			{
				int error = getSockError();
				NODECPP_TRACE("async on sock {} failed; error {}", sock, error);
				return false;
			}

			return true;
		}

		static
		bool internal_tcp_no_delay_socket(SOCKET sock, bool noDelay)
		{
		#if defined _MSC_VER || defined __MINGW32__
			DWORD value = static_cast<DWORD>(noDelay);
			int result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));
		#else
			int value = static_cast<int>(noDelay);
			int result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));
		#endif
	
			if (0 != result)
			{
				int error = getSockError();
				NODECPP_TRACE("TCP_NODELAY on sock {} failed; error {}", sock, error);
		//		internal_close(sock);
				return false;
			}

			return true;
		}



		static
		bool internal_socket_keep_alive(SOCKET sock, bool enable)
		{
		#if defined _MSC_VER || defined __MINGW32__
			DWORD value = static_cast<DWORD>(enable);
			int result = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&value, sizeof(value));
		#else
			int value = static_cast<int>(enable);
			int result = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&value, sizeof(value));
		#endif

			if (0 != result)
			{
				int error = getSockError();
				NODECPP_TRACE("TCP_NODELAY on sock {} failed; error {}", sock, error);
				//		internal_close(sock);
				return false;
			}

			return true;
		}

		//static
		bool internal_linger_zero_socket(SOCKET sock)
		{
			linger value;
			value.l_onoff = 1;
			value.l_linger = 0;
		#if defined _MSC_VER || defined __MINGW32__
			int result = setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &value, sizeof(value));
		#else
			int result = setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *) &value, sizeof(value));
		#endif
	
			if (0 != result)
			{
				int error = getSockError();
				NODECPP_TRACE("SO_LINGER on sock {} failed; error {}", sock, error);
				return false;
			}

			return true;
		}


		static
		SOCKET internal_make_tcp_socket()
		{
			SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == sock)
			{
				int error = getSockError();
				NODECPP_TRACE("socket() failed; error {}", error);
				return INVALID_SOCKET;
			}

			NODECPP_TRACE("socket() success {}", sock);

			if (!internal_async_socket(sock))
			{
				internal_close(sock);
				return INVALID_SOCKET;
			}
	
			return sock;
		}

		static
		bool internal_bind_socket(SOCKET sock, struct sockaddr_in& sa_self)
		{
			int res = ::bind(sock, (struct sockaddr *)(&sa_self), sizeof(struct sockaddr_in));
			if (0 != res)
			{
				int error = getSockError();
				NODECPP_TRACE("bind() on sock {} failed; error {}", sock, error);
				return false;
			}

			return true;
		}

		static
		uint8_t internal_connect_socket(struct sockaddr_in& sa_other, SOCKET sock)
		{
			int res = connect(sock, (struct sockaddr *)(&sa_other), sizeof(struct sockaddr_in));
			if (0 != res)
			{
				int error = getSockError();
				if (isErrorWouldBlock(error))
				{
					NODECPP_TRACE("connect() on sock {} in progress", sock);
					return COMMLAYER_RET_PENDING;
				}
				else
				{
					NODECPP_TRACE("connect() on sock {} failed; error {}", sock, error);
					return COMMLAYER_RET_FAILED;
				}
			}
			else
				NODECPP_TRACE("connect() on sock {} completed", sock);

			return COMMLAYER_RET_OK;
		}


		static
		bool internal_bind_socket(SOCKET sock, Ip4 ip, Port port)
		{
			NODECPP_TRACE("internal_bind_socket() on sock {} to {}:{}", sock, ip.toStr(), port.toStr());
			struct sockaddr_in sa;
			memset(&sa, 0, sizeof(struct sockaddr_in));
			sa.sin_family = AF_INET;
			sa.sin_addr.s_addr = ip.getNetwork();
			sa.sin_port = port.getNetwork();

			return internal_bind_socket(sock, sa);
		}

		static
		bool internal_listen_tcp_socket(SOCKET sock)
		{
			int res = listen(sock, tcpListenBacklogSize);
			if (0 != res) {
				int error = getSockError();
				NODECPP_TRACE("listen() on sock {} failed; error {}", sock, error);
				return false;
			}
			NODECPP_TRACE("listen() on sock {} success", sock);

			return true;
		}

		static
		uint8_t internal_connect_for_address(Ip4 peerIp, Port peerPort, SOCKET sock)
		{
			NODECPP_TRACE("internal_connect_for_address() on sock {} to {}:{}", sock, peerIp.toStr(), peerPort.toStr());

			struct sockaddr_in saOther;
			memset(&saOther, 0, sizeof(struct sockaddr_in));
			saOther.sin_family = AF_INET;
			saOther.sin_addr.s_addr = peerIp.getNetwork();
			saOther.sin_port = peerPort.getNetwork();

			return internal_connect_socket(saOther, sock);
		}

		static
		SOCKET internal_tcp_accept(Ip4& ip, Port& port, SOCKET sock)
		{
			NODECPP_TRACE("internal_tcp_accept() on sock {}", sock);
			struct sockaddr_in sa;
			socklen_t sz = sizeof(struct sockaddr_in);
			memset(&sa, 0, sz);

			SOCKET outSock = accept(sock, (struct sockaddr *)&sa, &sz);
			if (INVALID_SOCKET == outSock)
			{
				int error = getSockError();
				NODECPP_TRACE("accept() on sock {} failed; error {}", error);

				return INVALID_SOCKET;
			}

	
			ip = Ip4::fromNetwork(sa.sin_addr.s_addr);
			port = Port::fromNetwork(sa.sin_port);
			NODECPP_TRACE("accept() new sock {} from {}:{}", outSock, ip.toStr(), port.toStr());

			if (!internal_async_socket(outSock))
			{
				internal_close(outSock);
				return INVALID_SOCKET;
			}
			return outSock;
		}

		//static
		bool internal_getsockopt_so_error(SOCKET sock)
		{
			int result;
			socklen_t result_len = sizeof(result);

			int err = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)(&result), &result_len);
			if (err != 0)
			{
				int error = getSockError();
				NODECPP_TRACE("getsockopt() SO_ERROR on sock {} failed; error {}", sock, error);
				return false;
			}

			if (result != 0)
			{
				NODECPP_TRACE("getsockopt() SO_ERROR on sock {} error {}", sock, result);
				return false;
			}

			return true;
		}

		static
		uint8_t internal_send_packet(const uint8_t* data, size_t size, SOCKET sock, size_t& sentSize)
		{
			const char* ptr = reinterpret_cast<const char*>(data); //windows uses char*, linux void*
			ssize_t bytes_sent = sendto(sock, ptr, (int)size, 0, nullptr, 0);

			if (bytes_sent < 0)
			{
				sentSize = 0;
				int error = getSockError();
				if (isErrorWouldBlock(error))
				{
					NODECPP_TRACE("internal_send_packet() on sock {} size {} PENDING", sock, size, sentSize);

					return COMMLAYER_RET_PENDING;
				}
				else
				{
					NODECPP_TRACE("internal_send_packet() on sock {} size {} ERROR {}", sock, size, error);
					return COMMLAYER_RET_FAILED;
				}
			}
			else
			{
				sentSize = static_cast<size_t>(bytes_sent);
				if(sentSize == size)
				{
					NODECPP_TRACE("internal_send_packet() on sock {} size {} OK", sock, size);

					return COMMLAYER_RET_OK;
				}
				else
				{
					NODECPP_TRACE("internal_send_packet() on sock {} size {} PENDING sent {} ", sock, size, sentSize);
			
					return COMMLAYER_RET_PENDING;
				}
			}
		}

		static
		uint8_t internal_get_packet_bytes2(SOCKET sock, uint8_t* buff, size_t buffSz, size_t& retSz, struct sockaddr_in& sa_other, socklen_t& fromlen)
		{
			retSz = 0;
	
			NODECPP_ASSERT(buff);
			NODECPP_ASSERT(buffSz != 0);
			ssize_t ret = recvfrom(sock, (char*)buff, (int)buffSz, 0, (struct sockaddr *)(&sa_other), &fromlen);

			if (ret < 0)
			{
				int error = getSockError();
				if (isErrorWouldBlock(error))
				{
					NODECPP_TRACE("internal_get_packet_bytes2() on sock {} PENDING", sock);
					return COMMLAYER_RET_PENDING;
				}
				else
				{
					NODECPP_TRACE("internal_get_packet_bytes2() on sock {} ERROR {}", sock, error);
					return COMMLAYER_RET_FAILED;
				}
			}

			retSz = static_cast<size_t>(ret);
			NODECPP_TRACE("internal_get_packet_bytes2() on sock {} size {} OK", sock, retSz);
			return COMMLAYER_RET_OK;
		}

	} // internal_usage_only
} // nodecpp


SocketRiia::~SocketRiia() { if (s != INVALID_SOCKET) internal_usage_only::internal_close(s); }


/* static */
Ip4 Ip4::parse(const char* ip)
{
	return Ip4(inet_addr(ip));
}

/* static */
Ip4 Ip4::fromNetwork(uint32_t ip)
{
	return Ip4(ip);
}


/* static */
Port Port::fromHost(uint16_t port)
{
	return Port(htons(port));
}

/* static */
Port Port::fromNetwork(uint16_t port)
{
	return Port(port);
}



SocketRiia NetSocketManagerBase::appAcquireSocket(const char* ip, uint16_t port)
{
	Ip4 peerIp = Ip4::parse(ip);
//	Ip4 myIp = Ip4::parse(ip);

	Port peerPort = Port::fromHost(port);
//	Port myPort = Port::fromHost(port);


	SocketRiia s(internal_usage_only::internal_make_tcp_socket());
	if (!s)
		throw Error();

	uint8_t st = internal_usage_only::internal_connect_for_address(peerIp, peerPort, s.get());

	if (st != COMMLAYER_RET_PENDING && st != COMMLAYER_RET_OK)
		throw Error();

	return s;
}

void NetSocketManagerBase::appDestroy(net::SocketBase::DataForCommandProcessing& sockData)
{
	if (!sockData.isValid())
	{
		NODECPP_TRACE("Unexpected id {} on NetSocketManager::destroy", sockData.id);
		throw Error();
	}
	//	
	//	if(force)
	internal_usage_only::internal_linger_zero_socket(sockData.osSocket);
	//entry.state = net::SocketBase::DataForCommandProcessing::Closing;
	//pendingCloseEvents.emplace_back(entry.index, false);
	closeSocket(sockData);
}

void NetSocketManagerBase::appEnd(net::SocketBase::DataForCommandProcessing& sockData)
{
	if (!sockData.isValid())
	{
		NODECPP_TRACE("Unexpected id {} on NetSocketManager::end", sockData.id);
		throw Error();
	}

//	NODECPP_ASSERT(entry.state == net::SocketBase::DataForCommandProcessing::Connected);
	
	if (sockData.writeBuffer.empty())
	{
//		entry.localEnded = true;
		internal_usage_only::internal_shutdown_send(sockData.osSocket);
		if (sockData.remoteEnded)
		{
			//entry.state = net::SocketBase::DataForCommandProcessing::Closing;
			//pendingCloseEvents.emplace_back(entry.index, false);
			closeSocket(sockData);
		}
		else
			sockData.state = net::SocketBase::DataForCommandProcessing::LocalEnded;

	}
	else
	{
		sockData.state = net::SocketBase::DataForCommandProcessing::LocalEnding;
//		entry.pendingLocalEnd = true;
	}
}

void NetSocketManagerBase::appSetKeepAlive(net::SocketBase::DataForCommandProcessing& sockData, bool enable)
{
	if (!sockData.isValid())
	{
		NODECPP_TRACE("Unexpected id {} on NetSocketManager::setKeepAlive", sockData.id);
		throw Error();
	}

	if (!internal_usage_only::internal_socket_keep_alive(sockData.osSocket, enable))
	{
//		errorCloseSocket(entry, storeError(Error()));
		Error e;
		errorCloseSocket(sockData, e);
	}
}

void NetSocketManagerBase::appSetNoDelay(net::SocketBase::DataForCommandProcessing& sockData, bool noDelay)
{
	if (!sockData.isValid())
	{
		NODECPP_TRACE("Unexpected id {} on NetSocketManager::setNoDelay", sockData.id);
		throw Error();
	}

	if (!internal_usage_only::internal_tcp_no_delay_socket(sockData.osSocket, noDelay))
	{
//		errorCloseSocket(entry, storeError(Error()));
		Error e;
		errorCloseSocket(sockData, e);
	}
}

bool NetSocketManagerBase::appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size)
{
	if (!sockData.isValid())
	{
		NODECPP_TRACE("Unexpected StreamSocket {} on sendStreamSegment", sockData.id);
		throw Error();
	}

	if (sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnding || sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnded)
	{
		NODECPP_TRACE("StreamSocket {} already ended", sockData.id);
//		errorCloseSocket(sockData, storeError(Error()));
		Error e;
		errorCloseSocket(sockData, e);
		return false;
	}

	if (sockData.writeBuffer.size() == 0)
	{
		size_t sentSize = 0;
		uint8_t res = internal_usage_only::internal_send_packet(data, size, sockData.osSocket, sentSize);
		if (res == COMMLAYER_RET_FAILED)
		{
//			errorCloseSocket(sockData, storeError(Error()));
			Error e;
			errorCloseSocket(sockData, e);
			return false;
		}
		else if (sentSize == size)
		{
			return true;
		}
		else 
		{
			NODECPP_ASSERT(sentSize < size);
			sockData.writeBuffer.append(data + sentSize, size - sentSize);
			return false;
		}
	}
	else
	{
		sockData.writeBuffer.append(data, size);
		return false;
	}
}

std::pair<bool, Buffer> NetSocketManagerBase::infraGetPacketBytes(Buffer& buff, SOCKET sock)
{
	socklen_t fromlen = sizeof(struct sockaddr_in);
	struct sockaddr_in sa_other;
	size_t sz = 0;
	uint8_t ret = internal_usage_only::internal_get_packet_bytes2(sock, buff.begin(), buff.capacity(), sz, sa_other, fromlen);

	if (ret != COMMLAYER_RET_OK)
		return make_pair(false, Buffer());

	Buffer res(sz);
	res.append(buff.begin(), sz);
	buff.clear();

	return make_pair(true, std::move(res));
}

NetSocketManagerBase::ShouldEmit NetSocketManagerBase::infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData)
{
	NetSocketManagerBase::ShouldEmit ret = EmitNone;
//	if (current.connecting)
	if(sockData.state == net::SocketBase::DataForCommandProcessing::Connecting)
	{
//		current.connecting = false;

#if defined _MSC_VER || defined __MINGW32__
		bool success = true;
#else
//		bool success = internal_getsockopt_so_error(current.osSocket);
		bool success = internal_usage_only::internal_getsockopt_so_error(sockData.osSocket);
#endif					
		if (success)
		{
//			entry.ptr->emitConnect();
			sockData.state = net::SocketBase::DataForCommandProcessing::Connected;
//			evs.add(&net::Socket::emitConnect, current.getPtr());
//			current.getEmitter().emitConnect();
			ret = EmitConnect;
		}
		else
		{
//			errorCloseSocket(current, storeError(Error()));
			Error e;
			errorCloseSocket(sockData, e);
		}
	}
	else if (!sockData.writeBuffer.empty())
	{
//		assert(entry.writeBuffer.size() != 0);
		size_t sentSize = 0;
		uint8_t res = internal_usage_only::internal_send_packet(sockData.writeBuffer.begin(), sockData.writeBuffer.size(), sockData.osSocket, sentSize);
		if (res == COMMLAYER_RET_FAILED)
		{
			//			pendingCloseEvents.push_back(entry.id);
//			errorCloseSocket(current, storeError(Error()));
			Error e;
			errorCloseSocket(sockData, e);
		}
		else if (sentSize == sockData.writeBuffer.size())
		{
//			entry.writeEvents = false;
			sockData.writeBuffer.clear();
			if (sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnding)
			{
				internal_usage_only::internal_shutdown_send(sockData.osSocket);
				//current.pendingLocalEnd = false;
				//current.localEnded = true;

				if (sockData.remoteEnded)
				{
					//current.state = net::SocketBase::DataForCommandProcessing::Closing;
					//pendingCloseEvents.emplace_back(current.index, false);
					closeSocket(sockData);
				}
				else
					sockData.state = net::SocketBase::DataForCommandProcessing::LocalEnded;
			}
				
//			entry.ptr->emitDrain();
//			evs.add(&net::Socket::emitDrain, current.getPtr());
//			current.getEmitter().emitDrain();
			ret = EmitDrain;
		}
		else
		{
			NODECPP_ASSERT(sentSize < sockData.writeBuffer.size());
//			entry.writeEvents = true;
			sockData.writeBuffer.popFront(sentSize);
		}
	}
	else //ignore?
		NODECPP_ASSERT(false, "Not supported yet!");

	return ret;
}

/*
 * TODO: for performace reasons, the poll data should be cached inside NetSocketManager
 * and updated at the same time that StreamSocketEntry.
 * Avoid to have to appWrite it all over again every time
 * 
 */

void NetSocketManagerBase::closeSocket(net::SocketBase::DataForCommandProcessing& sockData)
{
	sockData.state = net::SocketBase::DataForCommandProcessing::Closing;
	pendingCloseEvents.push_back(std::make_pair( sockData.id, std::make_pair( false, Error())));
}

void NetSocketManagerBase::errorCloseSocket(net::SocketBase::DataForCommandProcessing& sockData, Error& err)
{
	sockData.state = net::SocketBase::DataForCommandProcessing::ErrorClosing;
	pendingCloseEvents.push_back(std::make_pair( sockData.id, std::make_pair( true, err)));
}


#ifndef NET_CLIENT_ONLY
NetServerManager::NetServerManager()
{
	//mb ioSockets[0] is always invalid
	ioSockets.emplace_back(0);
}

void NetServerManager::appListen(net::Server* ptr, uint16_t port, const char* ip, int backlog)
{
	Ip4 myIp = Ip4::parse(ip);

	Port myPort = Port::fromHost(port);


	SocketRiia s(internal_usage_only::internal_make_tcp_socket());
	if (!s)
	{
		throw Error();
	}

	if (!internal_usage_only::internal_bind_socket(s.get(), myIp, myPort))
	{
		throw Error();
	}

	if (!internal_usage_only::internal_listen_tcp_socket(s.get()))
	{
		throw Error();
	}

	size_t id = addEntry(ptr);
	if (id == 0)
	{
		NODECPP_TRACE0("Failed to addEntry at NetServerManager::listen");
		throw Error();
	}

	auto& entry = appGetEntry(id);
	entry.osSocket = s.release();

	net::Address addr;

	pendingEvents.add(id, &net::Server::emitListening, ptr, id, std::move(addr));
}


void NetServerManager::appClose(size_t id)
{
	auto& entry = appGetEntry(id);
	if (!entry.isValid())
	{
		NODECPP_TRACE("Unexpected id {} on NetServerManager::close", id);
		return;
	}

	pendingCloseEvents.emplace_back(entry.index, false);
}

size_t NetServerManager::infraGetPollFdSetSize() const
{
	return ioSockets.size();
}

/*
* TODO: for performace reasons, the poll data should be cached inside NetSocketManager
* and updated at the same time that StreamSocketEntry.
* Avoid to have to write it all over again every time
*
*/
bool NetServerManager::infraSetPollFdSet(pollfd* begin, const pollfd* end) const
{
	size_t sz = end - begin;
	assert(sz >= ioSockets.size());
	bool anyRefed = false;

	for (size_t i = 0; i != sz; ++i)
	{
		if (i < ioSockets.size() && ioSockets[i].isValid())
		{
			const auto& current = ioSockets[i];
			NODECPP_ASSERT(current.osSocket != INVALID_SOCKET);

			anyRefed = anyRefed || current.refed;

			begin[i].fd = current.osSocket;
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

void NetServerManager::infraGetCloseEvents(EvQueue& evs)
{
	// if there is an issue with a socket, we may need to close it,
	// and push an event here to notify later.

	for (auto& current : pendingCloseEvents)
	{
		//first remove any pending event for this socket
		pendingEvents.remove(current.first);
		if (current.first < ioSockets.size())
		{
			auto& entry = ioSockets[current.first];
			if (entry.isValid())
			{
				if (entry.osSocket != INVALID_SOCKET)
					internal_usage_only::internal_close(entry.osSocket);
				//			entry.getPtr()->emitClose(entry.second);
				evs.add(&net::Server::emitClose, entry.getPtr(), current.second);
			}
			entry = NetServerEntry(current.first);
		}
	}
	pendingCloseEvents.clear();
}


void NetServerManager::infraCheckPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs)
{
	assert(end - begin >= static_cast<ptrdiff_t>(ioSockets.size()));
	for (size_t i = 0; i != ioSockets.size(); ++i)
	{
		auto& current = ioSockets[i];
		if (begin[i].fd != INVALID_SOCKET)
		{
			if ((begin[i].revents & (POLLERR | POLLNVAL)) != 0) // check errors first
			{
				NODECPP_TRACE("POLLERR event at {}", begin[i].fd);
				internal_usage_only::internal_getsockopt_so_error(current.osSocket);
				infraMakeErrorEventAndClose(current, evs);
			}
			else if ((begin[i].revents & POLLIN) != 0)
			{
				NODECPP_TRACE("POLLIN event at {}", begin[i].fd);
				infraProcessAcceptEvent(current, evs);
			}
			else if (begin[i].revents != 0)
			{
				NODECPP_TRACE("Unexpected event at {}, value {:x}", begin[i].fd, begin[i].revents);
				internal_usage_only::internal_getsockopt_so_error(current.osSocket);
				infraMakeErrorEventAndClose(current, evs);
			}
		}
	}
}


void NetServerManager::infraProcessAcceptEvent(NetServerEntry& entry, EvQueue& evs)
{
	Ip4 remoteIp;
	Port remotePort;

	SocketRiia newSock(internal_usage_only::internal_tcp_accept(remoteIp, remotePort, entry.osSocket));
	if (!newSock)
		return;

	net::Socket* ptr = entry.getPtr()->makeSocket();
	auto& man = getInfra().getNetSocket();

	bool ok = man.infraAddAccepted(ptr, newSock.release(), evs);

	if (!ok)
		return;

//	entry.getPtr()->emitConnection(ptr);
	evs.add(&net::Server::emitConnection, entry.getPtr(), ptr);

	return;
}

size_t NetServerManager::addEntry(net::Server* ptr)
{
	for (size_t i = 1; i != ioSockets.size(); ++i) // skip ioSockets[0]
	{
		if (!ioSockets[i].isValid())
		{
			NetServerEntry entry(i, ptr);
			ioSockets[i] = std::move(entry);
			return i;
		}
	}

	if (ioSockets.size() == MAX_SOCKETS)
	{
		return 0;
	}

	size_t ix = ioSockets.size();
	ioSockets.emplace_back(ix, ptr);
	return ix;
}

NetServerEntry& NetServerManager::appGetEntry(size_t id)
{
	return ioSockets.at(id);
}

const NetServerEntry& NetServerManager::appGetEntry(size_t id) const
{
	return ioSockets.at(id);
}



void NetServerManager::infraMakeErrorEventAndClose(NetServerEntry& entry, EvQueue& evs)
{
	evs.add(&net::Server::emitError, entry.getPtr(), std::ref(infraStoreError(Error())));
	pendingCloseEvents.emplace_back(entry.index, true);
}
#endif // NO_SERVER_STAFF
