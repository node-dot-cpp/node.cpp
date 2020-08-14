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
#include <nodecpp/nls.h>

#include <time.h>
#include <climits>

#ifndef _MSC_VER
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


namespace nodecpp {

thread_local NLS threadLocalData;

} // namespace nodecpp


uint64_t infraGetCurrentTime()
{
#ifdef _MSC_VER
	return GetTickCount64() * 1000; // mks
#else
    struct timespec ts;
//    timespec_get(&ts, TIME_UTC);
	clock_gettime(CLOCK_MONOTONIC, &ts);
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
//		else if ( h.h )
//			h.h();
		else if ( h.h != nullptr )
		{
			auto hr = h.h;
			if ( h.setExceptionWhenDone )
			{
				nodecpp::setException(h.h, std::exception()); // TODO: switch to our exceptions ASAP!
			}
			h.h = nullptr;
			hr();
		}
	}
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

namespace nodecpp {
	nodecpp::Timeout setTimeout(std::function<void()> cb, int32_t ms)
	{
		return timeoutManager->appSetTimeout(cb, ms);
	}

#ifndef NODECPP_NO_COROUTINES
	nodecpp::Timeout setTimeoutForAction(awaitable_handle_t h, int32_t ms)
	{
		return timeoutManager->appSetTimeoutForAction(h, ms);
	}
#endif // NODECPP_NO_COROUTINES

	void refreshTimeout(Timeout& to)
	{
		return timeoutManager->appRefresh(to.getId());
	}

	void clearTimeout(const Timeout& to)
	{
		return timeoutManager->appClearTimeout(to.getId());
	}

	void setInmediate(std::function<void()> cb)
	{
		inmediateQueue->add(std::move(cb));
	}

	namespace time
	{
		size_t now()
		{
#if defined NODECPP_MSVC || ( (defined NODECPP_WINDOWS) && (defined NODECPP_CLANG) )
#ifdef NODECPP_X64
			return GetTickCount64();
#else
			return GetTickCount();
#endif // NODECPP_X86 or NODECPP_X64
#elif (defined NODECPP_CLANG) || (defined NODECPP_GCC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (size_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
#else
#error not implemented for this compiler
#endif
		}

#ifdef USE_TEMP_PERF_CTRS
		std::pair<long, long> get_thread_time()
		{
#if defined NODECPP_MSVC || ( (defined NODECPP_WINDOWS) && (defined NODECPP_CLANG) )
			FILETIME creationTime;
			FILETIME exitTime;
			FILETIME kernelTime;
			FILETIME userTime;
			BOOL ok = GetThreadTimes( GetCurrentThread(),&creationTime, &exitTime, &kernelTime, &userTime );
			uint64_t user = userTime.dwHighDateTime;
			user <<= 32;
			user += userTime.dwLowDateTime;
			uint64_t kernel = kernelTime.dwHighDateTime;
			kernel <<= 32;
			kernel += kernelTime.dwLowDateTime;
			return std::make_pair( (long)(user/10), (long)(kernel/10) );
#elif (defined NODECPP_CLANG) || (defined NODECPP_GCC)
			/*int i;
			char proc_filename[256];
			char buffer[1024];

			pid_t pid = getpid();
			pid_t tid = gettid();

			sprintf(proc_filename, "/proc/%d/task/%d/stat",pid,tid);
			int fd;
			int num_read = 0;
			fd = open(proc_filename, O_RDONLY, 0);
			if ( fd < 0 )
			{
//				printf( "failed to open /proc/%d/task/%d/stat \n", pid, tid);
				return std::make_pair(0, 0);
			}
			num_read = read(fd, buffer, 1023);
			close(fd);
			if ( num_read < 0 )
			{
//				printf( "failed to read /proc/%d/task/%d/stat \n", pid, tid);
				return std::make_pair(0, 0);
			}
			buffer[num_read] = '\0';

			char* ptr_usr = strrchr(buffer, ')') + 1;
			for(i = 3; i != 14; ++i) {
				ptr_usr = strchr(ptr_usr+1, ' ');
				if ( ptr_usr == nullptr )
				{
//					printf( "%s\n", buffer );
					return std::make_pair(0, 0);
				}
			}

			ptr_usr++;
			uint64_t user = atol(ptr_usr);
			user *= 1000000;
			user /= sysconf(_SC_CLK_TCK);
			ptr_usr = strchr(ptr_usr,' ');
			if ( ptr_usr == nullptr )
			{
				printf( "%s\n", buffer );
				return std::make_pair(0, 0);
			}
			uint64_t kernel = atol(ptr_usr + 1);
			kernel *= 1000000;
			kernel /= sysconf(_SC_CLK_TCK);

			return std::make_pair(user, kernel);*/
			struct rusage usage;
			int ret = getrusage( RUSAGE_THREAD, &usage);
			if ( ret == 0 )
			{
				uint64_t user = usage.ru_utime.tv_sec;
				user *= 1000000;
				user += usage.ru_utime.tv_usec;
				uint64_t kernel = usage.ru_stime.tv_sec;
				kernel *= 1000000;
				kernel += usage.ru_stime.tv_usec;
				return std::make_pair( (long)(user), (long)(kernel) );
			}
			else
				return std::make_pair(0, 0);
#else
#error not implemented for this compiler
#endif
		}	
#endif // USE_TEMP_PERF_CTRS
	} // namespace time

} // namespace nodecpp


