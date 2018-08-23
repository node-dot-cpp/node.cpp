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


#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "../../include/nodecpp/common.h"
#include "../../include/nodecpp/net.h"
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

namespace nodecpp
{
	namespace internal_usage_only
	{
		bool internal_getsockopt_so_error(SOCKET sock);
		void internal_shutdown_send(SOCKET sock);
		bool internal_linger_zero_socket(SOCKET sock);
		void internal_close(SOCKET sock);
	} // internal_usage_only
} // nodecpp




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


class Ip4
{
	uint32_t ip = -1;
	Ip4(uint32_t ip) :ip(ip) {}
public:
	Ip4() {}
	Ip4(const Ip4&) = default;
	Ip4(Ip4&&) = default;
	Ip4& operator=(const Ip4&) = default;
	Ip4& operator=(Ip4&&) = default;


	uint32_t getNetwork() const { return ip; }
	static Ip4 parse(const char* ip);
	static Ip4 fromNetwork(uint32_t ip);
	std::string toStr() const { return std::string("TODO"); }
};

class Port
{
	uint16_t port = -1;
	Port(uint16_t port) :port(port) {}
public:
	Port() {}
	Port(const Port&) = default;
	Port(Port&&) = default;
	Port& operator=(const Port&) = default;
	Port& operator=(Port&&) = default;

	uint16_t getNetwork() const { return port; }
	static Port fromHost(uint16_t port);
	static Port fromNetwork(uint16_t port);
	std::string toStr() const { return std::string("TODO"); }

};

class SocketRiia // moved to .h
{
	SOCKET s;
public:
	SocketRiia(SOCKET s) :s(s) {}

	SocketRiia(const SocketRiia&) = delete;
	SocketRiia& operator=(const SocketRiia&) = delete;

	SocketRiia(SocketRiia&&other) {s = other.s; other.s = INVALID_SOCKET; }
	SocketRiia& operator=(SocketRiia&&other) {s = other.s; other.s = INVALID_SOCKET; return *this; }

	~SocketRiia();// { if (s != INVALID_SOCKET) internal_close(s); }

	SOCKET get() const noexcept { return s; }
	SOCKET release() noexcept { SOCKET tmp = s; s = INVALID_SOCKET; return tmp; }
	explicit operator bool() const noexcept { return s != INVALID_SOCKET; }

};



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

class NetSocketManagerBase {
protected:
	//mb: ioSockets[0] is always reserved and invalid.
//	std::vector<std::pair<size_t, std::function<void()>>> pendingCloseEvents;
	std::vector<std::pair<size_t, std::pair<bool, Error>>> pendingCloseEvents;

//	std::vector<Buffer> bufferStore; // TODO: improve
//	std::vector<Error> errorStore;

	std::string family = "IPv4";

protected:
	SocketRiia appAcquireSocket(const char* ip, uint16_t port);

public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetSocketManagerBase() {}

	void appDestroy(net::SocketBase::DataForCommandProcessing& sockData);
	void appEnd(net::SocketBase::DataForCommandProcessing& sockData);
	bool appWrite(net::SocketBase::DataForCommandProcessing& sockData, const uint8_t* data, uint32_t size);
	void appSetKeepAlive(net::SocketBase::DataForCommandProcessing& sockData, bool enable);
	void appSetNoDelay(net::SocketBase::DataForCommandProcessing& sockData, bool noDelay);

	std::pair<bool, Buffer> infraGetPacketBytes(Buffer& buff, SOCKET sock);

	enum ShouldEmit { EmitNone, EmitConnect, EmitDrain };
//	void infraProcessReadEvent(net::SocketBase::DataForCommandProcessing& sockData);
	ShouldEmit infraProcessWriteEvent(net::SocketBase::DataForCommandProcessing& sockData);

/*	//TODO quick workaround until definitive life managment is in place
	Buffer& infraStoreBuffer(Buffer buff) {
		bufferStore.push_back(std::move(buff));
		return bufferStore.back();
	}

	Error& storeError(Error err) { //app-infra neutral
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void infraClearStores() {
		bufferStore.clear();
//		errorStore.clear();
	}*/
	
	
public:

