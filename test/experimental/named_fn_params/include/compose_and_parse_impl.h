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

namespace m {

class Composer
{
public: // just temporary TODO: rework!
	Buffer& buff;

public:
	Composer( Buffer& buff_ ) : buff( buff_ ) {}
};

} // namespace m



namespace m::impl {

static constexpr size_t integer_max_size = 8;

// supported types
struct SignedIntegralType {static constexpr bool dummy = false;};
struct UnsignedIntegralType {int d = 0;};
struct StringType {static constexpr bool dummy = false;};
struct BlobType {static constexpr bool dummy = false;};
struct ByteArrayType {static constexpr bool dummy = false;};
struct EnumType {static constexpr bool dummy = false;};
struct VectorType {static constexpr bool dummy = false;};
struct VectorOfSympleTypesBase : public VectorType {static constexpr bool dummy = false;};
template<class value_type_>
struct VectorOfSympleTypes : public VectorOfSympleTypesBase {static constexpr bool dummy = false; using value_type = value_type_;};
struct VectorOfMessageType : public VectorType {static constexpr bool dummy = false;};
struct VectorOfNonextMessageTypesBase : public VectorType {static constexpr bool dummy = false;};
//template<class value_type_>
//struct VectorOfNonextMessageTypes : public VectorOfNonextMessageTypesBase {static constexpr bool dummy = false; using value_type = value_type_;};
struct VectorOfNonextMessageTypes : public VectorOfNonextMessageTypesBase {static constexpr bool dummy = false;};

struct NoDefaultValueType {static constexpr bool dummy = false;};
// helper types

struct StringLiteralForComposing
{
	const char* const str;
	const size_t size;
	constexpr StringLiteralForComposing( const char* const str_, const size_t size_ ) : str( str_ ), size( size_ ) {}
};


// composing

template <typename T>
void composeSignedInteger(Composer& composer, T num )
{
	static_assert( std::is_integral<T>::value );
	if constexpr ( std::is_unsigned<T>::value && sizeof( T ) >= integer_max_size )
	{
		assert( num <= INT64_MAX );
	}
	/*temporary solution TODO: actual implementation*/ { 
		int64_t val = num; 
		composer.buff.append( &val, sizeof( val ) );
		printf( "composeSignedInteger(%zd\n", val );
	}
}

template <typename T>
void composeUnsignedInteger(Composer& composer, T num )
{
	if constexpr ( std::is_signed<T>::value )
	{
		assert( num >= 0 );
	}
	/*temporary solution TODO: actual implementation*/ { 
		uint64_t val = num; 
		composer.buff.append( &val, sizeof( val ) );
		printf( "composeSignedInteger(%zd\n", val );
	}
}

inline
void composeString(Composer& composer, const nodecpp::string& str )
{
	composer.buff.appendString( str );
	composer.buff.appendUint8( 0 );
	printf( "composeString(nodecpp::string \"%s\"\n", str.c_str() );
}

inline
void composeString(Composer& composer, const StringLiteralForComposing* str )
{
	composer.buff.append( str->str, str->size );
	composer.buff.appendUint8( 0 );
	printf( "composeString(StringLiteralForComposing \"%s\"\n", str->str );
}

inline
void composeString(Composer& composer, std::string str )
{
	composer.buff.append( str.c_str(), str.size() );
	composer.buff.appendUint8( 0 );
	printf( "composeString(std::string \"%s\"\n", str.c_str() );
}

inline
void composeString(Composer& composer, const char* str )
{
	size_t sz = strlen( str );
	composer.buff.append( str, sz );
	composer.buff.appendUint8( 0 );
	printf( "composeString(const char* \"%s\"\n", str );
}

/*template<typename Fn>
void composeVector(Composer& composer, Fn& fn )
{
	size_t startOffset = composer.buff.size();
	size_t currOffset = composer.buff.size();
	size_t count = 0;
	size_t ret = fn( composer, count );
	while ( ret )
	{
		uint64_t sz = composer.buff.size() - currOffset;
		composer.buff.set_size( composer.buff.size() + integer_max_size );
		memmove( composer.buff.begin() + currOffset, composer.buff.begin() + currOffset + integer_max_size, sz );
		memcpy( composer.buff.begin() + currOffset, &sz, sizeof( integer_max_size ) );
		currOffset = composer.buff.size();
		++count;
		ret = fn( composer, count );
	}
	uint64_t fullSz = currOffset - startOffset;
	composer.buff.set_size( composer.buff.size() + integer_max_size );
	memmove( composer.buff.begin() + startOffset, composer.buff.begin() + startOffset + integer_max_size, fullSz );
	memcpy( composer.buff.begin() + startOffset, &fullSz, sizeof( integer_max_size ) );
}*/

namespace json
{

template <typename T>
void composeSignedInteger(Composer& composer, T num )
{
	static_assert( std::is_integral<T>::value );
	if constexpr ( std::is_unsigned<T>::value && sizeof( T ) >= integer_max_size )
	{
		assert( num <= INT64_MAX );
	}
	composer.buff.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

template <typename T>
void composeUnsignedInteger(Composer& composer, T num )
{
	if constexpr ( std::is_signed<T>::value )
	{
		assert( num >= 0 );
	}
	composer.buff.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

inline
void composeString(Composer& composer, const nodecpp::string& str )
{
	composer.buff.appendUint8( '\"' );
	composer.buff.appendString( str );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(nodecpp::string \"%s\"\n", str.c_str() );
}

inline
void composeString(Composer& composer, const StringLiteralForComposing* str )
{
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str->str, str->size );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(StringLiteralForComposing \"%s\"\n", str->str );
}

inline
void composeString(Composer& composer, std::string str )
{
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str.c_str(), str.size() );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(std::string \"%s\"\n", str.c_str() );
}

inline
void composeString(Composer& composer, const char* str )
{
	size_t sz = strlen( str );
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str, sz );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(const char* \"%s\"\n", str );
}

inline
void addNamePart(Composer& composer, nodecpp::string name )
{
	composer.buff.appendUint8( '\"' );
	composer.buff.appendString( name );
	composer.buff.appendUint8( '\"' );
	composer.buff.appendUint8( ':' );
	composer.buff.appendUint8( ' ' );
}

template <typename T>
void composeNamedSignedInteger(Composer& composer, nodecpp::string name, T num )
{
	static_assert( std::is_integral<T>::value );
	if constexpr ( std::is_unsigned<T>::value && sizeof( T ) >= integer_max_size )
	{
		assert( num <= INT64_MAX );
	}
	addNamePart( composer, name );
	composer.buff.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

template <typename T>
void composeNamedUnsignedInteger(Composer& composer, nodecpp::string name, T num )
{
	if constexpr ( std::is_signed<T>::value )
	{
		assert( num >= 0 );
	}
	addNamePart( composer, name );
	composer.buff.appendString( nodecpp::format( "{}", (int64_t)num ) );
}

inline
void composeNamedString(Composer& composer, nodecpp::string name, const nodecpp::string& str )
{
	addNamePart( composer, name );
	composer.buff.appendUint8( '\"' );
	composer.buff.appendString( str );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(nodecpp::string \"%s\"\n", str.c_str() );
}

inline
void composeNamedString(Composer& composer, nodecpp::string name, const StringLiteralForComposing* str )
{
	addNamePart( composer, name );
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str->str, str->size );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(StringLiteralForComposing \"%s\"\n", str->str );
}

inline
void composeNamedString(Composer& composer, nodecpp::string name, std::string str )
{
	addNamePart( composer, name );
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str.c_str(), str.size() );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(std::string \"%s\"\n", str.c_str() );
}

