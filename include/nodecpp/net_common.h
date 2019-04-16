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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "common.h"
//#include "event.h"

namespace nodecpp {

	//TODO quick and temp implementation
	static constexpr size_t MIN_BUFFER = 1024;

	class Buffer {
	private:
		size_t _size = 0;
		size_t _capacity = 0;
		std::unique_ptr<uint8_t> _data;

	private:
		void ensureCapacity(size_t sz) { // NOTE: may invalidate pointers
			if (_data == nullptr) {
				reserve(sz);
			}
			else if (sz > _capacity) {
				size_t cp = std::max(sz, MIN_BUFFER);
				std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));
				memcpy(tmp.get(), _data.get(), _size);
				_capacity = cp;
				_data = std::move(tmp);
			}
		}

	public:
		Buffer() {}
		Buffer(size_t res) { reserve(res); }
		Buffer(Buffer&& p) {
			std::swap(_size, p._size);
			std::swap(_capacity, p._capacity);
			std::swap(_data, p._data);
		}
		Buffer& operator = (Buffer&& p) {
			std::swap(_size, p._size);
			std::swap(_capacity, p._capacity);
			std::swap(_data, p._data);
			return *this;
		}
		Buffer(const Buffer&) = delete;
		Buffer& operator = (const Buffer& p) = delete;

		Buffer clone() {
			Buffer cp(size());
			cp.append(begin(), size());
			return cp;
		}

		void reserve(size_t sz) {
			assert(_size == 0);
			assert(_capacity == 0);
			assert(_data == nullptr);

			size_t cp = std::max(sz, MIN_BUFFER);
			std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));

			_capacity = cp;
			_data = std::move(tmp);
		}

		void append(const void* dt, size_t sz) { // NOTE: may invalidate pointers
/*			if (_data == nullptr) {
				reserve(sz);
				memcpy(end(), dt, sz);
				_size += sz;
			}
			else if (_size + sz <= _capacity) {
				memcpy(end(), dt, sz);
				_size += sz;
			}
			else {
				size_t cp = std::max(sz + _size, MIN_BUFFER);
				std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));


				memcpy(tmp.get(), _data.get(), _size);
				memcpy(tmp.get() + _size, dt, sz);

				_size = sz + _size;
				_capacity = cp;
				_data = std::move(tmp);
			}*/
			ensureCapacity(_size + sz);
			memcpy(end(), dt, sz);
			_size += sz;
		}

		void trim(size_t sz) { // NOTE: keeps pointers
			assert(sz <= _size);
			assert(_data != nullptr);
			_size -= sz;
		}

		void clear() {
			trim(size());
		}

		void set_size(size_t sz) { // NOTE: keeps pointers
			assert(sz <= _capacity);
			assert(_data != nullptr);
			_size = sz;
		}

		size_t size() const { return _size; }
		bool empty() const { return _size == 0; }
		size_t capacity() const { return _capacity; }
		uint8_t* begin() { return _data.get(); }
		const uint8_t* begin() const { return _data.get(); }
		uint8_t* end() { return _data.get() + _size; }
		const uint8_t* end() const { return _data.get() + _size; }

		//TODO mb: to avoid memmove things around, popFront and pushFront
		// may be implemented by always
		// reserving some bytes at front, and using an extra header pointer
		// like linux kernel skbuff does.
		void popFront(size_t sz) {
			assert(sz <= size());
			size_t remaining = size() - sz;
			memmove(begin(), begin() + sz, remaining);
			trim(sz);
		}

		// TODO: revise and add other relevant calls
		size_t writeInt8( int8_t val, size_t pos ) {
			ensureCapacity(pos + 1);
			*reinterpret_cast<uint8_t*>(begin() + pos ) = val;
			if ( _size < pos + 1 )
				_size = pos + 1;
			return pos + 1;
		}
		size_t appendUint8( int8_t val ) {
			ensureCapacity(size() + 1);
			*reinterpret_cast<uint8_t*>(begin() + size() ) = val;
			return ++_size;
		}
		size_t appendString( const char* str, size_t sz ) { // TODO: revision required
			if ( str )
			{
				ensureCapacity(size() + sz + 1);
				memcpy(begin() + size(), str, sz );
				_size += sz;
			}
			*reinterpret_cast<uint8_t*>(begin() + size() ) = 0;
			return ++_size;
		}

		// an attempt to follow node.js buffer-related interface
		void writeUInt32LE(uint8_t value, size_t offset) {
			assert( offset + 1 <= _capacity );
			*(begin() + size()) = value;
			if ( _size < offset + 4 )
				_size = offset + 4;
		}
		void writeUInt32BE(uint32_t value, size_t offset) {
			assert( false ); // TODO: implement
		}
		void writeUInt32LE(uint32_t value, size_t offset) {
			assert( offset + 4 <= _capacity );
			memcpy(begin() + size(), &value, 4 );
			if ( _size < offset + 4 )
				_size = offset + 4;
		}
		uint32_t readUInt8(size_t offset) {
			assert( offset + 1 <= _size );
			return *(begin() + offset);
		}
		uint32_t readUInt32BE(size_t offset) {
			assert( false ); // TODO: implement
			return 0;
		}
		uint8_t readUInt32LE(size_t offset) {
			assert( offset + 4 <= _size );
			return *reinterpret_cast<uint32_t*>(begin() + offset);
		}
	};

	class CircularByteBuffer
	{
		std::unique_ptr<uint8_t[]> buff; // TODO: switch to using safe memory objects ASAP
		size_t size_exp;
		uint8_t* begin = nullptr;
		uint8_t* end = nullptr;
	public:
		CircularByteBuffer(size_t sz_exp) { 
			size_exp = sz_exp; 
			buff = std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(malloc(alloc_size())));
			begin = end = buff.get();
		}
		CircularByteBuffer( const CircularByteBuffer& ) = delete;
		CircularByteBuffer& operator = ( const CircularByteBuffer& ) = delete;
		CircularByteBuffer( CircularByteBuffer&& other ) : buff( std::move( other.buff ) ) {
			size_exp = other.size_exp;
			begin = other.begin;
			end = other.end;
		}
		CircularByteBuffer& operator = ( CircularByteBuffer&& other ) {
			size_exp = other.size_exp;
			begin = other.begin;
			end = other.end;
		}
		size_t used_size() const { return begin < end ? end - begin : alloc_size() - (begin - end); }
		size_t remaining_capacity() const { return alloc_size() - 1 - used_size(); }
		bool empty() const { return begin == end; }
		size_t alloc_size() const { return ((size_t)1)<<size_exp; }
		bool append( uint8_t* ptr, size_t sz ) { 
			if ( sz > remaining_capacity() )
				return false;
			size_t fwd_free_sz = buff.get() + alloc_size() - end;
			if ( sz <= fwd_free_sz )
			{
				memcpy( end,  ptr, sz );
				end += sz;
			}
			else
			{
				memcpy( end,  ptr, fwd_free_sz );
				memcpy( buff.get(),  ptr + fwd_free_sz, sz - fwd_free_sz );
				end = buff.get() + sz - fwd_free_sz;
			}
			return true; 
		}
		bool push_front( size_t sz ) {
			if ( sz > remaining_capacity() )
				return false;
			if ( begin > end ) {
				end += sz;
			}
			else
			{
				size_t sz_at_end = buff.get() + alloc_size() - begin;
				if ( sz <= sz_at_end )
					end += sz;
				else
					end = buff.get() + sz - sz_at_end;
			}
			return true;
		}
		bool pop_back( size_t sz ) {
			if ( sz > used_size() ) // TODO: think about void function and assertion
				return false;
			
			if ( begin <= end )
				begin += sz;
			else
			{
				size_t sz_at_end = buff.get() + alloc_size() - begin;
				if ( sz < sz_at_end )
					begin += sz;
				else
					begin = buff.get() + sz - sz_at_end;
			}
			return true;
		}
		std::pair<uint8_t*, size_t> get_begin() {
			if ( begin <= end )
				return std::make_pair( begin, end - begin );
			else
				return std::make_pair( begin, buff.get() + alloc_size() - begin );
		}
		std::pair<uint8_t*, size_t> get_end() {
			if ( begin <= end )
				return std::make_pair( end, buff.get() + alloc_size() - end );
			else
				return std::make_pair( end, begin - end - 1 );
		}
	};

	template <class ListItem>
	class IntrusiveList
	{
		size_t _size = 0;
		ListItem* _begin = nullptr;
	public:
		ListItem* add(ListItem* item)
		{
			//ListItem* item = new ListItem(sdata);
			//owning_ptr<ListItem> p1 = make_owning<ListItem>(sdata);
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, item != nullptr );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _begin == nullptr || _begin->prev_ == nullptr );
			if ( _begin == nullptr )
			{
				_begin = item;
				item->next_ = nullptr;
				item->prev_ = nullptr;
			}
			else
			{
				item->next_ = _begin;
				_begin->prev_ = item;
				item->prev_ = nullptr;
				_begin = item;
			}
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _begin == nullptr || _begin->prev_ == nullptr );
			++_size;
			return item;
		}
		void removeAndDelete( ListItem* item )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, item != nullptr );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _begin == nullptr || _begin->prev_ == nullptr );
			if ( _begin == item )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, item == _begin );
				_begin = item->next_;
			}
			if ( item->prev_ )
				item->prev_->next_ = item->next_;
			if ( item->next_ )
				item->next_->prev_ = item->prev_;
			--_size;
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _begin == nullptr || _begin->prev_ == nullptr );
			delete item;
		}
		void clear()
		{
			while ( _begin )
			{
				ListItem* tmp = _begin;
				delete _begin;
				_begin = tmp;

			}
		}
		size_t getCount() { return _size; }
	};

	template <class ItemT>
	class MultiOwner
	{
		std::vector<owning_ptr<ItemT>> items;
		size_t cnt;
	public:
		soft_ptr<ItemT> add(owning_ptr<ItemT>&& item)
//		void add(owning_ptr<ItemT>&& item)
		{
			//ItemT* item = new ItemT(sdata);
			//owning_ptr<ItemT> p1 = make_owning<ItemT>(sdata);
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, item );
			for ( size_t idx=0; idx<items.size(); ++idx )
				if ( !items[idx] )
				{
					items[idx] = std::move( item );
					++cnt;
					soft_ptr<ItemT> ret(items[idx]);
					return ret;
				}
			size_t idx = items.size();
			items.emplace_back( std::move( item ) );
			++cnt;
			soft_ptr<ItemT> ret(items[idx]);
			return ret;
		}
		void removeAndDelete( soft_ptr<ItemT> item )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, item );
			for ( size_t idx=0; idx<items.size(); ++idx )
				if ( items[idx] == item )
				{
					items[idx].reset();
					--cnt;
				}
		}
		void clear()
		{
			items.clear();
			cnt = 0;
		}
		size_t getCount() { return cnt; }
	};


	namespace net {


		struct Address {
			uint16_t port;
			std::string family;
			std::string address;
		};

		enum Mode { callable, awaitable };

		class SocketBase
		{
		public:
			nodecpp::safememory::soft_this_ptr<SocketBase> myThis;
		public:
			SocketBase* prev_;
			SocketBase* next_;

		public:
//			UserDefID userDefID;
			NodeBase* node = nullptr;
			net::Mode mode() { return dataForCommandProcessing.mode; }

			public:
			class DataForCommandProcessing {
			public:
				enum State { Uninitialized, Connecting, Connected, LocalEnding, LocalEnded, Closing, ErrorClosing, Closed}
				state = Uninitialized;
				size_t index = 0;

//				net::Mode mode = net::Mode::callable;
				net::Mode mode = net::Mode::awaitable;

				struct awaitable_handle_data
				{
					std::experimental::coroutine_handle<> h = nullptr;
					bool is_exception = false;
					std::exception exception; // TODO: consider possibility of switching to nodecpp::error
				};

				std::experimental::coroutine_handle<> h_connect = nullptr;
				std::experimental::coroutine_handle<> h_accepted = nullptr;
				//std::experimental::coroutine_handle<> h_read = nullptr;
				//std::experimental::coroutine_handle<> h_write = nullptr;
				struct awaitable_read_handle_data : public awaitable_handle_data
				{
					Buffer b;
					size_t min_bytes;
				};
				struct awaitable_write_handle_data : public awaitable_handle_data
				{
					Buffer b;
				};
				awaitable_read_handle_data ahd_read;
				awaitable_write_handle_data ahd_write;

			//	bool connecting = false;
				bool remoteEnded = false;
				//bool localEnded = false;
				//bool pendingLocalEnd = false;
				bool paused = false;
				bool allowHalfOpen = true;

				bool refed = false;

				//Buffer writeBuffer = Buffer(64 * 1024);
				CircularByteBuffer writeBuffer = CircularByteBuffer( 16 );
				Buffer recvBuffer = Buffer(64 * 1024);

				//SOCKET osSocket = INVALID_SOCKET;
				//UINT_PTR osSocket = INVALID_SOCKET;
				unsigned long long osSocket = 0;


				DataForCommandProcessing() {}
				DataForCommandProcessing(const DataForCommandProcessing& other) = delete;
				DataForCommandProcessing& operator=(const DataForCommandProcessing& other) = delete;

				DataForCommandProcessing(DataForCommandProcessing&& other) = default;
				DataForCommandProcessing& operator=(DataForCommandProcessing&& other) = default;

				bool isValid() const { return state != State::Uninitialized; }
			};
		//protected:
			DataForCommandProcessing dataForCommandProcessing;

		public:
			Address _local;
			Address _remote;
			//std::string _remoteAddress;
			//std::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			enum State { UNINITIALIZED = 0, CONNECTING, CONNECTED, DESTROYED } state = UNINITIALIZED;

			SocketBase(NodeBase* node_) {node = node_;}

			SocketBase(const SocketBase&) = delete;
			SocketBase& operator=(const SocketBase&) = delete;

			SocketBase(SocketBase&&) = default;
			SocketBase& operator=(SocketBase&&) = default;

			virtual ~SocketBase() {
				if (state == CONNECTING || state == CONNECTED) destroy();
				reportBeingDestructed(); 
				unref(); /*or assert that is must already be unrefed*/
			}

		public:

			const Address& address() const { return _local; }

			size_t bufferSize() const { return dataForCommandProcessing.writeBuffer.used_size(); }
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			bool connecting() const { return state == CONNECTING; }
			void destroy();
			bool destroyed() const { return state == DESTROYED; };
			void end();
			const std::string& localAddress() const { return _local.address; }
			uint16_t localPort() const { return _local.port; }


			const std::string& remoteAddress() const { return _remote.address; }
			const std::string& remoteFamily() const { return _remote.family; }
			uint16_t remotePort() const { return _remote.port; }

//			void ref() { dataForCommandProcessing.refed = true; }
//			void resume() { dataForCommandProcessing.paused = false; }
//			void unref() { dataForCommandProcessing.refed = false; }
//			void pause() { dataForCommandProcessing.paused = true; }
			void ref();
			void unref();
			void pause();
			void resume();
			void reportBeingDestructed();

			bool write(const uint8_t* data, uint32_t size);
			bool write(Buffer& buff) { return write( buff.begin(), (uint32_t)(buff.size()) ); }

			bool write2(Buffer& b);
		};

	} //namespace net

} //namespace nodecpp

#endif //NET_COMMON_H