	void closeSocket(net::SocketBase::DataForCommandProcessing& sockData);//app-infra neutral
	void errorCloseSocket(net::SocketBase::DataForCommandProcessing& sockData, Error& err);//app-infra neutral
};

template<class EmitterType>
class NetSocketEntry {
public:
	size_t index;

	net::SocketBase* sockPtr = nullptr;
	EmitterType emitter;


	NetSocketEntry(size_t index) : index(index) {}
#ifdef USING_T_SOCKETS
	NetSocketEntry(size_t index, net::SocketTBase* ptr_, int type) : index(index), sockPtr(ptr_), emitter(ptr_, type) {}
#else
	template<class SocketType>
	NetSocketEntry(size_t index, SocketType* ptr_) : index(index), sockPtr(ptr_), emitter(ptr_) {}
#endif // USING_T_SOCKETS

	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	bool isValid() const { return emitter.isValid(); }

	const EmitterType& getEmitter() const { return emitter; }
	net::SocketBase::DataForCommandProcessing* getSockData() const { assert(sockPtr != nullptr); return sockPtr ? &(sockPtr->dataForCommandProcessing) : nullptr; }
};

template<class EmitterType>
class NetSocketManager : public NetSocketManagerBase {
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetSocketEntry<EmitterType>> ioSockets; // TODO: improve
	std::vector<Buffer> bufferStore; // TODO: improve

public:
	//TODO quick workaround until definitive life managment is in place
	Buffer& infraStoreBuffer(Buffer buff) {
		bufferStore.push_back(std::move(buff));
		return bufferStore.back();
	}

	void infraClearStores() {
		bufferStore.clear();
	}
	
public:
	NetSocketManager() { ioSockets.emplace_back(0); }

