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

#include "infrastructure.h"

#include <time.h>
#include <climits>

#ifndef _MSC_VER
#include <poll.h>
#endif


uint64_t infraGetCurrentTime()
{
#ifdef _MSC_VER
	return GetTickCount64() * 1000; // mks
#else
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
	return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000; // mks
#endif
}

void TimeoutManager::appSetTimeout(TimeoutEntry& entry)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,entry.active == false);

	entry.lastSchedule = infraGetCurrentTime();

	entry.nextTimeout = entry.lastSchedule + entry.delay;

	entry.active = true;
	nextTimeouts.insert(std::make_pair(entry.nextTimeout, entry.id));

}

void TimeoutManager::appClearTimeout(TimeoutEntry& entry)
{
	if (entry.active)
	{
		auto it2 = nextTimeouts.equal_range(entry.nextTimeout);
		bool found = false;
		while (it2.first != it2.second)
		{
			if (it2.first->second == entry.id)
			{
				nextTimeouts.erase(it2.first);
				entry.active = false;
				break;
			}
			++(it2.first);
		}
	}

	//if it was active, we must have deactivated it
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,entry.active == false);
}


nodecpp::Timeout TimeoutManager::appSetTimeout(std::function<void()> cb, int32_t ms)
{
	if (ms == 0)
		ms = 1;
	else if (ms < 0)
		ms = std::numeric_limits<int32_t>::max();

	uint64_t id = ++lastId;

	TimeoutEntry entry;
	entry.id = id;
	entry.cb = std::move(cb);
	entry.delay = ms * 1000;

	auto res = timers.insert(std::make_pair(id, std::move(entry)));
	if (res.second)
	{
		appSetTimeout(res.first->second);

		return Timeout(id);
	}
	else
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Failed to insert Timeout {}", id);
		return Timeout(0);
	}

}

void TimeoutManager::appClearTimeout(const nodecpp::Timeout& to)
{
	uint64_t id = to.getId();

	auto it = timers.find(id);
	if (it != timers.end())
	{
		appClearTimeout(it->second);
	}
}

void TimeoutManager::appRefresh(uint64_t id)
{
	auto it = timers.find(id);
	if (it != timers.end())
	{
		appClearTimeout(it->second);
		appSetTimeout(it->second);
	}
}

#ifndef NODECPP_NO_COROUTINES
void TimeoutManager::appRefresh(uint64_t id, std::experimental::coroutine_handle<> h)
{
	auto it = timers.find(id);
	if (it != timers.end())
	{
		appClearTimeout(it->second);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, it->second.ahd.h == nullptr ); 
		it->second.ahd.h = h;
		appSetTimeout(it->second);
	}
}
#endif // NODECPP_NO_COROUTINES


void TimeoutManager::appTimeoutDestructor(uint64_t id)
{
	auto it = timers.find(id);
	if (it != timers.end())
	{
		it->second.handleDestroyed = true;

		if(it->second.active == false)
			timers.erase(it);
	}
	else
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("timer {} not found", id);
}

void TimeoutManager::infraTimeoutEvents(uint64_t now, EvQueue& evs)
{
	auto itBegin = nextTimeouts.begin();
	auto itEnd = nextTimeouts.upper_bound(now);
	auto it = itBegin;
	while (it != itEnd)
	{
		auto it2 = timers.find(it->second);

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,it2 != timers.end());
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,it2->second.active);
		
		it2->second.active = false;
			
		evs.add(it2->second.cb);

		if (it2->second.handleDestroyed)
			timers.erase(it2);

		++it;
	}

	nextTimeouts.erase(itBegin, itEnd);
}



int getPollTimeout(uint64_t nextTimeoutAt, uint64_t now)
{
	 	if(nextTimeoutAt == TimeOutNever)
	 		return -1;
	 	else if(nextTimeoutAt <= now)
	 		return 0;
	 	else if((nextTimeoutAt - now) / 1000 + 1 <= uint64_t(INT_MAX))
	 		return static_cast<int>((nextTimeoutAt - now) / 1000 + 1);
	 	else
	         return INT_MAX;
}