inline
void composeNamedString(Composer& composer, nodecpp::string name, const char* str )
{
	addNamePart( composer, name );
	size_t sz = strlen( str );
	composer.buff.appendUint8( '\"' );
	composer.buff.append( str, sz );
	composer.buff.appendUint8( '\"' );
//	printf( "composeString(const char* \"%s\"\n", str );
}

} // namespace json

} // namespace m::impl

// parsing

namespace m {

class Parser
{
	uint8_t* begin = nullptr;
	uint8_t* end = nullptr;

public:
	Parser() {}
	Parser( uint8_t* buff, size_t size ) { begin = buff; end = buff + size; }
	Parser( const Parser& other, size_t size ) { begin = other.begin; end = other.begin + size; }

	void adjustParsingPos( size_t toSkip )
	{
		begin += toSkip;
	}

	// JSON  ///////////////////////////////////////////////////////////////////

	void skipSpacesEtc()
	{
		while ( begin < end && ( *begin == ' ' || *begin == '\t' || *begin == '\r' || *begin == '\n' ) ) ++begin;
	}

	bool isComma()
	{
		skipSpacesEtc();
		return *begin == ',';
	}

	void skipComma()
	{
		if ( *begin++ != ',' )
			throw std::exception(); // TODO
	}

	bool isDelimiter( char delim )
	{
		skipSpacesEtc();
		return *begin == delim;
	}

	void skipDelimiter( char delim )
	{
		skipSpacesEtc();
		if ( *begin++ != delim )
			throw std::exception(); // TODO
	}

