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

#include "tcp_socket/tcp_socket.h"

using EvVector = std::vector<std::function<void()>>;



class Infrastructure
{
	NetSocketManager netSocket;
	NetServerManager netServer;
	EvVector inmediateQueue;
public:
	bool running = true;
	uint64_t nextTimeoutAt = 0;
	NetSocketManager& getNetSocket() { return netSocket; }
	NetServerManager& getNetServer() { return netServer; }
	void setInmediate(std::function<void()> cb) { inmediateQueue.push_back(std::move(cb)); }
	EvVector& getInmediates() { return inmediateQueue; }
};

extern thread_local Infrastructure infra;

inline
Infrastructure& getInfra() { return infra; }

static constexpr uint64_t TimeOutNever = static_cast<uint64_t>(-1);

//using EvQueue = std::vector<std::function<void()>>;


bool pollPhase(uint64_t nextTimeoutAt, EvQueue& evs);
void pendingCloseEvents(EvQueue& evs);

void fireEvents(EvVector& evs);

#endif //INFRASTRUCTURE_H
