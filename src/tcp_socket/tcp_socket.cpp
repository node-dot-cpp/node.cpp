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
	nodecpp::log::init_log(); // just make sure
#ifdef _MSC_VER
	// do Windows magic
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("WSAStartup failed with error: {}", iResult);
		return false;
	}
	nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("WSAStartup success");
#endif
	return true;
}


namespace nodecpp
{
	namespace internal_usage_only
	{
		void internal_close(SOCKET sock)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_close() on sock {}", sock);
			CLOSE_SOCKET(sock);
		}

		void internal_shutdown_send(SOCKET sock)
		{
		#ifdef _MSC_VER
			int how = SD_SEND;
		#else
			int how = SHUT_WR;
		#endif

			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_shutdown_send() on sock {}", sock);

			int res = shutdown(sock, how);
			if (0 != res)
			{
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("shutdown on sock {} failed; error {}", sock, error);
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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("async on sock {} failed; error {}", sock, error);
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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("TCP_NODELAY on sock {} failed; error {}", sock, error);
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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("TCP_NODELAY on sock {} failed; error {}", sock, error);
				//		internal_close(sock);
				return false;
			}

			return true;
		}

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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("SO_LINGER on sock {} failed; error {}", sock, error);
				return false;
			}

			return true;
		}

		SOCKET internal_make_tcp_socket()
		{
			SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == sock)
			{
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("socket() failed; error {}", error);
				return INVALID_SOCKET;
			}

			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("socket() success {}", sock);

			if (!internal_async_socket(sock))
			{
				internal_close(sock);
				return INVALID_SOCKET;
			}
	
			return sock;
		}

		bool internal_bind_socket(SOCKET sock, struct ::sockaddr_in& sa_self)
		{
			int res = ::bind(sock, (struct sockaddr *)(&sa_self), sizeof(struct ::sockaddr_in));
			if (0 != res)
			{
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("bind() on sock {} failed; error {}", sock, error);
				return false;
			}

			return true;
		}

		static
		uint8_t internal_connect_socket(struct ::sockaddr_in& sa_other, SOCKET sock)
		{
			int res = connect(sock, (struct sockaddr *)(&sa_other), sizeof(struct ::sockaddr_in));
			if (0 != res)
			{
				int error = getSockError();
				if (isErrorWouldBlock(error))
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("connect() on sock {} in progress", sock);
					return COMMLAYER_RET_PENDING;
				}
				else
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("connect() on sock {} failed; error {}", sock, error);
					return COMMLAYER_RET_FAILED;
				}
			}
			else
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("connect() on sock {} completed", sock);

			return COMMLAYER_RET_OK;
		}

		bool internal_bind_socket(SOCKET sock, Ip4 ip, Port port)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_bind_socket() on sock {} to {}:{}", sock, ip.toStr(), port.toStr());
			struct ::sockaddr_in sa;
			memset(&sa, 0, sizeof(struct ::sockaddr_in));
			sa.sin_family = AF_INET;
			sa.sin_addr.s_addr = ip.getNetwork();
			sa.sin_port = port.getNetwork();

			return internal_bind_socket(sock, sa);
		}

		bool internal_listen_tcp_socket(SOCKET sock)
		{
			int res = listen(sock, tcpListenBacklogSize);
			if (0 != res) {
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("listen() on sock {} failed; error {}", sock, error);
				return false;
			}
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("listen() on sock {} success", sock);

			return true;
		}

		static
		uint8_t internal_connect_for_address(Ip4 peerIp, Port peerPort, SOCKET sock)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_connect_for_address() on sock {} to {}:{}", sock, peerIp.toStr(), peerPort.toStr());

			struct ::sockaddr_in saOther;
			memset(&saOther, 0, sizeof(struct ::sockaddr_in));
			saOther.sin_family = AF_INET;
			saOther.sin_addr.s_addr = peerIp.getNetwork();
			saOther.sin_port = peerPort.getNetwork();

			return internal_connect_socket(saOther, sock);
		}

		SOCKET internal_tcp_accept(Ip4& ip, Port& port, SOCKET sock)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_tcp_accept() on sock {}", sock);
			struct ::sockaddr_in sa;
			socklen_t sz = sizeof(struct ::sockaddr_in);
			memset(&sa, 0, sz);

			SOCKET outSock = accept(sock, (struct sockaddr *)&sa, &sz);
			if (INVALID_SOCKET == outSock)
			{
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("accept() on sock {} failed; error {}", error);

				return INVALID_SOCKET;
			}

	
			ip = Ip4::fromNetwork(sa.sin_addr.s_addr);
			port = Port::fromNetwork(sa.sin_port);
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("accept() new sock {} from {}:{}", outSock, ip.toStr(), port.toStr());

			if (!internal_async_socket(outSock))
			{
				internal_close(outSock);
				return INVALID_SOCKET;
			}
			return outSock;
		}

		bool internal_getsockopt_so_error(SOCKET sock)
		{
			int result;
			socklen_t result_len = sizeof(result);

			int err = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)(&result), &result_len);
			if (err != 0)
			{
				int error = getSockError();
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("getsockopt() SO_ERROR on sock {} failed; error {}", sock, error);
				return false;
			}

			if (result != 0)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("getsockopt() SO_ERROR on sock {} error {}", sock, result);
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
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_send_packet() on sock {} size {} PENDING", sock, size, sentSize);

					return COMMLAYER_RET_PENDING;
				}
				else
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_send_packet() on sock {} size {} ERROR {}", sock, size, error);
					return COMMLAYER_RET_FAILED;
				}
			}
			else
			{
				sentSize = static_cast<size_t>(bytes_sent);
				if(sentSize == size)
				{
					//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_send_packet() on sock {} size {} OK", sock, size);

					return COMMLAYER_RET_OK;
				}
				else
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_send_packet() on sock {} size {} PENDING sent {} ", sock, size, sentSize);
			
					return COMMLAYER_RET_PENDING;
				}
			}
		}

		class internal_send_packet_object
		{
			SOCKET sock;
			uint8_t ret;
		public:
			internal_send_packet_object( SOCKET sock_ ) : sock( sock_ ) {};
			bool write( const uint8_t* data, size_t size, size_t& sentSize_ ) {
				ret =  internal_send_packet( data, size, sock, sentSize_ );
				return ret == COMMLAYER_RET_OK && size == sentSize_;
			}
			uint8_t get_ret_value() const { return ret; }
		};

		static
		uint8_t internal_get_packet_bytes2(SOCKET sock, uint8_t* buff, size_t buffSz, size_t& retSz, struct ::sockaddr_in& sa_other, socklen_t& fromlen)
		{
			retSz = 0;
	
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,buff);
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,buffSz != 0);
			ssize_t ret = recvfrom(sock, (char*)buff, (int)buffSz, 0, (struct sockaddr *)(&sa_other), &fromlen);

			if (ret < 0)
			{
				int error = getSockError();
				if (isErrorWouldBlock(error))
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_get_packet_bytes2() on sock {} PENDING", sock);
					return COMMLAYER_RET_PENDING;
				}
				else
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_get_packet_bytes2() on sock {} ERROR {}", sock, error);
					return COMMLAYER_RET_FAILED;
				}
			}

			retSz = static_cast<size_t>(ret);
			//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("internal_get_packet_bytes2() on sock {} size {} OK", sock, retSz);
			return COMMLAYER_RET_OK;
		}

		class internal_read_packet_object
		{
			SOCKET sock;
			uint8_t ret;
		public:
			internal_read_packet_object( SOCKET sock_ ) : sock( sock_ ) {};
			bool read( uint8_t* data, size_t size, size_t& sentSize_ ) {
				socklen_t fromlen = sizeof(struct ::sockaddr_in);
				struct ::sockaddr_in sa_other;
				ret =  internal_get_packet_bytes2( sock, data, size, sentSize_, sa_other, fromlen );
				return ret == COMMLAYER_RET_OK && size == sentSize_;
			}
			uint8_t get_ret_value() const { return ret; }
		};

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

