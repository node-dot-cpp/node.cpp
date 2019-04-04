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


#include <common.h>
#include <awaitable.h>
#include <assert.h>
#include <stdio.h>
#include <string>


struct handler_context
{
	bool used = false;
	std::experimental::coroutine_handle<> awaiting = nullptr;
	std::string data;
	bool is_exception = false;
	std::exception exception;
};

struct read_result
{
	std::string data;
	bool is_exception;
	std::exception exception;
};

static constexpr size_t max_h_count = 4;
static handler_context g_callbacks[max_h_count];
static int register_me() { printf( "register_me()\n"  ); for ( size_t i=0; i<max_h_count; ++i ) if ( !g_callbacks[i].used ) { g_callbacks[i].used = true; return (int)i; } assert( false ); return -1; }
static void unregister( size_t idx ) { assert( idx < max_h_count ); g_callbacks[idx].used = false; }

static void set_read_awaiting_handle(size_t idx, std::experimental::coroutine_handle<> awaiting) { g_callbacks[idx].awaiting = awaiting; } // returns immediately
bool is_data(size_t idx) { return g_callbacks[idx].data.size() != 0 ; }
read_result read_data(size_t idx) { 
	read_result ret; 
	ret.data = std::move( g_callbacks[idx].data ); 
	ret.is_exception = g_callbacks[idx].is_exception; 
	ret.exception = std::move( g_callbacks[idx].exception ); 
	return ret;
} // returns immediately

class data_reader
{
public:
	int myIdx = -1;

	auto read_data_or_wait() { 

		struct read_data_awaiter {
			int myIdx;

			std::experimental::coroutine_handle<> who_is_awaiting;

			read_data_awaiter(int myIdx_) {
				myIdx = myIdx_;
			}

			read_data_awaiter(const read_data_awaiter &) = delete;
			read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
			~read_data_awaiter() {}

			bool await_ready() {
				// consider checking is_data(myIdx) first
				return false;
			}

			void await_suspend(std::experimental::coroutine_handle<> awaiting) {
				who_is_awaiting = awaiting;
				set_read_awaiting_handle( myIdx, this->who_is_awaiting );
			}

			auto await_resume() {
				read_result r = read_data(myIdx);
				if ( r.is_exception )
					throw r.exception;
				return r.data;
			}
		};
		return read_data_awaiter(myIdx);
	}

public:
	data_reader() { 
		myIdx = register_me();
	}
	~data_reader() { 
		unregister( myIdx );
	}
};

class page_processor
{
	data_reader reader;

	nodecpp::awaitable<std::string> complete_block_1()
	{
		std::string accumulated;
		while ( accumulated.size() < 3 ) // jus some sample condition
		{
			int ctr = 0;
			try
			{
				auto v = co_await reader.read_data_or_wait();
				accumulated += v;
			}
			catch (std::exception& e)
			{
				printf("Exception caught at complete_block_1(): %s\n", e.what());
				throw;
			}
			++ctr;
		}

		accumulated += '\n';
		co_return accumulated;
	}

protected:
	nodecpp::awaitable<std::string> complete_page_1()
	{
		int ctr = 0;
		std::string accumulated;
		while ( ctr < 2 ) // an artificial sample condition
		{
			accumulated += co_await complete_block_1();
			++ctr;
		}

		co_return accumulated;
	}

public:
	page_processor() {}
	virtual ~page_processor() {}

	virtual nodecpp::awaitable<void> run() = 0;
	virtual nodecpp::awaitable<void> run1(size_t count) = 0;
	virtual nodecpp::awaitable<size_t> run2() = 0;
};

class page_processor_A : public page_processor
{
public:
	virtual nodecpp::awaitable<void> run()
	{
		for(int ctr = 0;;)
		{
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %d HAS BEEN PROCESSED with type A\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		co_return;
	}
	virtual nodecpp::awaitable<void> run1(size_t count)
	{
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		printf ( "run1( %zd ): about to co_return...\n", count ); 
		co_return;
	}
	virtual nodecpp::awaitable<size_t> run2()
	{
		size_t count = 2;
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		printf ( "type A run2( %zd ): about to co_return...\n", count ); 
		co_return count;
	}
};

class page_processor_B : public page_processor
{
public:
	virtual nodecpp::awaitable<void> run()
	{
		for(int ctr = 0;;)
		{
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %d HAS BEEN PROCESSED with type B\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_B::run(): %s\n", e.what());
			}
		}

		co_return;
	}
	virtual nodecpp::awaitable<void> run1(size_t count)
	{
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type B\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_B::run(): %s\n", e.what());
			}
		}

		printf ( "run1( %zd ): about to co_return...\n", count ); 
		co_return;
	}
	virtual nodecpp::awaitable<size_t> run2()
	{
		size_t count = 1;
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				auto page = co_await complete_page_1();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type B\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_B::run(): %s\n", e.what());
			}
		}

		printf ( "type B run2( %zd ): about to co_return...\n", count ); 
		co_return count;
	}
};



