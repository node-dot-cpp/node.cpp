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

#include <internal_msg.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

namespace nodecpp::platform::internal_msg { class InternalMsg; } // forward declaration

struct ThreadID
{
	size_t slotId = (size_t)(-1);
	uint64_t reincarnation = (uint64_t)(-1);
};

struct InterThreadMsg
{
	ThreadID sourceThreadID;
	ThreadID targetThreadID;
	size_t msgType = (size_t)(-1);
	nodecpp::platform::internal_msg::InternalMsg msg;

	InterThreadMsg() {}
	InterThreadMsg( nodecpp::platform::internal_msg::InternalMsg&& msg_, size_t msgType_, ThreadID sourceThreadID_, ThreadID targetThreadID_ ) : 
		sourceThreadID( sourceThreadID_ ), targetThreadID( targetThreadID_ ), msgType( msgType_ ), msg( std::move(msg_) )  {}
	InterThreadMsg( const InterThreadMsg& ) = delete;
	InterThreadMsg& operator = ( const InterThreadMsg& ) = delete;
	InterThreadMsg( InterThreadMsg&& other ) = default; 
	InterThreadMsg& operator = ( InterThreadMsg&& other ) = default;
};

template<class T, size_t maxsz_bits>
class CircularBuffer {
	static constexpr size_t bufsz = 1 << maxsz_bits;
	static constexpr size_t maxsz = bufsz - 1;
	//-1 to make sure that head==tail always means ‘empty’
	static constexpr size_t mask = maxsz;
	size_t head = 0;
	size_t tail = 0;
	alignas(T) uint8_t buffer[bufsz * sizeof(T)];
	//Having buffer as T[bufsz] is possible 
	//  IF we'll replace placement move constructors with move assignments
	//  AND drop explicit destructor calls
	//However, it will require T to have a default constructor,
	//  so at the moment I prefer to deal with pure buffers
	//  and to have the only requirement that T is move-constructible

public:
	using value_type = T;
public:
	size_t size() {
		return head - tail +
			//      return head – tail + 
			(((size_t)(head >= tail) - (size_t)1) & bufsz);
		//trickery to avoid pipeline stalls via arithmetic
		//supposedly equivalent to:
		//if(head >= tail)
		//  return head – tail;
		//else
		//  return head + bufsz - tail;
	}

	bool is_full() { return size() == maxsz; }

	void push_back(T&& t) {
		assert(size() < maxsz);
		new(tbuffer(head)) T(std::move(t));
		head = (head + 1) & mask;
	}

	T pop_front() {
		assert(size() > 0);
		T* ttail = tbuffer(tail);
		T ret = std::move(*ttail);
		ttail->~T();
		tail = (tail + 1) & mask;
		return ret;
	}

private:
	T* tbuffer(size_t idx) {
		return reinterpret_cast<T*>(buffer + (idx * sizeof(T)));
	}
};

template <class FixedSizeCollection>
class MWSRFixedSizeQueueWithFlowControl {
private:
	std::mutex mx;
	std::condition_variable waitrd;
	std::condition_variable waitwr;
	FixedSizeCollection coll;
	bool killflag = false;

	//stats:
	int nfulls = 0;
	size_t hwmsize = 0;//high watermark on queue size

public:
	using T = typename FixedSizeCollection::value_type;

	MWSRFixedSizeQueueWithFlowControl() {}
	MWSRFixedSizeQueueWithFlowControl( const MWSRFixedSizeQueueWithFlowControl& ) = delete;
	MWSRFixedSizeQueueWithFlowControl& operator = ( const MWSRFixedSizeQueueWithFlowControl& ) = delete;
	MWSRFixedSizeQueueWithFlowControl( MWSRFixedSizeQueueWithFlowControl&& ) = delete;
	MWSRFixedSizeQueueWithFlowControl& operator = ( MWSRFixedSizeQueueWithFlowControl&& ) = delete;
	void push_back(T&& it) {
		//if the queue is full, BLOCKS until some space is freed
		{//creating scope for lock
			std::unique_lock<std::mutex> lock(mx);
			while (coll.is_full() && !killflag) {
				waitwr.wait(lock);
				++nfulls;
				//this will also count spurious wakeups,
				//  but they’re supposedly rare
			}

			if (killflag)
				return;
			assert(!coll.is_full());
			coll.push_back(std::move(it));
			size_t sz = coll.size();
			hwmsize = std::max(hwmsize, sz);
		}//unlocking mx

		waitrd.notify_one();
	}

