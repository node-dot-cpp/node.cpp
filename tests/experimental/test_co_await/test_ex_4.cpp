
#include "test_ex_2_base.h"

static int object_id_base = 0;
static int getID() { return ++object_id_base; }

template<bool is_awaitable_>
struct awaitable_base
{
	static constexpr bool is_awaitable = is_awaitable_;
};

template<typename T>
struct my_sync_awaitable : public awaitable_base<false>  {
	struct promise_type;
	using handle_type = std::experimental::coroutine_handle<promise_type>;
	handle_type coro;
	using value_type = T;
	bool from_coro = false;

	int myID;

    my_sync_awaitable()  {
		myID = getID();
 		//if ( myID == 8 ) __asm{int 3}
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
		//myID = getID();
		printf( "my_sync_awaitable::operator =() with myID = %d (moving, other ID = %d)\n", myID, s.myID );
		//if ( myID == 8 ) __asm{int 3}
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
        return coro.promise().value;
    }

	bool await_ready() noexcept { 
		printf( "await_transform()::awaiter::await_ready() with myID = %d, this = 0x%zx\n", myID, (size_t)this ); 
		return false;
	}
	void await_suspend(std::experimental::coroutine_handle<> h_) noexcept {
		printf( "await_transform()::awaiter::await_suspend() with myID = %d, this = 0x%zx\n", myID, (size_t)this ); 
		who(); 
		//h = h_;
//						promise->hr_set = true;
//						promise->hr = h_;
		if ( coro )
		{
			coro.promise().hr_set = true;
			coro.promise().hr = h_;

			auto hx = handle_type::from_promise(coro.promise());
			if ( hx == h_ )
				coro = nullptr;
		}
	}
	T await_resume() noexcept { 
		printf( "await_transform()::awaiter::await_resume() with myID = %zd, this = 0x%zx\n", myID, (size_t)this );  
		return 0; 
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
       /*promise_type(promise_type&&other) : value( std::move(other.value) ), hr( std::move( other.hr ) ) {
            //printf( "my_sync_awaitable::promise_type::promise_type() [1]\n" );
			hr_set = other.hr_set;
			other.hr_set = false;
        }
		/*promise_type( promise_type&& other )
		{
            printf( "my_sync_awaitable::promise_type::promise_type() [move]\n" );
			hr = std::move( other.hr );
			hr_set = other.hr_set;
			other.hr_set = false;
		}*/
        ~promise_type() {
            printf( "my_sync_awaitable::promise_type::~promise_type() with myID == %d [1]\n", myID );
			if ( hr_set )
			{
				printf("my_sync_awaitable::promise_type::~promise_type() [2, releasing handle]\n");
				hr_set = false;
				//hr();
			}/**/
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
				printf("my_sync_awaitable::final_suspend() [2]\n");
				hr();
				hr_set = false;
			}/**/
//			return std::experimental::suspend_always{};
			return std::experimental::suspend_never{};
        }
        void unhandled_exception() {
            std::exit(1);
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

		struct myLazyInFn : public awaitable_base<true> {
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
				isData = read_data(myIdx);
				printf( "   ---> ---> ---> In await_ready(): id = %zd\n", myIdx );
				return isData;
			}

			void await_suspend(std::experimental::coroutine_handle<> awaiting) {
				who_is_awaiting = awaiting;
				printf( "   ---> ---> ---> In await_suspend(): about to call set_read_callback(); id = %zd\n", myIdx );
				set_read_awaiting_handle( myIdx, this->who_is_awaiting );
				printf( "   ---> ---> ---> In await_suspend(): after calling set_read_callback(); id = %zd\n", myIdx );
			}

			auto await_resume() {
					if ( read_data(myIdx) )
					{
						printf("line_reader::read_data_or_wait(): resuming handle (idx = %zd)\n", myIdx);
						//this->who_is_awaiting();
					}
				printf( "   ---> ---> ---> In await_resume(): id = %zd\n", myIdx );
				return 1;
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
			auto a = read_data_or_wait();
			printf( "   ---> my_sync<int> complete_block_1(): [2] (id = %d, ctr = %d)\n", myIdx, ctr );
			/**/auto v = co_await a;
			printf( "   ---> my_sync<int> complete_block_1(): [3] (id = %d, ctr = %d)\n", myIdx, ctr );
			++ctr;
		}

		printf( "   ---> my_sync<int> complete_block_1(): [4] (id = %d), ctr = %d)\n", myIdx, ctr );

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
			//auto a = l_reader.complete_block_1();
			printf( "   ---> my_sync<int> complete_page_1(): [2] (id = %d, ctr = %d)\n", myIdx, ctr );
			co_await l_reader.complete_block_1();
			printf( "   ---> my_sync<int> complete_page_1(): [3] (id = %d, ctr = %d)\n", myIdx, ctr );
		l_reader.get_ready_block();
			//v.get();
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
	struct awaitable_ret_holder
	{
		my_sync_awaitable<int> call_ret;
		int myID;
		awaitable_ret_holder()
		{
			myID = getID();
		}
		~awaitable_ret_holder() {
            printf( "page_processor::awaitable_ret_holder::~awaitable_ret_holder() with myID == %d\n", myID );
		}
	};
	awaitable_ret_holder run_arh;

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
			co_await processor.complete_page_1();
				processor.get_ready_page();
			++ctr;
			printf( "PAGE %d HAS BEEN PROCESSED\n", ctr );
		}

		co_return 0;
	}
};

void processing_loop_3()
{ 
	init_read_context();

	static constexpr size_t bep_cnt = 1;
	page_processor preader[bep_cnt];

	for ( size_t i=0; i<bep_cnt; ++i )
	{
		preader[i].set_idx( i );
		g_callbacks[i].dataAvailable = false;
	}

	for ( size_t i=0; i<bep_cnt; ++i )
		preader[i].run_arh.call_ret = std::move( preader[i].run() );
//		preader[i].run();

	for (;;)
	{
		char ch = getchar();
		if ( ch > '0' && ch <= '0' + max_h_count )
		{
			getchar();
			printf( "   --> got \'%c\' (continuing)\n", ch );
			//g_Processors[ch - '1']->onEvent(preader[ch - '1'].run_ret);
			g_callbacks[ch - '1'].dataAvailable = true;
//			g_callbacks[ch - '1'].cb(true);
			g_callbacks[ch - '1'].awaiting();
			g_callbacks[ch - '1'].dataAvailable = false;
		}
		else
		{
			printf( "   --> got \'%c\' (terminating)\n", ch );
			break;
		}
	}
}
