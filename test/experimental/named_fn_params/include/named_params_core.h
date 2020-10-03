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

#ifndef NAMED_PARAMS_CORE_H
#define NAMED_PARAMS_CORE_H

#include <tuple>
#include <string>

#include "compose_and_parse_impl.h"

namespace m {

template <typename T, typename NameTag>
class NamedParameterWithType;

template <typename NameTag>
class NamedParameter
{
public:
	explicit NamedParameter() {}

	using Name = NameTag;

	struct TypeConverter
	{
		template<typename DataType>
		NamedParameterWithType<DataType, NameTag> operator=(DataType&& value) const
		{
			return NamedParameterWithType<DataType, NameTag>(std::forward<DataType>(value));
		}
		TypeConverter() = default;
		TypeConverter(TypeConverter const&) = delete;
		TypeConverter(TypeConverter&&) = delete;
		TypeConverter& operator=(TypeConverter const&) = delete;
		TypeConverter& operator=(TypeConverter&&) = delete;
	};
};


template <typename T, typename NameTag_>
class NamedParameterWithType : public NamedParameter<NameTag_>
{
private:
	T value_;

public:
	explicit NamedParameterWithType(T const& value) : value_(value) {}
//	explicit NamedParameterWithType(T&& value) : value_(std::move(value)) {}
	T& get() { return value_; }
	T const& get() const { return value_; }

	using NameBase = NamedParameter<NameTag_>;
	using Name = typename NamedParameter<NameTag_>::Name;
	using Type = T;
	using typename NamedParameter<NameTag_>::TypeConverter;

	template<class NameT, class DataT>
	struct FullType
	{
		using NameTag = NameT;
		using DataType = DataT;
	};

	using NameAndTypeID = FullType<NameTag_, T>;
	static constexpr NameAndTypeID nameAndTypeID = {};
};

template<typename NameT, typename DataT>
struct ExpectedParameter
{
	using NameTag = NameT;
	using DataType = DataT;
};

struct AllowedDataType {};

struct AnyCollectionWrapperBase {
	static constexpr size_t unknown_size = (size_t)(-1);
};

struct CollectionWrapperBase : public AnyCollectionWrapperBase {};

struct SimpleTypeCollectionWrapperBase : public AnyCollectionWrapperBase {};

template<class Collection/*, class LambdaSize*/, class LambdaNext>
class CollectionWrapperForComposing : public CollectionWrapperBase {
	Collection& coll;
	typename Collection::iterator it;
//	LambdaSize lsize_;
	LambdaNext lnext_;
public:
	CollectionWrapperForComposing(Collection& coll_/*, LambdaSize &&lsize*/, LambdaNext &&lnext) : coll( coll_ ), it( coll.begin() )/*, lsize_(std::forward<LambdaSize>(lsize))*/, lnext_(std::forward<LambdaNext>(lnext)) {}
//	size_t size() { return lsize_(); }
	size_t size() { 
		return coll.size();
	}
	bool compose_next( m::Composer& composer ) { 
		/*if ( it != coll.end() )
		{
			lnext_( composer, *it ); 
			it++;
		}
		else
			return false; // no more items*/
		return lnext_( composer ); 
	}
};
template<class Collection/*, class LambdaSize*/, class LambdaNext>
CollectionWrapperForComposing<Collection/*, LambdaSize*/, LambdaNext> makeReadyForComposing(Collection& coll/*, LambdaSize &&lsize*/, LambdaNext &&lnext) { 
//	static_assert( std::is_invocable<LambdaSize>::value, "lambda-expression is expected" );
	static_assert( std::is_invocable<LambdaNext>::value, "lambda-expression is expected" );
	return { coll/*, std::forward<LambdaSize>(lsize)*/, std::forward<LambdaNext>(lnext) };
}

template<class LambdaSize, class LambdaNext>
class CollactionWrapperForParsing : public CollectionWrapperBase {
    LambdaSize lsize_;
	LambdaNext lnext_;
public:
	CollactionWrapperForParsing(LambdaSize &&lsizeHint, LambdaNext &&lnext) : lsize_(std::forward<LambdaSize>(lsizeHint)), lnext_(std::forward<LambdaNext>(lnext)) {
	}
	void size_hint( size_t sz ) { 
		if constexpr ( std::is_invocable<LambdaSize>::value )
			lsize_( sz );
	}
	void parse_next( m::Parser& p ) { 
		lnext_( p ); 
	}
};
template<class LambdaSize, class LambdaNext>
CollactionWrapperForParsing<LambdaSize, LambdaNext> makeReadyForParsing(LambdaSize &&lsizeHint, LambdaNext &&lnext) { 
	static_assert( std::is_same<nullptr_t, LambdaSize>::value || std::is_invocable<LambdaSize>::value, "lambda-expression is expected" );
	static_assert( std::is_invocable<LambdaNext>::value, "lambda-expression is expected" );
	return { std::forward<LambdaSize>(lsizeHint), std::forward<LambdaNext>(lnext) };
}

template<class T>
class SimpleTypeCollectionWrapper : public SimpleTypeCollectionWrapperBase
{
	T& coll;
	typename T::iterator it;

public:
	using value_type = typename T::value_type;
	static_assert( std::is_same<T, nodecpp::vector<value_type>>::value, "vector type is expected only" ); // TODO: add list
	static_assert( std::is_integral<value_type>::value || std::is_same<value_type, nodecpp::string>::value || std::is_enum<value_type>::value, "intended for simple idl types only" );

