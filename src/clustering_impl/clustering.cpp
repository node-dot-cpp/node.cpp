/* -------------------------------------------------------------------------------
* Copyright (c) 2019, OLogN Technologies AG
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

#include "clustering_impl.h"
#include "../../include/nodecpp/cluster.h"
#include "../infrastructure.h"
#include "../tcp_socket/tcp_socket.h"
#include <thread>

#include "interthread_comm.h"
#include "interthread_comm_impl.h"

static InterThreadCommData threadQueues[MAX_THREADS];

static InterThreadCommInitializer interThreadCommInitializer;

struct ThreadDescriptor
{
	ThreadID threadID;
};
static thread_local ThreadDescriptor thisThreadDescriptor;

size_t popFrontFromThisThreadQueue( InterThreadMsg* messages, size_t count )
{
	return threadQueues[thisThreadDescriptor.threadID.slotId].queue.pop_front( messages, count );
}

uint64_t initThreadSlot( size_t threadIdx, uintptr_t writeHandle ) // returns reincarnation
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, threadIdx < MAX_THREADS, "{} vs. {}", threadIdx, MAX_THREADS ); 
	auto commPair = interThreadCommInitializer.generateHandlePair();
	return 0;
}

uintptr_t InterThreadCommInitializer::init()
{
	Ip4 ip4 = Ip4::parse( "127.0.01" );
	myServerSocket = acquireSocketAndLetInterThreadCommServerListening( ip4, myServerPort, 128 );
	auto commPair = interThreadCommInitializer.generateHandlePair();
	threadQueues[0].setWriteHandleForFirstUse( commPair.writeHandle );
	return commPair.readHandle;
}

InterThreadCommPair InterThreadCommInitializer::generateHandlePair()
{
	auto res = acquireAndConnectSocketForInterThreadComm( myServerSocket, "127.0.01", myServerPort );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, res.first.second == res.second.second ); 
	return InterThreadCommPair({res.first.first, res.second.first});
}

uintptr_t initInterThreadCommSystemAndGetReadHandleForMainThread()
{
	return interThreadCommInitializer.init();
}

void sendInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, InterThreadMsgType msgType, ThreadID targetThreadId )
{
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, targetThreadId.slotId < MAX_THREADS, "{} vs. {}", targetThreadId.slotId, MAX_THREADS );
	auto writingMeans = threadQueues[ targetThreadId.slotId ].getWriteHandleAndReincarnation();
	if ( writingMeans.first )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "not implemented" );
		// TODO: process error instead
	}
	uintptr_t writeHandle = writingMeans.second.second;
	uint64_t reincarnation = writingMeans.second.first;
	
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == targetThreadId.reincarnation ); 
	threadQueues[ targetThreadId.slotId ].queue.push_back( InterThreadMsg( std::move( msg ), msgType, thisThreadDescriptor.threadID, targetThreadId ) );
	// write a byte to writeHandle
	uint8_t singleByte = 0x1;
	size_t sentSize = 0;
	auto ret = nodecpp::internal_usage_only::internal_send_packet( &singleByte, 1, writeHandle, sentSize );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret == COMMLAYER_RET_OK ); 
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sentSize == 1 ); 
}



extern void workerThreadMain( void* pdata );

namespace nodecpp
{
	thread_local Cluster cluster;

	bool clusterIsMaster() { return cluster.isMaster(); }

	void preinitMasterThreadClusterObject()
	{
		cluster.preinitMaster();
		thisThreadDescriptor.threadID.slotId = 0;
		thisThreadDescriptor.threadID.reincarnation = 0;
		// TODO: check consistency of startupData.reincarnation with threadQueues[startupData.assignedThreadID].reincarnation
	}

	void preinitSlaveThreadClusterObject(ThreadStartupData& startupData)
	{
		cluster.preinitSlave( startupData.assignedThreadID );
		thisThreadDescriptor.threadID.slotId = startupData.assignedThreadID;
		thisThreadDescriptor.threadID.reincarnation = startupData.reincarnation;
		// TODO: check consistency of startupData.reincarnation with threadQueues[startupData.assignedThreadID].reincarnation
	}

	void postinitThreadClusterObject()
	{
		cluster.postinit();
	}
	
	Worker& Cluster::fork()
	{
		size_t internalID = workers_.size();
		Worker worker;
		worker.id_ = ++coreCtr; // TODO: assign an actual value
		// TODO: init new thread, fill worker
		// note: startup data must be allocated using std allocator (reason: freeing memory will happen at a new thread)
		ThreadStartupData* startupData = nodecpp::stdalloc<ThreadStartupData>(1);
		InterThreadCommPair commPair = interThreadCommInitializer.generateHandlePair();
		startupData->reincarnation = threadQueues[worker.id_].resetForReuse( commPair.writeHandle );
		startupData->readHandle = commPair.readHandle;
		startupData->assignedThreadID = worker.id_;
		startupData->defaultLog = nodecpp::logging_impl::currentLog;
		// run worker thread
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Worker thread with threadID = {}...", worker.id_ );
		std::thread t1( workerThreadMain, (void*)(startupData) );
		// startupData is no longer valid
		startupData = nullptr;
		t1.detach();
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Worker thread with threadID = {} completed at Master thread side", worker.id_ );
		workers_.push_back( std::move(worker) );
		return workers_[internalID];
	}
	
	void Worker::disconnect()
	{
		// TODO: ...
	}

	void Cluster::AgentServer::registerServer() { 
		nodecpp::safememory::soft_ptr<Cluster::AgentServer> myPtr = myThis.getSoftPtr<Cluster::AgentServer>(this);
		::registerAgentServer(myPtr); 
	}
	void Cluster::AgentServer::listen(uint16_t port, nodecpp::Ip4 ip, int backlog)
	{
		netServerManagerBase->appListen(dataForCommandProcessing, ip, port, backlog);
	}
	void Cluster::AgentServer::ref() { netServerManagerBase->appRef(dataForCommandProcessing); }
	void Cluster::AgentServer::unref() { netServerManagerBase->appUnref(dataForCommandProcessing); }
	void Cluster::AgentServer::reportBeingDestructed() { netServerManagerBase->appReportBeingDestructed(dataForCommandProcessing); }

	void Cluster::AgentServer::close()
	{
		netServerManagerBase->appClose(dataForCommandProcessing);
		dataForCommandProcessing.state = DataForCommandProcessing::State::BeingClosed;
	}


	nodecpp::handler_ret_type Cluster::SlaveProcessor::processResponse( ThreadID requestingThreadId, InterThreadMsgType msgType, nodecpp::platform::internal_msg::InternalMsg::ReadIter& riter )
	{
		size_t sz = riter.availableSize();
		switch ( msgType )
		{
			case InterThreadMsgType::ServerListening:
			{
				// TODO: ...
				break;
			}
			case InterThreadMsgType::ConnAccepted:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ConnAcceptedEvMsg ) <= sz, "{} vs. {}", sizeof( ConnAcceptedEvMsg ), sz ); 
				const ConnAcceptedEvMsg* msg = reinterpret_cast<const ConnAcceptedEvMsg*>( riter.read( sizeof( ConnAcceptedEvMsg ) ) );

				/*const uint8_t* buff = riter.read( mh.bodySize );
				size_t serverIdx = *reinterpret_cast<const size_t*>(buff);
				uint64_t socket = *reinterpret_cast<const uint64_t*>(buff + sizeof(serverIdx));
				Ip4 remoteIp = Ip4::fromNetwork( *reinterpret_cast<const uint32_t*>(buff + sizeof(serverIdx) + sizeof(socket)) );
				Port remotePort = Port::fromNetwork( *reinterpret_cast<const uint16_t*>(buff + sizeof(serverIdx) + sizeof(socket) + sizeof(uint32_t)) );*/
				netServerManagerBase->addAcceptedSocket( msg->serverIdx, (SOCKET)(msg->socket), msg->ip, msg->uport );
				break;
			}
			case InterThreadMsgType::ServerError:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerErrorEvMsg ) <= sz, "{} vs. {}", sizeof( ServerErrorEvMsg ), sz ); 
				const ServerErrorEvMsg* msg = reinterpret_cast<const ServerErrorEvMsg*>( riter.read( sizeof( ServerErrorEvMsg ) ) );
				/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof(size_t) <= sz ); 
				size_t serverIdx = mh.entryIdx;*/