	template<class SockType>
	size_t appConnect(SockType* ptr, const char* ip, uint16_t port) // TODO: think about template with type checking inside
	{
		SocketRiia s( std::move( this->appAcquireSocket( ip, port ) ) );

		size_t id = addEntry(ptr);
		if (id == 0)
		{
			NODECPP_TRACE0("Failed to add entry on NetSocketManager::connect");
			throw Error();
		}

		auto& entry = appGetEntry(id);
		NODECPP_ASSERT(entry.getSockData()->state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		entry.getSockData()->osSocket = s.release();
		entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::Connecting;
	//	entry.connecting = true;

		return id;
	}

#ifdef USING_T_SOCKETS
	size_t appConnect(net::SocketTBase* ptr, int typeId, const char* ip, uint16_t port) // TODO: think about template with type checking inside
	{
		SocketRiia s( std::move( this->appAcquireSocket( ip, port ) ) );

		size_t id = addEntry(ptr, typeId);
		if (id == 0)
		{
			NODECPP_TRACE0("Failed to add entry on NetSocketManager::connect");
			throw Error();
		}

		auto& entry = appGetEntry(id);
		NODECPP_ASSERT(entry.getSockData()->state == net::SocketBase::DataForCommandProcessing::Uninitialized);
		entry.getSockData()->osSocket = s.release();
		entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::Connecting;
	//	entry.connecting = true;

		return id;
	}
#endif // USING_T_SOCKETS

#ifndef NET_CLIENT_ONLY
	template<class SocketType>
	bool infraAddAccepted(SocketType* ptr, SOCKET sock, EvQueue& evs)
	{
		SocketRiia s(sock);
		size_t ix = addEntry(ptr);
		if (ix == 0)
		{
			NODECPP_TRACE("Couldn't allocate new StreamSocket, closing {}", s.get());
			return false;
		}

		auto& entry = appGetEntry(ix);
		entry.getSockData()->osSocket = s.release();
		entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::Connected;

		evs.add(&net::Socket::emitAccepted, ptr, ix);

		return true;
	}
#endif // !NET_CLIENT_ONLY

	
	
public:
	// to help with 'poll'
	size_t infraGetPollFdSetSize() const { return ioSockets.size(); }
	bool infraSetPollFdSet(pollfd* begin, const pollfd* end) const
	{
		size_t sz = end - begin;
		assert(sz >= ioSockets.size());
		bool anyRefed = false;
	
		for (size_t i = 0; i != sz; ++i)
		{
			if(i < ioSockets.size() && ioSockets[i].isValid())
			{
				const auto& current = ioSockets[i];
				NODECPP_ASSERT(current.getSockData()->osSocket != INVALID_SOCKET);

				anyRefed = anyRefed || current.getSockData()->refed;

				begin[i].fd = current.getSockData()->osSocket;
				begin[i].events = 0;

				if(!current.getSockData()->remoteEnded && !current.getSockData()->paused)
					begin[i].events |= POLLIN;
				if (current.getSockData()->state == net::SocketBase::DataForCommandProcessing::Connecting || !current.getSockData()->writeBuffer.empty())
					begin[i].events |= POLLOUT;
			}
			else
				begin[i].fd = INVALID_SOCKET;
		}
		return anyRefed;
	}
	void infraGetCloseEvent(EvQueue& evs)
	{
		// if there is an issue with a socket, we may need to appClose it,
		// and push an event here to notify autom later.

		for (auto& current : pendingCloseEvents)
		{
			if (current.first < ioSockets.size())
			{
				auto& entry = ioSockets[current.first];
				if (entry.isValid())
				{
					bool err = entry.getSockData()->state == net::SocketBase::DataForCommandProcessing::ErrorClosing;
					if (entry.getSockData()->osSocket != INVALID_SOCKET)
					{
						if(err) // if error closing, then discard all buffers
							internal_usage_only::internal_linger_zero_socket(entry.getSockData()->osSocket);

						internal_usage_only::internal_close(entry.getSockData()->osSocket);
					}

					if (err) //if error closing, then first error event
					{
	//					evs.add(std::move(current.second));
//						std::function<void()> ev = std::bind(&net::SocketEmitter::emitError, entry.getEmitter(), current.second.second);
						std::function<void()> ev = std::bind(&EmitterType::emitError, entry.getEmitter(), current.second.second);
						evs.add(std::move(ev));
					}

	//				evs.add(&net::Socket::emitClose, entry.getPtr(), err);
	//				entry.getPtr()->emitClose(err);
					entry.getEmitter().emitClose(err);
					entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::Closed;
				}
				entry = NetSocketEntry<EmitterType>(current.first); 
			}
		}
		pendingCloseEvents.clear();
	}
	void infraCheckPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs)
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
					internal_usage_only::internal_getsockopt_so_error(current.getSockData()->osSocket);
					//errorCloseSocket(current, storeError(Error()));
					Error e;
					errorCloseSocket(current, e);
				}
				else {
					/*
						on Windows when the other appEnd sends a FIN,
						POLLHUP goes up, if we still have data pending to read
						both POLLHUP and POLLIN are up. POLLHUP continues to be up
						for the socket as long as the socket lives.
						on Linux, when remote appEnd sends a FIN, we get a zero size read
						after all pending data is read.
					*/

					if ((begin[i].revents & POLLIN) != 0)
					{
						if (!current.getSockData()->paused)
						{
							NODECPP_TRACE("POLLIN event at {}", begin[i].fd);
							infraProcessReadEvent(current, evs);
						}
					}
					else if ((begin[i].revents & POLLHUP) != 0)
					{
						NODECPP_TRACE("POLLHUP event at {}", begin[i].fd);
						infraProcessRemoteEnded(current, evs);
					}
				
					if ((begin[i].revents & POLLOUT) != 0)
					{
						NODECPP_TRACE("POLLOUT event at {}", begin[i].fd);
						infraProcessWriteEvent(current, evs);
					}
				}
				//else if (begin[i].revents != 0)
				//{
				//	NODECPP_TRACE("Unexpected event at {}, value {:x}", begin[i].fd, begin[i].revents);
				//	internal_getsockopt_so_error(entry.osSocket);
				//	errorCloseSocket(entry);
				//}
			}
		}
	}