	SimpleTypeCollectionWrapper( T& coll_ ) : coll( coll_ ), it( coll.begin() ) {};
	size_t size() const { return coll.size(); }
	template<class ExpectedType>
	bool compose_next_to_gmq( Composer& composer )
	{ 
		if ( it != coll.end() )
		{
			if constexpr ( std::is_same<typename ExpectedType::value_type, impl::SignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::composeSignedInteger( composer, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::UnsignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::composeUnsignedInteger( composer, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::StringType>::value && std::is_same<value_type, nodecpp::string>::value )
				impl::composeString( composer, *it );
			else
				static_assert( std::is_same<value_type, AllowedDataType>::value, "unsupported type" );
			it++;
			return it != coll.end();
		}
		else
			return false;
	}
	template<class ExpectedType>
	bool compose_next_to_json( Composer& composer )
	{ 
		if ( it != coll.end() )
		{
			if constexpr ( std::is_same<typename ExpectedType::value_type, impl::SignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::json::composeSignedInteger( composer, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::UnsignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::json::composeUnsignedInteger( composer, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::StringType>::value && std::is_same<value_type, nodecpp::string>::value )
				impl::json::composeString( composer, *it );
			else
				static_assert( std::is_same<value_type, AllowedDataType>::value, "unsupported type" );
			it++;
			return it != coll.end();
		}
		else
			return false;
	}

	void size_hint( size_t count )
	{
		// Note: here we can do some preliminary steps based on a number of collection elements declared in the message (if such a number exists for a particular protocol) 
		if constexpr ( std::is_same<T, nodecpp::vector<value_type>>::value )
		{
			if ( count != unknown_size )
				coll.reserve( count );
		}
	}
	template<class ExpectedType>
	void parse_next_from_gmq( Parser& p )
	{
		value_type val;
		if constexpr ( std::is_same<typename ExpectedType::value_type, impl::SignedIntegralType>::value && std::is_integral<value_type>::value )
			p.parseSignedInteger( &val );
		else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::UnsignedIntegralType>::value && std::is_integral<value_type>::value )
			p.parseUnsignedInteger( val );
		else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::StringType>::value && std::is_same<value_type, nodecpp::string>::value )
			p.parseString( val );
		else
			static_assert( std::is_same<value_type, AllowedDataType>::value, "unsupported type" );
		coll.push_back( val );
	}
	template<class ExpectedType>
	void parse_next_from_json( Parser& p )
	{
		value_type val;
		if constexpr ( std::is_same<typename ExpectedType::value_type, impl::SignedIntegralType>::value && std::is_integral<value_type>::value )
			p.readSignedIntegerFromJson( &val );
		else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::UnsignedIntegralType>::value && std::is_integral<value_type>::value )
			p.readUnsignedIntegerFromJson( val );
		else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::StringType>::value && std::is_same<value_type, nodecpp::string>::value )
			p.readStringFromJson( val );
		else
			static_assert( std::is_same<value_type, AllowedDataType>::value, "unsupported type" );
		coll.push_back( val );
	}
};


template<typename BaseT, typename Arg0, typename ... Args>
void findMatch(const Arg0 arg0, const Args ... args)
{
	static_assert( std::is_same<typename BaseT::NameTag, typename Arg0::NameTag>::value == false, "same name used more than once" );
	findMatch<BaseT>(args...);
}

template<typename BaseT, typename Arg0>
void findMatch(const Arg0 arg0)
{
	static_assert( std::is_same<typename BaseT::NameTag, typename Arg0::NameTag>::value == false, "same name used more than once" );
}

template<typename Arg0, typename ... Args>
void ensureUniqueness(const Arg0 arg0, const Args ... args)
{
	findMatch<Arg0>(args...);
	ensureUniqueness(args...);
}

template<typename Arg0>
void ensureUniqueness(const Arg0 arg0)
{
	return;
}


template<typename TypeToMatch, typename Arg0, typename ... Args>
constexpr size_t isMatched(const TypeToMatch matcher, const Arg0 arg0, const Args ... args)
{
	if constexpr ( std::is_same<typename Arg0::NameTag, typename TypeToMatch::NameTag>::value )
		return 1;
	else if constexpr ( std::is_same<typename TypeToMatch::NameTag, impl::SignedIntegralType>::value && std::is_integral<typename Arg0::NameTag>::value )
		 return 1;
	else if constexpr ( std::is_same<typename TypeToMatch::NameTag, impl::UnsignedIntegralType>::value && std::is_integral<typename Arg0::NameTag>::value )
		 return 1;
   else
		return isMatched<TypeToMatch>(matcher, args...);
}

template<typename TypeToMatch>
constexpr size_t isMatched(const TypeToMatch matcher)
{
	return 0;
}



template <class T>
struct unwrap_refwrapper
{
	using type = T;
};
 
template <class T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
	using type = T&;
};
 
