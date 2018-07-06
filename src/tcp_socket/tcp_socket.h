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

using namespace nodecpp;

#ifdef _MSC_VER
#include <winsock2.h>
using socklen_t = int;
#else
using SOCKET = int;
const SOCKET INVALID_SOCKET = -1;
struct pollfd;
#endif

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

class NetSocketEntry {
public:
	size_t index;

	net::Socket* ptr = nullptr;

	bool connecting = false;
	bool remoteEnded = false;
	bool localEnded = false;
	bool paused = false;

	Buffer writeBuffer = Buffer(1024);
	Buffer recvBuffer = Buffer(1024);

	SOCKET osSocket = INVALID_SOCKET;


	NetSocketEntry(size_t index) :index(index) {}
	NetSocketEntry(size_t index, net::Socket* ptr) :index(index), ptr(ptr) {}

	NetSocketEntry(const NetSocketEntry& other) = delete;
	NetSocketEntry& operator=(const NetSocketEntry& other) = delete;

	NetSocketEntry(NetSocketEntry&& other) = default;
	NetSocketEntry& operator=(NetSocketEntry&& other) = default;

	bool isValid() const { return ptr != nullptr; }

	net::Socket* getPtr() const {
		return ptr;
	}
};


class NetSocketManager {
	//mb: ioSockets[0] is always reserved and invalid.
	std::vector<NetSocketEntry> ioSockets; // TODO: improve
	std::vector<std::pair<size_t, bool>> pendingCloseEvents;

	std::string family = "IPv4";
	
public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetSocketManager();
	
	size_t connect(net::Socket* ptr, const char* ip, uint16_t port);
	void destroy(size_t id);
	void end(size_t id);
	void pause(size_t id) { getEntry(id).paused = true; }
	void resume(size_t id) { getEntry(id).paused = false; }

	size_t bufferSize(size_t id) { return getEntry(id).writeBuffer.size(); }

	void setKeepAlive(size_t id, bool enable);
	void setNoDelay(size_t id, bool noDelay);
	bool write(size_t id, const uint8_t* data, uint32_t size);


	// to help with 'poll'
	size_t getPollFdSetSize() const;
	void setPollFdSet(pollfd* begin, const pollfd* end) const;
	void getPendingEvent();
	void checkPollFdSet(const pollfd* begin, const pollfd* end);

private:
	void processReadEvent(NetSocketEntry& current);
	void processWriteEvent(NetSocketEntry& current);

	std::pair<bool, Buffer> getPacketBytes(Buffer& buff, SOCKET sock);
public:
	size_t addEntry(net::Socket* ptr);
	bool addAccepted(net::Socket* ptr, SOCKET sock);

	void releaseEntry(size_t id);
private:
	NetSocketEntry& getEntry(size_t id);
	const NetSocketEntry& getEntry(size_t id) const;

	void makeErrorEventAndClose(NetSocketEntry& entry);
};



class NetServerEntry {
public:
	size_t index;
	net::Server* ptr = nullptr;
	SOCKET osSocket = INVALID_SOCKET;

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

	std::string family = "IPv4";

public:
	static const size_t MAX_SOCKETS = 100; //arbitrary limit
	NetServerManager();

	void listen(net::Server* ptr, uint16_t port, const char* ip, int backlog);
	void close(size_t id);

	// to help with 'poll'
	size_t getPollFdSetSize() const;
	void setPollFdSet(pollfd* begin, const pollfd* end) const;
	void getPendingEvent();
	void checkPollFdSet(const pollfd* begin, const pollfd* end);

private:
	void processAcceptEvent(NetServerEntry& entry);
public:
	size_t addEntry(net::Server* ptr);
	void releaseEntry(size_t id);
private:
	NetServerEntry& getEntry(size_t id);
	const NetServerEntry& getEntry(size_t id) const;

	void makeErrorEventAndClose(NetServerEntry& entry);
};


#endif // TCP_SOCKET_H

