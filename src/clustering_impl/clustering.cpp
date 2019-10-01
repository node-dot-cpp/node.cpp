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

}

#endif // NODECPP_ENABLE_CLUSTERING
