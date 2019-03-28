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
};

void processing_loop()
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
			g_callbacks[ch - '1'].awaiting();
		}
		else if ( ch >= 'a' && ch < 'a' + max_cnt )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - 'a'].exception = std::exception();
			g_callbacks[ch - 'a'].is_exception = true;
			g_callbacks[ch - 'a'].awaiting();
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
