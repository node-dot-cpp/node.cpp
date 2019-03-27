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

#include <experimental/coroutine>
#include <stdio.h>
#include <assert.h>
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

static int object_id_base = 0;
static int getID() { return ++object_id_base; }

template<typename T>
struct my_sync_awaitable  {
	struct promise_type;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro;
	using value_type = T;
	bool from_coro = false;

	int myID;

    my_sync_awaitable()  {
		myID = getID();
		printf( "my_sync_awaitable::my_sync_awaitable() with myID = %d (no params)\n", myID );
   }
	my_sync_awaitable(handle_type h) : coro(h) {
		myID = getID();
		from_coro = true;
		printf( "my_sync_awaitable::my_sync_awaitable() with myID = %d (from coro; my coro = 0x%zx)\n", myID, (size_t)(&coro) );
	}

    my_sync_awaitable(const my_sync_awaitable &) = delete;
	my_sync_awaitable &operator = (const my_sync_awaitable &) = delete;

	my_sync_awaitable(my_sync_awaitable &&s) : coro(s.coro) {
		s.coro = nullptr;
		from_coro = s.from_coro;
		myID = getID();
		printf( "my_sync_awaitable::my_sync_awaitable() with myID = %d (moving, other ID = %d)\n", myID, s.myID );
	}
	my_sync_awaitable &operator = (my_sync_awaitable &&s) {
		coro = s.coro;
		s.coro = nullptr;
		from_coro = s.from_coro;
		printf( "my_sync_awaitable::operator =() with myID = %d (moving, other ID = %d)\n", myID, s.myID );
		return *this;
	}   
	
	~my_sync_awaitable() {
		printf( "my_sync_awaitable::~my_sync_awaitable() with myID = %d (this = 0x%zx)\n", myID, (size_t)this );
		if ( coro && !from_coro ) {
			printf( "destroying core: myid = %d, &coro = 0x%zx\n", myID, (size_t)(&coro) );
			coro.destroy();
		}
    }

    T get() {
		printf( "my_sync_awaitable::get() with myID = %d, this = 0x%zx\n", myID, (size_t)this ); 
        return coro.promise().value;
    }

	bool await_ready() noexcept { 
		printf( "my_sync_awaitable::await_ready() with myID = %d, this = 0x%zx\n", myID, (size_t)this ); 
		return false;
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		printf( "my_sync_awaitable::await_suspend() with myID = %d, this = 0x%zx\n", myID, (size_t)this ); 
		who(); 
		if ( coro )
		{
			coro.promise().hr_set = true;
			coro.promise().hr = h_;
		}
	}
	T await_resume() { 
		printf( "my_sync_awaitable::await_resume() with myid = %d, this = 0x%zx\n", myID, (size_t)this ); 
		if ( coro.promise().e_pending != nullptr )
		{
			std::exception_ptr ex = coro.promise().e_pending;
			coro.promise().e_pending = nullptr;
			std::rethrow_exception(ex);
		}
		return coro.promise().value; 
	}
	
	void who() const { printf( "WHO: my_sync_awaitable with myID = %d\n", myID ); }

	struct promise_type {
        T value;
		int myID;
		std::experimental::coroutine_handle<> hr = nullptr;
		bool hr_set = false;

        promise_type() {
			myID = getID();
            printf( "my_sync_awaitable::promise_type::promise_type() [1], myID = %d\n", myID );
        }
		promise_type(const promise_type &) = delete;
		promise_type &operator = (const promise_type &) = delete;
		/*promise_type( promise_type&& other ) : value( std::move(other.value) )
		{
            printf( "my_sync_awaitable::promise_type::promise_type() [move]\n" );
			hr = std::move( other.hr );
			hr_set = other.hr_set;
			other.hr_set = false;
		}*/
        ~promise_type() {
            printf( "my_sync_awaitable::promise_type::~promise_type() with myID == %d [1]\n", myID );
        }
        auto get_return_object() {
            printf( "my_sync_awaitable::get_return_object() [1]\n" );
			auto h = handle_type::from_promise(*this);
            printf( "my_sync_awaitable::get_return_object() [2]\n" );
			return my_sync_awaitable<T>{h};
        }
		void who() const { printf( "WHO: my_sync_awaitable::promise_type with myID = %d\n", myID ); }
        auto initial_suspend() {
            printf( "my_sync_awaitable::initial_suspend() [1]\n" );
//            return std::experimental::suspend_always{};
            return std::experimental::suspend_never{};
        }
        auto return_value(T v) {
            printf( "my_sync_awaitable::return_value() [1]\n" );
            value = v;
            return std::experimental::suspend_never{};
        }
		auto final_suspend() {
            printf( "my_sync_awaitable::final_suspend() [1] with myID = %d\n", myID );
			if ( hr_set )
			{
				printf("my_sync_awaitable::final_suspend() [2] with myID = %d\n", myID );
				hr_set = false;
				hr();
				printf("my_sync_awaitable::final_suspend() [3] with myID = %d\n", myID );
			}
//			return std::experimental::suspend_always{};
			return std::experimental::suspend_never{};
        }
		std::exception_ptr e_pending = nullptr;
		void unhandled_exception() {
            printf( "my_sync_awaitable::unhandled_exception() with myID = %d\n", myID );
			e_pending = std::current_exception();
        }
	};
};

