/* -------------------------------------------------------------------------------
* Copyright (c) 2020, OLogN Technologies AG
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


#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H

#include "../include/nodecpp/common.h"

#include "ev_queue.h"

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
	nodecpp::awaitable_handle_t h = nullptr;
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
	nodecpp::Timeout appSetTimeoutImpl(H h, int32_t ms, uint64_t now)
	{
		if (ms == 0) ms = 1;
		else if (ms < 0) ms = std::numeric_limits<int32_t>::max();

		uint64_t id = ++lastId;

		TimeoutEntry entry;
		entry.id = id;
		static_assert( !std::is_same<std::function<void()>, nodecpp::awaitable_handle_t>::value ); // we're in trouble anyway and not only here :)
		static_assert( std::is_same<H, std::function<void()>>::value || std::is_same<H, nodecpp::awaitable_handle_t>::value );
		if constexpr ( std::is_same<H, std::function<void()>>::value )
		{
			entry.cb = h;
			entry.h = nullptr;
		}
		else
		{
			static_assert( std::is_same<H, nodecpp::awaitable_handle_t>::value );
			entry.cb = nullptr;
			entry.h = h;
		}
		entry.delay = ms * 1000;

		auto res = timers.insert(std::make_pair(id, std::move(entry)));
		if (res.second)
		{
			appSetTimeout(res.first->second, now);

			return nodecpp::Timeout(id);
		}
		else
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"Failed to insert Timeout {}", id);
			return nodecpp::Timeout(0);
		}
	}

public:
	void appSetTimeout(TimeoutEntry& entry, uint64_t now);
	void appClearTimeout(TimeoutEntry& entry);

	nodecpp::Timeout appSetTimeout(std::function<void()> cb, int32_t ms, uint64_t now) { return appSetTimeoutImpl( cb, ms, now ); }
	void appClearTimeout(const nodecpp::Timeout& to);
	void appRefresh(uint64_t id, uint64_t now);
#ifndef NODECPP_NO_COROUTINES
	nodecpp::Timeout appSetTimeout(nodecpp::awaitable_handle_t ahd, int32_t ms, uint64_t now) { return appSetTimeoutImpl( ahd, ms, now ); }
	nodecpp::Timeout appSetTimeoutForAction(nodecpp::awaitable_handle_t ahd, int32_t ms, uint64_t now) { return appSetTimeoutImpl( ahd, ms, now ); }
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

#endif // TIMEOUT_MANAGER_H
