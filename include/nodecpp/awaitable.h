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
	uint32_t status = 0;
	bool isException = false;
	std::exception e;
};

struct awaitable_base {
	CoroEData edata;
	std::exception_ptr e_pending = nullptr;
	bool is_value = false;

	awaitable_base() {}
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
        return std::experimental::suspend_never{};
    }
	auto final_suspend() {
		if ( hr )
		{
			auto tmph = hr;
			hr = nullptr;
			tmph();
		}
		return std::experimental::suspend_never{};
    }

#if (defined NODECPP_MEMORY_SAFETY) && (NODECPP_MEMORY_SAFETY == 0)
	void* operator new  ( std::size_t count ) { 
		uint8_t* ret = reinterpret_cast<uint8_t*>( safememory::detail::allocate(count + NODECPP_MAX_SUPPORTED_ALIGNMENT_FOR_NEW) );
		*reinterpret_cast<uint16_t*>( ret ) = nodecpp::iibmalloc::g_CurrentAllocManager ? nodecpp::iibmalloc::g_CurrentAllocManager->allocatorID() : 0;
		return ret + NODECPP_MAX_SUPPORTED_ALIGNMENT_FOR_NEW;
	}
	void operator delete ( void* ptr ) { 
		uint8_t* delptr = reinterpret_cast<uint8_t*>( ptr ) - NODECPP_MAX_SUPPORTED_ALIGNMENT_FOR_NEW;
		uint16_t allocatorID = *reinterpret_cast<uint16_t*>( delptr );
		safememory::detail::deallocate( delptr, allocatorID );
	}
#else
	void* operator new  ( std::size_t count ) { return safememory::detail::allocate(count); }
	void operator delete ( void* ptr ) { safememory::detail::deallocate(ptr); }
#endif
};

template<typename T> struct awaitable; // forward declaration

template<class T>
struct promise_type_struct : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<T>>;

	awaitable<T>* myRetObject = nullptr;

	promise_type_struct() : promise_type_struct_base() {}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct()  {}

    auto get_return_object();
    auto return_value(T v);
	void unhandled_exception();
};

template<>
struct promise_type_struct<void> : public promise_type_struct_base {
	using handle_type = std::experimental::coroutine_handle<promise_type_struct<void>>;

	awaitable<void>* myRetObject = nullptr;

	promise_type_struct() : promise_type_struct_base() {}
	promise_type_struct(const promise_type_struct &) = delete;
	promise_type_struct &operator = (const promise_type_struct &) = delete;
	promise_type_struct(promise_type_struct &&) = delete;
	promise_type_struct &operator = (promise_type_struct &&) = delete;
	~promise_type_struct() {}

    auto get_return_object();
	auto return_void(void);
	void unhandled_exception();
};


inline
void initCoroData(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.status = 0;
}

inline
void setCoroException(std::experimental::coroutine_handle<> awaiting, std::exception e) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.isException = true;
	edata.e = e;
}

inline
bool isCoroException(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.isException;
}

inline
std::exception& getCoroException(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, edata.isException ); 
	return edata.e;
}

inline
void setCoroStatus(std::experimental::coroutine_handle<> awaiting, uint32_t status) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	edata.status = status;
}

inline
uint32_t getCoroStatus(std::experimental::coroutine_handle<> awaiting) {
	auto& edata = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address()).promise().edata;
	return edata.status;
}


template<typename T>
struct awaitable : public awaitable_base  {
	typename void_type_converter<T>::type value;

	using promise_type = promise_type_struct<T>;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro = nullptr;
	bool coroDestroyed = false;
	using value_type = T;

	awaitable()  {}
	awaitable(handle_type h) : coro(h) { 
		coro.promise().myRetObject = this;
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
	
	~awaitable() {
		if ( !coroDestroyed )
			coro.promise().myRetObject = nullptr;
	}

	typename void_type_converter<T>::type& getValue() {
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return value;
	}

	typename void_type_converter<T>::type get() {
		if constexpr ( std::is_same<void, T>::value )
			return placeholder_for_void_ret_type();
		else
			return value;
	}

	bool await_ready() noexcept { 
		return is_value;		
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !coroDestroyed ); 
		coro.promise().hr = h_;
	}
	typename void_type_converter<T>::type await_resume() { 
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
		auto h = handle_type::from_promise(*this);
		return awaitable<void>{h};
}

template<class T>
auto promise_type_struct<T>::get_return_object() {
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

inline
auto promise_type_struct<void>::return_void(void) {
	if ( myRetObject != nullptr )
		myRetObject->is_value = true;
	return std::experimental::suspend_never{};
}

template<class T>
void promise_type_struct<T>::unhandled_exception() {
	if ( myRetObject != nullptr )
		myRetObject->e_pending = std::current_exception();
}

inline
void promise_type_struct<void>::unhandled_exception() {
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

inline void initCoroData(void (*)()) {}
inline void setCoroException(void (*)(), std::exception) {}
inline bool isCoroException(void (*)()) { return false; }
inline std::exception getCoroException(void (*)()) { return std::exception(); }
inline void setCoroStatus(void (*)(), int status) {}
inline int getCoroStatus(void (*)()) { return 0; }

template<class T>
struct awaitable
{
};

} // namespace nodecpp

#endif


#endif // NODECPP_AWAITABLE_H