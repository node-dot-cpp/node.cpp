/* -------------------------------------------------------------------------------
* Copyright (c) 2018, OLogN Technologies AG
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

#ifndef COMPOSE_AND_PARSE_IMPL_H
#define COMPOSE_AND_PARSE_IMPL_H

#include <type_traits>
#include "../../../../include/nodecpp/common_structs.h" // for Buffer
#include <assert.h> // TODO: replace by nodecpp assertion system

using Buffer = nodecpp::Buffer;
using String = nodecpp::string;

namespace m::impl {

static constexpr size_t integer_max_size = 8;

// supported types
struct SignedIntegralType {static constexpr bool dummy = false;};
struct UnsignedIntegralType {int d = 0;};
struct StringType {static constexpr bool dummy = false;};
struct BlobType {static constexpr bool dummy = false;};
struct ByteArrayType {static constexpr bool dummy = false;};
struct EnumType {static constexpr bool dummy = false;};

// helper types

struct StringLiteralForComposing
{
	const char* const str;
	const size_t size;
	constexpr StringLiteralForComposing( const char* const str_, const size_t size_ ) : str( str_ ), size( size_ ) {}
};


// composing

template <typename T>
void composeSignedInteger( Buffer& b, T num )
{
	static_assert( std::is_integral<T>::value );
	if constexpr ( std::is_unsigned<T>::value && sizeof( T ) >= integer_max_size )
	{
		assert( num <= INT64_MAX );
	}
	/*temporary solution TODO: actauls implementation*/ { 
		int64_t val = num; 
		b.append( &val, sizeof( val ) );
		printf( "composeSignedInteger(%zd\n", val );
	}
}

template <typename T>
void composeUnsignedInteger( Buffer& b, T num )
{
	if constexpr ( std::is_signed<T>::value )
	{
		assert( num >= 0 );
	}
	/*temporary solution TODO: actauls implementation*/ { 
		uint64_t val = num; 
		b.append( &val, sizeof( val ) );
		printf( "composeSignedInteger(%zd\n", val );
	}
}

inline
void composeString( Buffer& b, const nodecpp::string& str )
{
	b.appendString( str );
	b.appendUint8( 0 );
	printf( "composeString(nodecpp::string \"%s\"\n", str.c_str() );
}

inline
void composeString( Buffer& b, const StringLiteralForComposing* str )
{
	b.append( str->str, str->size );
	b.appendUint8( 0 );
	printf( "composeString(StringLiteralForComposing \"%s\"\n", str->str );
}

inline
void composeString( Buffer& b, std::string str )
{
	b.append( str.c_str(), str.size() );
	b.appendUint8( 0 );
	printf( "composeString(std::string \"%s\"\n", str.c_str() );
}

inline
void composeString( Buffer& b, const char* str )
{
	size_t sz = strlen( str );
	b.append( str, sz );
	b.appendUint8( 0 );
	printf( "composeString(const char* \"%s\"\n", str );
}