private:
	void infraProcessReadEvent(NetSocketEntry<EmitterType>& entry, EvQueue& evs)
	{
		auto res = infraGetPacketBytes(entry.getSockData()->recvBuffer, entry.getSockData()->osSocket);
		if (res.first)
		{
			if (res.second.size() != 0)
			{
	//			entry.ptr->emitData(std::move(res.second));

	//			evs.add(&net::Socket::emitData, entry.getPtr(), std::ref(infraStoreBuffer(std::move(res.second))));
				entry.getEmitter().emitData(std::ref(infraStoreBuffer(std::move(res.second))));
			}
			else //if (!entry.remoteEnded)
			{
				infraProcessRemoteEnded(entry, evs);
			}
		}
		else
		{
			internal_usage_only::internal_getsockopt_so_error(entry.getSockData()->osSocket);
	//		return errorCloseSocket(entry, storeError(Error()));
			Error e;
			errorCloseSocket(entry, e);
		}
	}

	void infraProcessRemoteEnded(NetSocketEntry<EmitterType>& entry, EvQueue& evs)
	{
		if (!entry.getSockData()->remoteEnded)
		{
			entry.getSockData()->remoteEnded = true;
	//		evs.add(&net::Socket::emitEnd, entry.getPtr());
			entry.getEmitter().emitEnd();
			if (entry.getSockData()->state == net::SocketBase::DataForCommandProcessing::LocalEnded)
			{
				//pendingCloseEvents.emplace_back(entry.index, false);
				closeSocket(entry);
			}
			else if (!entry.getSockData()->allowHalfOpen && entry.getSockData()->state != net::SocketBase::DataForCommandProcessing::LocalEnding)
			{
				if (!entry.getSockData()->writeBuffer.empty())
				{
	//				entry.pendingLocalEnd = true;
					entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::LocalEnding;
				}
				else
				{
					internal_usage_only::internal_shutdown_send(entry.getSockData()->osSocket);
					entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::LocalEnded;
	//				entry.localEnded = true;
				}
			}
		}
		else
		{
			NODECPP_TRACE("Unexpected end on socket {}, already ended", entry.getSockData()->osSocket);
		}

	}

	void infraProcessWriteEvent(NetSocketEntry<EmitterType>& current, EvQueue& evs)
	{
		NetSocketManagerBase::ShouldEmit status = NetSocketManagerBase::infraProcessWriteEvent(*current.getSockData());
		switch ( status )
		{
			case NetSocketManagerBase::ShouldEmit::EmitConnect:
				current.getEmitter().emitConnect();
				break;
			case NetSocketManagerBase::ShouldEmit::EmitDrain:
				current.getEmitter().emitDrain();
				break;
			default:
				NODECPP_ASSERT(status == NetSocketManagerBase::ShouldEmit::EmitNone, "unexpected value {}", (size_t)status);
		}
	}

private:
#ifdef USING_T_SOCKETS
	size_t addEntry(net::SocketTBase* ptr, int typeId) //app-infra neutral
	{
		for (size_t i = 1; i != ioSockets.size(); ++i) // skip ioSockets[0]
		{
			if (!ioSockets[i].isValid())
			{
				NetSocketEntry<EmitterType> entry(i, ptr, typeId);
				ioSockets[i] = std::move(entry);
				return i;
			}
		}

		if (ioSockets.size() >= MAX_SOCKETS)
		{
			return 0;
		}

		size_t ix = ioSockets.size();
		ioSockets.emplace_back(ix, ptr, typeId);
		return ix;
	}

