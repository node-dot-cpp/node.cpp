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


#ifndef EV_QUEUE_H
#define EV_QUEUE_H

#include <vector>
#include <functional>

#include "../include/nodecpp/common.h"


class EvQueue
{
	template<class _Ty>
	using thisallocator = ::nodecpp::selective_allocator<::nodecpp::StdRawAllocator, _Ty>; // revise to use current node allocator
	std::vector<std::function<void()>, thisallocator<std::function<void()>>> evQueue;

	static constexpr bool DBG_SYNC = false;//for easier debug only
public:
	bool empty() const noexcept { return evQueue.empty(); }

	template<class M, class T, class... Args>
	void add(M T::* pm, T* inst, Args... args)
	{
		//code to call events async
		std::function<void()> ev = std::bind(pm, inst, args...);
		if (DBG_SYNC)
			emit(ev);
		else
			evQueue.push_back(std::move(ev));
	}

	void add(std::function<void()> ev)
	{
		if (DBG_SYNC)
			emit(ev);
		else
			evQueue.push_back(std::move(ev));
	}

	void emit() noexcept
	{
		//TODO: verify if exceptions may reach here from user code
		for (auto& current : evQueue)
		{
			emit(current);
		}
		evQueue.clear();
	}

	static
	void emit(std::function<void()>& ev) noexcept
	{
		//TODO wrapper so we don't let exceptions out of ev handler
		try
		{
			ev();
		}
		catch (...)
		{
			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"!!! Exception out of user handler !!!");
		}

	}
};


#endif // EV_QUEUE_H