//thread_local std::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;
thread_local NetSocketManagerBase* netSocketManagerBase;
//thread_local int typeIndexOfSocketO = -1;
//thread_local int typeIndexOfSocketL = -1;
#ifndef NET_CLIENT_ONLY
thread_local NetServerManagerBase* netServerManagerBase;
thread_local TimeoutManager* timeoutManager;
thread_local EvQueue* inmediateQueue;
#endif


SocketRiia OSLayer::appAcquireSocket()
{
	SocketRiia s(internal_usage_only::internal_make_tcp_socket());
	if (!s)
		throw Error();

	return s;
}

void OSLayer::appConnectSocket(SOCKET s, const char* ip, uint16_t port)
{
	Ip4 peerIp = Ip4::parse(ip);
//	Ip4 myIp = Ip4::parse(ip);

	Port peerPort = Port::fromHost(port);
//	Port myPort = Port::fromHost(port);


	uint8_t st = internal_usage_only::internal_connect_for_address(peerIp, peerPort, s);

	if (st != COMMLAYER_RET_PENDING && st != COMMLAYER_RET_OK)
		throw Error();
}

void OSLayer::appDestroy(net::SocketBase::DataForCommandProcessing& sockData)
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetSocketManager::destroy", sockData.index);
		throw Error();
	}
	//	
	//	if(force)
	internal_usage_only::internal_linger_zero_socket(sockData.osSocket);
	//entry.state = net::SocketBase::DataForCommandProcessing::Closing;
	//pendingCloseEvents.emplace_back(entry.index, false);
	closeSocket(sockData);
}

