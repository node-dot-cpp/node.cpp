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

#ifndef TEST_2_H
#define TEST_2_H

#include "named_params_core.h"

namespace m {

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

// GENERATED VALUES

template<class T>
using FirstParam = NamedType<T, struct FirstParamTagStruct>;

template<class T>
using SecondParam = NamedType<T, struct SecondParamTagStruct>;

template<class T>
using ThirdParam = NamedType<T, struct ThirdParamTagStruct>;
	
extern const FirstParam<int>::argument firstParam2;
extern const SecondParam<int>::argument secondParam2;
extern const ThirdParam<int>::argument thirdParam2;

void test2CallImpl_A( FirstParam<int> const& fp, SecondParam<int> const& sp, ThirdParam<int> const& tp );
void test2CallImpl_B( FirstParam<int> const& fp, SecondParam<std::string> const& sp, ThirdParam<int> const& tp );
void test2CallImpl_C( FirstParam<std::string> const& fp, SecondParam<std::string> const& sp, ThirdParam<std::string> const& tp );

/*template<typename T1, typename T2>
struct IsSameType
{
    bool value = false;
};

template<bool ok, class DefaultT, DefaultT defaultValue>
DefaultT requiredParam()
{
    static_assert( ok, "required parameter" );
    return defaultValue;
}*/

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0, typename ... Args>
TypeToPick test2Call_pick1(Arg0&& arg0, Args&& ... args)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>, TypeToPick>::value )
        return arg0;
    else
        return test2Call_pick1<TypeToPick, required, AssumedDefaultT, DefaultT, defaultValue>(args...);
}

template<typename TypeToPick, bool required, class AssumedDefaultT, class DefaultT, DefaultT defaultValue, typename Arg0>
TypeToPick test2Call_pick1(Arg0&& arg0)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>, TypeToPick>::value )
        return arg0;
    else
    {
        static_assert( !required, "required parameter" );
        if constexpr ( std::is_same<AssumedDefaultT, DefaultT>::value )
            return TypeToPick(defaultValue);
        else
            return TypeToPick( AssumedDefaultT(defaultValue) );
        /*    return TypeToPick(defaultValue);*/
    }
}

/*template<typename TypeToPick, bool required, typename Arg0, typename ... Args>
TypeToPick test2Call_pick2(Arg0&& arg0, Args&& ... args)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>, TypeToPick>::value )
        return arg0;
    else
        return test2Call_pick2<TypeToPick, required>(args...);
}

template<typename TypeToPick, bool required, typename Arg0>
TypeToPick test2Call_pick2(Arg0&& arg0)
{
    if constexpr ( std::is_same<special_decay_t<Arg0>, TypeToPick>::value )
        return arg0;
    else
    {
        static_assert( !required, "required parameter" );
        return TypeToPick("default");
    }
}*/

template<typename Arg0, typename ... Args>
void test2Call_A(Arg0&& arg0, Args&& ... args)
{
    auto fp = test2Call_pick1<FirstParam<int>, false, int, int, 10>(arg0, args...);
    auto sp = test2Call_pick1<SecondParam<int>, false, int, int, 20>(arg0, args...);
    auto tp = test2Call_pick1<ThirdParam<int>, false, int, int, 30>(arg0, args...);
    test2CallImpl_A( fp, sp, tp );
}

template<typename Arg0, typename ... Args>
void test2Call_B(Arg0&& arg0, Args&& ... args)
{
    auto fp = test2Call_pick1<FirstParam<int>, false, int, int, 10>(arg0, args...);
    auto sp = test2Call_pick1<SecondParam<std::string>, false, std::string, const char [], "default">(arg0, args...);
    auto tp = test2Call_pick1<ThirdParam<int>, false, int, int, 30>(arg0, args...);
    test2CallImpl_B( fp, sp, tp );
}

static constexpr char default_1[] = "default_1";
static constexpr char default_2[] = "default_2";
static constexpr char default_3[] = "default_3";
template<typename Arg0, typename ... Args>
void test2Call_C(Arg0&& arg0, Args&& ... args)
{
    auto fp = test2Call_pick1<FirstParam<std::string>, false, std::string, const char [10], default_1>(arg0, args...);
    auto sp = test2Call_pick1<SecondParam<std::string>, false, std::string, const char [10], default_2>(arg0, args...);
    auto tp = test2Call_pick1<ThirdParam<std::string>, false, std::string, const char [10], default_3>(arg0, args...);
    test2CallImpl_C( fp, sp, tp );
}

} // namespace m

#endif // TEST_2_H
