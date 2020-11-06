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
	/*void notifyAll( nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType )
	{
		for ( size_t i=0; i< maxUsed; ++i )
			if ( listeners[i].threadID.slotId != ThreadID::InvalidSlotID )
				postInterThreadMsg( std::move( msg ), msgType, listeners[i].threadID );
	}*/
	std::pair<const ListenerThreadDescriptor*, size_t> getListeners()
	{
		return std::make_pair( listeners, maxUsed );
	}
};
static Listeners listeners;
std::pair<const ListenerThreadDescriptor*, size_t> getListeners() { return listeners.getListeners(); }

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

	void _validate()
	{
		/*size_t sum = 0;
		for ( size_t i=0; i<usedSlotCnt; ++i )
			sum += workers[i].load;
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, totalLoadCtr == sum, "{} vs. {}", totalLoadCtr, sum );*/
	}

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
_validate();
		return assignedIdx;
	}

	void decrementLoadCtr( size_t idx )
	{
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < usedSlotCnt, "{} vs. {}", idx, usedSlotCnt ); 
_validate();
		--( workers[ idx ].load );
		--totalLoadCtr;
_validate();
	}

	ThreadID getCandidateAndIncrementLoad() // updates 'current'; current is always set to a valid value (if at all possible)
	{
		std::unique_lock<std::mutex> lock(mx);
//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, usedSlotCnt != 0 ); 
_validate();
		if ( usedSlotCnt )
		{
			size_t comparisonBase = (totalLoadCtr << 32) + (totalLoadCtr << 28) / usedSlotCnt + 1;
			size_t presentCurrent = current;
			++current;
			for ( ; current < usedSlotCnt; ++current )
				if ( (workers[current].load << 32) < comparisonBase )
				{
					++( workers[current].load );
					++totalLoadCtr;
_validate();
					return workers[current].id;
				}
			for ( current=0; current <= presentCurrent; ++current )
				if ( (workers[current].load << 32) < comparisonBase )
				{
					++( workers[current].load );
					++totalLoadCtr;
_validate();
					return workers[current].id;
				}
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "failed to find a candidate out of {} used slots", usedSlotCnt ); 
			return ThreadID();
		}
		else
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "failed to find a candidate: no used slots" ); 
			return ThreadID();
		}
	}
};
static WorkerLoad workerLoad;

size_t addWorkerEntryForLoadTracking( ThreadID id ) { return workerLoad.addWorker( id ); }
void decrementWorkerLoadCtr( size_t idx ) { workerLoad.decrementLoadCtr( idx ); }
ThreadID getLeastLoadedWorkerAndIncrementLoad() { return workerLoad.getCandidateAndIncrementLoad(); }

thread_local ListenerThreadWorker listenerThreadWorker;
thread_local NetServerManagerForListenerThread netServerManagerBaseForListenerThread;

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
	netServerManagerBaseForListenerThread.runLoop( startupData.readHandle );
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
					listenerThreadWorker.createAgentServerWithExistingSocket( msg->entryIndex, msg->socket, msg->ip, msg->port.getHost() );
					break;
				case RequestToListenerThread::Type::CreateSharedServerSocket:
					// for some internal details see
					// SO_REUSEPORT socket option
					// http://man7.org/linux/man-pages/man7/socket.7.html
					// https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ
					// https://docs.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse?redirectedfrom=MSDN
nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "processing CreateSharedServerSocket request (for thread id: {}), entryIndex = {:x}, ip = {} port: {}", requestingThreadId.slotId, msg->entryIndex, msg->ip.toStr(), msg->port.toStr() );
					listenerThreadWorker.createAgentServerWithSharedSocket( msg->entryIndex, msg->ip, msg->port.getHost(), msg->backlog );
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
		case InterThreadMsgType::Infrastructural:
		{
			// TODO: ...
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "InterThreadMsgType::Infrastructural request ({}) is unexpected at MasterSocket", (size_t)(msgType) );
			break;
		}
		default:
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(msgType) ); 
			break;
	}
}

void ListenerThreadWorker::AgentServer::addServerSocketAndStartListening( SOCKET socket) { 
	nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> myPtr = myThis.getSoftPtr<ListenerThreadWorker::AgentServer>(this);
	netServerManagerBaseForListenerThread.appAddAgentServerSocketAndStartListening(myPtr, socket); 
}

void ListenerThreadWorker::AgentServer::acquireSharedServerSocketAndStartListening(int backlog) { 
	nodecpp::soft_ptr<ListenerThreadWorker::AgentServer> myPtr = myThis.getSoftPtr<ListenerThreadWorker::AgentServer>(this);
	netServerManagerBaseForListenerThread.acquireSharedServerSocketAndStartListening(myPtr, backlog); 
}

#endif // NODECPP_ENABLE_CLUSTERING