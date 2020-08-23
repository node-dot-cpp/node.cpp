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

#ifndef NAMED_PARAMS_CORE_H
#define NAMED_PARAMS_CORE_H

#include <tuple>
#include <string>

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


template <typename T, typename NameTag>
class NamedParameterWithType : public NamedParameter<NameTag>
{
private:
	T value_;

public:
	explicit NamedParameterWithType(T const& value) : value_(value) {}
	explicit NamedParameterWithType(T&& value) : value_(std::move(value)) {}
	T& get() { return value_; }
	T const& get() const { return value_; }

	using NameBase = NamedParameter<NameTag>;
	using Name = typename NamedParameter<NameTag>::Name;
	using NamedParameter<NameTag>::TypeConverter;

    template<class NameT, class DataT>
    struct FullType
    {
        using NameType = NameT;
        using DataType = DataT;
    };

    using NameAndTypeID = FullType<NameTag, T>;
    static constexpr NameAndTypeID nameAndTypeID = {};
};



template<typename Arg0, typename ... Args>
constexpr size_t paramCount()
{
    return 1 + paramCount<Args...>();
}

template<typename Arg0>
constexpr size_t paramCount()
{
    return 1;
}


template<typename BaseT, typename Arg0, typename ... Args>
void findMatch(const Arg0 arg0, const Args ... args)
{
    static_assert( std::is_same<BaseT::NameType, Arg0::NameType>::value == false, "same name used more than once" );
    findMatch<BaseT>(args...);
}

template<typename BaseT, typename Arg0>
void findMatch(const Arg0 arg0)
{
    static_assert( std::is_same<BaseT::NameType, Arg0::NameType>::value == false, "same name used more than once" );
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



template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
TypeToPick pickParam(Arg0&& arg0, Args&& ... args)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>::Name, TypeToPick::Name>::value )
        return arg0;
    else
        return pickParam<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0>
TypeToPick pickParam(Arg0&& arg0)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>::Name, TypeToPick::Name>::value )
        return arg0;
    else
    {
        static_assert( !required, "required parameter" );
        if constexpr ( std::is_same<AssumedDefaultT, DefaultT>::value )
            return TypeToPick(defaultValue);
        else
            return TypeToPick( AssumedDefaultT(defaultValue) );
    }
}


template<typename TypeToMatch, typename Arg0, typename ... Args>
constexpr size_t isMatched_(const Arg0, const Args ... args)
{
    if constexpr ( std::is_same<Arg0::NameType, TypeToMatch::Name>::value )
        return 1;
    else
        return isMatched_<TypeToMatch>(args...);
}

template<typename TypeToMatch, typename Arg0>
constexpr size_t isMatched_(const Arg0)
{
    if constexpr ( std::is_same<Arg0::NameType, TypeToMatch::Name>::value )
        return 1;
    else
        return 0;
}


template<typename TypeToMatch, typename Arg0, typename ... Args>
constexpr size_t isMatched(const Arg0, const Args ... args)
{
    if constexpr ( std::is_same<Arg0::NameType, TypeToMatch::Name>::value )
        return 1;
    else
        return isMatched<TypeToMatch>(args...);
}

template<typename TypeToMatch, typename Arg0>
constexpr size_t isMatched(const Arg0)
{
    if constexpr ( std::is_same<Arg0::NameType, TypeToMatch::Name>::value )
        return 1;
    else
        return 0;
}

template<typename TypeToMatch0, typename ... TipesToMatch, typename ... Args>
constexpr size_t countMatches(const Args ... args)
{
    return isMatched<TypeToMatch0>(args...) + countMatches<TipesToMatch...>(args...);
}

template<typename TypeToMatch0, typename ... Args>
constexpr size_t countMatches(const Args ... args)
{
    return isMatched<TypeToMatch0>(args...);
}


} // namespace m

#endif // NAMED_PARAMS_CORE_H
