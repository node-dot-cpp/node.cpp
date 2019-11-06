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

#if (defined NODECPP_WINDOWS) && (defined NODECPP_CLANG)
#include <coroutine.h>
#else
#include <experimental/coroutine>
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

struct awaitable_base {
	CoroEData edata;
	std::exception_ptr e_pending = nullptr;
	bool is_value = false;

	static constexpr size_t valueAlignmentSize = alignof(std::max_align_t);
	static constexpr size_t valueMemSizeBase = std::max( alignof( std::max_align_t ), 
		( sizeof( std::string ) / alignof(std::max_align_t) ) * alignof(std::max_align_t) + std::min( (size_t)1, sizeof( std::string ) % alignof(std::max_align_t)) * alignof(std::max_align_t) );
	static_assert( valueMemSizeBase % alignof(std::max_align_t) == 0 );
	static constexpr size_t valueMemSizeItems = valueMemSizeBase / sizeof(std::max_align_t) + std::min( (size_t)1, valueMemSizeBase % sizeof(std::max_align_t));
	std::max_align_t retValueMem[valueMemSizeItems];
	static constexpr size_t valueMemSize = sizeof( retValueMem );
	static_assert( valueMemSize >= sizeof( std::string ) );

	awaitable_base() {memset( retValueMem, 0xeb, sizeof(retValueMem));}
	awaitable_base(const awaitable_base &) = delete;
	awaitable_base &operator = (const awaitable_base &) = delete;
	awaitable_base(awaitable_base &&) = delete;
	awaitable_base &operator = (awaitable_base &&) = delete;
	~awaitable_base() {}
};

template<typename T>
struct awaitable; // forward declaration

struct promise_type_struct_base {
	CoroEData edata;
	std::experimental::coroutine_handle<> hr = nullptr;
	std::exception_ptr e_pending = nullptr;

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
//printf( "promise_type_struct_base::final_suspend(), this = 0x%zx [2], refCtr = %zd\n", (size_t)(this), refCtr );
printf( "promise_type_struct_base::final_suspend(), this = 0x%zx [2]\n", (size_t)(this) );
		if ( hr )
		{
			auto tmph = hr;
			hr = nullptr;
			tmph();
		}
//			return std::experimental::suspend_always{};
		return std::experimental::suspend_never{};
//		return std::experimental::suspend_if{refCtr != 0};
    }
};

template<typename T> struct awaitable; // forward declaration

template<class T>
struct promise_type_struct : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<T>>;

	awaitable<T>* myRetObject = nullptr;

	promise_type_struct() : promise_type_struct_base() {printf( "promise_type_struct(), this = 0x%zx [2]\n", (size_t)(this));}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct()  {printf( "~promise_type_struct(), this = 0x%zx [2]\n", (size_t)(this));}

    auto get_return_object();
    auto return_value(T v);
	void unhandled_exception();
};

template<>
struct promise_type_struct<void> : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<void>>;

	awaitable<void>* myRetObject = nullptr;

	promise_type_struct() : promise_type_struct_base() {printf( "promise_type_struct<void>(), this = 0x%zx\n", (size_t)(this));}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct() {printf( "~promise_type_struct<void>(), this = 0x%zx\n", (size_t)(this));}

    auto get_return_object();
	auto return_void(void);
	void unhandled_exception();
};


inline
void setNoException(std::experimental::coroutine_handle<> awaiting) {
	printf( "setNoException(), awaiting = 0x%zx[3]\n", (size_t)(awaiting.address()));
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.is_exception = false;
}

inline
void setException(std::experimental::coroutine_handle<> awaiting, std::exception e) {
	printf( "setException(), awaiting = 0x%zx[3]\n", (size_t)(awaiting.address()));
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.is_exception = true;
	edata.exception = e;
}

inline
bool isException(std::experimental::coroutine_handle<> awaiting) {
	printf( "isException(), awaiting = 0x%zx[3]\n", (size_t)(awaiting.address()));
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.is_exception;
}

inline
std::exception& getException(std::experimental::coroutine_handle<> awaiting) {
	printf( "getException(), awaiting = 0x%zx[3]\n", (size_t)(awaiting.address()));
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.exception;
}


template<typename T>
struct awaitable : public awaitable_base  {
	static_assert( sizeof(promise_type_struct<T>) == sizeof(promise_type_struct<void>) );
#ifdef NODECPP_MSVC
	// well, clang refuses considering casts as const_expr, and msvc agrees...
//	static_assert( &((reinterpret_cast<promise_type_struct<T>*>((void*)(0x100000)))->edata) == &((reinterpret_cast<promise_type_struct<void>*>((void*)(0x100000)))->edata) );
#endif
	typename void_type_converter<T>::type value;

