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


#include <nodecpp/common.h>
#include <nodecpp/awaitable.h>
#include <assert.h>
#include <stdio.h>
#include <string>

struct ret_str
{
//	int  NODECPP_ALIGNED( 32 ) dummy;
	std::string /*NODECPP_ALIGNED( 32 )*/ str;
	int  /*NODECPP_ALIGNED( 32 )*/ dummy1;
};


struct handler_context
{
	bool used = false;
	std::experimental::coroutine_handle<> awaiting = nullptr;
	std::string data;
};

struct read_result
{
	std::string data;
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
	return ret;
} // returns immediately

static int trigger = 0;

class data_reader
{
public:
	int myIdx = -1;

	auto read_data_or_wait_void() { 

		struct read_data_awaiter {
			int myIdx;
			std::experimental::coroutine_handle<> ah = nullptr;

			read_data_awaiter(int myIdx_) {
				myIdx = myIdx_;
			}

			read_data_awaiter(const read_data_awaiter &) = delete;
			read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
			~read_data_awaiter() {}

			bool await_ready() {
				return (trigger++)%3;
			}

			void await_suspend(std::experimental::coroutine_handle<> awaiting) {
				ah = awaiting;
				nodecpp::initCoroData(awaiting);
				set_read_awaiting_handle( myIdx, awaiting );
			}

			auto await_resume() {
				if ( ah != nullptr && nodecpp::isCoroException(ah) )
					throw nodecpp::getCoroException(ah);
				return;
			}
		};
		return read_data_awaiter(myIdx);
	}