void OSLayer::appEnd(net::SocketBase::DataForCommandProcessing& sockData)
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetSocketManager::end", sockData.index);
		throw Error();
	}

//	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,entry.state == net::SocketBase::DataForCommandProcessing::Connected);
	
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

void OSLayer::appSetKeepAlive(net::SocketBase::DataForCommandProcessing& sockData, bool enable)
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetSocketManager::setKeepAlive", sockData.index);
		throw Error();
	}

	if (!internal_usage_only::internal_socket_keep_alive(sockData.osSocket, enable))
	{
//		errorCloseSocket(entry, storeError(Error()));
		Error e;
		errorCloseSocket(sockData, e);
	}
}

void OSLayer::appSetNoDelay(net::SocketBase::DataForCommandProcessing& sockData, bool noDelay)
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetSocketManager::setNoDelay", sockData.index);
		throw Error();
	}

	if (!internal_usage_only::internal_tcp_no_delay_socket(sockData.osSocket, noDelay))
	{
//		errorCloseSocket(entry, storeError(Error()));
		Error e;
		errorCloseSocket(sockData, e);
	}
}

//bool OSLayer::appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size)
bool NetSocketManagerBase::appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size)
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected StreamSocket {} on sendStreamSegment", sockData.index);
		throw Error();
	}

	if (sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnding || sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnded)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("StreamSocket {} already ended", sockData.index);
//		errorCloseSocket(sockData, storeError(Error()));
		Error e;
		OSLayer::errorCloseSocket(sockData, e);
		return false;
	}

	if (sockData.writeBuffer.used_size() == 0)
	{
		size_t sentSize = 0;
		uint8_t res = internal_usage_only::internal_send_packet(data, size, sockData.osSocket, sentSize);
		if (res == COMMLAYER_RET_FAILED)
		{
//			errorCloseSocket(sockData, storeError(Error()));
			Error e;
			OSLayer::errorCloseSocket(sockData, e);
			return false;
		}
		else if (sentSize == size)
		{
			return true;
		}
		else 
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sentSize < size);
			sockData.writeBuffer.append(data + sentSize, size - sentSize);
			ioSockets.setPollout( sockData.index );
			return false;
		}
	}
	else
	{
		sockData.writeBuffer.append(data, size);
		return false;
	}
}

