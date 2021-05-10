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

#ifndef INPROC_QUEUE_H
#define INPROC_QUEUE_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include <gmqueue.h>
#include "marshalling.h"
#include <clustering_impl/interthread_comm_impl.h>

namespace nodecpp {

class ThreadQueuePostman : public ::globalmq::marshalling::InProcessMessagePostmanBase
{
private:
	MsgQueue& msgQueue;
	uint64_t recipientID;

public:
	ThreadQueuePostman( MsgQueue& msgQueue_, uint64_t recipientID_ ) : msgQueue( msgQueue_ ), recipientID( recipientID_ ) {}
	virtual ~ThreadQueuePostman() {}
	virtual void postMessage( ::globalmq::marshalling::MessageBufferT&& msg ) override
	{
		msgQueue.push_back( InterThreadMsg( std::move( msg ), InterThreadMsgType::GlobalMQ, NodeAddress(), NodeAddress(), recipientID ) );
	}
};

template<class PlatformSupportT>
class GMQThreadQueueTransport : public ::globalmq::marshalling::GMQTransportBase<PlatformSupportT>
{
public:
	GMQThreadQueueTransport( ::globalmq::marshalling::GMQueue<PlatformSupportT>& gmq, GMQ_COLL string name, MsgQueue& queue, uint64_t recipientID ) : ::globalmq::marshalling::GMQTransportBase<PlatformSupportT>( gmq, name, gmq.template allocPostman<ThreadQueuePostman>( queue, recipientID ) ) {}
	GMQThreadQueueTransport( ::globalmq::marshalling::GMQueue<PlatformSupportT>& gmq, MsgQueue& queue, int recipientID ) : ::globalmq::marshalling::GMQTransportBase<PlatformSupportT>( gmq, gmq.template allocPostman<ThreadQueuePostman>( queue, recipientID ) ) {}
	virtual ~GMQThreadQueueTransport() {}
};

} // namespace nodecpp

#endif // INPROC_QUEUE_H
