

/*
 * This file contain type names and functions names that are declared as 'safe'
 * under Rule S8.
 * At this moment only names are of interest, we don't care about arguments,
 * return type or template parameters. This simplifies this file a lot.
 * 
 */

void getArgv();

namespace std {
 	void move();
 	void forward();

	class function {
		void operator=(function&);
	};

	class fake {};
 	void operator==(fake, fake);

	// apple gcc std lib uses namespace __1
	namespace __1 {
		void move();
		void forward();
		class function {
			void operator=(function&);
		};

	}

	namespace experimental {

		class suspend_never {
			void await_ready();
			void await_resume();
			void await_suspend();

		};

		class coroutine_handle {
			void from_address();
		};
		namespace coroutines_v1 {
			class coroutine_handle;
			void operator!=(void*, coroutine_handle);
		}
	}

	// containers below are not really safe, but we allow them until we
	// we get our own safe containers library

	class vector {
		void size();
		void operator[](int);
	};

	class basic_string {
		void size();
		void substr();

	};

}

class NodeBase {};

namespace nodecpp {

	// void wait_for_all();
	// void await_function();
	// void no_await_function();

	class awaitable {
		void await_ready();
		void await_resume();
		void await_suspend();
	};

	class promise_type_struct {
		void yield_value();
		void return_void();
	};


	class Buffer {
		void writeInt8();
		void readUInt8();
		void appendUint8();
		void appendString();
		void append();
		void size();
	};


	namespace net {
		void createSocket();

		class SocketBase {
			void on();
			void write();
			void end();
			void unref();
			void connect();
			void addHandler();
		};

		void createHttpServer();

		class HttpServerBase {
			void addHttpHandler();
		};

		void createServer();

		class ServerBase {
			void close();
			void listen();
			void on();
			void getSockCount();
		};

		class HttpServerResponse {
			void end();
			void writeHead();
		};
		class IncomingHttpMessageAtServer {
			void getMethod();
			void getUrl();
		};
	}


	class DataParent {};

	class Url {
		void parseUrlQueryString();
	};

	class UrlQuery {
		void operator[](int);
	};
	class UrlQueryItem {
		void toStr();
	};

	namespace log {
		class default_log {
			void info();
		};
	}

	class Log {
		void add();
		void setLevel();
		void setGuaranteedLevel();
	};

	namespace safememory {
		void make_owning();


		// osn ptrs are hardcoded with special safety rules
		// they must not be included here
		// class owning_ptr_impl;
		// class owning_ptr_no_checks;
		// class soft_ptr_impl;
		// class soft_ptr_no_checks;
		// class soft_this_ptr_impl;
		// class soft_this_ptr_no_checks;
		// class naked_ptr_impl;
		// class naked_ptr_no_checks;
	}
}