#else

	template<class SocketType>
	size_t addEntry(SocketType* ptr) //app-infra neutral
	{
		for (size_t i = 1; i != ioSockets.size(); ++i) // skip ioSockets[0]
		{
			if (!ioSockets[i].isValid())
			{
				NetSocketEntry<EmitterType> entry(i, ptr);
				ioSockets[i] = std::move(entry);
				return i;
			}
		}

		if (ioSockets.size() >= MAX_SOCKETS)
		{
			return 0;
		}

		size_t ix = ioSockets.size();
		ioSockets.emplace_back(ix, ptr);
		return ix;
	}
#endif // USING_T_SOCKETS

	NetSocketEntry<EmitterType>& appGetEntry(size_t id) { return ioSockets.at(id); }
	const NetSocketEntry<EmitterType>& appGetEntry(size_t id) const { return ioSockets.at(id); }

	void closeSocket(NetSocketEntry<EmitterType>& entry) //app-infra neutral
	{
		entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::Closing;

	//	evs.add(&net::Socket::emitError, entry.getPtr(), storeError(Error()));
	//	pendingCloseEvents.emplace_back(entry.index, std::function<void()>());
		pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( false, Error())));
	}
	void errorCloseSocket(NetSocketEntry<EmitterType>& entry, Error& err) //app-infra neutral
	{
		entry.getSockData()->state = net::SocketBase::DataForCommandProcessing::ErrorClosing;

	//	std::function<void()> ev = std::bind(&net::Socket::emitError, entry.getPtr(), std::ref(err));
	//	std::function<void()> ev = std::bind(&net::SocketEmitter::emitError, entry.getEmitter(), std::ref(err));
	//	pendingCloseEvents.emplace_back(entry.index, std::move(ev));
		pendingCloseEvents.push_back(std::make_pair( entry.index, std::make_pair( true, err)));
	}
};



#ifndef NET_CLIENT_ONLY
class NetServerEntry {
public:
	size_t index;
	net::Server* ptr = nullptr;
	bool refed = true;
	SOCKET osSocket = INVALID_SOCKET;
	short fdEvents = 0;

	NetServerEntry(size_t index) :index(index) {}
	NetServerEntry(size_t index, net::Server* ptr) :index(index), ptr(ptr) {}
	
	NetServerEntry(const NetServerEntry& other) = delete;
	NetServerEntry& operator=(const NetServerEntry& other) = delete;

	NetServerEntry(NetServerEntry&& other) = default;
	NetServerEntry& operator=(NetServerEntry&& other) = default;

	bool isValid() const { return ptr != nullptr; }

	net::Server* getPtr() const { return ptr; }
};


class NetServerManager
{
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetServerEntry> ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;
	PendingEvQueue pendingEvents;
	std::vector<Error> errorStore;

	std::string family = "IPv4";

public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetServerManager();

	void appClose(size_t id);
	void appListen(net::Server* ptr, uint16_t port, const char* ip, int backlog);

	void appRef(size_t id) { appGetEntry(id).refed = true; }
	void appUnref(size_t id) { appGetEntry(id).refed = false; }


	//TODO quick workaround until definitive life managment is in place
	Error& infraStoreError(Error err) {
		errorStore.push_back(std::move(err));
		return errorStore.back();
	}

	void infraClearStores() {
		errorStore.clear();
	}

	// to help with 'poll'
	size_t infraGetPollFdSetSize() const;
	bool infraSetPollFdSet(pollfd* begin, const pollfd* end) const;
	void infraGetCloseEvents(EvQueue& evs);
	void infraGetPendingEvents(EvQueue& evs) { pendingEvents.toQueue(evs); }
	void infraCheckPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs);

private:
	void infraProcessAcceptEvent(NetServerEntry& entry, EvQueue& evs);

	size_t addEntry(net::Server* ptr);
	NetServerEntry& appGetEntry(size_t id);
	const NetServerEntry& appGetEntry(size_t id) const;

	void infraMakeErrorEventAndClose(NetServerEntry& entry, EvQueue& evs);
};

#endif // !NET_CLIENT_ONLY


#endif // TCP_SOCKET_H

