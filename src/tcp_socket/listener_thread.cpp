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

thread_local ListenerThreadWorker listenerThreadWorker;
thread_local NetServerManagerForListenerThread netServerManagerBase;

struct ThreadDescriptor
{
	ThreadID threadID;
};
static thread_local ThreadDescriptor thisThreadDescriptor;

void listenerThreadMain( void* pdata )
{
	ThreadStartupData* sd = reinterpret_cast<ThreadStartupData*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sd->assignedThreadID != 0 ); 
	ThreadStartupData startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	nodecpp::logging_impl::currentLog = startupData.defaultLog;
	nodecpp::logging_impl::instanceId = startupData.assignedThreadID;
	thisThreadDescriptor.threadID.slotId = startupData.assignedThreadID;
	thisThreadDescriptor.threadID.reincarnation = startupData.reincarnation;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"starting Listener thread with threadID = {}", startupData.assignedThreadID );
	listenerThreadWorker.preinit();
	netServerManagerBase.runLoop( startupData.readHandle );
}

void ListenerThreadWorker::processInterthreadRequest( ThreadID requestingThreadId, InterThreadMsgType msgType, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
{
	size_t sz = riter.availableSize();
	switch ( msgType )
	{
		case InterThreadMsgType::ListeningThreadAddServer:
		{
			// TODO: ...
			break;
		}
		case InterThreadMsgType::ServerError:
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerErrorEvMsg ) <= sz, "{} vs. {}", sizeof( ServerErrorEvMsg ), sz ); 
			const ServerErrorEvMsg* msg = reinterpret_cast<const ServerErrorEvMsg*>( riter.read( sizeof( ServerErrorEvMsg ) ) );
			/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof(size_t) <= sz ); 
			size_t serverIdx = mh.entryIdx;
//				netServerManagerBase->addAcceptedSocket( serverIdx, (SOCKET)socket, remoteIp, remotePort );*/
			break;
		}
		case InterThreadMsgType::ServerCloseRequest:
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerCloseRequest ) <= sz, "{} vs. {}", sizeof( ServerCloseRequest ), sz ); 
			const ServerCloseRequest* msg = reinterpret_cast<const ServerCloseRequest*>( riter.read( sizeof( ServerCloseRequest ) ) );
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MasterSocket: processing ServerCloseRequest({}) request (for thread id: {}), entryIndex = {:x}", (size_t)(msgType), requestingThreadId.slotId, msg->entryIdx );
//					nodecpp::safememory::soft_ptr<MasterSocket> me = myThis.getSoftPtr<MasterSocket>(this);
//				processRequestForServerCloseAtMaster( requestingThreadId, mh );
			break;
		}
		default:
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(msgType) ); 
			break;
	}
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