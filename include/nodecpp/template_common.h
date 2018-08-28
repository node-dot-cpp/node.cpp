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
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------------------*/
#ifndef TEMPLATE_COMMON_H
#define TEMPLATE_COMMON_H

#include "common.h"

template<class T, bool ok>
class NODECPP_BRANCH_CHECK
{
public:
	NODECPP_BRANCH_CHECK() {static_assert( ok );}
};

template<class T, class T1, class ... args>
int getTypeIndex( T* ptr NODECPP_UNUSED_VAR )
{
	if constexpr ( std::is_same< T, T1 >::value )
		return 0;
	else
		return 1 + getTypeIndex<T, args...>(ptr);
}

template<class T>
int getTypeIndex( T* )
{
	NODECPP_BRANCH_CHECK<T, false> bc;
	return -1;
}

template<class T, class T1, class ... args>
int softGetTypeIndexIfTypeExists()
// Note: difference with getTypeIndex() is that 
// getTypeIndex() assumes that type T does exist in the list, and will fail at compile time, if a type DNE (which might be good to avoid run-time checks)
// softGetTypeIndexIfTypeExists() has no such assumptions 
{
	if constexpr ( std::is_same< T, T1 >::value )
		return 0;
	else
		return 1 + getTypeIndex<T, args...>(ptr);
}

template<class T>
int softGetTypeIndexIfTypeExists()
{
	return -1;
}


#endif // TEMPLATE_COMMON_H
