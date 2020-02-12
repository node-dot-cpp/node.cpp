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


	// reading socket
	/*nodecpp::handler_ret_type onConnectReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket)
	{
		// comm system is ready TODO: report, ...
		{
			nodecpp::platform::internal_msg::InternalMsg msg;
			ThreadID threadId = {0, 0};
			auto reportStr = nodecpp::format( "Thread {} has almost initialized interthread comm system ...", threadIdx );
			msg.append( const_cast<char*>(reportStr.c_str()), reportStr.size() + 1 ); // TODO: update Foundation and remove cast ASAP!!!
			sendInterThreadMsg( std::move( msg ), 17, threadId );
		}
		CO_RETURN;
	}*/

	/*nodecpp::handler_ret_type onDataReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket, Buffer& buffer)
	{
		// TODO: processMessage(s)
		MsgQueue& myQueue = threadQueues[threadIdx].queue;
		for ( size_t i=0; i<buffer.size(); ++i )
		{
			auto popRes = std::move( myQueue.pop_front() );
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, popRes.first ); // unless we've already killed it
			InterThreadMsg& msg = popRes.second;
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, msg.reincarnation == reincarnation, "{} vs. {}", msg.reincarnation, reincarnation ); // unless we've already killed it
			// TODO: process message
			auto readit = msg.msg.getReadIter();
			size_t sz = readit.availableSize();
			const char* text = reinterpret_cast<const char*>( readit.read(sz) );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "msg = \"{}\"", text );
		}

		CO_RETURN;
	}

	nodecpp::handler_ret_type onEndReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket)
	{
		socket->end();
		CO_RETURN;
	}

	nodecpp::handler_ret_type onErrorReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket, Error& e)
	{
		socket->end();
		CO_RETURN;
	}*/

	// published socket
	/*nodecpp::handler_ret_type onConnectPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket)
	{
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, threadIdx < MAX_THREADS, "{} vs. {}", threadIdx, MAX_THREADS ); // unless we've already killed it
		threadQueues[threadIdx].sock = socket->dataForCommandProcessing.osSocket;
		{
			nodecpp::platform::internal_msg::InternalMsg msg;
			ThreadID threadId = {0, 0};
			auto reportStr = nodecpp::format( "Thread {} has initialized interthread comm system; socket = {}", threadIdx, socket->dataForCommandProcessing.osSocket );
			msg.append( const_cast<char*>(reportStr.c_str()), reportStr.size() + 1 ); // TODO: update Foundation and remove cast ASAP!!!
			sendInterThreadMsg( std::move( msg ), 17, threadId );
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type onEndPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket)
	{
		// TODO: handling (consider both cases: "initiated by our side" and "errors")
		socket->end();
		CO_RETURN;
	}

	nodecpp::handler_ret_type onErrorPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket, Error& e)
	{
		// TODO: handling (consider both cases: "initiated by our side" and "errors")
		socket->end();
		CO_RETURN;
	}*/
};

#endif // INTERTHREAD_COMM_IMPL_H