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

#include "../include/infrastructure/q_based_infrastructure.h"

#include "../include/nodecpp/nls.h"

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

//thread_local NLS threadLocalData;
	thread_local void* nodeLocalData;
} // namespace nodecpp

thread_local TimeoutManager* timeoutManager;


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
		return timeoutManager->appSetTimeout(cb, ms, infraGetCurrentTime());
	}

#ifndef NODECPP_NO_COROUTINES
	nodecpp::Timeout setTimeoutForAction(awaitable_handle_t h, int32_t ms)
	{
		return timeoutManager->appSetTimeoutForAction(h, ms, infraGetCurrentTime());
	}
#endif // NODECPP_NO_COROUTINES

	void refreshTimeout(Timeout& to)
	{
		return timeoutManager->appRefresh(to.getId(), infraGetCurrentTime());
	}

	void clearTimeout(const Timeout& to)
	{
		return timeoutManager->appClearTimeout(to.getId());
	}

	Timeout::~Timeout()
	{
		if ( id != 0 )
			timeoutManager->appTimeoutDestructor(id);
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

InterThreadCommData threadQueues[MAX_THREADS];

class PostmanToInterthreadQueue : public InterThreadMessagePostmanBase
{
public: 
	PostmanToInterthreadQueue() {}
	void postMessage( InterThreadMsg&& msg ) override
	{
		auto slotId = msg.targetThreadID.slotId;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, msg.targetThreadID.slotId <= MAX_THREADS, "indeed: {} vs. {}", msg.targetThreadID.slotId, MAX_THREADS ); 
		threadQueues[ slotId ].queue.push_back( std::move( msg ) );
	}
};
static PostmanToInterthreadQueue qPostman; // does not support distinguishing nodes
InterThreadMessagePostmanBase* useQueuePostman() {return &qPostman; }


static InterThreadCommInitializer interThreadCommInitializer;

struct ThreadDescriptor
{
	ThreadID threadID;
};
static thread_local ThreadDescriptor thisThreadDescriptor;
void setThisThreadDescriptor(ThreadStartupData& startupData) { thisThreadDescriptor.threadID = startupData.threadCommID; }

//thread_local NodeBase* thisThreadNode = nullptr;

size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count )
{
	return threadQueues[thisThreadDescriptor.threadID.slotId].queue.pop_front( messages, count );
}

size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count, uint64_t timeout )
{
	return threadQueues[thisThreadDescriptor.threadID.slotId].queue.pop_front( messages, count, timeout );
}

#ifdef NODECPP_USE_GMQUEUE
globalmq::marshalling::GMQueue<GMQueueStatePublisherSubscriberTypeInfo> gmqueue;

globalmq::marshalling::GMQTransportBase<GMQueueStatePublisherSubscriberTypeInfo>* getTransport() {
	return ((nodecpp::NLS*)(nodecpp::nodeLocalData))->transport;
}
#endif


void preinitThreadStartupData( ThreadStartupData& startupData, InterThreadMessagePostmanBase* postman )
{
	for ( size_t slotIdx = 1; slotIdx < MAX_THREADS; ++slotIdx )
	{
		auto ret = threadQueues[slotIdx].acquireForReuse( postman );
		if ( ret.first )
		{
			startupData.threadCommID.reincarnation = ret.second;
			startupData.threadCommID.slotId = slotIdx;
			break;
		}
	}
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, startupData.threadCommID.slotId != ThreadID::InvalidSlotID ); 
	startupData.defaultLog = nodecpp::logging_impl::currentLog;
}

void postInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType, NodeAddress targetThreadId )
{
	//NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "not implemented" ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, targetThreadId.slotId < MAX_THREADS, "{} vs. {}", targetThreadId.slotId, MAX_THREADS );
	auto writingMeans = threadQueues[ targetThreadId.slotId ].getPostmanAndReincarnation();
	if ( !writingMeans.first )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "getWriteHandleAndReincarnation() for ID {} failed; handling not implemented", targetThreadId.slotId );
		// TODO: process error instead
	}
	uint64_t reincarnation = writingMeans.second.second;
	InterThreadMessagePostmanBase* postman = writingMeans.second.first;
	
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == targetThreadId.reincarnation, "for idx = {}: {} vs. {}", targetThreadId.slotId, reincarnation, targetThreadId.reincarnation ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, postman != nullptr ); 
	postman->postMessage( InterThreadMsg( std::move( msg ), msgType, NodeAddress(thisThreadDescriptor.threadID, 0 /*TODO: we need means to supply nodeID, if applicable*/), targetThreadId ) );
}

void postGmqMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, size_t recipientID, size_t slotId )
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, slotId < MAX_THREADS, "{} vs. {}", slotId, MAX_THREADS );
	threadQueues[ slotId ].queue.push_back( InterThreadMsg( std::move( msg ), InterThreadMsgType::GlobalMQ, NodeAddress(thisThreadDescriptor.threadID, 0), NodeAddress(), recipientID ) );
}

void postInfrastructuralMsg(nodecpp::Message&& msg, NodeAddress threadId )
{
	postInterThreadMsg( std::move( msg ), InterThreadMsgType::Infrastructural, threadId );
}

void internalPostlGlobalMQ(nodecpp::Message&& msg, NodeAddress threadId )
{
	postInterThreadMsg( std::move( msg ), InterThreadMsgType::GlobalMQ, threadId );
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
