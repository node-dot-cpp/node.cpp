
#include <assert.h>
#include <experimental/coroutine>
#include <future>
#include <stdio.h>
#include <optional>
#include <iostream>
using namespace std;


struct handler_context
{
	bool used = false;
	std::experimental::coroutine_handle<> awaiting;
	std::string data;
	bool is_exception;
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
static void init_handler_context() { memset(g_callbacks, 0, sizeof( g_callbacks ) ); }
static size_t register_me() { printf( "register_me()\n"  ); for ( size_t i=0; i<max_h_count; ++i ) if ( !g_callbacks[i].used ) { g_callbacks[i].used = true; return i; } assert( false ); return (size_t)(-1); }
static void unregister( size_t idx ) { assert( idx < max_h_count ); g_callbacks[idx].used = false; }

static void set_read_awaiting_handle(size_t idx, std::experimental::coroutine_handle<> awaiting) { g_callbacks[idx].awaiting = awaiting; } // returns immediately
read_result read_data(size_t idx) { read_result ret; ret.data = std::move( g_callbacks[idx].data ); ret.is_exception = g_callbacks[idx].is_exception; ret.exception = std::move( g_callbacks[idx].exception ); return ret; } // returns immediately
bool is_data(size_t idx) { return g_callbacks[idx].data.size() != 0 ; }

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
			printf( "destroying core: myID = %zd, &coro = 0x%zx\n", myID, (size_t)(&coro) );
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
		printf( "my_sync_awaitable::await_resume() with myID = %zd, this = 0x%zx\n", myID, (size_t)this ); 
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
            //printf( "my_sync_awaitable::get_return_object() [1]\n" );
			auto h = handle_type::from_promise(*this);
            //printf( "my_sync_awaitable::get_return_object() [2]\n" );
			return my_sync_awaitable<T>{h};
        }
		void who() const { printf( "WHO: my_sync_awaitable::promise_type with myID = %d\n", myID ); }
        auto initial_suspend() {
            //printf( "my_sync_awaitable::initial_suspend() [1]\n" );
//            return std::experimental::suspend_always{};
            return std::experimental::suspend_never{};
        }
        auto return_value(T v) {
            //printf( "my_sync_awaitable::return_value() [1]\n" );
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
	int ctr = 0;
	size_t myIdx = (size_t)(-1);
	static constexpr int threashold = 3;

	auto read_data_or_wait() { 

		printf( "   +++> +++> read_data_or_wait(): [1] (id = %d) \n", myIdx );

		struct myLazyInFn {
			int myID;
			void who() const { printf("WHO: myLazyInFn with myID = %d\n", myID); }
			std::experimental::coroutine_handle<> who_is_awaiting;

			size_t myIdx;
			bool isData = false;


			myLazyInFn(size_t myIdx_) {
				myID = getID();
				myIdx = myIdx_;
				printf( "myLazyInFn::myLazyInFn() with id %zd, myID = %zd\n", myIdx, myID );
			}

			myLazyInFn(const myLazyInFn &) = delete;
			myLazyInFn &operator = (const myLazyInFn &) = delete;

			myLazyInFn(myLazyInFn &&s) {
				myID = getID();
				myIdx = s.myIdx;
				isData = s.isData;
				printf( "myLazyInFn::myLazyInFn() with id %zd, myID = %zd from other.myID = %zd\n", myIdx, myID, s.myID );
			}
			myLazyInFn &operator = (myLazyInFn &&s) {
				myIdx = s.myIdx;
				isData = s.isData;
				printf( "myLazyInFn::operator =() with id %zd, myID = %zd from other.myID = %zd\n", myIdx, myID, s.myID );
				return *this;
			}   
	
			~myLazyInFn() {}

			bool await_ready() {
				isData = is_data(myIdx);
				printf( "   ---> ---> ---> In await_ready(): id = %zd\n", myIdx );
				//return isData;
				return false;
			}

			void await_suspend(std::experimental::coroutine_handle<> awaiting) {
				who_is_awaiting = awaiting;
				printf( "   ---> ---> ---> In await_suspend(): about to call set_read_callback(); id = %zd\n", myIdx );
				set_read_awaiting_handle( myIdx, this->who_is_awaiting );
				printf( "   ---> ---> ---> In await_suspend(): after calling set_read_callback(); id = %zd\n", myIdx );
			}

			auto await_resume() {
				read_result r = read_data(myIdx);
				printf( "   ---> ---> ---> In await_resume(): id = %d %s\n", myIdx, r.is_exception ? "with exception" : "" );
				if ( r.is_exception )
					throw r.exception;
				return r.data;
			}
		};
		printf( "   +++> +++> read_data_or_wait(): [2] (id = %d)\n", myIdx );
		printf( "about to create myLazyInFn(%zd)\n", myIdx );
		return myLazyInFn(myIdx);
	}

	my_sync_awaitable<int> complete_block_1()
	{
		while ( ctr < threashold )
		{
			printf( "   ---> my_sync<int> complete_block_1(): [1] (id = %d, ctr = %d)\n", myIdx, ctr );
			try
			{
				auto v = co_await read_data_or_wait();
			}
			catch (std::exception& e)
			{
				printf("Exception caught at complete_block_1(): %s\n", e.what());
			}
			printf( "   ---> my_sync<int> complete_block_1(): [3] (id = %d, ctr = %d)\n", myIdx, ctr );
			++ctr;
		}

		printf( "   ---> my_sync<int> complete_block_1(): [about to exit] (id = %d), ctr = %d)\n", myIdx, ctr );
		co_return ctr;
	}

public:
	bool is_block() { return ctr >= threashold; }
	int get_ready_block() { int ret = ctr; ctr = 0; return ret ? 1 : 0; }

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
	int ctr = 0;
	size_t myIdx = (size_t)(-1);
	static constexpr int threashold = 2;

	my_sync_awaitable<int> complete_page_1()
	{
		while ( ctr < threashold )
		{
			printf( "   ---> my_sync<int> complete_page_1(): [1] (id = %d, ctr = %d)\n", myIdx, ctr );
			co_await l_reader.complete_block_1();
			printf( "   ---> my_sync<int> complete_page_1(): [2] (id = %d, ctr = %d)\n", myIdx, ctr );
			l_reader.get_ready_block();
			++ctr;
		}
		printf( "   ---> my_sync<int> complete_page_1(): [about to exit] (id = %d)\n", myIdx );

		co_return 0;
	}

public:
	bool is_page() { return ctr >= threashold; }
	int get_ready_page() { int ret = ctr; ctr = 0; return ret ? 1 : 0; }

	page_reader() {
		printf( "page_reader()::page_reader\n" );
	}
	~page_reader() {
		printf( "page_reader()::~page_reader\n" );
	}
};