	using promise_type = promise_type_struct<T>;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro = nullptr;
	bool coroDestroyed = false;
	using value_type = T;

//	std::experimental::coroutine_handle<void> hh = nullptr;

	awaitable()  {printf( "awaitable(), this = 0x%zx\n", (size_t)(this));}
	awaitable(handle_type h) : coro(h) { 
		coro.promise().myRetObject = this;
		printf( "awaitable(), this = 0x%zx, coro = 0x%zx[2]\n", (size_t)(this), (size_t)(coro.address()) - 0x80 ); /*hh = coro.promise().hr; */
	}

    awaitable(const awaitable &) = delete;
	awaitable &operator = (const awaitable &) = delete;

	awaitable(awaitable &&s) : coro(s.coro) {
		s.coro = nullptr;
		printf( "awaitable(), this = 0x%zx, coro = 0x%zx, from awaitable = 0x%zx[3]\n", (size_t)(this)), (size_t)(coro.address(), &s);
	}
	awaitable &operator = (awaitable &&s) {
		coro = s.coro;
		s.coro = nullptr;
		printf( "awaitable(), this = 0x%zx, coro = 0x%zx, from awaitable = 0x%zx[3]\n", (size_t)(this)), (size_t)(coro.address(), &s);
		return *this;
	}   
	
	~awaitable() {
		if ( !coroDestroyed )
		{
			printf( "    ~awaitable(): dereferencing coro 0x%zx\n", (size_t)(coro.address()) - 0x80 );
			coro.promise().myRetObject = nullptr;
		}
		else
			printf( "~awaitable(), this = 0x%zx\n", (size_t)(this) );
	}

	T& getValue() {
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return value;
	}

	typename void_type_converter<T>::type get() {
		printf( "get(), this = 0x%zx\n", (size_t)(this));
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return value;
	}

	bool await_ready() noexcept { 
		printf( "await_ready(), this = 0x%zx\n", (size_t)(this));
		return is_value;		
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		printf( "await_suspend(), this = 0x%zx, coro? %s\n", (size_t)(this), coro == nullptr ? "NO" : "Yes" );
		if ( coro )
			coro.promise().hr = h_;
		else
			h_();
		printf( "    await_suspend() -- done\n" );
	}
	typename void_type_converter<T>::type await_resume() { 
		printf( "await_resume(), this = 0x%zx\n", (size_t)(this));
		if ( e_pending != nullptr )
		{
			std::exception_ptr ex = e_pending;
			e_pending = nullptr;
			std::rethrow_exception(ex);
		}
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return this->getValue();
	}

};

inline
auto promise_type_struct<void>::get_return_object() {
		printf( "promise_type_struct<void>::get_return_object(), this = 0x%zx\n", (size_t)(this));
		auto h = handle_type::from_promise(*this);
		return awaitable<void>{h};
}

template<class T>
auto promise_type_struct<T>::get_return_object() {
		printf( "promise_type_struct<T>::get_return_object(), this = 0x%zx\n", (size_t)(this));
		printf( "await_resume(), this = 0x%zx\n", (size_t)(this));
		auto h = handle_type::from_promise(*this);
		return awaitable<T>{h};
}

template<class T>
auto promise_type_struct<T>::return_value(T v) {
	if ( myRetObject != nullptr )
	{
		myRetObject->getValue() = v;
		myRetObject->is_value = true;
	}
    return std::experimental::suspend_never{};
}

template<class T>
void promise_type_struct<T>::unhandled_exception() {
printf( "promise_type_struct::unhandled_exception(), this = 0x%zx [2]\n", (size_t)(this));
	if ( myRetObject != nullptr )
		myRetObject->e_pending = std::current_exception();
}

inline
auto promise_type_struct<void>::return_void(void) {
printf( "promise_type_struct::return_void(), this = 0x%zx\n", (size_t)(this));
if ( myRetObject != nullptr )
	myRetObject->is_value = true;
    return std::experimental::suspend_never{};
}

inline
void promise_type_struct<void>::unhandled_exception() {
printf( "promise_type_struct<void>::unhandled_exception(), this = 0x%zx [2]\n", (size_t)(this));
	if ( myRetObject != nullptr )
		myRetObject->e_pending = std::current_exception();
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

inline void setNoException(void (*)()) {}
inline void setException(void (*)(), std::exception) {}
inline bool isException(void (*)()) { return false; }
inline std::exception getException(void (*)()) { return std::exception(); }

template<class T>
struct awaitable
{
};

} // namespace nodecpp

#endif


#endif // NODECPP_AWAITABLE_H