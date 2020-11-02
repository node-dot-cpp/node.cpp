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

#include "timeout_manager.h"

//#include <nodecpp/nls.h>
#include "../include/nodecpp/nls.h"
#include "../include/nodecpp/net_common.h"

#include <time.h>
#include <climits>

#ifdef _MSC_VER
#include <Windows.h>
#else
#define _GNU_SOURCE
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/syscall.h>
/* pid_t */
#include <sys/types.h>
/* open */
#include <sys/stat.h>
#include <fcntl.h>
/* strrchr */
#include <string.h>
#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif
#include <sys/resource.h>
#endif


void TimeoutManager::appSetTimeout(TimeoutEntry& entry, uint64_t now)
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,entry.active == false);

	entry.lastSchedule = now;

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


void TimeoutManager::appClearTimeout(const nodecpp::Timeout& to)
{
	uint64_t id = to.getId();

	auto it = timers.find(id);
	if (it != timers.end())
	{
		appClearTimeout(it->second);
	}
}

void TimeoutManager::appRefresh(uint64_t id, uint64_t now)
{
	auto it = timers.find(id);
	if (it != timers.end())
	{
		appClearTimeout(it->second);
		appSetTimeout(it->second, now);
	}
}


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
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"timer {} not found", id);
}

void TimeoutManager::infraTimeoutEvents(uint64_t now, EvQueue& evs)
{
	auto itBegin = nextTimeouts.begin();
	auto itEnd = nextTimeouts.upper_bound(now);
	auto it = itBegin;
	nodecpp::vector<TimeoutEntryHandlerData> handlers; // TODO: this approach could potentially be generalized
	while (it != itEnd)
	{
		auto it2 = timers.find(it->second);

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,it2 != timers.end());
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,it2->second.active);
		
		it2->second.active = false;
			
//		evs.add(it2->second.cb);
		handlers.push_back( it2->second );

		if (it2->second.handleDestroyed)
			timers.erase(it2);

		++it;
	}

	nextTimeouts.erase(itBegin, itEnd);

	for ( auto h : handlers )
	{
		if ( h.cb != nullptr )
			h.cb();
		else if ( h.h != nullptr )
		{
			auto hr = h.h;
			nodecpp::setCoroStatus( hr, nodecpp::CoroStandardOutcomes::timeout );
			h.h = nullptr;
			hr();
		}
	}
}

thread_local TimeoutManager* timeoutManager;

