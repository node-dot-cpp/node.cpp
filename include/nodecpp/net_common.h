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
#include "ip_and_port.h"
#include "timers.h"
#include <map>
#include <typeinfo>
#include <typeindex>

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
			std::unique_ptr<uint8_t[]> tmp(new uint8_t[cp]);

			_capacity = cp;
			_data = std::move(tmp);
		}

		void append(const void* dt, size_t sz) { // NOTE: may invalidate pointers
			ensureCapacity(_size + sz);
			memcpy(end(), dt, sz);
			_size += sz;
		}

		void append(const Buffer& b) { // NOTE: may invalidate pointers
			append( b.begin(), b.size() );
		}

		void append(const Buffer& b, size_t offset) { // NOTE: may invalidate pointers
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset <= b.size() ); 
			append( b.begin() + offset, b.size() - offset );
		}

		void append(const Buffer& b, size_t start, size_t count) { // NOTE: may invalidate pointers
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !b.empty() ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, start + count <= b.size() ); 
			append( b.begin() + start, count );
		}

		void trim(size_t sz) { // NOTE: keeps pointers
			assert(sz <= _size);
			assert(_data != nullptr || (_size == 0 && sz == 0) );
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
		uint32_t readUInt8(size_t offset) const {
			assert( offset + 1 <= _size );
			return *(begin() + offset);
		}
		uint32_t readUInt32BE(size_t offset) const {
			assert( false ); // TODO: implement
			return 0;
		}
		uint8_t readUInt32LE(size_t offset) const {
			assert( offset + 4 <= _size );
			return *reinterpret_cast<const uint32_t*>(begin() + offset);
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

		// direct access to data
		struct AvailableDataDescriptor
		{
			uint8_t* ptr1;
			uint8_t* ptr2;
			size_t sz1;
			size_t sz2;
		};
		void get_available_data(AvailableDataDescriptor& d)
		{
			d.ptr1 = begin;
			if ( begin < end )
			{
				d.sz1 = end - begin;
				d.ptr2 = nullptr;
				d.sz2 = 0;
			}
			else if ( begin > end )
			{
				d.sz1 = buff.get() + alloc_size() - begin;
				d.ptr2 = buff.get();
				d.sz2 = end - buff.get();
			}
			else
			{
				d.ptr1 = nullptr;
				d.sz1 = 0;
				d.ptr2 = nullptr;
				d.sz2 = 0;
			}
		}

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
			get_ready_data( b, b.capacity() );
		}
		void get_ready_data( Buffer& b, size_t bytes2read ) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, b.size() == 0 );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, bytes2read <= b.capacity(), "indeed: {} vs. {}", bytes2read, b.capacity() );

			if ( begin <= end )
			{
				size_t diff = (size_t)(end - begin);
				size_t sz2copy = bytes2read >= diff ? diff : bytes2read;
				b.append( begin, sz2copy );
				begin += sz2copy;
			}
			else
			{
				size_t sz2copy = buff.get() + alloc_size() - begin;
				if ( sz2copy > bytes2read )
				{
					b.append( begin, bytes2read );
					begin += bytes2read;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin < buff.get() + alloc_size() );
				}
				else if ( sz2copy < bytes2read )
				{
					b.append( begin, sz2copy );
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin + sz2copy == buff.get() + alloc_size() );
					begin = buff.get();
					size_t sz2copy2 = bytes2read - sz2copy;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
					size_t diff = (size_t)(end - begin);
					if ( sz2copy2 > diff )
						sz2copy2 = diff;
					b.append( begin, sz2copy2 );
					begin += sz2copy2;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
				}
				else
				{
					b.append( begin, sz2copy );
					begin = buff.get();
				}
			}
		}
		void skip_data( size_t bytes2skip ) // "read" without reading
		{
			if ( begin <= end )
			{
				size_t diff = (size_t)(end - begin);
				size_t sz2skip = bytes2skip >= diff ? diff : bytes2skip;
				begin += sz2skip;
			}
			else
			{
				size_t sz2skip = buff.get() + alloc_size() - begin;
				if ( sz2skip > bytes2skip )
				{
					begin += bytes2skip;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin < buff.get() + alloc_size() );
				}
				else if ( sz2skip < bytes2skip )
				{
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin + sz2skip == buff.get() + alloc_size() );
					begin = buff.get();
					size_t sz2skip2 = bytes2skip - sz2skip;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
					size_t diff = (size_t)(end - begin);
					if ( sz2skip2 > diff )
						sz2skip2 = diff;
					begin += sz2skip2;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, begin <= end );
				}
				else
				{
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
				uint8_t* endpoint = begin != buff.get() ? buff.get() + alloc_size() : buff.get() + alloc_size() - 1;
				size_t sz2read = endpoint - end;
				bool can_continue = reader.read( end, sz2read, bytesRead );
				end += bytesRead;
				bool till_end = end == (buff.get() + alloc_size());
				if( till_end )
					end = buff.get();
				if (!can_continue || !till_end )
					return;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, end == buff.get() );
				if ( buff.get() + alloc_size() >= begin + target_sz )
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
		nodecpp::vector<owning_ptr<ItemT>> items;
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
			Ip4 ip;

			bool operator == ( const Address& other ) { return port == other.port && ip == other.ip && family == other.family; }
		};

