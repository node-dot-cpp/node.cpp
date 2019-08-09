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

#include <foundation.h>

#ifndef NODECPP_NO_COROUTINES

#include <experimental/coroutine>

#if defined NODECPP_MSVC
#define NODECPP_ALIGNED(x) __declspec(align(x))
#elif (defined NODECPP_CLANG) || (defined NODECPP_GCC)
#define NODECPP_ALIGNED(x) __attribute__ ((aligned(x)))
#endif

#define CO_RETURN co_return

namespace nodecpp {

class placeholder_for_void_ret_type
{
//	bool dummy = false;
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

struct CoroEData
{
	bool is_exception = false;
	std::exception exception;
};

struct promise_type_struct_base {
	static constexpr size_t valueAlignmentSize = 16;
	static constexpr size_t valueMemSize = std::max( valueAlignmentSize, std::max( sizeof( std::max_align_t ), sizeof( std::string ) ) );
	CoroEData edata;
	std::experimental::coroutine_handle<> hr = nullptr;
	std::exception_ptr e_pending = nullptr;
	bool is_value = false;
	uint8_t NODECPP_ALIGNED( valueAlignmentSize ) retValueMem[valueMemSize];

	promise_type_struct_base() {}
	promise_type_struct_base(const promise_type_struct_base &) = delete;
	promise_type_struct_base &operator = (const promise_type_struct_base &) = delete;
	promise_type_struct_base(promise_type_struct_base &&) = delete;
	promise_type_struct_base &operator = (promise_type_struct_base &&) = delete;
	~promise_type_struct_base() {}

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
};

template<typename T> struct awaitable; // forward declaration

template<class T>
struct promise_type_struct : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<T>>;
	static constexpr bool fitsToMem = sizeof(T) <= promise_type_struct_base::valueMemSize || valueAlignmentSize < alignof( T );

	T& getValue() { 
		if constexpr ( fitsToMem )
			return *reinterpret_cast<T*>(this->retValueMem); 
		else
			return **reinterpret_cast<T**>(this->retValueMem); 
	}

	promise_type_struct() : promise_type_struct_base() {
		if constexpr ( fitsToMem )
			new(this->retValueMem)T();
		else
			*reinterpret_cast<T**>(this->retValueMem) = new T;
	}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct() {
		if constexpr ( fitsToMem )
			getValue().~T();
		else
			delete *reinterpret_cast<T**>(this->retValueMem);
	}

    auto get_return_object();
    auto return_value(T v) {
//        value = v;
		getValue() = v;
		is_value = true;
        return std::experimental::suspend_never{};
    }
};

template<>
struct promise_type_struct<void> : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<void>>;
	promise_type_struct() : promise_type_struct_base() {}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct() {}

    auto get_return_object();
	auto return_void(void) {
		is_value = true;
        return std::experimental::suspend_never{};
    }
};


inline
void setNoException(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.is_exception = false;
}

inline
void setException(std::experimental::coroutine_handle<> awaiting, std::exception e) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.is_exception = true;
	edata.exception = e;
}

inline
bool isException(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.is_exception;
}

inline
std::exception& getException(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.exception;
}


template<typename T>
struct awaitable  {
	static_assert( sizeof(promise_type_struct<T>) == sizeof(promise_type_struct<void>) );
#ifdef NODECPP_MSVC
	static_assert( &((reinterpret_cast<promise_type_struct<T>*>((void*)(0x100000)))->edata) == &((reinterpret_cast<promise_type_struct<void>*>((void*)(0x100000)))->edata) );
#endif

	using promise_type = promise_type_struct<T>;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro = nullptr;
	using value_type = T;

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

	typename void_type_converter<T>::type get() {
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
//			return coro.promise().value;
			return coro.promise().getValue();
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
//			return coro.promise().value;
			return coro.promise().getValue();
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