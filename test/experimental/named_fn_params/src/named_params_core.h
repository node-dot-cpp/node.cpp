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
	explicit NamedParameterWithType(T&& value) : value_(std::move(value)) {}
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
constexpr size_t isMatched(const TypeToMatch matcher, const Arg0, const Args ... args)
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

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
TypeToPick pickParam(::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			return SignedIntegralType( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			 return UnsignedIntegralType( arg0.get() );
		else
			return arg0;
	}
	else
		return pickParam<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0>
TypeToPick pickParam(::nodecpp::Buffer& b, Arg0&& arg0)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			return SignedIntegralType( arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			 return UnsignedIntegralType( arg0.get() );
		else
			return arg0;
	}
	else
	{
		static_assert( !required, "required parameter" );
		if constexpr ( std::is_same<AssumedDefaultT, DefaultT>::value )
			return TypeToPick(defaultValue);
		else
			return TypeToPick( AssumedDefaultT(defaultValue) );
	}
}

namespace impl {

// composing - general

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParam(::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		composeParam<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(b, args...);
//		return composeParam(b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
//template<typename Arg0>
void composeParam(::nodecpp::Buffer& b)
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
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeString( b, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );

#if 0
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		/**/if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
			
	}
	else
	{
		static_assert( !required, "required parameter" );
		/*if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
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
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeString( b, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
			*/
	}
#endif // 0
}



// parsing - general


template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void parseParam(Parser& p, Arg0* arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			parseSignedInteger( p, arg0->get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			parseUnsignedInteger( p, arg0->get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
			parseString( p, arg0->get() );
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		parseParam<p, TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(p, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0>
void parseParam(Parser& p, Arg0&& arg0)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			parseSignedInteger( p, arg0->get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			parseUnsignedInteger( p, arg0->get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, StringType>::value )
			parseString( p, arg0->get() );
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
	{
		return;
	}
}


template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void composeParam2(::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		composeParam2<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0>
void composeParam2(::nodecpp::Buffer& b, Arg0&& arg0)
{
#if 1
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		/**/if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
			
	}
	else
	{
		static_assert( !required, "required parameter" );
		/**/if constexpr ( std::is_same<typename TypeToPick::Type, SignedIntegralType>::value )
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
			static_assert ( std::is_integral<AssumedDefaultT>::value );
			composeString( b, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
			
	}
#endif // 0
}

///////////////////////////////////////////
template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
void pickParam3(const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b, Arg0&& arg0, Args&& ... args)
{
	using Agr0Type = special_decay_t<Arg0>;
	if constexpr ( std::is_same<typename special_decay_t<Arg0>::Name, typename TypeToPick::Name>::value ) // same parameter name
	{
		if constexpr ( std::is_same<typename TypeToPick::Type, impl::SignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeSignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::UnsignedIntegralType>::value && std::is_integral<typename Agr0Type::Type>::value )
			composeUnsignedInteger( b, arg0.get() );
		else if constexpr ( std::is_same<typename TypeToPick::Type, impl::StringType>::value )
			composeString( b, arg0.get() );
		else
			static_assert( std::is_same<typename Agr0Type::Type, AllowedDataType>::value, "unsupported type" );
	}
	else
		pickParam3<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(expected, b, args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue>
void pickParam3(const typename TypeToPick::NameAndTypeID expected, ::nodecpp::Buffer& b)
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
			/*if constexpr ( std::is_same<DefaultT, StringLiteralForComposing>::value )
			{
				composeString( b, nodecpp::string( defaultValue ) );
			}
			else
				composeString( b, nodecpp::string( defaultValue ) );*/
				composeString( b, defaultValue );
		}
		// TODO: add supported types here
		else
			static_assert( std::is_same<typename TypeToPick::Type, AllowedDataType>::value, "unsupported type" );
}


} // namespace impl

} // namespace m

#endif // NAMED_PARAMS_CORE_H
