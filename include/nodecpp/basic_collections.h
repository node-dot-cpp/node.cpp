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

//#define NODECPP_USE_SAFE_MEMORY_CONTAINERS

#ifdef NODECPP_USE_SAFE_MEMORY_CONTAINERS
#include <safememory/safe_ptr.h>
#include <safememory/vector.h>
#include <safememory/unordered_map.h>
#include <safememory/string.h>
#include <safememory/string_literal.h>
#include <safememory/string_format.h>
#endif // NODECPP_USE_SAFE_MEMORY_CONTAINERS

namespace nodecpp
{
	template<class T>
	using stdvector = ::std::vector<T, nodecpp::stdallocator<T>>;

	template<class Key, class T>
	using stdmap = ::std::map<Key, T, std::less<Key>, nodecpp::stdallocator<std::pair<const Key,T>>>;

	using stdstring = ::std::basic_string<char, std::char_traits<char>, nodecpp::stdallocator<char>>;

#ifdef NODECPP_USE_SAFE_MEMORY_CONTAINERS

	template<class T>
	using vector = ::safememory::vector<T>;

	template<class T1, class T2>
	using pair = eastl::pair<T1, T2>;

	template< class T1, class T2 >
	auto make_pair( T1&& t1, T2&& t2 ) {
		return eastl::make_pair(std::forward<T1>(t1), std::forward<T2>(t2));
	}

	template<class Key, class T, class Hash = safememory::hash<Key>, class Predicate = safememory::equal_to<Key>>
	using map = ::safememory::unordered_map<Key, T, Hash, Predicate>;

	using string = ::safememory::string;

	using string_literal = ::safememory::string_literal;


#else

	template<class T>
	using vector = ::std::vector<T, safememory::detail::iiballocator<T>>;

	template<class T1, class T2>
	using pair = std::pair<T1, T2>;

	template< class T1, class T2 >
	auto make_pair( T1&& t1, T2&& t2 ) {
		return std::make_pair(std::forward<T1>(t1), std::forward<T2>(t2));
	}

	template<class Key, class T, class Less = std::less<Key>>
	using map = ::std::map<Key, T, Less, safememory::detail::iiballocator<std::pair<const Key,T>>>;


	using string = ::std::basic_string<char, std::char_traits<char>, safememory::detail::iiballocator<char>>;

	template<typename T>
	class basic_string_literal
	{
	public:
		typedef T                                                         value_type;
		typedef const T&                                                  const_reference;
		typedef std::size_t                                               size_type;

	private:
		const value_type* str = nullptr;
		size_type sz = 0;

	public:
		basic_string_literal() {}
		
		template<size_type N>
		basic_string_literal(const value_type (&ptr)[N]) : str(ptr), sz(N - 1) {
			static_assert(N >= 1);
			static_assert(N < std::numeric_limits<size_type>::max());
		}

		basic_string_literal(const basic_string_literal& other) = default;
		basic_string_literal& operator=(const basic_string_literal& other) = default;
		basic_string_literal(basic_string_literal&& other) = default;
		basic_string_literal& operator=(basic_string_literal&& other) = default;

		~basic_string_literal() = default; // string literals have infinite lifetime, no need to zero pointers

		constexpr bool empty() const noexcept { return sz == 0; }
		constexpr size_type size() const noexcept { return sz; }

		constexpr const T* c_str() const noexcept { return str; }
		constexpr const T* data() const noexcept { return str; }

		std::basic_string_view<T> to_string_view_unsafe() const {
			return std::basic_string_view<T>(data(), size());
		}
	};

	typedef basic_string_literal<char>    string_literal;
	typedef basic_string_literal<wchar_t> wstring_literal;

	/// string8 / string16 / string32
	// typedef basic_string_literal<char8_t>  u8string_literal;
	typedef basic_string_literal<char16_t> u16string_literal;
	typedef basic_string_literal<char32_t> u32string_literal;


	template<class T>
	bool operator==( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator==(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

	template<class T>
	bool operator!=( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator!=(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

	template<class T>
	bool operator<( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator<(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

	template<class T>
	bool operator<=( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator<=(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

	template<class T>
	bool operator>( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator>(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

	template<class T>
	bool operator>=( const basic_string_literal<T>& a, const basic_string_literal<T>& b ) {
		return std::operator>=(a.to_string_view_unsafe(), b.to_string_view_unsafe());
	}

#endif // NODECPP_USE_SAFE_MEMORY_CONTAINERS

	// inline string_literal operator"" _s(const char* str, size_t len) noexcept { return {str}; }


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
		safememory::detail::iiballocator<T> iiball;
		T* ret = iiball.allocate( count );
		for ( size_t i=0; i<count; ++i )
			new (ret + i) T();
		return ret;
	}

	template<class T, class ... Args>
	T* alloc( size_t count,  Args&& ... args ) {
		safememory::detail::iiballocator<T> iiball;
		T* ret = iiball.allocate( count );
		for ( size_t i=0; i<count; ++i )
			new (ret + i) T( std::forward<Args>( args )...);
		return ret;
	}

	template<class T>
	void dealloc( T* ptr, size_t count ) {
		for ( size_t i=0; i<count; ++i )
			(ptr + i)->~T();
		safememory::detail::iiballocator<T> iiball;
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

	template<class T, class ... Args>
	T* stdalloc( size_t count,  Args&& ... args) {
		nodecpp::stdallocator<T> stdall;
		T* ret = stdall.allocate( count );
		for ( size_t i=0; i<count; ++i )
			new (ret + i) T( std::forward<Args>( args )...);
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