/*template<class T, class ... TT>
constexpr size_t count_all_types(T x  NODECPP_UNUSED_VAR)
{
	constexpr size_t ret = count_all_types<TT...>(x);
	return ret + 1;
}

template<class T>
constexpr size_t count_all_types(T& x)
{
	return 1;
}*/

template<class T, class ... TT>
constexpr size_t count_void_type()
{
	constexpr size_t ret = count_void_type<TT...>();
	if constexpr ( std::is_same<void, T>::value )
		return ret + 1;
	else
		return ret;
}

template<class T>
constexpr size_t count_void_type()
{
	if constexpr ( std::is_same<void, T>::value )
		return 1;
	else
		return 0;
}

#if 0
template<class T, class ... TT>
constexpr size_t count( T call, TT ... callls )
{
	constexpr size_t ret = count( calls ... );
	return ret + 1;
}

template<class T>
constexpr size_t count(T call)
{
	return 1;
}

template<class T, class ... TT>
constexpr size_t run_them( nodecpp::awaitable<void>* call_holder, T call_1, TT ... callls )
{
	size_t ret = run_them( call_holder, calls... );
	call_holder[ret]();
	return ret + 1;
}

template<class T>
constexpr size_t run_them( nodecpp::awaitable<void>* call_holder, T call_1 )
{
	call_holder[0]();
	return 1;
}

template<class T, class ... TT>
constexpr size_t run_them_2( nodecpp::awaitable<void>* call_holder, T call_1, TT ... callls )
{
	size_t ret = run_them_2( call_holder, calls... );
	call_holder[ret] = std::move( call_1 );
	return ret + 1;
}

template<class T>
constexpr size_t run_them_2( nodecpp::awaitable<void>* call_holder, T&& call_1 )
{
	call_holder[0] = std::move( call_1 );
	return 1;
}

/*template<class T, class ... TT>
constexpr size_t co_await_them( nodecpp::awaitable<void>* call_holder, T call_1, TT ... callls )
{
	size_t ret = co_await_them( call_holder, calls... );
	call_holder[ret]();
	return ret + 1;
}

template<class T>
constexpr size_t co_await_them( nodecpp::awaitable<void>* call_holder, T call_1 )
{
	co_await call_holder[0]();
	return 1;
}*/

template<class T, class ... TT>
nodecpp::awaitable<void> wait_for_all( T call_1, TT ... callls )
{
	constexpr size_t c = count( call1, calls ... );
	nodecpp::awaitable<void> call_holder[c];
	run_them( call_holder, call1, calls ... );
	co_await call_1();
	//wait_for_all( calls );
	for ( size_t i=0; i<c; ++i )
		co_await call_holder[i];
	printf( "all of %zd are done now\n", c );
	co_return;
}

template<class T, class ... TT>
nodecpp::awaitable<void> wait_for_all_2( T&& call_1, TT&& ... callls )
{
	constexpr size_t c = count( call1, calls ... );
	nodecpp::awaitable<void> call_holder[c];
	run_them_2( call_holder, call1, calls ... );
	//wait_for_all( calls );
	for ( size_t i=0; i<c; ++i )
		co_await call_holder[i];
	printf( "all of %zd are done now\n", c );
	co_return;
}
#endif // 0

class void_placeholder
{
	bool dummy = false;
public:
	void_placeholder() {}
	void_placeholder(const void_placeholder&) {}
	void_placeholder(void_placeholder&&) {}
	void_placeholder operator = (const void_placeholder&) {return *this;}
	void_placeholder operator = (void_placeholder&&) {return *this;}
};

template<class T>
struct void_type_filter
{
	using type = T;
};

template<>
struct void_type_filter<void>
{
	using type = void_placeholder;
};

/*template<class T>
T void_value_wrapper(T t)
{
	return t;
};

template<>
void_placeholder void_value_wrapper(void)
{
	return void_placeholder;
};*/

template<class TupleT, class RetTupleT, size_t idx>
nodecpp::awaitable<int> co_await_them( TupleT& t, RetTupleT& ret )
{
	printf( "co_await_them<%zd>()...\n", idx );
	using value_type = typename std::tuple_element<idx, RetTupleT>::type;
	if constexpr ( std::is_same< void_placeholder, value_type >::value )
		co_await std::get<idx>(t);
	else
		std::get<idx>(ret) = co_await std::get<idx>(t);
	printf( "co_await_them<%zd>(): continuing after awaiting\n", idx );
	if constexpr (idx )
		co_await co_await_them<TupleT, RetTupleT, idx-1>(t,ret);
	co_return 0;
}

