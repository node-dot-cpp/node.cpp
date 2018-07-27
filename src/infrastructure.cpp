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

#include "infrastructure.h"

thread_local Infrastructure infra;

uint64_t infraGetCurrentTime()
{
#ifdef _MSC_VER
	return GetTickCount64() * 1000; // mks
#else
#error infraGetCurrentTime() is not defined for this architecture
#endif
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
	entry.lastSchedule = infraGetCurrentTime();
	entry.delay = ms * 1000;

	entry.nextTimeout = entry.lastSchedule + entry.delay;
	
	entry.active = true;
	nextTimeouts.insert(std::make_pair(entry.nextTimeout, id));

	timers.insert(std::make_pair(id, std::move(entry)));

	return Timeout(id);
}

void TimeoutManager::appClearTimeout(const nodecpp::Timeout& to)
{
	uint64_t id = to.getId();

	auto it = timers.find(id);
	if (it != timers.end())
	{
		if (it->second.active)
		{
			auto it2 = nextTimeouts.equal_range(it->second.nextTimeout);
			bool found = false;
			while (it2.first != it2.second)
			{
				if (it2.first->second == id)
				{
					nextTimeouts.erase(it2.first);
					it->second.active = false;
					break;
				}
				++(it2.first);
			}
		}

		//if it was active, we must have deactivated it
		NODECPP_ASSERT(it->second.active == false);
	}
}

void TimeoutManager::neutralTimeoutDestructor(uint64_t id)
{
	auto it = timers.find(id);
	if (it != timers.end())
	{
		it->second.handleDestroyed = true;

		if(it->second.active == false)
			timers.erase(it);
	}
	else
		NODECPP_TRACE("timer {} not found", id);
}

void TimeoutManager::infraTimeoutEvents(uint64_t now, EvQueue& evs)
{
	auto itBegin = nextTimeouts.begin();
	auto itEnd = nextTimeouts.upper_bound(now);
	auto it = itBegin;
	while (it != itEnd)
	{
		auto it2 = timers.find(it->second);

		NODECPP_ASSERT(it2 != timers.end());
		NODECPP_ASSERT(it2->second.active);
		
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


bool Infrastructure::pollPhase(bool refed, uint64_t nextTimeoutAt, uint64_t now, EvQueue& evs)
{
	size_t fds_sz = NetSocketManager::MAX_SOCKETS + NetServerManager::MAX_SOCKETS;
	std::unique_ptr<pollfd[]> fds(new pollfd[fds_sz]);

	
	pollfd* fds_begin = fds.get();
	pollfd* fds_end = fds_begin + NetSocketManager::MAX_SOCKETS;
	bool refedSocket = netSocket.infraSetPollFdSet(fds_begin, fds_end);

	fds_begin = fds_end;
	fds_end += NetServerManager::MAX_SOCKETS;
	bool refedServer = netServer.infraSetPollFdSet(fds_begin, fds_end);

	if (refed == false && refedSocket == false && refedServer == false)
		return false; //stop here

	int timeoutToUse = getPollTimeout(nextTimeoutAt, now);

#ifdef _MSC_VER
	int retval = WSAPoll(fds.get(), static_cast<ULONG>(fds_sz), timeoutToUse);
#else
	int retval = poll(fds.get(), fds_sz, timeoutToUse);
#endif


	if (retval < 0)
	{
#ifdef _MSC_VER
		int error = WSAGetLastError();
		//		if ( error == WSAEWOULDBLOCK )
		NODECPP_TRACE("error {}", error);
#else
		perror("select()");
		//		int error = errno;
		//		if ( error == EAGAIN || error == EWOULDBLOCK )
#endif
		/*        return WAIT_RESULTED_IN_TIMEOUT;*/
		NODECPP_ASSERT(false);
		NODECPP_TRACE("COMMLAYER_RET_FAILED");
		return false;
	}
	else if (retval == 0)
	{
		//timeout, just return with empty queue
		return true; 
	}
	else //if(retval)
	{
		fds_begin = fds.get();
		fds_end = fds_begin + NetSocketManager::MAX_SOCKETS;
		netSocket.infraCheckPollFdSet(fds_begin, fds_end, evs);

		fds_begin = fds_end;
		fds_end += NetServerManager::MAX_SOCKETS;
		netServer.infraCheckPollFdSet(fds_begin, fds_end, evs);

		//if (queue.empty())
		//{
		//	NODECPP_TRACE("No event generated from poll wake up (non timeout)");
		//	for (size_t i = 0; i != fds_sz; ++i)
		//	{
		//		if (fds[i].fd >= 0 && fds[i].revents != 0)
		//		{
		//			NODECPP_TRACE("At id {}, socket {}, revent {:x}", i, fds[i].fd, fds[i].revents);
		//		}
		//	}
		//}
		return true;
	}
}

void Infrastructure::runLoop()
{
	NODECPP_ASSERT(isNetInitialized());

	while (running)
	{

		EvQueue queue;
		netServer.infraGetPendingEvents(queue);
		queue.emit();

		uint64_t now = infraGetCurrentTime();
		timeout.infraTimeoutEvents(now, queue);
		queue.emit();

		now = infraGetCurrentTime();
		bool refed = pollPhase(refedTimeout(), nextTimeout(), now, queue);
		if(!refed)
			return;

		queue.emit();
		emitInmediates();

		netSocket.infraGetCloseEvent(queue);
		netServer.infraGetCloseEvents(queue);
		queue.emit();

		netSocket.infraClearStores();
		netServer.infraClearStores();
	}
}