bool NetSocketManagerBase::appWrite2(net::SocketBase::DataForCommandProcessing& sockData, Buffer& buff )
{
	if (!sockData.isValid())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected StreamSocket {} on sendStreamSegment", sockData.index);
		throw Error();
	}

	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !nodecpp::isException(sockData.ahd_write.h) ); // should not be set yet

	if (sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnding || sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnded)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("StreamSocket {} already ended", sockData.index);
		nodecpp::setException(sockData.ahd_write.h, std::exception()); // TODO: switch to our exceptions ASAP!
//		errorCloseSocket(sockData, storeError(Error()));
		Error e;
		OSLayer::errorCloseSocket(sockData, e);
		return false;
	}

	if (sockData.writeBuffer.used_size() == 0)
	{
		size_t sentSize = 0;
		uint8_t res = internal_usage_only::internal_send_packet(buff.begin(), buff.size(), sockData.osSocket, sentSize);
		if (res == COMMLAYER_RET_FAILED)
		{
			nodecpp::setException(sockData.ahd_write.h, std::exception()); // TODO: switch to our exceptions ASAP!
//			errorCloseSocket(sockData, storeError(Error()));
			Error e;
			OSLayer::errorCloseSocket(sockData, e);
			return false;
		}
		else if (sentSize == buff.size())
		{
			return true;
		}
		else 
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sentSize < buff.size());
			sockData.writeBuffer.append(buff.begin() + sentSize, buff.size() - sentSize);
			ioSockets.setPollout( sockData.index );
			return false;
		}
	}
	else
	{
		if ( sockData.writeBuffer.remaining_capacity() >= buff.size() )
		{
			sockData.writeBuffer.append(buff.begin(), buff.size());
			return true;
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sockData.ahd_write.b.size() == 0 );
			sockData.ahd_write.b = std::move( buff );
			return false;
		}
	}
}

std::pair<bool, Buffer> OSLayer::infraGetPacketBytes(Buffer& buff, SOCKET sock)
{
	size_t sz = 0;
	socklen_t fromlen = sizeof(struct ::sockaddr_in);
	struct ::sockaddr_in sa_other;
	uint8_t ret = internal_usage_only::internal_get_packet_bytes2(sock, buff.begin(), buff.capacity(), sz, sa_other, fromlen);

	if (ret != COMMLAYER_RET_OK)
		return make_pair(false, Buffer());

	Buffer res(sz);
	res.append(buff.begin(), sz);
	buff.clear();

	return make_pair(true, std::move(res));
}

bool OSLayer::infraGetPacketBytes2(CircularByteBuffer& buff, SOCKET sock, size_t target_sz)
{
	size_t sz = 0;
	internal_usage_only::internal_read_packet_object reader( sock );
	buff.read( reader, sz, target_sz );

	if ( !(reader.get_ret_value() == COMMLAYER_RET_OK || reader.get_ret_value() == COMMLAYER_RET_PENDING ) )
		return false;

	return true;
}

NetSocketManagerBase::ShouldEmit NetSocketManagerBase::_infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData)
{
	NetSocketManagerBase::ShouldEmit ret = EmitNone; // as a base assumption
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
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sockData.writeBuffer.empty());
			ioSockets.unsetPollout(sockData.index);