class line_reader
{
public:
	int myIdx = -1;
	static constexpr int threashold = 3;

	auto read_data_or_wait() { 

		printf( "   +++> +++> read_data_or_wait(): [1] (id = %d) \n", myIdx );

		struct myLazyInFn {
			int myID;
			int myIdx;

			void who() const { printf("WHO: myLazyInFn with myID = %d\n", myID); }
			std::experimental::coroutine_handle<> who_is_awaiting;

			myLazyInFn(int myIdx_) {
				myID = getID();
				myIdx = myIdx_;
				printf( "myLazyInFn::myLazyInFn() with id %d, myID = %d\n", myIdx, myID );
			}

			myLazyInFn(const myLazyInFn &) = delete;
			myLazyInFn &operator = (const myLazyInFn &) = delete;

			/*myLazyInFn(myLazyInFn &&s) {
				myID = getID();
				myIdx = s.myIdx;
				printf( "myLazyInFn::myLazyInFn() with id %zd, myid = %d from other.myid = %d\n", myIdx, myID, s.myID );
			}
			myLazyInFn &operator = (myLazyInFn &&s) {
				myIdx = s.myIdx;
				printf( "myLazyInFn::operator =() with id %zd, myid = %d from other.myid = %d\n", myIdx, myID, s.myID );
				return *this;
			}  */ 
	
			~myLazyInFn() {}

			bool await_ready() {
				// consider checking is_data(myIdx) first
				printf( "   ---> ---> ---> In await_ready(): id = %d\n", myIdx );
				return false;
			}

			void await_suspend(std::experimental::coroutine_handle<> awaiting) {
				who_is_awaiting = awaiting;
				printf( "   ---> ---> ---> In await_suspend(): about to call set_read_callback(); id = %d\n", myIdx );
				set_read_awaiting_handle( myIdx, this->who_is_awaiting );
				printf( "   ---> ---> ---> In await_suspend(): after calling set_read_callback(); id = %d\n", myIdx );
			}

			auto await_resume() {
				read_result r = read_data(myIdx);
				printf( "   ---> ---> ---> In await_resume(): id = %d %s\n", myIdx, r.is_exception ? "with exception" : "" );
				if ( r.is_exception )
					throw r.exception;
				return r.data;
			}
		};
		printf( "   +++> +++> read_data_or_wait(): [about to create myLazyInFn] (id = %d)\n", myIdx );
		return myLazyInFn(myIdx);
	}

	my_sync_awaitable<std::string> complete_block_1()
	{
		std::string accumulated;
		int ctr = 0;
		while ( accumulated.size() < threashold )
		{
			printf( "   ---> my_sync<int> complete_block_1(): [1] (id = %d, ctr = %d)\n", myIdx, ctr );
			try
			{
				auto v = co_await read_data_or_wait();
				printf( "v.size() = %zd, v = %s\n", v.size(), v.c_str() );
				accumulated += v;
			}
			catch (std::exception& e)
			{
				printf("Exception caught at complete_block_1(): %s\n", e.what());
			}
			printf( "   ---> my_sync<int> complete_block_1(): [3] (id = %d, ctr = %d)\n", myIdx, ctr );
			++ctr;
		}

		printf( "   ---> my_sync<int> complete_block_1(): [about to exit] (id = %d), ctr = %d)\n", myIdx, ctr );
		accumulated += '\n';
		co_return accumulated;
	}

public:
	line_reader() { 
		printf( "line_reader()::line_reader\n" );
		myIdx = register_me();
	}
	~line_reader() { 
		unregister( myIdx );
		printf( "line_reader()::~line_reader\n" );
	}
};

class page_reader
{
public:
	line_reader l_reader;

public:
	static constexpr int threashold = 2;

	my_sync_awaitable<std::string> complete_page_1()
	{
		int ctr = 0;
		std::string accumulated;
		while ( ctr < threashold )
		{
			printf( "   ---> my_sync<int> complete_page_1(): [1] (ctr = %d)\n", ctr );
			accumulated += co_await l_reader.complete_block_1();
			printf( "   ---> my_sync<int> complete_page_1(): [2] (ctr = %d)\n", ctr );
			++ctr;
		}
		printf( "   ---> my_sync<int> complete_page_1(): [about to exit]\n" );

		co_return accumulated;
	}

public:
	page_reader() {
		printf( "page_reader()::page_reader\n" );
	}
	~page_reader() {
		printf( "page_reader()::~page_reader\n" );
	}
};

class page_processor
{
	page_reader processor;

public:
	page_processor() {
		printf( "page_processor()::page_processor\n" );
	}
	~page_processor() {
		printf( "page_processor()::~page_processor\n" );
	}
	my_sync_awaitable<int> run()
	{
		for(int ctr = 0;;)
		{
			try
			{
				auto page = co_await processor.complete_page_1();
				++ctr;
				printf( "PAGE %d HAS BEEN PROCESSED\n%s", ctr, page.c_str() );
			}
			catch (std::exception& e)
			{
				printf("Exception caught at run(): %s\n", e.what());
			}
		}
		co_return 0;
	}
};

void processing_loop_3()
{ 
	static constexpr size_t bep_cnt = 1;
	page_processor preader[bep_cnt];
	my_sync_awaitable<int> run_ret[bep_cnt];

	for ( size_t i=0; i<bep_cnt; ++i )
		run_ret[i] = preader[i].run();

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