template <class T>
using special_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;

namespace impl {

// composing - general



// parsing - general


namespace gmq {

template<typename TypeToPick, bool required, typename Arg0, typename ... Args>
void parseGmqParam(const typename TypeToPick::NameAndTypeID expected, Parser& p, Arg0&& arg0, Args&& ... args)
{
//	using Agr0Type = std::remove_pointer<Arg0>;
	using Agr0Type = special_decay_t<Arg0>;
	using Agr0DataType = typename std::remove_pointer<typename Agr0Type::Type>::type;
	if constexpr ( std::is_same<typename Agr0Type::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value && std::is_integral<Agr0DataType>::value )
			p.parseSignedInteger( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value && std::is_integral<Agr0DataType>::value )
			p.parseUnsignedInteger( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
			p.parseString( arg0.get() );
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			if constexpr ( std::is_base_of<VectorOfSympleTypesBase, typename TypeToPick::Type>::value && std::is_base_of<SimpleTypeCollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				size_t sz = 0;
				p.parseUnsignedInteger( &sz );
				auto& coll = arg0.get();
				coll.size_hint( sz );
				for ( size_t i=0; i<sz; ++i )
					coll.parse_next_from_gmq<typename TypeToPick::Type>( p );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				size_t sz = 0;
				p.parseUnsignedInteger( &sz );
				auto& coll = arg0.get();
				coll.size_hint( sz );
				for ( size_t i=0; i<sz; ++i )
					coll.parse_next( p );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				size_t collSz = 0;
				p.parseUnsignedInteger( &collSz );
				auto& coll = arg0.get();
				coll.size_hint( collSz );
				for ( size_t i=0; i<collSz; ++i )
				{
					size_t itemSz = 0;
					p.parseUnsignedInteger( &itemSz );
					Parser itemParser( p, itemSz );
					coll.parse_next( itemParser );
					p.adjustParsingPos( itemSz );
				}
			}
			else
				static_assert( std::is_same<Agr0DataType, AllowedDataType>::value, "unsupported type" );
		}
		else
			static_assert( std::is_same<Agr0DataType, AllowedDataType>::value, "unsupported type" );
	}
	else
		parseGmqParam<TypeToPick, required>(expected, p, args...);
}

template<typename TypeToPick, bool required>
void parseGmqParam(const typename TypeToPick::NameAndTypeID expected, Parser& p)
{
	if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		p.skipSignedInteger();
	else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		p.skipUnsignedInteger();
	else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		p.skipString();
//		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::VectorType>::value && std::is_function<typename Agr0Type::Type>::value )
	else if constexpr ( std::is_same<typename TypeToPick::Type, impl::VectorType>::value )
		;
	else
		static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}


///////////////////////////////////////////

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
void composeParamToGmq(const typename TypeToPick::NameAndTypeID expected, Composer& composer)
{
		static_assert( !required, "required parameter" );
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeSignedInteger( composer, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeUnsignedInteger( composer, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		{
			composeString( composer, defaultValue );
		}
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			composeUnsignedInteger( composer, 0 );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParamToGmq(const typename TypeToPick::NameAndTypeID expected, Composer& composer, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeSignedInteger( composer, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeUnsignedInteger( composer, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( composer, arg0.get() );
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			if constexpr ( std::is_base_of<VectorOfSympleTypesBase, typename TypeToPick::Type>::value && std::is_base_of<SimpleTypeCollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t sz = coll.size();
				composeUnsignedInteger( composer, sz );
				while ( coll.compose_next_to_gmq<typename TypeToPick::Type>(composer) );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t sz = coll.size();
				composeUnsignedInteger( composer, sz );
				while ( coll.compose_next(composer) );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t collSz = coll.size();
				composeUnsignedInteger( composer, collSz );
				if ( collSz )
				{
					size_t pos = composer.buff.size();
					composer.buff.set_size( composer.buff.size() + integer_max_size ); // TODO: revise toward lowest estimation of 1 with move is longer
					for ( size_t i=0; i<collSz; ++i )
					{
						bool ok = coll.compose_next(composer);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ok, "wrapper declared {} items, provided only {}", collSz, i + 1 );
						int64_t written = composer.buff.size() - pos - integer_max_size;
						memcpy( composer.buff.begin() + pos, &written, integer_max_size );
						pos = composer.buff.size();
						composer.buff.set_size( composer.buff.size() + integer_max_size ); // TODO: revise toward lowest estimation of 1 with move is longer
					}
				}
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coll.compose_next(composer), "wrapper declared {} items and more is ready for processing", collSz );
			}
		}
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		composeParamToGmq<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(expected, composer, args...);
}

} // namespace gmq

namespace json {

template<typename TypeToPick, bool required, typename Arg0, typename ... Args>
void parseJsonParam(const typename TypeToPick::NameAndTypeID expected, Parser& p, Arg0&& arg0, Args&& ... args)
{
//	using Agr0Type = std::remove_pointer<Arg0>;
	using Agr0Type = special_decay_t<Arg0>;
	using Agr0DataType = typename std::remove_pointer<typename Agr0Type::Type>::type;
	if constexpr ( std::is_same<typename Agr0Type::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value && std::is_integral<Agr0DataType>::value )
			p.readSignedIntegerFromJson( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value && std::is_integral<Agr0DataType>::value )
			p.readUnsignedIntegerFromJson( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
			p.readStringFromJson( arg0.get() );
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			auto& coll = arg0.get();
			coll.size_hint( CollectionWrapperBase::unknown_size );
			if constexpr ( std::is_base_of<VectorOfSympleTypesBase, typename TypeToPick::Type>::value && std::is_base_of<SimpleTypeCollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				p.skipDelimiter( '[' );
				if ( !p.isDelimiter( ']' ) ) // there are some items there
				{
					for ( ;; )
					{
						coll.parse_next_from_json<typename TypeToPick::Type>( p );
						if ( p.isDelimiter( ',' ) )
						{
							p.skipDelimiter( ',' );
							continue;
						}
						if ( p.isDelimiter( ']' ) )
						{
							p.skipDelimiter( ']' );
							break;
						}
					}
				}
				else
					p.skipDelimiter( ']' );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				p.skipDelimiter( '[' );
				if ( !p.isDelimiter( ']' ) ) // there are some items there
				{
					for ( ;; )
					{
						coll.parse_next( p );
						if ( p.isDelimiter( ',' ) )
						{
							p.skipDelimiter( ',' );
							continue;
						}
						if ( p.isDelimiter( ']' ) )
						{
							p.skipDelimiter( ']' );
							break;
						}
					}
				}
				else
					p.skipDelimiter( ']' );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				p.skipDelimiter( '[' );
				if ( !p.isDelimiter( ']' ) ) // there are some items there
				{
					for ( ;; )
					{
						coll.parse_next( p );
						if ( p.isDelimiter( ',' ) )
						{
							p.skipDelimiter( ',' );
							continue;
						}
						if ( p.isDelimiter( ']' ) )
						{
							p.skipDelimiter( ']' );
							break;
						}
					}
				}
				else
					p.skipDelimiter( ']' );
			}
			else
				static_assert( std::is_same<Agr0DataType, AllowedDataType>::value, "unsupported type" );
		}
		else
			static_assert( std::is_same<Agr0DataType, AllowedDataType>::value, "unsupported type" );
	}
	else
		parseJsonParam<TypeToPick, required>(expected, p, args...);
}

template<typename TypeToPick, bool required>
void parseJsonParam(const typename TypeToPick::NameAndTypeID expected, Parser& p)
{
	if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		p.skipSignedIntegerFromJson();
	else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		p.skipUnsignedIntegerFromJson();
	else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		p.skipStringFromJson();
	else
		static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
void composeParamToJson(nodecpp::string name, const typename TypeToPick::NameAndTypeID expected, Composer& composer)
{
		static_assert( !required, "required parameter" );
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeNamedSignedInteger( composer, name, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeNamedUnsignedInteger( composer, name, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		{
			composeNamedString( composer, name, defaultValue );
		}
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			json::addNamePart( composer, name );
			composer.buff.appendUint8( '[' );
			composer.buff.appendUint8( ']' );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParamToJson(nodecpp::string name, const typename TypeToPick::NameAndTypeID expected, Composer& composer, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeNamedSignedInteger( composer, name, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeNamedUnsignedInteger( composer, name, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeNamedString( composer, name, arg0.get() );
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
			if constexpr ( std::is_base_of<VectorOfSympleTypesBase, typename TypeToPick::Type>::value && std::is_base_of<SimpleTypeCollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				json::addNamePart( composer, name );
				composer.buff.appendUint8( '[' );
				auto& coll = arg0.get();
				size_t collSz = coll.size();
				for ( size_t i=0; i<collSz; ++i )
				{
					if ( i )
						composer.buff.append( ", ", 2 );
					bool ok = coll.compose_next_to_json<typename TypeToPick::Type>(composer);
//					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ok, "wrapper declared {} items, provided only {}", collSz, i + 1 );
				}
				composer.buff.appendUint8( ']' );
//				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coll.compose_next_to_json<typename TypeToPick::Type>(composer), "wrapper declared {} items and more is ready for processing", collSz );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				json::addNamePart( composer, name );
				composer.buff.appendUint8( '[' );
				auto& coll = arg0.get();
				size_t collSz = coll.size();
				for ( size_t i=0; i<collSz; ++i )
				{
					if ( i )
						composer.buff.append( ", ", 2 );
					bool ok = coll.compose_next(composer);
//					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ok, "wrapper declared {} items, provided only {}", collSz, i + 1 );
				}
				composer.buff.appendUint8( ']' );
//				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coll.compose_next_to_json(composer), "wrapper declared {} items and more is ready for processing", collSz );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				json::addNamePart( composer, name );
				composer.buff.appendUint8( '[' );
				auto& coll = arg0.get();
				size_t collSz = coll.size();
				for ( size_t i=0; i<collSz; ++i )
				{
					if ( i )
						composer.buff.append( ", ", 2 );
					bool ok = coll.compose_next(composer);
//					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ok, "wrapper declared {} items, provided only {}", collSz, i + 1 );
				}
				composer.buff.appendUint8( ']' );
//				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coll.compose_next_to_json(composer), "wrapper declared {} items and more is ready for processing", collSz );
			}
		}
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		json::composeParamToJson<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(name, expected, composer, args...);
}

} // namespace json


} // namespace impl

} // namespace m

#endif // NAMED_PARAMS_CORE_H