class page_processor
{
public:
	page_reader processor;
	int ctr = 0;
	static constexpr int threashold = 2;
	size_t myIdx = (size_t)(-1);

public:
	my_sync_awaitable<int> run_ret;

public:
	page_processor() {
		printf( "page_processor()::page_processor\n" );
	}
	~page_processor() {
		printf( "page_processor()::~page_processor\n" );
	}
	void set_idx( size_t idx ) { myIdx = idx; }
	my_sync_awaitable<int> run()
	{
		for(;;)
		{
			try
			{
				co_await processor.complete_page_1();
			}
			catch (std::exception& e)
			{
				printf("Exception caught at run(): %s\n", e.what());
			}
			processor.get_ready_page();
			++ctr;
			printf( "PAGE %d HAS BEEN PROCESSED\n", ctr );
		}

		co_return 0;
	}
};

void processing_loop_3()
{ 
	init_handler_context();

	static constexpr size_t bep_cnt = 1;
	page_processor preader[bep_cnt];

	for ( size_t i=0; i<bep_cnt; ++i )
	{
		preader[i].set_idx( i );
	}

	for ( size_t i=0; i<bep_cnt; ++i )
		preader[i].run_ret = preader[i].run();

	for (;;)
	{
		char ch = getchar();
		if ( ch > '0' && ch <= '0' + max_h_count )
		{
			getchar(); // take '\n' out of stream
			printf( "   --> got \'%c\' (continuing)\n", ch );
			g_callbacks[ch - '1'].data.push_back( ch );
			g_callbacks[ch - '1'].is_exception = false;
			g_callbacks[ch - '1'].awaiting();
		}
		else if ( ch >= 'a' && ch < 'a' + max_h_count )
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
