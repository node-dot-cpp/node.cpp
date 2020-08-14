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

#ifndef BASIC_COLLECTIONS_H
#define BASIC_COLLECTIONS_H

#include <utility>

#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <array>
#include <algorithm>

#include <malloc_based_allocator.h>
#include <safe_memory/safe_ptr.h>
#include <safe_memory/vector.h>
#include <safe_memory/unordered_map.h>
#include <safe_memory/string.h>
#include <safe_memory/string_literal.h>
#include <safe_memory/string_format.h>


namespace nodecpp
{
	template<class T>
	using vector = ::safe_memory::vector<T>;
	// template<class T>
	// using vector = ::std::vector<T, nodecpp::safememory::iiballocator<T>>;

	template<class T>
	using stdvector = ::std::vector<T, nodecpp::stdallocator<T>>;

	// template<class Key, class T>
	// using map = ::safe_memory::unordered_map<Key, T>;
	template<class Key, class T>
	using map = ::std::map<Key, T, std::less<Key>, nodecpp::safememory::iiballocator<std::pair<const Key,T>>>;

	template<class Key, class T>
	using stdmap = ::std::map<Key, T, std::less<Key>, nodecpp::stdallocator<std::pair<const Key,T>>>;

	using string = ::safe_memory::string;
	// using string = ::std::basic_string<char, std::char_traits<char>, nodecpp::safememory::iiballocator<char>>;

	using stdstring = ::std::basic_string<char, std::char_traits<char>, nodecpp::stdallocator<char>>;

	using string_literal = ::safe_memory::string_literal;


	template <typename... Args>
	inline nodecpp::string format(
		const char* format_str, const Args &... args) {
		nodecpp::string s;
	  ::fmt::format_to( std::back_inserter(s), format_str, args... );
	  return s;
	}

	template <typename... Args>
	inline nodecpp::string format(
		const nodecpp::string& format_str, const Args &... args) {
		nodecpp::string s;
	  ::fmt::format_to( std::back_inserter(s), format_str.c_str(), args... );
	  return s;
	}

	template <typename... Args>
	inline nodecpp::string format(
		const nodecpp::string_literal& format_str, const Args &... args) {
		nodecpp::string s;
	  ::fmt::format_to( std::back_inserter(s), format_str.c_str(), args... );
	  return s;
	}

	template<class T>
	T* alloc( size_t count ) {
		nodecpp::safememory::iiballocator<T> iiball;
		T* ret = iiball.allocate( count );
		for ( size_t i=0; i<count; ++i )
			new (ret + i) T();
		return ret;
	}

	template<class T>
	void dealloc( T* ptr, size_t count ) {
		for ( size_t i=0; i<count; ++i )
			(ptr + i)->~T();
		nodecpp::safememory::iiballocator<T> iiball;
		iiball.deallocate( ptr, count );
	}

	template<class T>
	T* stdalloc( size_t count ) {
		nodecpp::stdallocator<T> stdall;
		T* ret = stdall.allocate( count );
		for ( size_t i=0; i<count; ++i )
			new (ret + i) T();
		return ret;
	}

	template<class T>
	void stddealloc( T* ptr, size_t count ) {
		for ( size_t i=0; i<count; ++i )
			(ptr + i)->~T();
		nodecpp::stdallocator<T> stdall;
		stdall.deallocate( ptr, count );
	}
} // nodecpp


#endif // BASIC_COLLECTIONS_H