	bool isData() { return begin != end && *begin != 0;  }

	void readStringFromJson(nodecpp::string* s)
	{
		skipSpacesEtc();
		if ( *begin++ != '\"' )
			throw std::exception(); // TODO
		const char* start = reinterpret_cast<const char*>( begin );
		while ( begin < end && *begin != '\"' ) ++begin;
		*s = nodecpp::string( start, reinterpret_cast<const char*>( begin ) - start );
		if ( begin == end )
			throw std::exception(); // TODO
		++begin;
	}
	void skipStringFromJson()
	{
		skipSpacesEtc();
		if ( *begin++ != '\"' )
			throw std::exception(); // TODO
		while ( begin < end && *begin != '\"' ) ++begin;
		if ( begin == end )
			throw std::exception(); // TODO
		++begin;
	}

	template <typename T>
	void readUnsignedIntegerFromJson( T* num )
	{
		skipSpacesEtc();
		if ( *begin == '-' )
			throw std::exception(); // TODO: (negative is unexpected)
		auto start = begin;
		uint64_t ret = strtoull( reinterpret_cast<const char*>( begin ), reinterpret_cast<char**>( &begin ), 10 );
		if ( start == begin )
			throw std::exception(); // TODO: (NaN)
		*num = (T)ret; // TODO: add boundary checking
	}
	void skipUnsignedIntegerFromJson()
	{
		skipSpacesEtc();
		if ( *begin == '-' )
			throw std::exception(); // TODO: (negative is unexpected)
		auto start = begin;
		uint64_t ret = strtoull( reinterpret_cast<const char*>( begin ), reinterpret_cast<char**>( &begin ), 10 );
		if ( start == begin )
			throw std::exception(); // TODO: (NaN)
	}

	template <typename T>
	void readSignedIntegerFromJson( T* num )
	{
		skipSpacesEtc();
		auto start = begin;
		int64_t ret = strtoll( reinterpret_cast<const char*>( begin ), reinterpret_cast<char**>( &begin ), 10 );
		if ( start == begin )
			throw std::exception(); // TODO: (NaN)
		*num = (T)ret; // TODO: add boundary checking
	}
	void skipSignedIntegerFromJson()
	{
		skipSpacesEtc();
		auto start = begin;
		int64_t ret = strtoll( reinterpret_cast<const char*>( begin ), reinterpret_cast<char**>( &begin ), 10 );
		if ( start == begin )
			throw std::exception(); // TODO: (NaN)
	}

	void readKey(nodecpp::string* s)
	{
		skipSpacesEtc();
		readStringFromJson(s);
		skipSpacesEtc();
		if ( *begin++ != ':' )
			throw std::exception(); // TODO (expected ':')
	}

	// GMQ  ///////////////////////////////////////////////////////////////////

	template <typename T>
	void parseSignedInteger( T* num )
	{
		static_assert( sizeof( T ) <= impl::integer_max_size );
		static_assert( std::is_integral<T>::value );
		/*temporary solution TODO: actual implementation*/ int64_t val = *reinterpret_cast<int64_t*>(begin); begin += sizeof( val );
		static_assert( impl::integer_max_size == 8, "revise implementation otherwise" );
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
				static_assert( sizeof( T ) > impl::integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
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
				static_assert( sizeof( T ) > impl::integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
	}
	void skipSignedInteger()
	{
		/*temporary solution TODO: actual implementation*/ begin += impl::integer_max_size;
		static_assert( impl::integer_max_size == 8, "revise implementation otherwise" );
	}

	template <typename T>
	void parseUnsignedInteger( T* num )
	{
		static_assert( sizeof( T ) <= impl::integer_max_size );
		static_assert( std::is_integral<T>::value );
		/*temporary solution TODO: actual implementation*/ uint64_t val = *reinterpret_cast<uint64_t*>(begin); begin += sizeof( val );
		static_assert( impl::integer_max_size == 8, "revise implementation otherwise" );
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
				static_assert( sizeof( T ) > impl::integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
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
				static_assert( sizeof( T ) > impl::integer_max_size ); // kinda chitting with a compiler, which treats static_assert( false ) here as an unconditional error
		}
	}
	void skipUnsignedInteger()
	{
		/*temporary solution TODO: actual implementation*/ uint64_t val = *reinterpret_cast<uint64_t*>(begin); begin += impl::integer_max_size;
		static_assert( impl::integer_max_size == 8, "revise implementation otherwise" );
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

	void skipString()
	{
		while( *begin++ != 0 );
	}
};

} // namespace m

#endif // COMPOSE_AND_PARSE_IMPL_H
