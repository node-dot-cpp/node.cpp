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

using namespace nodecpp;

#ifdef _MSC_VER
#include <winsock2.h>
using socklen_t = int;
#else
using SOCKET = int;
const SOCKET INVALID_SOCKET = -1;
struct pollfd;
#endif


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

bool isNetInitialized();

class NetSocketEntryBase {
public:
	size_t index;
	enum State { Uninitialized, Connecting, Connected, LocalEnding, LocalEnded, Closing, ErrorClosing, Closed}
	state = Uninitialized;

//	bool connecting = false;
	bool remoteEnded = false;
	//bool localEnded = false;
	//bool pendingLocalEnd = false;
	bool paused = false;
	bool allowHalfOpen = true;

	bool refed = true;

	Buffer writeBuffer = Buffer(64 * 1024);
	Buffer recvBuffer = Buffer(64 * 1024);

	SOCKET osSocket = INVALID_SOCKET;

	NetSocketEntryBase(size_t index) :index(index) {}

	NetSocketEntryBase(const NetSocketEntryBase& other) = delete;
	NetSocketEntryBase& operator=(const NetSocketEntryBase& other) = delete;

	NetSocketEntryBase(NetSocketEntryBase&& other) = default;
	NetSocketEntryBase& operator=(NetSocketEntryBase&& other) = default;

};

template<class SocketT>
class NetSocketEntry : public NetSocketEntryBase {
public:
	SocketT* ptr = nullptr;


	NetSocketEntry(size_t index) :NetSocketEntryBase(index) {}
	NetSocketEntry(size_t index, SocketT* ptr) :NetSocketEntryBase(index), ptr(ptr) {}

	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	bool isValid() const { return ptr != nullptr; }

	SocketT* getPtr() const {
		return ptr;
	}
};

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

class NetSocketManager {
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetSocketEntry<net::Socket>> ioSockets; // TODO: improve
	std::vector<std::pair<size_t, std::function<void()>>> pendingCloseEvents;

	std::vector<NetSocketEntry<net::SocketO>> ioSocketsO; // TODO: improve
	std::vector<std::pair<size_t, NetSocketEntry<net::SocketO>>> pendingCloseEventsO;

	std::vector<Buffer> bufferStore; // TODO: improve
	std::vector<Error> errorStore;

	std::string family = "IPv4";
	
public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetSocketManager();
	
	size_t appConnect(net::Socket* ptr, const char* ip, uint16_t port);

	void appDestroy(size_t id);
	void appEnd(size_t id);
	void appPause(size_t id) { appGetEntry(id).paused = true; }
	void appResume(size_t id) { appGetEntry(id).paused = false; }

	size_t appBufferSize(size_t id) { return appGetEntry(id).writeBuffer.size(); }

	void appRef(size_t id) { appGetEntry(id).refed = true; }
	void appSetKeepAlive(size_t id, bool enable);
	void appSetNoDelay(size_t id, bool noDelay);
	void appUnref(size_t id) { appGetEntry(id).refed = false; }
	bool appWrite(size_t id, const uint8_t* data, uint32_t size);

	bool infraAddAccepted(net::Socket* ptr, SOCKET sock, EvQueue& evs);

	//TODO quick workaround until definitive life managment is in place
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
		errorStore.clear();
	}
	
	
public:
	// to help with 'poll'
	size_t infraGetPollFdSetSize() const;
	bool infraSetPollFdSet(pollfd* begin, const pollfd* end) const;
	void infraGetCloseEvent(EvQueue& evs);
	void infraCheckPollFdSet(const pollfd* begin, const pollfd* end, EvQueue& evs);

private:
	void infraProcessReadEvent(NetSocketEntry<net::Socket>& current, EvQueue& evs);
	void infraProcessRemoteEnded(NetSocketEntry<net::Socket>& current, EvQueue& evs);
	void infraProcessWriteEvent(NetSocketEntry<net::Socket>& current, EvQueue& evs);

	std::pair<bool, Buffer> infraGetPacketBytes(Buffer& buff, SOCKET sock);

private:
	size_t addEntry(net::Socket* ptr);//app-infra neutral
	NetSocketEntry<net::Socket>& appGetEntry(size_t id);
	const NetSocketEntry<net::Socket>& appGetEntry(size_t id) const;

	void closeSocket(NetSocketEntry<net::Socket>& entry);//app-infra neutral
	void errorCloseSocket(NetSocketEntry<net::Socket>& entry, Error& err);//app-infra neutral
};



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


#endif // TCP_SOCKET_H