//		enum Mode { callable, awaitable };

		template<class ObjectT, auto memberFunc>
		struct HandlerData
		{
			using ObjT = ObjectT;
			static constexpr auto memberFn = memberFunc;
		};

		template<class FnT>
		struct UserDefHandlersBase
		{
			struct HandlerInstance
			{
				FnT handler = nullptr;
				void *object = nullptr;
			};
//			std::vector<HandlerInstance, nodecpp::safememory::stdallocator<HandlerInstance>> handlers;
			std::vector<HandlerInstance> handlers;

			bool willHandle() { return handlers.size(); }
			void from(const UserDefHandlersBase<FnT>& patternUH, void* defaultObjPtr)
			{
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers.size() == 0);
				handlers = patternUH.handlers;
				for (size_t i = 0; i < handlers.size(); ++i)
					if (handlers[i].object == nullptr)
						handlers[i].object = defaultObjPtr;
			}
		};

		template<class FnT>
		struct UserDefHandlersWithOptimizedStorage
		{
			struct HandlerInstance
			{
				FnT handler = nullptr;
				void *object = nullptr;
			};
			using HandlerInstanceVectorT = nodecpp::vector<HandlerInstance>;
			enum Type { uninitialized, zero, one, two, many };
			Type type = Type::uninitialized;
			static constexpr size_t fixed_size = 2;
			uint8_t basebytes[ sizeof( HandlerInstanceVectorT ) > sizeof( HandlerInstance ) * fixed_size ? sizeof( HandlerInstanceVectorT ) : sizeof( HandlerInstance ) * fixed_size ];
			HandlerInstance* handlers_a() { return reinterpret_cast<HandlerInstance*>(basebytes); }
			HandlerInstanceVectorT& handlers_v() { return *reinterpret_cast<HandlerInstanceVectorT*>(basebytes); }


			bool willHandle() { 
				return type != Type::uninitialized && type != Type::zero; 
			}
			void from(const UserDefHandlersBase<FnT>& patternUH, void* defaultObjPtr)
			{
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::uninitialized);
				if ( patternUH.handlers.size() == 0 )
				{
					type = Type::zero;
					return;
				}
				else if ( patternUH.handlers.size() == 1 )
				{
					static_assert( fixed_size >= 1 );
					type = Type::one;
					handlers_a()[0].handler = patternUH.handlers[0].handler;
					handlers_a()[0].object = patternUH.handlers[0].object != nullptr ? patternUH.handlers[0].object : defaultObjPtr;
					return;
				}
				else if ( patternUH.handlers.size() == 2 )
				{
					static_assert( fixed_size >= 2 );
					type = Type::two;
					handlers_a()[0].handler = patternUH.handlers[0].handler;
					handlers_a()[0].object = patternUH.handlers[0].object != nullptr ? patternUH.handlers[0].object : defaultObjPtr;
					handlers_a()[1].handler = patternUH.handlers[1].handler;
					handlers_a()[1].object = patternUH.handlers[1].object != nullptr ? patternUH.handlers[1].object : defaultObjPtr;
					return;
				}
				else
				{
					static_assert( fixed_size <= 2 );
					type = Type::many;
					new(basebytes)HandlerInstanceVectorT();
					handlers_v().resize( patternUH.handlers.size() );
					for (size_t i = 0; i < handlers_v().size(); ++i)
					{
						handlers_v()[i].handler = patternUH.handlers[i].handler;
						handlers_v()[i].object = patternUH.handlers[i].object != nullptr ? patternUH.handlers[i].object : defaultObjPtr;
					}
				}
			}
			template<class ... ARGS>
			void execute(ARGS&& ... args) { 
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type != Type::uninitialized );
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type != Type::zero );
				if ( type == Type::one )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[0].object != nullptr ); 
					handlers_a()[0].handler(handlers_a()[0].object, ::std::forward<ARGS>(args)...);
				}
				else if ( type == Type::two )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[0].object != nullptr ); 
					handlers_a()[0].handler(handlers_a()[0].object, ::std::forward<ARGS>(args)...);
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, handlers_a()[1].object != nullptr ); 
					handlers_a()[1].handler(handlers_a()[1].object, ::std::forward<ARGS>(args)...);
				}
				else
				{
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, type == Type::many );
					for (auto h : handlers_v()) 
					{
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.object != nullptr ); 
						h.handler(h.object, ::std::forward<ARGS>(args)...);
					}
				}
			}
			~UserDefHandlersWithOptimizedStorage() {
				if ( type == Type::many ) {
					using myvectort = HandlerInstanceVectorT;
					handlers_v().~myvectort();
				}
			}
		};

		template<class FnT>
		struct UserDefHandlers : public UserDefHandlersBase<FnT>
		{
			template<class ObjectT>
			void add( ObjectT* object, FnT handler )
			{
				for (auto h : this->handlers)
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.handler != handler || h.object != object, "already added" );
				typename UserDefHandlersBase<FnT>::HandlerInstance inst;
				inst.object = object;
				inst.handler = handler;
				this->handlers.push_back(inst);
			}
			void add(FnT handler)
			{
				for (auto h : this->handlers)
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.handler != handler || h.object != nullptr, "already added");
				typename UserDefHandlersBase<FnT>::HandlerInstance inst;
				inst.object = nullptr;
				inst.handler = handler;
				this->handlers.push_back(inst);
			}
			template<class ObjectT>
			void remove(ObjectT* object, FnT handler)
			{
				for (size_t i=0; i<this->handlers.size(); ++i)
					if (this->handlers[i].handler == handler && this->handlers[i].object == object)
					{
						this->handlers.erase(this->handlers.begin() + i);
						return;
					}
			}
			void remove(FnT handler)
			{
				for (size_t i = 0; i < this->handlers.size(); ++i)
					if (this->handlers[i].handler == handler && this->handlers[i].object == nullptr)
					{
						this->handlers.erase(this->handlers.begin() + i);
						return;
					}
			}
		};

		template<class UserHandlerType>
		class UserHandlerClassPatterns
		{
//			using MapType = std::map<std::type_index, std::pair<UserHandlerType, bool>>;
//			using MapType = std::map<std::type_index, std::pair<UserHandlerType, bool>, std::less<std::type_index>, nodecpp::safememory::stdallocator<std::pair<const std::type_index, std::pair<UserHandlerType, bool>>>>;
			using MapType = nodecpp::map<std::type_index, std::pair<UserHandlerType, bool>>;
#ifndef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			MapType _patterns;
			MapType& patterns() { return _patterns; }
#else
			uint8_t mapbytes[sizeof(MapType)];
			MapType& patterns() { return *reinterpret_cast<MapType*>(mapbytes); }
#endif
			std::pair<UserHandlerType, bool>& getPattern( std::type_index idx )
			{
				auto pattern = patterns().find( idx );
				if ( pattern != patterns().end() )
					return pattern->second;
				else
				{
//					interceptNewDeleteOperators(false);
					auto ins = patterns().insert( make_pair( idx, std::make_pair(UserHandlerType(), false) ) );
//					interceptNewDeleteOperators(true);
					return ins.first->second;
				}
			}
		public:
#ifdef NODECPP_THREADLOCAL_INIT_BUG_GCC_60702
			void init()
			{
				new(&(patterns()))MapType();
			}
			void destroy()
			{
				patterns().~MapType();
			}
#endif
			template<class UserClass>
			UserHandlerType& getPatternForUpdate()
			{
				auto& ret = getPattern( std::type_index(typeid(UserClass)) );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !ret.second, "handlers must be added before any instance is created" );
				return ret.first;
			}
			template<class UserClass>
			const UserHandlerType& getPatternForApplying()
			{
				auto& ret = getPattern( std::type_index(typeid(UserClass)) );
				ret.second = true;
				return ret.first;
			}
			const UserHandlerType& getPatternForApplying(std::type_index idx)
			{
				auto& ret = getPattern( idx );
				ret.second = true;
				return ret.first;
			}
		};
	} //namespace net

	template<class DataParentT>
	class DataParent
	{
		DataParentT* parent = nullptr;
	public:
		DataParent() {}
		DataParent(DataParentT* parent_) { parent = parent_; }
		DataParentT* getDataParent() { return parent; }
	};

} //namespace nodecpp

#endif //NET_COMMON_H