//			evs.add(&net::Socket::emitConnect, current.getPtr());
//			current.getEmitter().emitConnect();
			ret = EmitConnect;
		}
		else
		{
//			errorCloseSocket(current, storeError(Error()));
			Error e;
			OSLayer::errorCloseSocket(sockData, e);
		}
	}
	else if (!sockData.writeBuffer.empty())
	{
		size_t sentSize = 0;
		//uint8_t res = internal_usage_only::internal_send_packet(sockData.writeBuffer.begin(), sockData.writeBuffer.used_size(), sockData.osSocket, sentSize);
		internal_usage_only::internal_send_packet_object writer(sockData.osSocket);
		sockData.writeBuffer.write(writer, sentSize);
		if ( writer.get_ret_value() == COMMLAYER_RET_FAILED )
		{
			//			pendingCloseEvents.push_back(entry.id);
//			errorCloseSocket(current, storeError(Error()));
			Error e;
			OSLayer::errorCloseSocket(sockData, e);
		}
//		else if (sentSize == sockData.writeBuffer.size())
		else if ( sockData.writeBuffer.empty() )
		{
//			entry.writeEvents = false;
//			sockData.writeBuffer.clear();

			bool all_done = true;

			if ( sockData.ahd_write.b.size() )
			{
				uint8_t res = internal_usage_only::internal_send_packet(sockData.ahd_write.b.begin(), sockData.ahd_write.b.size(), sockData.osSocket, sentSize);
				if (res == COMMLAYER_RET_FAILED)
				{
					Error e;
					OSLayer::errorCloseSocket(sockData, e);
				}
				else if (sentSize == sockData.ahd_write.b.size())
					sockData.ahd_write.b.clear();
				else
					all_done = false;
			}

			if ( all_done )
			{
				//updateEventMaskOnWriteBufferStatusChanged( sockData.index, true );
				if (sockData.state == net::SocketBase::DataForCommandProcessing::LocalEnding)
				{
					internal_usage_only::internal_shutdown_send(sockData.osSocket);
					//current.pendingLocalEnd = false;
					//current.localEnded = true;

					if (sockData.remoteEnded)
					{
						//current.state = net::SocketBase::DataForCommandProcessing::Closing;
						//pendingCloseEvents.emplace_back(current.index, false);
						OSLayer::closeSocket(sockData);
					}
					else
						sockData.state = net::SocketBase::DataForCommandProcessing::LocalEnded;
				}
				ioSockets.unsetPollout( sockData.index );
				
	//			entry.ptr->emitDrain();
	//			evs.add(&net::Socket::emitDrain, current.getPtr());
	//			current.getEmitter().emitDrain();

				ret = EmitDrain;
			}
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,sentSize < sockData.writeBuffer.used_size());
//			entry.writeEvents = true;
		}
	}
	else //ignore?
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,false, "Not supported yet!");

	return ret;
}

void OSLayer::closeSocket(net::SocketBase::DataForCommandProcessing& sockData)
{
	sockData.state = net::SocketBase::DataForCommandProcessing::Closing;
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, netSocketManagerBase != nullptr );
	netSocketManagerBase->pendingCloseEvents.push_back(std::make_pair( sockData.index, std::make_pair( false, Error())));
}

void OSLayer::errorCloseSocket(net::SocketBase::DataForCommandProcessing& sockData, Error& err)
{
	sockData.state = net::SocketBase::DataForCommandProcessing::ErrorClosing;
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, netSocketManagerBase != nullptr );
	netSocketManagerBase->pendingCloseEvents.push_back(std::make_pair( sockData.index, std::make_pair( true, err)));
}

void NetServerManagerBase::appClose(net::ServerBase::DataForCommandProcessing& serverData)
{
	size_t id = serverData.index;
	auto& entry = appGetEntry(id);
	if (!entry.isUsed())
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Unexpected id {} on NetServerManager::close", id);
		return;
	}

	internal_usage_only::internal_close(serverData.osSocket);
	ioSockets.setSocketClosed( entry.index );

	//pendingCloseEvents.emplace_back(entry.index, false); note: it will be finally closed only after all accepted connections are ended
}

size_t NetServerManagerBase::addServerEntry(/*NodeBase* node, */nodecpp::safememory::soft_ptr<net::ServerBase> ptr, int typeId)
{
	return ioSockets.addEntry<net::ServerBase>( /*node, */ptr, typeId );
}
