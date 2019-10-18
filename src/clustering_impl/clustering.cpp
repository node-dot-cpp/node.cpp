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
#include "../../src/infrastructure.h"
#include "../../src/tcp_socket/tcp_socket.h"
#include <thread>

extern void workerThreadMain( void* pdata );

namespace nodecpp
{
	thread_local Cluster cluster;

	void preinitMasterThreadClusterObject()
	{
		cluster.preinitMaster();
	}

	void preinitSlaveThreadClusterObject(size_t id)
	{
		cluster.preinitSlave( id );
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
		bool newDelInterceptionState = interceptNewDeleteOperators(false);
		ThreadStartupData* startupData = new ThreadStartupData;
		startupData->assignedThreadID = worker.id_;
		interceptNewDeleteOperators(newDelInterceptionState);
		// run worker thread
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("about to start Worker thread with threadID = {}...", worker.id_ );
		std::thread t1( workerThreadMain, (void*)(startupData) );
		// startupData is no longer valid
		startupData = nullptr;
		t1.detach();
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("...starting Worker thread with threadID = {} completed at Master thread side", worker.id_ );
		workers_.push_back( std::move(worker) );
		return workers_[internalID];
	}
	
	void Worker::disconnect()
	{
		// TODO: ...
	}

	void Cluster::AgentServer::registerServer() { 
		nodecpp::safememory::soft_ptr<Cluster::AgentServer> myPtr = myThis.getSoftPtr<Cluster::AgentServer>(this);
		::registerAgentServer(/*node, */myPtr, -1); 
	}
	void Cluster::AgentServer::listen(uint16_t port, const char* ip, int backlog)
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
			default:
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected type {}", (size_t)(mh.type) ); 
				break;
		}
		CO_RETURN;
	}
}

#endif // NODECPP_ENABLE_CLUSTERING