	std::pair<bool, T> pop_front() {
		std::unique_lock<std::mutex> lock(mx);
		while (coll.size() == 0 && !killflag) {
			waitrd.wait(lock);
		}
		if (killflag)
			return std::pair<bool, T>(false, T());

		assert(coll.size() > 0);
//		T ret = std::move(coll.front());
//		coll.pop_front();
		T ret = std::move(coll.pop_front());
		lock.unlock();
		waitwr.notify_one();

		return std::pair<bool, T>(true, std::move(ret));
	}

	void kill() {
		{//creating scope for lock
			std::unique_lock<std::mutex> lock(mx);
			killflag = true;
		}//unlocking mx

		waitrd.notify_all();
		waitwr.notify_all();
	}
};

using MsgQueue = MWSRFixedSizeQueueWithFlowControl<CircularBuffer<InterThreadMsg, 4>>; // TODO: revise the second param value

struct ThreadMsgQueue
{
	uintptr_t writeHandle = (uintptr_t)(-1);
	uint64_t reincarnation = 0;
	MsgQueue queue;
};

#define MAX_THREADS 128

extern ThreadMsgQueue threadQueues[MAX_THREADS];

void sendInterThreadMsg(nodecpp::platform::internal_msg::InternalMsg&& msg, size_t msgType, ThreadID threadId );


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace nodecpp;

class InterThreadComm
{
	class MyReadingSocket : public nodecpp::net::SocketBase {};
	class PublishedSocket : public nodecpp::net::SocketBase {};
	class MyTempServer : public nodecpp::net::ServerBase {};
	uint64_t reincarnation = 0;
	size_t threadIdx = 0;

public:
	InterThreadComm() {}

	virtual nodecpp::handler_ret_type init( size_t threadIdx_, uint64_t reincarnation_ )
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "InterThreadComm::init(), threadIdx = {}, reincarnation = {}", threadIdx_, reincarnation_ );
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, threadIdx_ < MAX_THREADS );
		threadIdx = threadIdx_;
		reincarnation = reincarnation_;

		// registering handlers
		nodecpp::net::ServerBase::addHandler<MyTempServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &InterThreadComm::onListeningTempServer>(this);
		nodecpp::net::ServerBase::addHandler<MyTempServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &InterThreadComm::onConnectionTempServer>(this);

		nodecpp::net::SocketBase::addHandler<MyReadingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &InterThreadComm::onConnectReadingSocket>(this);
		nodecpp::net::SocketBase::addHandler<MyReadingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &InterThreadComm::onDataReadingSocket>(this);
		nodecpp::net::SocketBase::addHandler<MyReadingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::End, &InterThreadComm::onEndReadingSocket>(this);
		nodecpp::net::SocketBase::addHandler<MyReadingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Error, &InterThreadComm::onErrorReadingSocket>(this);

		nodecpp::net::SocketBase::addHandler<PublishedSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &InterThreadComm::onConnectPublishedSocket>(this);
		nodecpp::net::SocketBase::addHandler<PublishedSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::End, &InterThreadComm::onEndPublishedSocket>(this);
		nodecpp::net::SocketBase::addHandler<PublishedSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Error, &InterThreadComm::onErrorPublishedSocket>(this);

		srv = nodecpp::net::createServer<MyTempServer, MyReadingSocket>();
		publishedSock = nodecpp::net::createSocket<PublishedSocket>();

		srv->listen(0, "127.0.0.1", 5);

		CO_RETURN;
	}

