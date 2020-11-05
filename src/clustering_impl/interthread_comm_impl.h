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

#ifndef INTERTHREAD_COMM_IMPL_H
#define INTERTHREAD_COMM_IMPL_H

#ifdef NODECPP_ENABLE_CLUSTERING


#include <thread>
#include <mutex>
#include <condition_variable>
#include "interthread_comm.h"


template<class T, size_t maxsz_bits>
class CircularBuffer {
	static constexpr size_t bufsz = 1 << maxsz_bits;
	static constexpr size_t maxsz = bufsz - 1;
	//-1 to make sure that head==tail always means 'empty'
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
			(((size_t)(head >= tail) - (size_t)1) & bufsz);
		//trickery to avoid pipeline stalls via arithmetic
		//supposedly equivalent to:
		//if(head >= tail)
		//  return head - tail;
		//else
		//  return head + bufsz - tail;
	}

	bool is_full() { return size() == maxsz; }

	void push_back(T&& t) {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, size() < maxsz);
		new(tbuffer(head)) T(std::move(t));
		head = (head + 1) & mask;
	}

	T pop_front() {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, size() > 0);
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
				//  but they're supposedly rare
			}

			if (killflag)
				return;
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coll.is_full());
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

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, coll.size() > 0);
//		T ret = std::move(coll.front());
//		coll.pop_front();
		T ret = std::move(coll.pop_front());
		lock.unlock();
		waitwr.notify_one();

		return std::pair<bool, T>(true, std::move(ret));
	}

	size_t pop_front( T* messages, size_t count ) {
		std::unique_lock<std::mutex> lock(mx);
		while (coll.size() == 0 && !killflag) {
			waitrd.wait(lock);
		}
		if (killflag)
			return 0;

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, coll.size() > 0);
		size_t sz2move = count <= coll.size() ? count : coll.size();
		for ( size_t i=0; i<sz2move; ++i )
			messages[i] = std::move(coll.pop_front());
		lock.unlock();
		waitwr.notify_one();

		return sz2move;
	}

	size_t pop_front( T* messages, size_t count, uint64_t timeout ) {
		std::unique_lock<std::mutex> lock(mx);
		bool expired = false;
		while (coll.size() == 0 && !expired && !killflag) {
			expired = waitrd.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout;
		}
		if (killflag)
			return 0;

//		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, coll.size() > 0);
		size_t sz2move = count <= coll.size() ? count : coll.size();
		for ( size_t i=0; i<sz2move; ++i )
			messages[i] = std::move(coll.pop_front());
		lock.unlock();
		waitwr.notify_one();

		return sz2move;
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

class InterThreadMessagePosterBase
{
public:
	InterThreadMessagePosterBase() {};
	virtual void postMessage() = 0;
	virtual ~InterThreadMessagePosterBase() {}
};

class InterThreadCommData
{
public:
	static constexpr uintptr_t invalid_write_handle = (uintptr_t)(-1);
private:
	std::mutex mx;
	uintptr_t writeHandle = invalid_write_handle; // mx-protected
	uint64_t reincarnation = 0; // mx-protected
	InterThreadMessagePosterBase* postman = nullptr;
	enum Status {unused, acquired, running, terminating};
	Status status = unused;

public:
	InterThreadCommData() {}
	InterThreadCommData( const InterThreadCommData& ) = delete;
	InterThreadCommData& operator = ( const InterThreadCommData& ) = delete;
	InterThreadCommData( InterThreadCommData&& ) = delete;
	InterThreadCommData& operator = ( InterThreadCommData&& ) = delete;

	MsgQueue queue;

	std::pair<bool, std::pair<uint64_t, uintptr_t>> getWriteHandleAndReincarnation() {
		std::unique_lock<std::mutex> lock(mx);
		return std::make_pair(status != Status::terminating && status != Status::unused, std::make_pair(reincarnation, writeHandle));
	}
	std::pair<bool, std::pair<InterThreadMessagePosterBase*, uint64_t>> getPosterAndReincarnation() {
		std::unique_lock<std::mutex> lock(mx);
		return std::make_pair(status != Status::terminating && status != Status::unused, std::make_pair(postman, reincarnation));
	}
	void setTerminating() {
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, status == Status::running, "indeed: {}", status ); 
		status = Status::terminating;
	}
	void setUnused( uintptr_t writeHandle_ ) {
		std::unique_lock<std::mutex> lock(mx);
		status = Status::unused;
		writeHandle = invalid_write_handle;
		if ( postman ) 
			delete postman;
		postman = nullptr;
		// TODO: what should we do with non-empty queue (or why is it empty)?
	}
	std::pair<bool, uint64_t> acquireForReuse( uintptr_t writeHandle_ ) {
		std::unique_lock<std::mutex> lock(mx);
		if ( status == Status::unused )
		{
			++reincarnation;
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, postman == nullptr ); 
			writeHandle = writeHandle_;
			status = Status::acquired;
			return std::make_pair( true, reincarnation );
		}
		else
			return std::make_pair( false, (uint64_t)(-1) );
	}
	std::pair<bool, uint64_t> acquireForReuse( InterThreadMessagePosterBase* postman_ ) {
		std::unique_lock<std::mutex> lock(mx);
		if ( status == Status::unused )
		{
			++reincarnation;
			writeHandle = invalid_write_handle;
			postman = postman_;
			status = Status::acquired;
			return std::make_pair( true, reincarnation );
		}
		else
			return std::make_pair( false, (uint64_t)(-1) );
	}
	void setWriteHandleForFirstUse( uintptr_t writeHandle_ ) {
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, postman == nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeHandle == (uintptr_t)(-1) ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, status == Status::unused ); 
		status = Status::acquired;
		writeHandle = writeHandle_;
	}
	void setPostmanForFirstUse( InterThreadMessagePosterBase* postman_ ) {
		std::unique_lock<std::mutex> lock(mx);
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, reincarnation == 0 ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, postman == nullptr ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeHandle == (uintptr_t)(-1) ); 
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, status == Status::unused ); 
		status = Status::acquired;
		postman = postman_;
	}
	bool isUnused() {
		std::unique_lock<std::mutex> lock(mx);
		return status == Status::unused;
	}
};

struct InterThreadCommPair
{
	uintptr_t readHandle;
	uintptr_t writeHandle;
};

class InterThreadCommInitializer
{
	bool isInitialized_ = false;
	uint16_t myServerPort;
	uintptr_t myServerSocket;

public:
	InterThreadCommInitializer() {}

	uintptr_t init(); // returns read handle for the main (calling) thread

public:
	InterThreadCommPair generateHandlePair();
};

#endif // NODECPP_ENABLE_CLUSTERING

#endif // INTERTHREAD_COMM_IMPL_H