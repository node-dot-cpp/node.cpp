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

#ifdef NODECPP_ENABLE_CLUSTERING

#include "listener_thread_impl.h"
#include "../clustering_impl/clustering_impl.h"

struct ListenerThreadDescriptor
{
	ThreadID threadID;
};

class Listeners
{
	ListenerThreadDescriptor listeners[MAX_THREADS];
	size_t maxUsed = 0;

public:
	size_t add( ThreadID id ) {
		for ( size_t i=0; i< maxUsed; ++i )
			if ( listeners[i].threadID.slotId == ThreadID::InvalidSlotID )
			{
				listeners[i].threadID = id;
				return i;
			}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, maxUsed < MAX_THREADS, "{} vs. {}", maxUsed, MAX_THREADS ); 
		listeners[maxUsed].threadID = id;
		return maxUsed++;
	}
	bool remove( ThreadID id ) {
		for ( size_t i=0; i< maxUsed; ++i )
			if ( listeners[i].threadID.slotId == id.slotId && listeners[i].threadID.reincarnation == id.reincarnation )
			{
				listeners[i].threadID = ThreadID();
				if ( i + 1 == maxUsed )
					--maxUsed;
				return true;
			}
		return false;
	}
	void notifyAll( nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType )
	{
		for ( size_t i=0; i< maxUsed; ++i )
			if ( listeners[i].threadID.slotId == ThreadID::InvalidSlotID )
				sendInterThreadMsg( std::move( msg ), msgType, listeners[i].threadID );
	}
};
static Listeners listeners;

class WorkerLoad
{
private:
	std::mutex mx;
	size_t usedSlotCnt;
	struct Worker
	{
		size_t load = 0;
		ThreadID id;
	};
	static constexpr size_t workerMax = MAX_THREADS; // to awoid dyn allocation
	Worker workers[MAX_THREADS];
	size_t totalLoadCtr = 0;

	size_t current = 0;

public:
	size_t addWorker( ThreadID id_ )
	{
		size_t assignedIdx;
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, usedSlotCnt < MAX_THREADS, "{} vs. {}", usedSlotCnt, MAX_THREADS );
		workers[usedSlotCnt].id = id_;
		assignedIdx = usedSlotCnt;
		++usedSlotCnt;
		lock.unlock();
		return assignedIdx;
	}

	void incrementLoadCtr( size_t idx )
	{
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < usedSlotCnt, "{} vs. {}", idx, usedSlotCnt ); 
		++( workers[ idx ].load );
		++totalLoadCtr;
	}

	void decrementLoadCtr( size_t idx )
	{
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < usedSlotCnt, "{} vs. {}", idx, usedSlotCnt ); 
		++( workers[ idx ].load );
		++totalLoadCtr;
	}

	ThreadID getCandidate() // updates 'current'; current is always set to a valid value (if at all possible)
	{
		std::unique_lock<std::mutex> lock(mx);
//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, usedSlotCnt != 0 ); 
		if ( usedSlotCnt )
		{
			size_t comparisonBase = ((totalLoadCtr << 28) + (totalLoadCtr << 32)) / usedSlotCnt;
			size_t presentCurrent = current++;
			do
			{
				if ( current != usedSlotCnt )
				{
					if ( (workers[current].load << 32) < comparisonBase )
						return workers[current].id;
					else
						++current;
				}
				else
					current = 0;
			}
			while ( current != presentCurrent );
		}
		else
			return ThreadID();
	}
};
static WorkerLoad workerLoad;

void addWorkerEntryForLoadTracking( ThreadID id ) { workerLoad.addWorker( id ); }
void incrementWorkerLoadCtr( size_t idx ) { workerLoad.incrementLoadCtr( idx ); }
void decrementWorkerLoadCtr( size_t idx ) { workerLoad.decrementLoadCtr( idx ); }
ThreadID getLeastLoadedWorker() { return workerLoad.getCandidate(); }

thread_local ListenerThreadWorker listenerThreadWorker;
thread_local NetServerManagerForListenerThread netServerManagerBase;

void listenerThreadMain( void* pdata )
{
	ThreadStartupData* sd = reinterpret_cast<ThreadStartupData*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sd->threadCommID.slotId != 0 ); 
	ThreadStartupData startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	setThisThreadDescriptor( startupData );
