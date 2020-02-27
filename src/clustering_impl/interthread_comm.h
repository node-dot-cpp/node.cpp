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

#ifndef INTERTHREAD_COMM_H
#define INTERTHREAD_COMM_H

#include <internal_msg.h>

#define MAX_THREADS 128


struct ThreadID
{
	static constexpr size_t InvalidSlotID = (size_t)(-1);
	static constexpr size_t InvalidReincarnation = (uint64_t)(-1);
	size_t slotId = InvalidSlotID;
	uint64_t reincarnation = InvalidReincarnation;
};

enum class InterThreadMsgType { UserDefined, ThreadStarted, ThreadTerminate, ServerListening, ConnAccepted, ServerError, ServerCloseRequest, ServerClosedNotification, RequestToListeningThread, Undefined };

struct InterThreadMsg
{
	ThreadID sourceThreadID;
	ThreadID targetThreadID;
	InterThreadMsgType msgType = InterThreadMsgType::Undefined;
	nodecpp::platform::internal_msg::InternalMsg msg;

	InterThreadMsg() {}
	InterThreadMsg( nodecpp::platform::internal_msg::InternalMsg&& msg_, InterThreadMsgType msgType_, ThreadID sourceThreadID_, ThreadID targetThreadID_ ) : 
		sourceThreadID( sourceThreadID_ ), targetThreadID( targetThreadID_ ), msgType( msgType_ ), msg( std::move(msg_) )  {}
	InterThreadMsg( const InterThreadMsg& ) = delete;
	InterThreadMsg& operator = ( const InterThreadMsg& ) = delete;
	InterThreadMsg( InterThreadMsg&& other ) = default; 
	InterThreadMsg& operator = ( InterThreadMsg&& other ) = default;
};

uintptr_t initInterThreadCommSystemAndGetReadHandleForMainThread();
void sendInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType, ThreadID threadId );
void setThisThreadDescriptor(ThreadStartupData& startupData);
size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count );

void addWorkerEntryForLoadTracking( ThreadID id );
void incrementWorkerLoadCtr( size_t idx );
void decrementWorkerLoadCtr( size_t idx );
ThreadID getLeastLoadedWorker();
void createListenerThread();

#endif // INTERTHREAD_COMM_H