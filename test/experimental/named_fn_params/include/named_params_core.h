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


struct CollectionWrapperBase {};
struct SimpleTypeCollectionWrapperBase {};

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
	bool compose_next( Buffer& b )
	{ 
		if ( it != coll.end() )
		{
			if constexpr ( std::is_same<typename ExpectedType::value_type, impl::SignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::composeSignedInteger( b, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::UnsignedIntegralType>::value && std::is_integral<value_type>::value )
				impl::composeUnsignedInteger( b, *it );
			else if constexpr ( std::is_same<typename ExpectedType::value_type, impl::StringType>::value && std::is_same<value_type, nodecpp::string>::value )
				impl::composeString( b, *it );
			else
				static_assert( std::is_same<value_type, AllowedDataType>::value, "unsupported type" );
			it++;
			return it != coll.end();
		}
		else
			return false;
	}
	template<class ExpectedType>
	void parse_next( impl::Parser& p )
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


template<typename TypeToPick, bool required, typename Arg0, typename ... Args>
void parseParam(const typename TypeToPick::NameAndTypeID expected, Parser& p, Arg0&& arg0, Args&& ... args)
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
				for ( size_t i=0; i<sz; ++i )
					coll.parse_next<typename TypeToPick::Type>( p );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				size_t sz = 0;
				p.parseUnsignedInteger( &sz );
				auto& coll = arg0.get();
				for ( size_t i=0; i<sz; ++i )
					coll.parse_next( p );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				size_t collSz = 0;
				p.parseUnsignedInteger( &collSz );
				auto& coll = arg0.get();
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
		parseParam<TypeToPick, required>(expected, p, args...);
}

template<typename TypeToPick, bool required>
void parseParam(const typename TypeToPick::NameAndTypeID expected, Parser& p)
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

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParam(const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		else if constexpr ( std::is_base_of<impl::VectorType, typename TypeToPick::Type>::value )
		{
//			if constexpr ( std::is_invocable<typename Agr0Type::Type, ::nodecpp::Buffer&, size_t>::value )
//				composeVector( b, arg0.get() );
			if constexpr ( std::is_base_of<VectorOfSympleTypesBase, typename TypeToPick::Type>::value && std::is_base_of<SimpleTypeCollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t sz = coll.size();
				composeUnsignedInteger( b, sz );
				while ( coll.compose_next<typename TypeToPick::Type>( b ) );
			}
			else if constexpr ( std::is_base_of<VectorOfNonextMessageTypesBase, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t sz = coll.size();
				composeUnsignedInteger( b, sz );
				while ( coll.compose_next( b ) );
			}
			else if constexpr ( std::is_base_of<VectorOfMessageType, typename TypeToPick::Type>::value && std::is_base_of<CollectionWrapperBase, typename Agr0Type::Type>::value )
			{
				auto& coll = arg0.get();
				size_t collSz = coll.size();
				composeUnsignedInteger( b, collSz );
				if ( collSz )
				{
					size_t pos = b.size();
					b.set_size( b.size() + integer_max_size ); // TODO: revise toward lowest estimation of 1 with move is longer
					for ( size_t i=0; i<collSz; ++i )
					{
						bool ok = coll.compose_next( b );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ok, "wrapper declared {} items, provided only {}", collSz, i + 1 );
						int64_t written = b.size() - pos - integer_max_size;
						memcpy( b.begin() + pos, &written, integer_max_size );
						pos = b.size();
						b.set_size( b.size() + integer_max_size ); // TODO: revise toward lowest estimation of 1 with move is longer
					}
				}
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, coll.compose_next( b ), "wrapper declared {} items and more is ready for processing", collSz );
			}
		}
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		composeParam<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(expected, b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
void composeParam(const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b)
{
		static_assert( !required, "required parameter" );
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeSignedInteger( b, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeUnsignedInteger( b, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		{
			composeString( b, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}

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

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParam(nodecpp::string name, const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeSignedInteger( b, name, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && (std::is_integral<typename Agr0Type::Type>::value || std::is_integral<typename std::remove_reference<typename Agr0Type::Type>::type>::value) )
			composeUnsignedInteger( b, name, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, name, arg0.get() );
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		composeParam<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(name, expected, b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
void composeParam(nodecpp::string name, const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b)
{
		static_assert( !required, "required parameter" );
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeSignedInteger( b, name, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value )
		{
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeUnsignedInteger( b, name, defaultValue );
		}
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
		{
			composeString( b, name, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}

} // namespace json


} // namespace impl

} // namespace m

#endif // NAMED_PARAMS_CORE_H
