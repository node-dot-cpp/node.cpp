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

#include "clustering_common.h"
#include "../../include/nodecpp/cluster.h"
#include "../infrastructure.h"
#include "../tcp_socket/tcp_socket.h"
#include <thread>

#include "interthread_comm.h"
#include "interthread_comm_impl.h"

ThreadMsgQueue threadQueues[MAX_THREADS];

thread_local InterThreadComm interThreadComm;

static InterThreadCommInitializer interThreadCommInitializer;

InterThreadCommPair InterThreadCommInitializer::init()
{
	Ip4 ip4 = Ip4::parse( "127.0.01" );
	myServerSocket = acquireSocketAndLetInterThreadCommServerListening( ip4, myServerPort, 128 );
	return generateHandlePair();
}

InterThreadCommPair InterThreadCommInitializer::generateHandlePair()
{
	auto res = acquireAndConnectSocketForInterThreadComm( myServerSocket, "127.0.01", myServerPort );
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, res.first.second == res.second.second ); 
	return InterThreadCommPair({res.first.first, res.second.first});
}

InterThreadCommPair initInterThreadCommSystem()
{
	return interThreadCommInitializer.init();
}

InterThreadCommPair generateHandlePair()
{
	return interThreadCommInitializer.generateHandlePair();
}

void sendInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, size_t msgType, ThreadID threadId )
{
	// validate idx
	int sock;
	uint64_t reincarnation;
	
	{// get socket and reincarnation from under the mutex
		sock = threadQueues[ threadId.slotId ].sock;
		reincarnation = threadQueues[ threadId.slotId ].reincarnation;
	}// release mutex
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == threadId.reincarnation ); 
	threadQueues[ threadId.slotId ].queue.push_back( InterThreadMsg( std::move( msg ), msgType, reincarnation ) );
	// write a byte to sock
	uint8_t singleByte = 0x1;
	size_t sentSize = 0;
	auto ret = nodecpp::internal_usage_only::internal_send_packet( &singleByte, 1, sock, sentSize );
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
	}

	void preinitSlaveThreadClusterObject(ThreadStartupData& startupData)
	{
		cluster.preinitSlave( startupData );
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
		startupData->assignedThreadID = worker.id_;
		startupData->commPort = ctrlServer->dataForCommandProcessing.localAddress.port;
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


	nodecpp::handler_ret_type Cluster::SlaveSocket::processResponse( ClusteringMsgHeader& mh, nodecpp::Buffer& b, size_t offset )
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, mh.bodySize + offset <= b.size() ); 
		switch ( mh.type )
		{
			case ClusteringMsgHeader::ClusteringMsgType::ServerListening:
			{
				// TODO: ...
				break;
			}
			case ClusteringMsgHeader::ClusteringMsgType::ConnAccepted:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + sizeof(size_t) + sizeof(uint64_t) <= b.size() ); 
				size_t serverIdx = *reinterpret_cast<size_t*>(b.begin() + offset);
				uint64_t socket = *reinterpret_cast<uint64_t*>(b.begin() + offset + sizeof(serverIdx));
				Ip4 remoteIp = Ip4::fromNetwork( *reinterpret_cast<uint32_t*>(b.begin() + offset + sizeof(serverIdx) + sizeof(socket)) );
				Port remotePort = Port::fromNetwork( *reinterpret_cast<uint16_t*>(b.begin() + offset + sizeof(serverIdx) + sizeof(socket) + sizeof(uint32_t)) );
				netServerManagerBase->addAcceptedSocket( serverIdx, (SOCKET)socket, remoteIp, remotePort );
				break;
			}
			case ClusteringMsgHeader::ClusteringMsgType::ServerError:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + sizeof(size_t) <= b.size() ); 
				size_t serverIdx = mh.entryIdx;
//				netServerManagerBase->addAcceptedSocket( serverIdx, (SOCKET)socket, remoteIp, remotePort );
				break;
			}
			case ClusteringMsgHeader::ClusteringMsgType::ServerClosedNotification:
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + 1 <= b.size() ); 
				size_t serverIdx = mh.entryIdx;
				bool hasError = b.readUInt8( offset + sizeof(size_t) ) != 0;
				NetSocketEntry& entry = netServerManagerBase->appGetSlaveServerEntry( serverIdx );
				entry.getServerSocket()->closeByWorkingCluster();
				break;
			}
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(mh.type) ); 
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
