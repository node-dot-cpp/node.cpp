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


struct handler_context
{
	bool used = false;
	std::experimental::coroutine_handle<> awaiting = nullptr;
	std::string data;
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
	return ret;
} // returns immediately

class data_reader
{
public:
	int myIdx = -1;

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
				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(awaiting.address());
				ah = awaiting;
				h.promise().edata.is_exception = false;
				set_read_awaiting_handle( myIdx, awaiting );
			}

			auto await_resume() {
				std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>> h = std::experimental::coroutine_handle<nodecpp::promise_type_struct<void>>::from_address(ah.address());
				read_result r = read_data(myIdx);
				if ( h.promise().edata.is_exception )
					throw h.promise().edata.exception;
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
				h.promise().edata.exception = std::exception();
				h.promise().edata.is_exception = true;
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
	nodecpp::safememory::interceptNewDeleteOperators(true);
	{
		Processors p;
//		/*auto core =*/ processing_loop_core_3(p);
		processing_loop_core_4(p);
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
					h.promise().edata.exception = std::exception();
					h.promise().edata.is_exception = true;
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
	nodecpp::safememory::interceptNewDeleteOperators(false);
}