//				netServerManagerBase->addAcceptedSocket( serverIdx, (SOCKET)socket, remoteIp, remotePort );
				break;
			}
			case InterThreadMsgType::ServerClosedNotification:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sizeof( ServerCloseNotificationMsg ) <= sz, "{} vs. {}", sizeof( ServerCloseNotificationMsg ), sz ); 
				const ServerCloseNotificationMsg* msg = reinterpret_cast<const ServerCloseNotificationMsg*>( riter.read( sizeof( ServerCloseNotificationMsg ) ) );
				/*const uint8_t* buff = riter.read( mh.bodySize );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, 1 <= sz ); 
				size_t serverIdx = mh.entryIdx;
				bool hasError = *buff != 0;*/
				NetSocketEntry& entry = netServerManagerBase->appGetSlaveServerEntry( msg->entryIdx );
				entry.getServerSocket()->closeByWorkingCluster();
				break;
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(msgType) ); 
				break;
		}
		CO_RETURN;
	}
}

#include "../include/nodecpp/logging.h"
namespace nodecpp {
	namespace logging_impl {
		nodecpp::stdvector<nodecpp::log::Log*> logs;
	} // namespace logging_impl
	thread_local size_t Log::ordinalBase = 0;
} // namespace nodecpp

#endif // NODECPP_ENABLE_CLUSTERING