#ifdef USE_TEMP_PERF_CTRS
//thread_local int pollRetMax = 0;
//thread_local size_t sessionCreationtime = 0;
//thread_local int sessionCnt = 0;
thread_local uint64_t waitTime = 0;
thread_local uint64_t lastWaitTime = 0;

thread_local uint64_t writeTime = 0;
thread_local uint64_t lastWriteTime = 0;

thread_local int eventCnt = 0;
thread_local int lastEventCnt = 0;

thread_local uint64_t eventProcTime = 0;
thread_local uint64_t lastEventProcTime = 0;

thread_local int pollCnt = 0;
thread_local int lastPollCnt = 0;

thread_local int pollRetCnt = 0;
thread_local int lastPollRetCnt = 0;

thread_local int zeroSockCnt = 0;
thread_local int lastZeroSockCnt = 0;

thread_local long lastUserT = 0;
thread_local long lastKernelT = 0;
thread_local uint64_t lastTimeReportT = 0;
void reportTimes( uint64_t currentT)
{
	if ( currentT > lastTimeReportT + 1000000 )
	{
		uint64_t wallClockDelta = currentT - lastTimeReportT;
		lastTimeReportT = currentT;

		auto times = nodecpp::time::get_thread_time();
		bool userAndKernelTimeOK = times.first + times.second != 0;
		long userDelta = 0;
		long kernelDelta = 0;
		if ( userAndKernelTimeOK )
		{
			userDelta = times.first - lastUserT;
			kernelDelta = times.second - lastKernelT;
			lastUserT = times.first;
			lastKernelT = times.second;
		}
		long totalT = userDelta + kernelDelta;

		uint64_t waitDelta = waitTime - lastWaitTime;
		lastWaitTime = waitTime;

		uint64_t writeDelta = writeTime - lastWriteTime;
		lastWriteTime = writeTime;

		int eventDelta = eventCnt - lastEventCnt;
		lastEventCnt = eventCnt;
			
		uint64_t eventProcTimeDelta = eventProcTime - lastEventProcTime;
		lastEventProcTime = eventProcTime;

		int pollCntDelta = pollCnt - lastPollCnt;
		lastPollCnt = pollCnt;

		int pollRetCntDelta = pollRetCnt - lastPollRetCnt;
		lastPollRetCnt = pollRetCnt;

		int zeroSockDelta = zeroSockCnt - lastZeroSockCnt;
		lastZeroSockCnt = zeroSockCnt;

		if ( userAndKernelTimeOK )
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "[{}ms] user: {}%, kernel: {}%, wait: {} ({}%), write: {} ({}%); events: {} ({}), ev proc time: {} ({}%); polls: {}, pollRetAvg: {}, ZeroSockCnt: {}", 
				wallClockDelta / 1000, userDelta * 100 / wallClockDelta, kernelDelta * 100 / wallClockDelta, waitDelta / 1000, waitDelta * 100 / wallClockDelta, writeDelta / 1000, writeDelta * 100 / wallClockDelta, 
				eventDelta, eventCnt, eventProcTimeDelta/1000, eventProcTimeDelta * 100 / wallClockDelta, pollCntDelta, pollCntDelta ? pollRetCntDelta / pollCntDelta : 0, zeroSockDelta );
		else
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "[{}ms] user: --%, kernel: --%, wait: {} ({}%), write: {} ({}%); events: {} ({}), ev proc time: {} ({}%); polls: {}, pollRetAvg: {}, ZeroSockCnt: {}", 
				wallClockDelta / 1000, waitDelta / 1000, waitDelta * 100 / wallClockDelta, writeDelta / 1000, writeDelta * 100 / wallClockDelta, 
				eventDelta, eventCnt, eventProcTimeDelta/1000, eventProcTimeDelta * 100 / wallClockDelta, pollCntDelta, pollCntDelta ? pollRetCntDelta / pollCntDelta : 0, zeroSockDelta );
	}
}
#endif // USE_TEMP_PERF_CTRS

