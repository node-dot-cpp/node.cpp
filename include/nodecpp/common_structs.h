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

#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include "common.h"
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
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _size == 0 ); 
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _capacity == 0 );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _data == nullptr );

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
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz <= _size );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _data != nullptr || (_size == 0 && sz == 0) );
			_size -= sz;
		}

		void clear() {
			trim(size());
		}

		void set_size(size_t sz) { // NOTE: keeps pointers
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz <= _capacity );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, _data != nullptr );
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
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, sz <= size() );
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
		/*size_t appendString( const char* str, size_t sz ) { // TODO: revision required
			if ( str )
			{
				ensureCapacity(size() + sz + 1);
				memcpy(begin() + size(), str, sz );
				_size += sz;
			}
			*reinterpret_cast<uint8_t*>(begin() + size() ) = 0;
			return ++_size;
		}*/
		size_t appendString( const nodecpp::string& str ) {
			append( str.c_str(), str.size() );
			return _size;
		}
		size_t appendString( nodecpp::string_literal str ) {
			append( str.c_str(), strlen( str.c_str() ) );
			return _size;
		}

		// an attempt to follow node.js buffer-related interface
		void writeUInt32LE(uint8_t value, size_t offset) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + 1 <= _capacity );
			*(begin() + size()) = value;
			if ( _size < offset + 4 )
				_size = offset + 4;
		}
		void writeUInt32BE(uint32_t value, size_t offset) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false ); // TODO: implement
		}
		void writeUInt32LE(uint32_t value, size_t offset) {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + 4 <= _capacity );
			memcpy(begin() + size(), &value, 4 );
			if ( _size < offset + 4 )
				_size = offset + 4;
		}
		uint32_t readUInt8(size_t offset) const {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + 1 <= _size );
			return *(begin() + offset);
		}
		uint32_t readUInt32BE(size_t offset) const {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false ); // TODO: implement
			return 0;
		}
		uint8_t readUInt32LE(size_t offset) const {
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, offset + 4 <= _size );
			return *reinterpret_cast<const uint32_t*>(begin() + offset);
		}
	};

	template <class ItemT>
	class MultiOwner
	{
		nodecpp::vector<owning_ptr<ItemT>> items;
		size_t cnt;
	public:
		soft_ptr<ItemT> add(owning_ptr<ItemT>&& item)
		{
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

} //namespace nodecpp

#endif // COMMON_STRUCTS_H
