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

#if (defined NODECPP_ENABLE_CLUSTERING) || (defined NODECPP_USE_Q_BASED_INFRA)

#include <internal_msg.h>

#define MAX_THREADS 128


struct ThreadID
{
	static constexpr size_t InvalidSlotID = (size_t)(-1);
	static constexpr size_t InvalidReincarnation = (uint64_t)(-1);
	size_t slotId = InvalidSlotID;
	uint64_t reincarnation = InvalidReincarnation;
};

struct NodeAddress : public ThreadID
{
	NodeAddress() {}
	NodeAddress( ThreadID other ) : ThreadID( other ) {}
};

enum class InterThreadMsgType { UserDefined, ThreadStarted, ThreadTerminate, ServerListening, ConnAccepted, ServerError, ServerCloseRequest, ServerClosedNotification, RequestToListeningThread, Infrastructural, Undefined };

extern thread_local size_t workerIdxInLoadCollector;

struct InterThreadMsg;
class InterThreadMsgPtr
{
	friend struct InterThreadMsg;
	nodecpp::platform::internal_msg::InternalMsg* ptr;
	InterThreadMsgPtr(nodecpp::platform::internal_msg::InternalMsg* val) { ptr = val; }
public:
	operator uintptr_t () const { return (uintptr_t)(ptr); }
	InterThreadMsgPtr(uintptr_t val) { ptr = (nodecpp::platform::internal_msg::InternalMsg*)val; }
};

struct InterThreadMsg
{
	NodeAddress sourceThreadID;
	NodeAddress targetThreadID;
	InterThreadMsgType msgType = InterThreadMsgType::Undefined;
	nodecpp::platform::internal_msg::InternalMsg msg;

	InterThreadMsg() {}
	InterThreadMsg( InterThreadMsgPtr ptr )
	{
		msg.restoreFromPointer( ptr.ptr );
		msg.appReadData( &sourceThreadID, 0, sizeof( sourceThreadID ) );
		msg.appReadData( &targetThreadID, sizeof( sourceThreadID ), sizeof( targetThreadID ) );
		msg.appReadData( &msgType, sizeof( sourceThreadID ) + sizeof( targetThreadID ), sizeof( msgType ) );
	}
	InterThreadMsg( nodecpp::platform::internal_msg::InternalMsg&& msg_, InterThreadMsgType msgType_, NodeAddress sourceThreadID_, NodeAddress targetThreadID_ ) : 
		sourceThreadID( sourceThreadID_ ), targetThreadID( targetThreadID_ ), msgType( msgType_ ), msg( std::move(msg_) )  {}
	InterThreadMsg( const InterThreadMsg& ) = delete;
	InterThreadMsg& operator = ( const InterThreadMsg& ) = delete;
	InterThreadMsg( InterThreadMsg&& other ) = default; 
	InterThreadMsg& operator = ( InterThreadMsg&& other ) = default;

	InterThreadMsgPtr convertToPointer()
	{
		msg.appWriteData( &sourceThreadID, 0, sizeof( sourceThreadID ) );
		msg.appWriteData( &targetThreadID, sizeof( sourceThreadID ), sizeof( targetThreadID ) );
		msg.appWriteData( &msgType, sizeof( sourceThreadID ) + sizeof( targetThreadID ), sizeof( msgType ) );
		return InterThreadMsgPtr( msg.convertToPointer() );
	}

//	void restoreFromPointer( InterThreadMsgPtr ptr )
};

uintptr_t initInterThreadCommSystemAndGetReadHandleForMainThread();
void postInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType, NodeAddress threadId );
#include "../../include/nodecpp/common_structs.h"
void postInfrastructuralMsg(nodecpp::Message&& msg, InterThreadMsgType msgType, NodeAddress threadId );
void setThisThreadDescriptor(ThreadStartupData& startupData);
size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count );
size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count, uint64_t timeout );

struct ListenerThreadDescriptor
{
	ThreadID threadID;
};
std::pair<const ListenerThreadDescriptor*, size_t> getListeners();

size_t addWorkerEntryForLoadTracking( ThreadID id );
void incrementWorkerLoadCtr( size_t idx );
void decrementWorkerLoadCtr( size_t idx );
ThreadID getLeastLoadedWorkerAndIncrementLoad();
void createListenerThread();

#endif // NODECPP_ENABLE_CLUSTERING
#endif // INTERTHREAD_COMM_H