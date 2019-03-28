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

#ifndef NODECPP_AWAITABLE_H
#define NODECPP_AWAITABLE_H

#include <experimental/coroutine>

namespace nodecpp {

template<typename T>
struct awaitable  {
	struct promise_type;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro = nullptr;
	using value_type = T;

	int myID;

	awaitable()  {}
	awaitable(handle_type h) : coro(h) {}

    awaitable(const awaitable &) = delete;
	awaitable &operator = (const awaitable &) = delete;

	awaitable(awaitable &&s) : coro(s.coro) {
		s.coro = nullptr;
	}
	awaitable &operator = (awaitable &&s) {
		coro = s.coro;
		s.coro = nullptr;
		return *this;
	}   
	
	~awaitable() {}

    T get() {
        return coro.promise().value;
    }

	bool await_ready() noexcept { 
		return false;
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		if ( coro )
			coro.promise().hr = h_;
	}
	T await_resume() { 
		if ( coro.promise().e_pending != nullptr )
		{
			std::exception_ptr ex = coro.promise().e_pending;
			coro.promise().e_pending = nullptr;
			std::rethrow_exception(ex);
		}
		return coro.promise().value; 
	}

	struct promise_type {
        T value;
		std::experimental::coroutine_handle<> hr = nullptr;
		std::exception_ptr e_pending = nullptr;

		promise_type() {}
		promise_type(const promise_type &) = delete;
		promise_type &operator = (const promise_type &) = delete;
		~promise_type() {}

        auto get_return_object() {
			auto h = handle_type::from_promise(*this);
			return awaitable<T>{h};
        }
        auto initial_suspend() {
//            return std::experimental::suspend_always{};
            return std::experimental::suspend_never{};
        }
        auto return_value(T v) {
            value = v;
            return std::experimental::suspend_never{};
        }
		auto final_suspend() {
			if ( hr )
			{
				auto tmph = hr;
				hr = nullptr;
				tmph();
			}
//			return std::experimental::suspend_always{};
			return std::experimental::suspend_never{};
        }
		void unhandled_exception() {
			e_pending = std::current_exception();
        }
	};
};

} // namespace nodecpp::awaitable


#endif // NODECPP_AWAITABLE_H