	auto read_data_or_wait() { 

		struct read_data_awaiter {
			int myIdx;
			std::experimental::coroutine_handle<> ah = nullptr;

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
				ah = awaiting;
//				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address());
//				h.promise().edata.is_exception = false;
				nodecpp::initCoroData(awaiting);
				set_read_awaiting_handle( myIdx, awaiting );
			}

			auto await_resume() {
//				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(ah.address());
				read_result r = read_data(myIdx);
				if ( ah != nullptr && nodecpp::isCoroException(ah) )
					throw nodecpp::getCoroException(ah);
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

	nodecpp::awaitable<void> complete_block_void()
	{
		size_t accumulated = 0;
		while ( accumulated < 6 ) // jus some sample condition
		{
			try
			{
				co_await reader.read_data_or_wait_void();
				accumulated++;
			}
			catch (std::exception& e)
			{
				printf("Exception caught at complete_block_1(): %s\n", e.what());
				throw;
			}
		}
		co_return;
	}

	nodecpp::awaitable<std::string> complete_block_1()
//	nodecpp::awaitable<ret_str> complete_block_1()
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
/*		ret_str r;
		r.str = accumulated;
		co_return r;*/
	}

protected:
	nodecpp::awaitable<void> complete_page_void()
	{
		size_t accumulated = 0;
		while ( accumulated < 2 ) // an artificial sample condition
		{
			co_await complete_block_void();
			accumulated++;
		}
		co_return;

	}

	nodecpp::awaitable<std::string> complete_page_1()
//	nodecpp::awaitable<ret_str> complete_page_1()
	{
		int ctr = 0;
		std::string accumulated;
		while ( ctr < 2 ) // an artificial sample condition
		{
//			accumulated += (co_await complete_block_1()).str;
			accumulated += co_await complete_block_1();
			++ctr;
		}

		co_return accumulated;
/*		ret_str r;
		r.str = accumulated;
		co_return r;*/
	}

public:
	page_processor() {}
	virtual ~page_processor() {}

	virtual nodecpp::awaitable<void> runVoid() = 0;
	virtual nodecpp::awaitable<void> run1Void(size_t count) = 0;
	virtual nodecpp::awaitable<size_t> run2Void() = 0;

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
//				auto page = (co_await complete_page_1()).str;
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
//				auto page = (co_await complete_page_1()).str;
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
//				auto page = (co_await complete_page_1()).str;
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

	virtual nodecpp::awaitable<void> runVoid()
	{
		for(int ctr = 0;;)
		{
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %d HAS BEEN PROCESSED with type A\n", ctr );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		co_return;
	}
	virtual nodecpp::awaitable<void> run1Void(size_t count)
	{
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n", ctr );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		printf ( "run1( %zd ): about to co_return...\n", count ); 
		co_return;
	}
	virtual nodecpp::awaitable<size_t> run2Void()
	{
		size_t count = 2;
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n", ctr );
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
//				auto page = (co_await complete_page_1()).str;
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
//				auto page = (co_await complete_page_1()).str;
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
//				auto page = (co_await complete_page_1()).str;
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

	virtual nodecpp::awaitable<void> runVoid()
	{
		for(int ctr = 0;;)
		{
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %d HAS BEEN PROCESSED with type A\n", ctr );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		co_return;
	}
	virtual nodecpp::awaitable<void> run1Void(size_t count)
	{
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n", ctr );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at page_processor_A::run(): %s\n", e.what());
			}
		}

		printf ( "run1( %zd ): about to co_return...\n", count ); 
		co_return;
	}
	virtual nodecpp::awaitable<size_t> run2Void()
	{
		size_t count = 2;
		printf ( "run1( %zd )\n", count ); 
		for(size_t ctr = 0; ctr<count;)
		{
			printf ( "run1(): starting page %zd\n", ctr );
			try
			{
				co_await complete_page_void();
				++ctr;
				printf( "PAGE %zd HAS BEEN PROCESSED with type A\n", ctr );
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
			if ( g_callbacks[ch - 'a'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - 'a'].awaiting;
				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(tmp.address());
				nodecpp::setCoroException(tmp, std::exception());
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

class page_processor_alt
{
	data_reader reader;

	nodecpp::awaitable<void> complete_block_void()
	{
		size_t accumulated = 0;
		while ( accumulated < 6 ) // jus some sample condition
		{
			try
			{
				co_await reader.read_data_or_wait_void();
				accumulated++;
			}
			catch (std::exception& e)
			{
				printf("Exception caught at complete_block_1(): %s\n", e.what());
				throw;
			}
		}
		co_return;
	}

protected:
	nodecpp::awaitable<void> complete_page_void()
	{
		size_t accumulated = 0;
		while ( accumulated < 2 ) // an artificial sample condition
		{
			co_await complete_block_void();
			accumulated++;
		}
		co_return;
	}

public:
	page_processor_alt() {}
	virtual ~page_processor_alt() {}

	nodecpp::awaitable<void> coro_run()
	{
		co_await complete_page_void();
		fmt::print( "everything is done\n" );
		co_return;
	}

	void just_void_run()
	{
		complete_page_void();
		fmt::print( "if we're here immediately, something went wrong\n" );
	}

	nodecpp::awaitable<void> coro_run_no_await()
	{
		auto ar = complete_page_void();
		fmt::print( "if we're here immediately, something went wrong\n" );
		co_return;
	}

	nodecpp::awaitable<void> coro_run_no_await2()
	{
		complete_page_void();
		fmt::print( "if we're here immediately, something went wrong\n" );
		co_return;
	}

	nodecpp::awaitable<void> coro_run_no_await_admissible()
	{
		auto ar = complete_page_void();
		ar.dbgAwaitingNotPlannedReturnedValueOfNoInterest();
		fmt::print( "if we're here immediately, it could, in general, be wrong, but we don't care about it\n" );
		co_return;
	}
};

void test_coro_presence()
{
	page_processor_alt p;
//	nodecpp::awaitable<void> ar = p.just_void_run();
//	nodecpp::awaitable<void> ar = p.coro_run_no_await();
	nodecpp::awaitable<void> ar = p.coro_run_no_await_admissible();
//	nodecpp::awaitable<void> ar = p.coro_run();

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
			if ( g_callbacks[ch - 'a'].awaiting != nullptr )
			{
				auto tmp = g_callbacks[ch - 'a'].awaiting;
				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(tmp.address());
				nodecpp::setCoroException(tmp, std::exception());
				g_callbacks[ch - 'a'].awaiting = nullptr;
				tmp();
			}
			else
				printf("   --> ... just saving (nothing to resume)\n");
		}
		else
		{
			getchar(); // take '\n' out of stream
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

struct Processors
{
	page_processor_A* preader_0;
	page_processor_B* preader_1;
	page_processor_A* preader_2;
	Processors()
	{
		preader_0 = new page_processor_A;
		preader_1 = new page_processor_B;
		preader_2 = new page_processor_A;
	}
	~Processors()
	{
		delete preader_0;
		delete preader_1;
		delete preader_2;
	}
};

nodecpp::awaitable<void> processing_loop_core(Processors& p)
{
	auto a1 = p.preader_0->run2();
	auto a2 = p.preader_1->run2();
	auto a3 = p.preader_2->run1(1);
	auto run_all_ret = nodecpp::wait_for_all( a1, a2, a3 );
	auto fin_state = co_await run_all_ret;
	printf( "   results after waiting for all: %zd, %zd\n", std::get<0>(fin_state), std::get<1>(fin_state) );
}

void processing_loop_core_2(Processors& p)
{
	p.preader_0->run2();
	p.preader_1->run2();
	p.preader_2->run1(1);
}

nodecpp::awaitable<void> processing_loop_core_3(Processors& p)
{
	p.preader_0->run2();
	p.preader_1->run2();
	p.preader_2->run1(1);
	co_return;
}

template<class P>
nodecpp::awaitable<void> processing_loop_core_4(P& p)
{
	p.preader_0->run2();
	p.preader_1->run2();
	p.preader_2->run1(1);
	co_return;
}

template<class P>
nodecpp::awaitable<void> processing_loop_core_4_void(P& p)
{
	auto r1 = p.preader_0->runVoid();
	r1.dbgAwaitingNotPlannedReturnedValueOfNoInterest();
	auto r2 = p.preader_1->run2Void();
	r2.dbgAwaitingNotPlannedReturnedValueOfNoInterest();
	auto r3 = p.preader_2->run1Void(1);
	r3.dbgAwaitingNotPlannedReturnedValueOfNoInterest();
	co_return;
}

template<class P, class ... args>
nodecpp::awaitable<void> processing_loop_core_5(P& p)
{
	p.preader_0->run2();
	p.preader_1->run2();
	p.preader_2->run1(1);
	co_return;
}

void processing_loop()
{ 
	printf( "sizeof(srd::string) = %zd\n", sizeof(std::string) );
//	safememory::detail::interceptNewDeleteOperators(true);
	{
		Processors p;
//		/*auto core =*/ processing_loop_core_3(p);
//		processing_loop_core_4(p);
		processing_loop_core_4_void(p);
//		processing_loop_core_5<Processors, int>(p);

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
				if ( g_callbacks[ch - 'a'].awaiting != nullptr )
				{
					auto tmp = g_callbacks[ch - 'a'].awaiting;
					std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(tmp.address());
					nodecpp::setCoroException(tmp, std::exception());
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
//	safememory::detail::interceptNewDeleteOperators(false);
}
