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

// GENERATED VALUES

using FirstParam = NamedParameter<struct FirstParamTagStruct>;

using SecondParam = NamedParameter<struct SecondParamTagStruct>;

using ThirdParam = NamedParameter<struct ThirdParamTagStruct>;
	
extern const FirstParam::TypeConverter firstParam;
extern const SecondParam::TypeConverter secondParam;
extern const ThirdParam::TypeConverter thirdParam;

void test2CallImpl_A( NamedParameterWithType<int, FirstParam::Name> const& fp, NamedParameterWithType<int, SecondParam::Name> const& sp, NamedParameterWithType<int, ThirdParam::Name> const& tp );
void test2CallImpl_B( NamedParameterWithType<int, FirstParam::Name> const& fp, NamedParameterWithType<std::string, SecondParam::Name> const& sp, NamedParameterWithType<int, ThirdParam::Name> const& tp );
void test2CallImpl_C( NamedParameterWithType<std::string, FirstParam::Name> const& fp, NamedParameterWithType<std::string, SecondParam::Name> const& sp, NamedParameterWithType<std::string, ThirdParam::Name> const& tp );

template<typename Arg0, typename ... Args>
void test2Call_A(Arg0&& arg0, Args&& ... args)
{
    ensureUniqueness(arg0.nameAndTypeID, args.nameAndTypeID...);
    auto fp = pickParam<NamedParameterWithType<int, FirstParam::Name>, false, int, int, 10>(arg0, args...);
    auto sp = pickParam<NamedParameterWithType<int, SecondParam::Name>, false, int, int, 20>(arg0, args...);
    auto tp = pickParam<NamedParameterWithType<int, ThirdParam::Name>, false, int, int, 30>(arg0, args...);
    test2CallImpl_A( fp, sp, tp );
}

namespace test2Call_B_defaults {
//static constexpr char default_1[] = "default_1";
static constexpr char default_2[] = "default_2";
//static constexpr char default_3[] = "default_3";
}

template<typename Arg0, typename ... Args>
void test2Call_B(Arg0&& arg0, Args&& ... args)
{
    ensureUniqueness(arg0.nameAndTypeID, args.nameAndTypeID...);
    auto fp = pickParam<NamedParameterWithType<int, FirstParam::Name>, false, int, int, 10>(arg0, args...);
    auto sp = pickParam<NamedParameterWithType<std::string, SecondParam::Name>, false, std::string, const char [], test2Call_B_defaults::default_2>(arg0, args...);
    auto tp = pickParam<NamedParameterWithType<int, ThirdParam::Name>, false, int, int, 30>(arg0, args...);
    test2CallImpl_B( fp, sp, tp );
}

namespace test2Call_C_defaults {
static constexpr char default_1[] = "default_1";
static constexpr char default_2[] = "default_2";
static constexpr char default_3[] = "default_3";
}

template<typename ... Args>
void test2Call_C(Args&& ... args)
{
    using arg_1_type = NamedParameterWithType<std::string, FirstParam::Name>;
    using arg_2_type = NamedParameterWithType<std::string, SecondParam::Name>;
    using arg_3_type = NamedParameterWithType<std::string, ThirdParam::Name>;
    ensureUniqueness(args.nameAndTypeID...);
    constexpr size_t argCount = sizeof ... (Args);
    constexpr size_t matchCount = isMatched_<arg_1_type>(args.nameAndTypeID...) + isMatched_<arg_2_type>(args.nameAndTypeID...) + isMatched_<arg_3_type>(args.nameAndTypeID...);
    static_assert( argCount == matchCount, "unexpected arguments found" );
    auto fp = pickParam<NamedParameterWithType<std::string, FirstParam::Name>, false, std::string, const char*, test2Call_C_defaults::default_1>(args...);
    auto sp = pickParam<NamedParameterWithType<std::string, SecondParam::Name>, false, std::string, const char*, test2Call_C_defaults::default_2>(args...);
    auto tp = pickParam<NamedParameterWithType<std::string, ThirdParam::Name>, false, std::string, const char*, test2Call_C_defaults::default_3>(args...);
    test2CallImpl_C( fp, sp, tp );
}

} // namespace m

#endif // TEST_2_H
