

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

        class fake {};
        void operator==(fake, fake);
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

// from memory-safe-cpp safe_ptr.h
namespace nodecpp::safememory {

	// this are hardcoded inside checker
	// with non trivial rules

	// class owning_ptr;
	// class soft_ptr;
	// class naked_ptr;
	void make_owning();
	void soft_ptr_in_constructor();

}



class NodeBase {};

namespace nodecpp {

// from nodecpp/awaitable.h

	class awaitable {
		void await_ready();
		void await_resume();
		void await_suspend();
	};

	class promise_type_struct {
		void get_return_object();
		void return_value();
		void return_void();
		void unhandled_exception();
	};
	
	void wait_for_all();

// from nodecpp/net_common.h
	class Buffer {
		Buffer& operator = (Buffer&& p);
		void clone();

		void reserve();

		void append();

		void trim();

		void clear();
		void set_size();

		void size();
		bool empty();
		void capacity();
		void begin();
		void end();

		void popFront();

		void writeInt8();
		void appendUint8();
		void appendString();

		// an attempt to follow node.js buffer-related interface
		void writeUInt32BE();
		void writeUInt32LE();
		void readUInt8();
		void readUInt32BE();
		void readUInt32LE();
	};

	class CircularByteBuffer
	{
		//TODO
	};

	class MultiOwner
	{
		void add();
		void removeAndDelete();
		void clear();
		void getCount();
	};


	namespace net {


		struct Address {

			bool operator == ( const Address& other ) const;
		};

	}



	namespace net {

		// from nodecpp/socket_common.h
		void createSocket();

		class Socket {};

		class SocketBase {

			static
			void addHandler();
			
			void bufferSize();
			void bytesRead();
			void bytesWritten();

			void connecting();
			void destroy();
			void destroyed();
			void end();

			void localAddress();
			void localIP();
			void localPort();


			void remoteAddress();
			void remoteIP();
			void remoteFamily();
			void remotePort();

			void ref();
			void unref();
			void pause();
			void resume();
			void reportBeingDestructed();

			void connect();

			void write();

			void setNoDelay();
			void setKeepAlive();


			void forceReleasingAllCoroHandles();

			void a_connect() { 

				struct connect_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					auto await_resume() {
					}
				};
			}


			void a_read() { 

				struct read_data_awaiter {

					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}


			void a_write(Buffer& buff) { 

				struct write_data_awaiter {

					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void a_drain() { 

				struct drain_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

		public:
			void emitClose();
			// not in node.js
			void emitAccepted();
			void emitConnect();

			void emitData();

			void emitDrain();

			void emitEnd();
			void emitError();

			void on( );

			void once( );

		};

		// from nodecpp/http_server.h
		void createHttpServer();


		// from nodecpp/http_server_common.h
		class HttpServerBase 
		{

			void forceReleasingAllCoroHandles();

			void onNewRequest();

			void a_request() { 

				struct connection_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}


			static
			void addHttpHandler();

			void on();

		};

		class HttpServer {}; 

		class HttpMessageBase 
		{

			void parseContentLength();

			void parseConnStatus();
		};


		// from nodecpp/http_socket_at_server.h
        class HttpSocketBase
		{
			auto a_continueGetting() { 

				struct continue_getting_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void a_dataAvailable() { 

				struct data_awaiter {

					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void readLine();
			void run();

			void proceedToNext();

			void forceReleasingAllCoroHandles();

		};

		class HttpSocket {};

		class IncomingHttpMessageAtServer
		{
			IncomingHttpMessageAtServer& operator = (IncomingHttpMessageAtServer&& other);
			void clear();
			void a_readBody();


			void parseMethod( );

			void parseHeaderEntry( );

			void getMethod();
			void getUrl();
			void getHttpVersion();

			void getContentLength();

			void dbgTrace();
		};

		class HttpServerResponse 
		{
			class HeaderHolder
			{
				void toStr();
			};
			
			HttpServerResponse& operator = (HttpServerResponse&& other);
			void clear();

			void dbgTrace();
			void writeHead();

			void addHeader();

			void setStatus( );

			void flushHeaders();

			void writeBodyPart();

			void end();
		};


		//from nodecpp/server_common.h
		class ServerBase
		{
			static
			void addHandler();

			void registerServer();


			void address();
			void close();

			bool listening();
			void ref();
			void unref();
			void reportBeingDestructed();

			void listen();
			void forceReleasingAllCoroHandles();

			void a_listen() { 

				struct listen_awaiter {

					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void a_connection() { 

				struct connection_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void a_close() { 

				struct close_awaiter {
					bool await_ready() {
						return false;
					}

					void await_suspend() {
					}

					void await_resume() {
					}
				};
			}

			void reportAllAceptedConnectionsEnded();
			void removeSocket();

			void closingProcedure();
			void closeByWorkingCluster();
			void getSockCount();

			void emitClose() {
			}

			void emitConnection() {
			}

			void emitListening() {
			}

			void emitError() {
			}


			void on();
			void once();
		};

		class ServerSocket {};
		void createServer();
	}

// from nodecpp/url.h
	class UrlQueryItem {
		UrlQueryItem& operator = ( UrlQueryItem&& other );

		void add();
		void toStr();
	};

	class UrlQuery
	{
		UrlQuery& operator = ( UrlQuery&& other );

		void add();
		const UrlQueryItem& operator [] (int);
	};

	class Url {
		static
		void parseUrlQueryString();
	};

// from nodecpp/logging.h
	class Log
	{
		void log();

		void setLevel( );
		void setGuaranteedLevel( );
		void resetGuaranteedLevel();

		void fatal();
		void error();
		void warning();
		void info();
		void debug();

		void clear();
		void add();
	};
}


//from foundation log.h

namespace nodecpp::log::default_log
{
	void log();
	void fatal();
	void error();
	void warning();
	void info();
	void debug();
}



