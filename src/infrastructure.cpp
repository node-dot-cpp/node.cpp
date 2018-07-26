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

uint64_t getCurrentTime() { return 0; }

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


bool pollPhase(uint64_t nextTimeoutAt, EvQueue& evs)
{
	uint64_t now = getCurrentTime();
	nextTimeoutAt = now + 1000;

	size_t fds_sz = NetSocketManager::MAX_SOCKETS + NetServerManager::MAX_SOCKETS;
	std::unique_ptr<pollfd[]> fds(new pollfd[fds_sz]);

	
	pollfd* fds_begin = fds.get();
	pollfd* fds_end = fds_begin + NetSocketManager::MAX_SOCKETS;
	bool anySck = getInfra().getNetSocket().infraSetPollFdSet(fds_begin, fds_end);

	fds_begin = fds_end;
	fds_end += NetServerManager::MAX_SOCKETS;
	bool anySvr = getInfra().getNetServer().infraSetPollFdSet(fds_begin, fds_end);

	if (!anySck && !anySvr)
		return false;

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
		getInfra().getNetSocket().infraCheckPollFdSet(fds_begin, fds_end, evs);

		fds_begin = fds_end;
		fds_end += NetServerManager::MAX_SOCKETS;
		getInfra().getNetServer().infraCheckPollFdSet(fds_begin, fds_end, evs);

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