namespace json
{

inline
void addNamePart( Buffer& b, nodecpp::string name )
{
	b.appendUint8( '\"' );
	b.appendString( name );
	b.appendUint8( '\"' );
	b.appendUint8( ':' );
	b.appendUint8( ' ' );
}

template <typename T>
void composeSignedInteger( Buffer& b, nodecpp::string name, T num )
{
	static_assert( std::is_integral<T>::value );
	if constexpr ( std::is_unsigned<T>::value && sizeof( T ) >= integer_max_size )
	{
		assert( num <= INT64_MAX );
	}
	addNamePart( b, name );
	b.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

template <typename T>
void composeUnsignedInteger( Buffer& b, nodecpp::string name, T num )
{
	if constexpr ( std::is_signed<T>::value )
	{
		assert( num >= 0 );
	}
	addNamePart( b, name );
	b.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

inline
void composeString( Buffer& b, nodecpp::string name, const nodecpp::string& str )
{
	addNamePart( b, name );
	b.appendUint8( '\"' );
	b.appendString( str );
	b.appendUint8( '\"' );
//	printf( "composeString(nodecpp::string \"%s\"\n", str.c_str() );
}

inline
void composeString( Buffer& b, nodecpp::string name, const StringLiteralForComposing* str )
{
	addNamePart( b, name );
	b.appendUint8( '\"' );
	b.append( str->str, str->size );
	b.appendUint8( '\"' );
//	printf( "composeString(StringLiteralForComposing \"%s\"\n", str->str );
}

inline
void composeString( Buffer& b, nodecpp::string name, std::string str )
{
	addNamePart( b, name );
	b.appendUint8( '\"' );
	b.append( str.c_str(), str.size() );
	b.appendUint8( '\"' );
//	printf( "composeString(std::string \"%s\"\n", str.c_str() );
}

inline
void composeString( Buffer& b, nodecpp::string name, const char* str )
{
	addNamePart( b, name );
	size_t sz = strlen( str );
	b.appendUint8( '\"' );
	b.append( str, sz );
	b.appendUint8( '\"' );
//	printf( "composeString(const char* \"%s\"\n", str );
}

} // namespace json

// parsing

class Parser
{
	uint8_t* begin = nullptr;
	uint8_t* end;

public:
	Parser() {}
	Parser( uint8_t* buff, size_t size ) { begin = buff; end = buff + size; }

	template <typename T>
	void parseSignedInteger( T* num )
	{
		static_assert( sizeof( T ) <= integer_max_size );
		static_assert( std::is_integral<T>::value );
		/*temporary solution TODO: actauls implementation*/ int64_t val = *reinterpret_cast<int64_t*>(begin); begin += sizeof( val );
		static_assert( integer_max_size == 8, "revise implementation otherwise" );
		if constexpr ( std::is_signed< T >::value )
		{
			if constexpr ( sizeof( T ) == 8 )
				*num = (T)(val);
			else if constexpr ( sizeof( T ) == 4 )
			{
				assert( val >= INT32_MIN && val <= INT32_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 2 )
			{
				assert( val >= INT16_MIN && val <= INT16_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 1 )
			{
				assert( val >= INT8_MIN && val <= INT8_MAX );
				*num = (T)(val);
			}
			else
				static_assert( sizeof( T ) > integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
		else
		{
			if constexpr ( sizeof( T ) == 8 )
			{
				assert( val >= 0 );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 4 )
			{
				assert( val >= 0 && val <= UINT32_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 2 )
			{
				assert( val >= 0 && val <= UINT16_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 1 )
			{
				assert( val >= 0 && val <= UINT8_MAX );
				*num = (T)(val);
			}
			else
				static_assert( sizeof( T ) > integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
	}

	template <typename T>
	void parseUnsignedInteger( T* num )
	{
		static_assert( sizeof( T ) <= integer_max_size );
		static_assert( std::is_integral<T>::value );
		/*temporary solution TODO: actauls implementation*/ uint64_t val = *reinterpret_cast<uint64_t*>(begin); begin += sizeof( val );
		static_assert( integer_max_size == 8, "revise implementation otherwise" );
		if constexpr ( std::is_unsigned< T >::value )
		{
			if constexpr ( sizeof( T ) == 8 )
				*num = (T)(val);
			else if constexpr ( sizeof( T ) == 4 )
			{
				assert( val <= UINT32_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 2 )
			{
				assert( val <= UINT16_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 1 )
			{
				assert( val <= UINT8_MAX );
				*num = (T)(val);
			}
			else
				static_assert( sizeof( T ) > integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
		else
		{
			if constexpr ( sizeof( T ) == 8 )
			{
				assert( val <= INT64_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 4 )
			{
				assert( val <= INT32_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 2 )
			{
				assert( val <= INT16_MAX );
				*num = (T)(val);
			}
			else if constexpr ( sizeof( T ) == 1 )
			{
				assert( val <= INT8_MAX );
				*num = (T)(val);
			}
			else
				static_assert( sizeof( T ) > integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
	}

	void parseString( const char** str )
	{
		*str = reinterpret_cast<char*>(begin);
		while( *begin++ != 0 );
	}

	void parseString( nodecpp::string* str )
	{
		*str = reinterpret_cast<char*>(begin);
		while( *begin++ != 0 );
	}
};

} // namespace m::impl

#endif // COMPOSE_AND_PARSE_IMPL_H
