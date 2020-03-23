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

#ifndef TIMERS_H
#define TIMERS_H

#include "common.h"
#include <functional>

namespace nodecpp
{
	class Timeout
	{
		uint64_t id = 0;
	public:
		Timeout() {}
		Timeout(uint64_t id) :id(id) {}

		Timeout(const Timeout&) = delete;
		Timeout& operator=(const Timeout&) = delete;

		Timeout(Timeout&& other) :id(other.id) { other.id = 0; }
		Timeout& operator=(Timeout&& other) { std::swap(this->id, other.id); return *this; };

		~Timeout() {}

		uint64_t getId() const { return id; }

//		void refresh();
	};


	Timeout setTimeout(std::function<void()> cb, int32_t ms);
#ifndef NODECPP_NO_COROUTINES
	Timeout setTimeoutForAction(awaitable_handle_t h, int32_t ms);
#endif // NODECPP_NO_COROUTINES

	void clearTimeout(const Timeout& to);
	void refreshTimeout(Timeout& to);

	namespace time
	{
		size_t now();
	} // namespace time

	void setInmediate(std::function<void()> cb);
} // namespace nodecpp

uint64_t infraGetCurrentTime();

#ifdef USE_TEMP_PERF_CTRS
void reportTimes( uint64_t currentT);
#endif


#endif //TIMERS_H
