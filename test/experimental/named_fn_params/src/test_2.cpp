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

#include "test_2.h"
#include <stdio.h>

namespace m {

// GENERATED VALUES

static const FirstParam::TypeConverter firstParam;
static const SecondParam::TypeConverter secondParam;
static const ThirdParam::TypeConverter thirdParam;

void test2Call_A_Impl( NamedParameterWithType<int, FirstParam::Name> const& fp, NamedParameterWithType<std::string, SecondParam::Name> const& sp, NamedParameterWithType<int, ThirdParam::Name> const& tp )
{
	printf( "Test 2: %d - %s - %d\n", fp.get(), sp.get().c_str(), tp.get() );
}

void test2Call_B_Impl( NamedParameterWithType<std::string, FirstParam::Name> const& fp, NamedParameterWithType<std::string, SecondParam::Name> const& sp, NamedParameterWithType<std::string, ThirdParam::Name> const& tp )
{
	printf( "Test 2: %s - %s - %s\n", fp.get().c_str(), sp.get().c_str(), tp.get().c_str() );
}

} // namespace m