public:
	// server
	nodecpp::handler_ret_type onListeningTempServer(nodecpp::safememory::soft_ptr<MyTempServer> server, size_t id, nodecpp::net::Address addr ) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "onListeningTempServer(), threadIdx = {}, reincarnation = {}", threadIdx, reincarnation );
		publishedSock->connect( addr.port, "127.0.0.1" );
		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionTempServer(nodecpp::safememory::soft_ptr<MyTempServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket ) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "onConnectionTempServer, threadIdx = {}, reincarnation = {}", threadIdx, reincarnation );
		if ( socket->remoteAddress() == publishedSock->localAddress() )
			server->close();
		else
			socket->end();
		CO_RETURN;
	}

	// reading socket
	nodecpp::handler_ret_type onConnectReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket)
	{
		// comm system is ready TODO: report, ...
		{
			nodecpp::platform::internal_msg::InternalMsg msg;
			ThreadID threadId = {0, 0};
			auto reportStr = nodecpp::format( "Thread {} has almost initialized interthread comm system ...", threadIdx );
			msg.append( const_cast<char*>(reportStr.c_str()), reportStr.size() + 1 ); // TODO: update Foundation and remove cast ASAP!!!
			sendInterThreadMsg( std::move( msg ), 17, threadId );
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type onDataReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket, Buffer& buffer)
	{
		// TODO: processMessage(s)
		MsgQueue& myQueue = threadQueues[threadIdx].queue;
		for ( size_t i=0; i<buffer.size(); ++i )
		{
			/*auto popRes = std::move( myQueue.pop_front() );
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, popRes.first ); // unless we've already killed it
			InterThreadMsg& msg = popRes.second;
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, msg.reincarnation == reincarnation, "{} vs. {}", msg.reincarnation, reincarnation ); // unless we've already killed it
			// TODO: process message
			auto readit = msg.msg.getReadIter();
			size_t sz = readit.availableSize();
			const char* text = reinterpret_cast<const char*>( readit.read(sz) );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "msg = \"{}\"", text );*/
		}

		CO_RETURN;
	}

	nodecpp::handler_ret_type onEndReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket)
	{
		socket->end();
		CO_RETURN;
	}

	nodecpp::handler_ret_type onErrorReadingSocket(nodecpp::safememory::soft_ptr<MyReadingSocket> socket, Error& e)
	{
		socket->end();
		CO_RETURN;
	}

	// published socket
	nodecpp::handler_ret_type onConnectPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket)
	{
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "subject for revision" );
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, threadIdx < MAX_THREADS, "{} vs. {}", threadIdx, MAX_THREADS ); // unless we've already killed it
		threadQueues[threadIdx].writeHandle = socket->dataForCommandProcessing.osSocket;
		{
			nodecpp::platform::internal_msg::InternalMsg msg;
			ThreadID threadId = {0, 0};
			auto reportStr = nodecpp::format( "Thread {} has initialized interthread comm system; socket = {}", threadIdx, socket->dataForCommandProcessing.osSocket );
			msg.append( const_cast<char*>(reportStr.c_str()), reportStr.size() + 1 ); // TODO: update Foundation and remove cast ASAP!!!
			sendInterThreadMsg( std::move( msg ), 17, threadId );
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type onEndPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket)
	{
		// TODO: handling (consider both cases: "initiated by our side" and "errors")
		socket->end();
		CO_RETURN;
	}

	nodecpp::handler_ret_type onErrorPublishedSocket(nodecpp::safememory::soft_ptr<PublishedSocket> socket, Error& e)
	{
		// TODO: handling (consider both cases: "initiated by our side" and "errors")
		socket->end();
		CO_RETURN;
	}

	nodecpp::safememory::owning_ptr<MyTempServer> srv;
	nodecpp::safememory::owning_ptr<PublishedSocket> publishedSock;
};

extern thread_local InterThreadComm interThreadComm;

struct InterThreadCommPair
{
	SOCKET readHandle;
	SOCKET writeHandle;
};

InterThreadCommPair initInterThreadCommSystem();
InterThreadCommPair generateHandlePair();

#endif // INTERTHREAD_COMM_H