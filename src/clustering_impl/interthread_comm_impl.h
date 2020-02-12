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

#ifndef INTERTHREAD_COMM_IMPL_H
#define INTERTHREAD_COMM_IMPL_H

#include <internal_msg.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "interthread_comm.h"
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>
#include "../tcp_socket/tcp_socket.h"


struct InterThreadCommPair
{
	int readHandle;
	int writeHandle;
};

class InterThreadCommInitializer
{
	struct RequestData
	{
		size_t requestId;
		size_t userDefID;
		int writeHandle;
	};

	bool isInitialized_ = false;
	InterThreadCommPair commHandles;
	uint16_t myServerPort;
	uint16_t srcPort;
	size_t requestIdBase = 0;
	nodecpp::stdmap<uint16_t, RequestData> requests;

public:
	InterThreadCommInitializer() {}

	void startInitialization();

public:
	bool isInitialized() { return isInitialized_; }
	void generateHandlePair( size_t userDefID );

	// server
	nodecpp::handler_ret_type onConnectionTempServer( int sock, nodecpp::net::Address sourceAddr );
};

#endif // INTERTHREAD_COMM_IMPL_H