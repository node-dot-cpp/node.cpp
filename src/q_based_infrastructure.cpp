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

#ifdef NODECPP_USE_Q_BASED_INFRA

#include "q_based_infrastructure.h"

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


namespace nodecpp {

thread_local NLS threadLocalData;

} // namespace nodecpp

thread_local MsgQueue interThreadMessageQueues[1000]; // TODO:  way to customize


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
thread_local EvQueue* inmediateQueue;

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

	} // namespace time

} // namespace nodecpp

static InterThreadCommData threadQueues[MAX_THREADS];

static InterThreadCommInitializer interThreadCommInitializer;

struct ThreadDescriptor
{
	ThreadID threadID;
};
static thread_local ThreadDescriptor thisThreadDescriptor;
void setThisThreadDescriptor(ThreadStartupData& startupData) { thisThreadDescriptor.threadID = startupData.threadCommID; }

thread_local NodeBase* thisThreadNode = nullptr;

size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count )
{
	return threadQueues[thisThreadDescriptor.threadID.slotId].queue.pop_front( messages, count );
}

size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count, uint64_t timeout )
{
	return threadQueues[thisThreadDescriptor.threadID.slotId].queue.pop_front( messages, count, timeout );
}

/*uintptr_t initInterThreadCommSystemAndGetReadHandleForMainThread()
{
	return interThreadCommInitializer.init();
}*/

void preinitThreadStartupData( ThreadStartupData& startupData )
{
//	InterThreadCommPair commPair = interThreadCommInitializer.generateHandlePair();
	for ( size_t slotIdx = 1; slotIdx < MAX_THREADS; ++slotIdx )
	{
//		auto ret = threadQueues[slotIdx].acquireForReuse( commPair.writeHandle );
		auto ret = threadQueues[slotIdx].acquireForReuse( 0 );
		if ( ret.first )
		{
			startupData.threadCommID.reincarnation = ret.second;
			startupData.threadCommID.slotId = slotIdx;
			break;
		}
	}
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, startupData.threadCommID.slotId != ThreadID::InvalidSlotID ); 
//	startupData.readHandle = commPair.readHandle;
	startupData.defaultLog = nodecpp::logging_impl::currentLog;/**/
}

void sendInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType, ThreadID targetThreadId )
{
	//NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "not implemented" ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, targetThreadId.slotId < MAX_THREADS, "{} vs. {}", targetThreadId.slotId, MAX_THREADS );
	auto writingMeans = threadQueues[ targetThreadId.slotId ].getWriteHandleAndReincarnation();
	if ( !writingMeans.first )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "getWriteHandleAndReincarnation() for ID {} failed; handling not implemented", targetThreadId.slotId );
		// TODO: process error instead
	}
	uintptr_t writeHandle = writingMeans.second.second;
	uint64_t reincarnation = writingMeans.second.first;
	
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == targetThreadId.reincarnation, "for idx = {}: {} vs. {}", targetThreadId.slotId, reincarnation, targetThreadId.reincarnation ); 
	threadQueues[ targetThreadId.slotId ].queue.push_back( InterThreadMsg( std::move( msg ), msgType, thisThreadDescriptor.threadID, targetThreadId ) );
	/*// write a byte to writeHandle
	uint8_t singleByte = 0x1;
	size_t sentSize = 0;
	auto ret = nodecpp::internal_usage_only::internal_send_packet( &singleByte, 1, writeHandle, sentSize );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret == COMMLAYER_RET_OK ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sentSize == 1 ); */
}

#ifndef NODECPP_NO_COROUTINES
nodecpp::handler_ret_type nodecpp::a_timeout(uint32_t ms)
{
	co_await ::a_timeout_impl( ms );
	co_return;
}
nodecpp::handler_ret_type nodecpp::a_sleep(uint32_t ms)
{
	co_await ::a_timeout_impl( ms );
	co_return;
}
#endif

#endif // NODECPP_USE_Q_BASED_INFRA
