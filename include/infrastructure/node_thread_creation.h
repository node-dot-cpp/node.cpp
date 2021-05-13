/* -------------------------------------------------------------------------------
* Copyright (c) 2021, OLogN Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*	 * Redistributions of source code must retain the above copyright
*	   notice, this list of conditions and the following disclaimer.
*	 * Redistributions in binary form must reproduce the above copyright
*	   notice, this list of conditions and the following disclaimer in the
*	   documentation and/or other materials provided with the distribution.
*	 * Neither the name of the OLogN Technologies AG nor the
*	   names of its contributors may be used to endorse or promote products
*	   derived from this software without specific prior written permission.
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

#ifndef NODE_THREAD_CREATION_H
#define NODE_THREAD_CREATION_H

#include "../nodecpp/common.h"
#include "q_based_infrastructure.h"
#include "inproc_queue.h"

extern InterThreadCommData threadQueues[MAX_THREADS];

namespace nodecpp {

template<class NodeT, class ThreadStartupDataT>
void nodeThreadMain( void* pdata )
{
	ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	ThreadStartupDataT startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	QueueBasedNodeLoop<NodeT> r( startupData );
	r.init();
	r.run();
}

template<class NodeT, class PostmanT>
#ifdef NODECPP_USE_GMQUEUE
NodeAddress runNodeInAnotherThread( PostmanT* postman, const char* nodeName = nullptr )
#else
NodeAddress runNodeInAnotherThread( PostmanT* postman )
#endif
{
	auto startupDataAndAddr = QueueBasedNodeLoop<NodeT>::getInitializer(postman); // TODO: consider implementing q-based Postman (as lib-defined)
	using InitializerT = typename QueueBasedNodeLoop<NodeT>::Initializer;
	InitializerT* startupData = nodecpp::stdalloc<InitializerT>(1);
	*startupData = startupDataAndAddr.first;
	size_t threadIdx = startupDataAndAddr.second.slotId;
#ifdef NODECPP_USE_GMQUEUE
	if ( nodeName != nullptr && nodeName != "" )
	{
		nodecpp::GMQThreadQueueTransport<GMQueueStatePublisherSubscriberTypeInfo> transport4node( gmqueue, nodeName, threadQueues[threadIdx].queue, 0 ); // NOTE: recipientID = 0 is by default; TODO: revise
		startupData->transportData = transport4node.makeTransferrable();
	}
	else
	{
		nodecpp::GMQThreadQueueTransport<GMQueueStatePublisherSubscriberTypeInfo> transport4node( gmqueue, threadQueues[threadIdx].queue, 0 ); // NOTE: recipientID = 0 is by default; TODO: revise
		startupData->transportData = transport4node.makeTransferrable();
	}
#endif
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );
	std::thread t1( nodeThreadMain<NodeT, InitializerT>, (void*)(startupData) );
	// startupData is no longer valid
	startupData = nullptr;
	t1.detach();
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
	return startupDataAndAddr.second;
}

} // namespace nodecpp

#endif // NODE_THREAD_CREATION_H
