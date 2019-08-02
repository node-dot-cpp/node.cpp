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

#include <nodecpp/common.h>

#ifndef NODECPP_NO_COROUTINES

#include <experimental/coroutine>

#if defined NODECPP_MSVC
#define NODECPP_ALIGNED(x) __declspec(align(x))
#elif (defined NODECPP_CLANG) || (defined NODECPP_GCC)
#define NODECPP_ALIGNED(x) __attribute__ ((aligned(x)))
#endif

#if defined NODECPP_MSVC
#define NODECPP_PROMISE_ALIGNMENT (std::experimental::coroutine_handle<int>::_ALIGN_REQ)
#elif (defined NODECPP_CLANG) || (defined NODECPP_GCC)
#define NODECPP_PROMISE_ALIGNMENT 8 // just a best approximation TODO: make it more precise!
#endif

#define CO_RETURN co_return

namespace nodecpp {

class placeholder_for_void_ret_type
{
	bool dummy = false;
public:
	placeholder_for_void_ret_type() {}
	placeholder_for_void_ret_type(const placeholder_for_void_ret_type&) {}
	placeholder_for_void_ret_type(placeholder_for_void_ret_type&&) {}
	placeholder_for_void_ret_type operator = (const placeholder_for_void_ret_type&) {return *this;}
	placeholder_for_void_ret_type operator = (placeholder_for_void_ret_type&&) {return *this;}
};

template<class T>
struct void_type_converter
{
	using type = T;
};

template<>
struct void_type_converter<void>
{
	using type = placeholder_for_void_ret_type;
};

template<typename T> struct awaitable; // forward declaration

template<class T>
struct promise_type_struct {
	std::experimental::coroutine_handle<> hr = nullptr;
	std::exception_ptr e_pending = nullptr;
	bool is_value = false;
	T value;

	struct EData
	{
		bool is_exception = false;
		std::exception exception;
	};
	EData NODECPP_ALIGNED( NODECPP_PROMISE_ALIGNMENT ) edata;

    auto initial_suspend() {
//            return std::experimental::suspend_always{};
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

	using handle_type = std::experimental::coroutine_handle<promise_type_struct<T>>;

	promise_type_struct() {}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	~promise_type_struct() {}

    auto get_return_object();
    auto return_value(T v) {
        value = v;
		is_value = true;
        return std::experimental::suspend_never{};
    }
};

template<>
struct promise_type_struct<void> {
	std::experimental::coroutine_handle<> hr = nullptr;
	std::exception_ptr e_pending = nullptr;
	bool is_value = false;

	struct EData
	{
		bool is_exception = false;
		std::exception exception;
	};
	EData NODECPP_ALIGNED( NODECPP_PROMISE_ALIGNMENT ) edata;

    auto initial_suspend() {
//            return std::experimental::suspend_always{};
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

	using handle_type = std::experimental::coroutine_handle<promise_type_struct<void>>;
	promise_type_struct() {}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	~promise_type_struct() {}

    auto get_return_object();
	auto return_void(void) {
		is_value = true;
        return std::experimental::suspend_never{};
    }
};



template<typename T>
struct awaitable  {
	using promise_type = promise_type_struct<T>;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro = nullptr;
	using value_type = T;


	awaitable()  {
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<uint8_t*>(&(std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address((void*)(0x100000)).promise().edata)) ==
				reinterpret_cast<uint8_t*>(&(std::experimental::coroutine_handle<nodecpp::promise_type_struct<T>>::from_address((void*)(0x100000)).promise().edata)) );
	}
	awaitable(handle_type h) : coro(h) {
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<uint8_t*>(&(std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address((void*)(0x100000)).promise().edata)) ==
				reinterpret_cast<uint8_t*>(&(std::experimental::coroutine_handle<nodecpp::promise_type_struct<T>>::from_address((void*)(0x100000)).promise().edata)) );
	}

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

    typename void_type_converter<T>::type get() {
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return coro.promise().value;
    }

	bool await_ready() noexcept { 
        return coro.promise().is_value;
//		return false;
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		if ( coro )
			coro.promise().hr = h_;
	}
	typename void_type_converter<T>::type await_resume() { 
		if ( coro.promise().e_pending != nullptr )
		{
			std::exception_ptr ex = coro.promise().e_pending;
			coro.promise().e_pending = nullptr;
			std::rethrow_exception(ex);
		}
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return coro.promise().value;
	}

};

inline
auto promise_type_struct<void>::get_return_object() {
		auto h = handle_type::from_promise(*this);
		return awaitable<void>{h};
}

template<class T>
auto promise_type_struct<T>::get_return_object() {
		auto h = handle_type::from_promise(*this);
		return awaitable<T>{h};
}

template<class ... T>
auto wait_for_all( nodecpp::awaitable<T>& ... calls ) -> nodecpp::awaitable<std::tuple<typename nodecpp::void_type_converter<T>::type...>>
{
	using ret_tuple_type = std::tuple<typename nodecpp::void_type_converter<T>::type...>;
	ret_tuple_type ret_t(std::move(co_await calls) ...);
	co_return ret_t;
}

} // namespace nodecpp

#else // main "candidate" for this #if/#else branch is GCC who is not supporting coroutines yet

#define CO_RETURN return

namespace nodecpp {

template<class T>
struct awaitable
{
};

} // namespace nodecpp

#endif


#endif // NODECPP_AWAITABLE_H