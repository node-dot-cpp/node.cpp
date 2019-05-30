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
		std::unique_ptr<uint8_t[]> _data;

	private:
		void ensureCapacity(size_t sz) { // NOTE: may invalidate pointers
			if (_data == nullptr) {
				reserve(sz);
			}
			else if (sz > _capacity) {
				size_t cp = std::max(sz, MIN_BUFFER);
//				std::unique_ptr<uint8_t[]> tmp(static_cast<uint8_t*>(malloc(cp)));
				std::unique_ptr<uint8_t[]> tmp(new uint8_t[cp]);
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
//			std::unique_ptr<uint8_t[]> tmp(static_cast<uint8_t*>(malloc(cp)));
			std::unique_ptr<uint8_t[]> tmp(new uint8_t[cp]);

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
				std::unique_ptr<uint8_t[]> tmp(static_cast<uint8_t*>(malloc(cp)));


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
		size_t max_allowed_size_exp = 32;
		size_t size_exp;
		uint8_t* begin = nullptr;
		uint8_t* end = nullptr;

		bool resize_up_and_append( const uint8_t* data, size_t data_size) {
			// TODO: introduce upper limit and make this call bool
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff != nullptr );
			size_t total_sz = used_size() + data_size;
			size_t new_size_exp = size_exp + 1;
			while ( (((size_t)1) << new_size_exp) < total_sz + 1 )
			{
				// TODO: add upper bound control
				++new_size_exp;
				if ( new_size_exp > max_allowed_size_exp )
					return false;
			}
			size_t new_alloc_size = ((size_t)1) << new_size_exp;
//			std::unique_ptr<uint8_t[]> new_buff = std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(malloc( new_alloc_size )));
			std::unique_ptr<uint8_t[]> new_buff( new uint8_t[new_alloc_size] );
			size_t sz = 0;
			if ( begin <= end )
			{
				sz = end - begin;
				memcpy( new_buff.get(), begin, sz );
			}
			else
			{
				sz = buff.get() + alloc_size() - begin;
				memcpy( new_buff.get(), begin, sz );
				memcpy( new_buff.get() + sz, buff.get(), end - buff.get() );
				sz += end - buff.get();
			}
			buff = std::move( new_buff );
			size_exp = new_size_exp;
			begin = buff.get();
			end = begin + sz;

			memcpy( end, data, data_size );
			end += data_size;

			return true;
		}

	public:
		CircularByteBuffer(size_t sz_exp = 16) { 
			size_exp = sz_exp; 
//			buff = std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(malloc(alloc_size())));
			buff = std::unique_ptr<uint8_t[]>(new uint8_t[alloc_size()]);
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
			return *this;
		}
		size_t used_size() const { return begin <= end ? end - begin : alloc_size() - (begin - end); }
		size_t remaining_capacity() const { return alloc_size() - 1 - used_size(); }
		bool empty() const { return begin == end; }
		size_t alloc_size() const { return ((size_t)1)<<size_exp; }

		// writer-related
		bool append( const uint8_t* ptr, size_t sz ) { 
			if ( sz > remaining_capacity() )
			{
				//return false;
				return resize_up_and_append( ptr, sz );
			}

			size_t fwd_free_sz = buff.get() + alloc_size() - end;
			if ( sz <= fwd_free_sz )
			{
				memcpy( end,  ptr, sz );
				end += sz;
				if ( buff.get() + alloc_size() == end )
					end = buff.get();
			}
			else
			{
				memcpy( end,  ptr, fwd_free_sz );
				memcpy( buff.get(),  ptr + fwd_free_sz, sz - fwd_free_sz );
				end = buff.get() + sz - fwd_free_sz;
			}
			return true; 
		}

		template<class Writer>
		void write( Writer& writer, size_t& bytesWritten ) {
			bytesWritten = 0;
			if ( begin < end )
			{
				writer.write( begin, end - begin, bytesWritten );
				begin += bytesWritten;
			}
			else if ( begin > end )
			{
				size_t sz2write = buff.get() + alloc_size() - begin;
				bool can_continue = writer.write( end, sz2write, bytesWritten );
				begin += bytesWritten;
				bool till_end = begin == (buff.get() + alloc_size());
				if( till_end )
					begin = buff.get();
				if (!can_continue || !till_end)
					return;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, begin == buff.get() );
				if ( begin != end )
				{
					size_t bw = 0;
					writer.write( begin, end - begin, bw );
					begin += bw;
					bytesWritten += bw;
				}
			}
		}

		// reader-related
		/*bool get_ready_data( Buffer& b, size_t minsz ) {
			if ( used_size() < minsz )
				return false;
			get_ready_data( b );
			return true;
		}*/
		void get_ready_data( Buffer& b ) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, b.size() == 0 );
			if ( begin <= end )
			{
				size_t sz2copy = b.capacity() >= end - begin ? end - begin : b.capacity();
				b.append( begin, sz2copy );
				begin += sz2copy;
			}
			else
			{
				size_t sz2copy = buff.get() + alloc_size() - begin;
				if ( sz2copy > b.capacity() )
				{
					b.append( begin, b.capacity() );
					begin += b.capacity();
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, begin < buff.get() + alloc_size() );
				}
				else if ( sz2copy < b.capacity() )
				{
					b.append( begin, sz2copy );
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, begin + sz2copy == buff.get() + alloc_size() );
					begin = buff.get();
					size_t sz2copy2 = b.capacity() - sz2copy;
					if ( sz2copy2 > end - begin )
						sz2copy2 = end - begin;
					b.append( begin, sz2copy2 );
					begin += sz2copy2;
				}
				else
				{
					b.append( begin, sz2copy );
					begin = buff.get();
				}
			}
		}

		template<class Reader>
		void read( Reader& reader, size_t& bytesRead, size_t target_sz ) {
			bytesRead = 0;
			if ( begin > end )
			{
				reader.read( end, begin - end - 1, bytesRead );
				end += bytesRead;
			}
			else
			{
				size_t sz2read = buff.get() + alloc_size() - end;
				bool can_continue = reader.read( end, sz2read, bytesRead );
				end += bytesRead;
				bool till_end = end == (buff.get() + alloc_size());
				if( till_end )
					end = buff.get();
				if (!can_continue || !till_end )
					return;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, end == buff.get() );
				if ( buff.get() + alloc_size() - begin >= target_sz )
					return;
				if ( begin - end > 1 )
				{
					size_t br = 0;
					reader.read( end, begin - end - 1, br );
					end += br;
					bytesRead += br;
				}
			}
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

		struct awaitable_handle_data
		{
			std::experimental::coroutine_handle<> h = nullptr;
			bool is_exception = false;
			std::exception exception; // TODO: consider possibility of switching to nodecpp::error
		};

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

				//std::experimental::coroutine_handle<> h_read = nullptr;
				//std::experimental::coroutine_handle<> h_write = nullptr;
				struct awaitable_read_handle_data : public awaitable_handle_data
				{
					size_t min_bytes;
				};
				struct awaitable_write_handle_data : public awaitable_handle_data
				{
					Buffer b;
				};

				// NOTE: make sure all of them are addressed at forceResumeWithThrowing()
				awaitable_handle_data ahd_connect;
				awaitable_handle_data ahd_accepted;
				awaitable_read_handle_data ahd_read;
				awaitable_write_handle_data ahd_write;
				awaitable_handle_data ahd_drain;

			//	bool connecting = false;
				bool remoteEnded = false;
				//bool localEnded = false;
				//bool pendingLocalEnd = false;
				bool paused = false;
				bool allowHalfOpen = true;

				bool refed = false;

				//Buffer writeBuffer = Buffer(64 * 1024);
				CircularByteBuffer writeBuffer = CircularByteBuffer( 16 );
				CircularByteBuffer readBuffer = CircularByteBuffer( 16 );
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
			void connect(uint16_t port, const char* ip);


		//private:
			auto a_connect(uint16_t port, const char* ip) { 

				struct connect_awaiter {
					SocketBase& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					connect_awaiter(SocketBase& socket_) : socket( socket_ ) {}

					connect_awaiter(const connect_awaiter &) = delete;
					connect_awaiter &operator = (const connect_awaiter &) = delete;
	
					~connect_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						who_is_awaiting = awaiting;
						socket.dataForCommandProcessing.ahd_connect.h = who_is_awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_connect.is_exception )
						{
							socket.dataForCommandProcessing.ahd_connect.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_connect.exception;
						}
					}
				};
				connect( port, ip );
				return connect_awaiter(*this);
			}

			auto a_read( Buffer& buff, size_t min_bytes = 1 ) { 

				buff.clear();
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.capacity() >= min_bytes, "indeed: {} vs. {} bytes", buff.capacity(), min_bytes );

				struct read_data_awaiter {
					SocketBase& socket;
					Buffer& buff;
					size_t min_bytes;

					read_data_awaiter(SocketBase& socket_, Buffer& buff_, size_t min_bytes_) : socket( socket_ ), buff( buff_ ), min_bytes( min_bytes_ ) {}

					read_data_awaiter(const read_data_awaiter &) = delete;
					read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
					~read_data_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = min_bytes;
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_read.is_exception )
						{
							socket.dataForCommandProcessing.ahd_read.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_read.exception;
						}
						socket.dataForCommandProcessing.readBuffer.get_ready_data( buff );
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes);
					}
				};
				return read_data_awaiter(*this, buff, min_bytes);
			}

			auto a_write(Buffer& buff) { 

				struct write_data_awaiter {
					SocketBase& socket;
					Buffer& buff;
					bool write_ok = false;

					std::experimental::coroutine_handle<> who_is_awaiting;

					write_data_awaiter(SocketBase& socket_, Buffer& buff_) : socket( socket_ ), buff( buff_ )  {}

					write_data_awaiter(const write_data_awaiter &) = delete;
					write_data_awaiter &operator = (const write_data_awaiter &) = delete;
	
					~write_data_awaiter() {}

					bool await_ready() {
						write_ok = socket.write2( buff ); // so far we do it sync TODO: extend implementation for more complex (= requiring really async processing) cases
						return write_ok; // false means waiting (incl. exceptional cases)
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !write_ok ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_write.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_write.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_write.exception;
						}
					}
				};
				return write_data_awaiter(*this, buff);
			}

			auto a_drain() { 

				struct drain_awaiter {
					SocketBase& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					drain_awaiter(SocketBase& socket_) : socket( socket_ )  {}

					drain_awaiter(const drain_awaiter &) = delete;
					drain_awaiter &operator = (const drain_awaiter &) = delete;
	
					~drain_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.writeBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !socket.dataForCommandProcessing.writeBuffer.empty() ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_drain.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_drain.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_drain.exception;
						}
					}
				};
				return drain_awaiter(*this);
			}
		};

	} //namespace net

} //namespace nodecpp

#endif //NET_COMMON_H