#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::logging_impl::currentLog = startupData.defaultLog;
	nodecpp::logging_impl::instanceId = startupData.threadCommID.slotId;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"starting Listener thread with threadID = {}", startupData.threadCommID.slotId );
	listenerThreadWorker.preinit();
	netServerManagerBase.runLoop( startupData.readHandle );
}

void createListenerThread()
{
	// note: startup data must be allocated using std allocator (reason: freeing memory will happen at a new thread)
	ThreadStartupData* startupData = nodecpp::stdalloc<ThreadStartupData>(1);
	preinitThreadStartupData( *startupData );
	startupData->IdWithinGroup = listeners.add( startupData->threadCommID );
	size_t threadIdx = startupData->threadCommID.slotId;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {} and listenerID = {}...", threadIdx, startupData->IdWithinGroup );
	std::thread t1( listenerThreadMain, (void*)(startupData) );
	// startupData is no longer valid
	startupData = nullptr;
	t1.detach();
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
}

void ListenerThreadWorker::processInterthreadRequest( ThreadID requestingThreadId, InterThreadMsgType msgType, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
{
	size_t sz = riter.availableSize();
	switch ( msgType )
	{
		case InterThreadMsgType::RequestToListeningThread:
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( RequestToListenerThread ) <= sz, "{} vs. {}", sizeof( RequestToListenerThread ), sz ); 
			const RequestToListenerThread* msg = reinterpret_cast<const RequestToListenerThread*>( riter.read( sizeof( RequestToListenerThread ) ) );
			switch ( msg->type )
			{
				case RequestToListenerThread::Type::AddServerSocket:
					listenerThreadWorker.createAgentServerWithExistingSocket( msg->socket, msg->ip, msg->port.getHost() );
					break;
			}
			// TODO: ...
			break;
		}
		case InterThreadMsgType::ServerError:
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerErrorEvMsg ) <= sz, "{} vs. {}", sizeof( ServerErrorEvMsg ), sz ); 
			const ServerErrorEvMsg* msg = reinterpret_cast<const ServerErrorEvMsg*>( riter.read( sizeof( ServerErrorEvMsg ) ) );
			// TODO: ...
			break;
		}
		case InterThreadMsgType::ServerCloseRequest:
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerCloseRequest ) <= sz, "{} vs. {}", sizeof( ServerCloseRequest ), sz ); 
			const ServerCloseRequest* msg = reinterpret_cast<const ServerCloseRequest*>( riter.read( sizeof( ServerCloseRequest ) ) );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(msgType), requestingThreadId.slotId, msg->entryIdx );
			// TODO: ...
			break;
		}
		default:
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(msgType) ); 
			break;
	}
}

void ListenerThreadWorker::AgentServer::addServerSocketAndStartListening( SOCKET socket) { 
	nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> myPtr = myThis.getSoftPtr<ListenerThreadWorker::AgentServer>(this);
	netServerManagerBase.appAddAgentServerSocketAndStartListening(myPtr, socket); 
}
void ListenerThreadWorker::AgentServer::registerServer() { 
	nodecpp::safememory::soft_ptr<ListenerThreadWorker::AgentServer> myPtr = myThis.getSoftPtr<ListenerThreadWorker::AgentServer>(this);
	netServerManagerBase.appAddAgentServer(myPtr); 
}
void ListenerThreadWorker::AgentServer::listen(uint16_t port, nodecpp::Ip4 ip, int backlog)
{
	netServerManagerBase.appListen(dataForCommandProcessing, ip, port, backlog);
}
void ListenerThreadWorker::AgentServer::ref() { netServerManagerBase.appRef(dataForCommandProcessing.index); }
void ListenerThreadWorker::AgentServer::unref() { netServerManagerBase.appUnref(dataForCommandProcessing.index); }
void ListenerThreadWorker::AgentServer::reportBeingDestructed() { netServerManagerBase.appReportBeingDestructed(dataForCommandProcessing); }

void ListenerThreadWorker::AgentServer::close()
{
	netServerManagerBase.appClose(dataForCommandProcessing);
	dataForCommandProcessing.state = DataForCommandProcessing::State::BeingClosed;
}

#endif // NODECPP_ENABLE_CLUSTERING