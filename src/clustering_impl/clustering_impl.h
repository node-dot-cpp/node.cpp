/* -------------------------------------------------------------------------------
* Copyright (c) 2019, OLogN Technologies AG
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


#ifndef CLUSTERING_COMMON_H
#define CLUSTERING_COMMON_H

#if (defined NODECPP_ENABLE_CLUSTERING) || (defined NODECPP_USE_Q_BASED_INFRA)


#include "../../include/nodecpp/common.h"
#include "interthread_comm.h"
#include "interthread_comm_impl.h"

struct ThreadStartupData
{
	ThreadID threadCommID;
	uintptr_t readHandle;
	InterThreadCommData* postman = nullptr;
	nodecpp::log::Log* defaultLog = nullptr;
	size_t IdWithinGroup;
};

class InterThreadMessagePostmanBase;
void preinitThreadStartupData( ThreadStartupData& startupData, InterThreadMessagePostmanBase* postman );
void decrementThisWorkerLoadCtr();

#endif // NODECPP_ENABLE_CLUSTERING

#endif // CLUSTERING_COMMON_H
