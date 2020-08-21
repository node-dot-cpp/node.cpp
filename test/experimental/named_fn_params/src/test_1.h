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

#ifndef TEST_1_H
#define TEST_1_H

#include "named_params_core.h"

namespace m {

// GENERATED VALUES

template<class T>
using FirstParam = NamedType<T, struct FirstParamTagStruct>;

template<class T>
using SecondParam = NamedType<T, struct SecondParamTagStruct>;

template<class T>
using ThirdParam = NamedType<T, struct ThirdParamTagStruct>;
	
extern const FirstParam<int>::argument firstParam;
extern const SecondParam<int>::argument secondParam;
extern const ThirdParam<int>::argument thirdParam;

void test1CallImpl_A( FirstParam<int> const& fp, SecondParam<int> const& sp, ThirdParam<int> const& tp );
void test1CallImpl_B( FirstParam<int> const& fp, SecondParam<std::string> const& sp, ThirdParam<int> const& tp );

template<typename Arg0, typename Arg1, typename Arg2>
void test1Call_A(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2)
{
    auto fp = pick<FirstParam<int>>(arg0, arg1, arg2);
    auto sp = pick<SecondParam<int>>(arg0, arg1, arg2);
    auto tp = pick<ThirdParam<int>>(arg0, arg1, arg2);
    test1CallImpl_A( fp, sp, tp );
}

template<typename Arg0, typename Arg1, typename Arg2>
void test1Call_B(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2)
{
    auto fp = pick<FirstParam<int>>(arg0, arg1, arg2);
    auto sp = pick<SecondParam<std::string>>(arg0, arg1, arg2);
    auto tp = pick<ThirdParam<int>>(arg0, arg1, arg2);
    test1CallImpl_B( fp, sp, tp );
}

} // namespace m

#endif // TEST_1_H