template<class T, class TupleT, size_t idx>
constexpr size_t count_type( TupleT& t NODECPP_UNUSED_VAR )
{
	using item_value_type = typename std::tuple_element<idx, TupleT>::type;
	if constexpr ( idx == 0 )
	{
		if constexpr ( std::is_same< T, item_value_type >::value )
			return 1;
		else
			return 0;
	}
	else
	{
		if constexpr ( std::is_same< T, item_value_type >::value )
			return 1 + count_type<T, TupleT, idx-1>(t);
		else
			return count_type<T, TupleT, idx-1>(t);
	}
}

/*template<class T>
constexpr size_t co_await_them( nodecpp::awaitable<void>* call_holder, T call_1 )
{
	co_await call_holder[0]();
	return 1;
}*/

template<class ... T>
auto wait_for_all_3( nodecpp::awaitable<T>& ... calls ) -> nodecpp::awaitable<std::tuple<typename void_type_filter<T>::type...>>
//nodecpp::awaitable<size_t> wait_for_all_3( nodecpp::awaitable<T>& ... calls )
{
	using tuple_type = std::tuple<nodecpp::awaitable<T>...>;
	using ret_tuple_type = std::tuple<void_type_filter<T>::type...>;
	tuple_type t( std::move(calls)... );
	ret_tuple_type ret_t;
	constexpr size_t c = std::tuple_size<tuple_type>::value;
//	for ( size_t i=0; i<c; ++i )
//		co_await std::get<i>(t);
	co_await co_await_them<tuple_type, ret_tuple_type, c-1>(t, ret_t);
	printf( "all of %zd are done now\n", c );
//	printf( "all of zd are done now\n" );
	co_return ret_t;
//	co_return 0;
}

template<class T>
nodecpp::awaitable<T> await_me( nodecpp::awaitable<T>& a )
{
	if constexpr ( std::is_same<void, T>::value )
	{
		void_placeholder vp;
		co_await a;
		co_return vp;
	}
	else
		co_return co_await a;
}

template<class ... T>
auto wait_for_all_6( nodecpp::awaitable<T>& ... calls ) -> nodecpp::awaitable<std::tuple<typename void_type_filter<T>::type...>>
//nodecpp::awaitable<size_t> wait_for_all_3( nodecpp::awaitable<T>& ... calls )
{
	using ret_tuple_type = std::tuple<typename void_type_filter<T>::type...>;
	constexpr size_t total_cnt = std::tuple_size<ret_tuple_type>::value;
	ret_tuple_type t;
	constexpr size_t void_cnt = count_type<void_placeholder, ret_tuple_type, total_cnt-1>(t);
	if constexpr ( void_cnt == 0 )
	{
		ret_tuple_type ret_t(std::move(co_await calls) ...);
		co_return ret_t;
	}
	else if constexpr ( void_cnt == total_cnt )
	{
		ret_tuple_type ret_t( (co_await calls, std::move( void_placeholder())) ...);
		co_return ret_t;
	}
	else
	{
		ret_tuple_type ret_t( std::move(co_await await_me(calls)) ... );
		co_return ret_t;
	}
}

/*nodecpp::awaitable<size_t> wait_for_all_3()
{
	co_return 0;
}*/

#if 0
nodecpp::awaitable<int> wait_for_all_4( nodecpp::awaitable<int>& call_1, nodecpp::awaitable<int>& call_2 )
{
	using tuple_type = std::tuple<nodecpp::awaitable<int>,nodecpp::awaitable<int> >;
	tuple_type t( std::move(call_1), std::move(call_2) );
	constexpr size_t c = std::tuple_size<tuple_type>::value;
//	for ( size_t i=0; i<c; ++i )
//		co_await std::get<i>(t);
	int n;
	n = co_await std::get<0>(t);
	printf( "1st is done now\n" );
	n += co_await std::get<0>(t);
	printf( "all of %zd are done now\n", c );
//	printf( "all of zd are done now\n" );
	co_return 0;
}

nodecpp::awaitable<int> wait_for_all_5(nodecpp::awaitable<int>& call_1, nodecpp::awaitable<int>& call_2)
{
	int n;
	n = co_await call_1;
	printf( "1st is done now\n" );
//	bool ok = call_2.coro.done();
//	if ( !call_2.coro.promise().is_value )
//	if (!ok)
		n += co_await call_2;
	printf( "all are done now\n" );

	co_return 0;
}
#endif

