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


void runInfraLoop();
bool pollPhase(bool refed, uint64_t nextTimeoutAt, uint64_t now, EvQueue& evs);

template<class EmitterType>
class Infrastructure
{
	friend void runInfraLoop();
	friend bool pollPhase(bool refed, uint64_t nextTimeoutAt, uint64_t now, EvQueue& evs);

	NetSocketManager<EmitterType> netSocket;
	NetServerManager netServer;
	TimeoutManager timeout;
	EvQueue inmediateQueue;

public:
	NetSocketManagerBase& getNetSocketBase() { return netSocket; }
public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	NetSocketManager<EmitterType>& getNetSocket() { return netSocket; }
	NetServerManager& getNetServer() { return netServer; }
	TimeoutManager& getTimeout() { return timeout; }
	void setInmediate(std::function<void()> cb) { inmediateQueue.add(std::move(cb)); }
	void emitInmediates() { inmediateQueue.emit(); }

	bool refedTimeout() const noexcept
	{
		return !inmediateQueue.empty() || timeout.infraRefedTimeout();
	}

	uint64_t nextTimeout() const noexcept
	{
		return inmediateQueue.empty() ? timeout.infraNextTimeout() : 0;
	}

//	bool pollPhase(bool refed, uint64_t nextTimeoutAt, uint64_t now, EvQueue& evs);
//	void runLoop();
};

extern thread_local Infrastructure<net::SocketEmitter> infra;

inline
Infrastructure<net::SocketEmitter>& getInfra() { return infra; }

inline
NetSocketManagerBase& getNetSocket() { return infra.getNetSocket(); }

inline
NetServerManager& getNetServer() { return infra.getNetServer(); }

template<class T>
size_t connectToInfra(T* t, const char* ip, uint16_t port) { return infra.getNetSocket().appConnect(t, ip, port); }


//using EvQueue = std::vector<std::function<void()>>;




#endif //INFRASTRUCTURE_H