#if 0
void processing_loop_2()
{ 
	static constexpr size_t bep_cnt = 2;
	page_processor* preader[bep_cnt];
	nodecpp::awaitable<void> run_ret[bep_cnt];

	for ( size_t i=0; i<bep_cnt; ++i )
	{
		if ( i % 2 == 0 )
			preader[i] = new page_processor_A;
		else
			preader[i] = new page_processor_B;
		run_ret[i] = preader[i]->run();
	}

	for (;;)
	{
		char ch = getchar();
		static_assert( max_h_count < 10 );
		static constexpr char max_cnt = (char)max_h_count;
		if ( ch > '0' && ch <= '0' + max_cnt )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - '1'].data.push_back( ch );
			g_callbacks[ch - '1'].is_exception = false;
			if ( g_callbacks[ch - '1'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - '1'].awaiting;
				g_callbacks[ch - '1'].awaiting = nullptr;
				tmp();
			}
			else
				printf("   --> ... just saving (nothing to resume)\n");
		}
		else if ( ch >= 'a' && ch < 'a' + max_cnt )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - 'a'].exception = std::exception();
			g_callbacks[ch - 'a'].is_exception = true;
			if ( g_callbacks[ch - 'a'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - 'a'].awaiting;
				g_callbacks[ch - 'a'].awaiting = nullptr;
				tmp();
			}
			else
				printf("   --> ... just saving (nothing to resume)\n");
		}
		else
		{
			printf( "   --> got \'%c\' (terminating)\n", ch );
			break;
			for ( size_t i=0; i<max_h_count; ++i )
			{
				if ( g_callbacks[i].awaiting )
					g_callbacks[i].awaiting.destroy();
			}
		}
	}
}
#endif // 0

nodecpp::awaitable<void> processing_loop_core()
{
	page_processor_A* preader_0 = new page_processor_A;
	page_processor_B* preader_1 = new page_processor_B;
	auto a1 = preader_0->run2();
	auto a2 = preader_1->run2();
	auto run_all_ret = wait_for_all_6( a1, a2 );
	auto fin_state = co_await run_all_ret;
	printf( "   results after waiting for all: %zd, %zd\n", std::get<0>(fin_state), std::get<1>(fin_state) );
}

void processing_loop()
{ 
	auto core = processing_loop_core();
//	static constexpr size_t bep_cnt = 2; // do not change it for a present implementation
	//nodecpp::awaitable<void> run_ret[bep_cnt];


//	typedef nodecpp::awaitable<void> (page_processor_A::*my_pointer_to_function_A) ();
//	typedef nodecpp::awaitable<void> (page_processor_B::*my_pointer_to_function_B) ();

//	my_pointer_to_function_A p_A = &page_processor_A::run2;
//	my_pointer_to_function_B p_B = &page_processor_B::run2;

//	nodecpp::awaitable<void> run_all_ret = wait_for_all( (*preader_0).*p_A, &((*(preader_1)).*page_processor_B::run2) );
//	nodecpp::awaitable<void> run_all_ret = wait_for_all( preader_0->*p_A, &((*(preader_1)).*page_processor_B::run2) );
//	nodecpp::awaitable<int> run_all_ret = std::move( wait_for_all_4( preader_0->run2(), preader_1->run2() ) );
//	auto a = preader_0->run2();
//	nodecpp::awaitable<int> run_all_ret = std::move( wait_for_all_5( std::move( a ) ) );
//	nodecpp::awaitable<int> run_all_ret = std::move( wait_for_all_3( preader_0->run2(), preader_1->run2() ) );

//	auto run_all_ret = std::move( wait_for_all_3( preader_0->run2(), preader_1->run2() ) );
//	nodecpp::awaitable<std::tuple<size_t, size_t>> run_all_ret = std::move( wait_for_all_3( preader_0->run2(), preader_1->run2() ) );
//	std::tuple<size_t, size_t> run_all_ret = co_await wait_for_all_3( preader_0->run2(), preader_1->run2() );
//	size_t run_all_ret = co_await wait_for_all_3( preader_0->run2(), preader_1->run2() );
//	size_t run_all_ret = co_await wait_for_all_3();

	for (;;)
	{
		char ch = getchar();
		static_assert( max_h_count < 10 );
		static constexpr char max_cnt = (char)max_h_count;
		if ( ch > '0' && ch <= '0' + max_cnt )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - '1'].data.push_back( ch );
			g_callbacks[ch - '1'].is_exception = false;
			if ( g_callbacks[ch - '1'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - '1'].awaiting;
				g_callbacks[ch - '1'].awaiting = nullptr;
				tmp();
			}
			else
				printf("   --> ... just saving (nothing to resume)\n");
		}
		else if ( ch >= 'a' && ch < 'a' + max_cnt )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - 'a'].exception = std::exception();
			g_callbacks[ch - 'a'].is_exception = true;
			if ( g_callbacks[ch - 'a'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - 'a'].awaiting;
				g_callbacks[ch - 'a'].awaiting = nullptr;
				tmp();
			}
			else
				printf("   --> ... just saving (nothing to resume)\n");
		}
		else
		{
			printf( "   --> got \'%c\' (terminating)\n", ch );
			break;
		}
	}
}
