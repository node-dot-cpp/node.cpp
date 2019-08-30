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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace nodecpp {

	namespace net {

		class HttpServerBase; // forward declaration
		class HttpSocketBase; // forward declaration

		class IncomingHttpMessageAtServer; // forward declaration
		class OutgoingHttpMessageAtServer; // forward declaration

		class HttpServerBase : public nodecpp::net::ServerBase
		{
		public:

#ifndef NODECPP_NO_COROUTINES
			struct awaitable_request_data
			{
				awaitable_handle_t h = nullptr;
				nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request;
				nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer> response;
			};
			awaitable_request_data ahd_request;
			void forceReleasingAllCoroHandles()
			{
				if ( ahd_request.h != nullptr )
				{
					auto hr = ahd_request.h;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					ahd_request.h = nullptr;
					hr();
				}
			}

			void onNewRequest( nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer> response )
			{
				if ( ahd_request.h != nullptr )
				{
					ahd_request.request = request;
					ahd_request.response = response;
					auto hr = ahd_request.h;
					ahd_request.h = nullptr;
					hr();
				}
			}



			auto a_request(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response;

					connection_awaiter(HttpServerBase& server_, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response_) : 
						server( server_ ), request( request_ ), response( response_) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.ahd_request.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.request != nullptr ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.response != nullptr ); 
						request = server.ahd_request.request;
						response = server.ahd_request.response;
					}
				};
				return connection_awaiter(*this, request, response);
			}

			auto a_request(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response, uint32_t period) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response;
					uint32_t period;
					nodecpp::Timeout to;

					connection_awaiter(HttpServerBase& server_, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::safememory::soft_ptr<OutgoingHttpMessageAtServer>& response_, uint32_t period_) : 
						server( server_ ), request( request_ ), response( response_), period( period_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.ahd_request.h = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( server.ahd_request.h, period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( to );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.request != nullptr ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.response != nullptr ); 
						request = server.ahd_request.request;
						response = server.ahd_request.response;
					}
				};
				return connection_awaiter(*this, request, response, period);
			}

#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES

		public:
			HttpServerBase() {}
			virtual ~HttpServerBase() {
//				MessageList.clear();
			}

			struct UserHandlersCommon
			{
			public:
				// originating from member functions of ServertBase-derived classes
				template<class T> using userIncomingRequestMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>);
				template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
				template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

				// originating from member functions of NodeBase-derived classes
				template<class T, class ServerT> using userIncomingRequestNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>);
				template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, bool);
				template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, Error&);

				using userDefIncomingRequestHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>);
				using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, bool);
				using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, Error&);

				// originating from member functions of HttpServerBase-derived classes

				template<class ObjectT, userIncomingRequestMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type incomingRequestHandler( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(request);
					CO_RETURN;
				}

				template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type closeHandler( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, bool hadError )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
					CO_RETURN;
				}

				template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type errorHandler( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, Error& e )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
					CO_RETURN;
				}

				// originating from member functions of NodeBase-derived classes

				template<class ObjectT, class ServerT, userIncomingRequestNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type incomingRequestHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), request);
					CO_RETURN;
				}

				template<class ObjectT, class ServerT, userCloseNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type closeHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, bool hadError )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), hadError);
					CO_RETURN;
				}

				template<class ObjectT, class ServerT, userErrorNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type errorHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, Error& e )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), e);
					CO_RETURN;
				}

				enum class Handler { IncomingReques, Close, Error };
			};

			struct UserHandlersForDataCollecting : public UserHandlersCommon
			{
				UserDefHandlers<UserHandlersCommon::userDefIncomingRequestHandlerFnT> userDefIncomingRequestHandlers;
				UserDefHandlers<UserHandlersCommon::userDefCloseHandlerFnT> userDefCloseHandlers;
				UserDefHandlers<UserHandlersCommon::userDefErrorHandlerFnT> userDefErrorHandlers;

				template<UserHandlersCommon::Handler handler, auto memmberFn, class ObjectT>
				void addHandler()
				{
					if constexpr (handler == Handler::IncomingReques)
					{
						userDefIncomingRequestHandlers.add( &UserHandlers::template IncomingRequestHandler<ObjectT, memmberFn>);
					}
					else if constexpr (handler == Handler::Close)
					{
						userDefCloseHandlers.add(&UserHandlers::template closeHandler<ObjectT, memmberFn>);
					}
					else
					{
						static_assert(handler == Handler::Error); // the only remaining option
						userDefErrorHandlers.add(&UserHandlers::template errorHandler<ObjectT, memmberFn>);
					}
				}

				template<UserHandlersCommon::Handler handler, auto memmberFn, class ObjectT, class ServerT>
				void addHandlerFromNode(ObjectT* object)
				{
					if constexpr (handler == Handler::IncomingReques)
					{
						userDefIncomingRequestHandlers.add(object, &UserHandlers::template IncomingRequestHandlerFromNode<ObjectT, ServerT, memmberFn>);
					}
					else if constexpr (handler == Handler::Close)
					{
						userDefCloseHandlers.add(object, &UserHandlers::template closeHandlerFromNode<ObjectT, ServerT, memmberFn>);
					}
					else
					{
						static_assert(handler == Handler::Error); // the only remaining option
						userDefErrorHandlers.add(object, &UserHandlers::template errorHandlerFromNode<ObjectT, ServerT, memmberFn>);
					}
				}
			};
			thread_local static UserHandlerClassPatterns<UserHandlersForDataCollecting> userHandlerClassPattern; // TODO: consider using thread-local allocator

			struct UserHandlers : public UserHandlersCommon
			{
				bool initialized = false;
			public:
				UserDefHandlersWithOptimizedStorage<UserHandlersCommon::userDefIncomingRequestHandlerFnT> userDefIncomingRequestHandlers;
				UserDefHandlersWithOptimizedStorage<UserHandlersCommon::userDefCloseHandlerFnT> userDefCloseHandlers;
				UserDefHandlersWithOptimizedStorage<UserHandlersCommon::userDefErrorHandlerFnT> userDefErrorHandlers;

				void from(const UserHandlersForDataCollecting& patternUH, void* defaultObjPtr)
				{
					if ( initialized )
						return;
					NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjPtr != nullptr);
					userDefIncomingRequestHandlers.from(patternUH.userDefIncomingRequestHandlers, defaultObjPtr);
					userDefCloseHandlers.from(patternUH.userDefCloseHandlers, defaultObjPtr);
					userDefErrorHandlers.from(patternUH.userDefErrorHandlers, defaultObjPtr);
					initialized = true;
				}
			};
			UserHandlers userHandlers;

			bool isIncomingRequesEventHandler() { return userHandlers.userDefIncomingRequestHandlers.willHandle(); }
			void handleIncomingRequesEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request) { userHandlers.userDefIncomingRequestHandlers.execute(server, request); }

			bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
			void handleCloseEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, bool hasError) { userHandlers.userDefCloseHandlers.execute(server, hasError); }

			bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
			void handleErrorEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, Error& e) { userHandlers.userDefErrorHandlers.execute(server, e); }
		};

		template<class DataParentT>
		class HttpServer : public HttpServerBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpServer<DataParentT>() {};
			HttpServer<DataParentT>(DataParentT* dataParent ) : HttpServerBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~HttpServer<DataParentT>() {}
		};

		template<>
		class HttpServer<void> : public HttpServerBase
		{
		public:
			using DataParentType = void;
			HttpServer() {};
			virtual ~HttpServer() {}
		};

		class HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;

		protected:
			nodecpp::safememory::soft_ptr<HttpSocketBase> sock;

			static constexpr std::pair<const char *, size_t> MethodNames[] = { 
				std::make_pair( "GET", sizeof( "GET" ) - 1 ),
				std::make_pair( "HEAD", sizeof( "HEAD" ) - 1 ),
				std::make_pair( "POST", sizeof( "POST" ) - 1 ),
				std::make_pair( "PUT", sizeof( "PUT" ) - 1 ),
				std::make_pair( "DELETE", sizeof( "DELETE" ) - 1 ),
				std::make_pair( "TRACE", sizeof( "TRACE" ) - 1 ),
				std::make_pair( "OPTIONS", sizeof( "OPTIONS" ) - 1 ),
				std::make_pair( "CONNECT", sizeof( "CONNECT" ) - 1 ),
				std::make_pair( "PATCH", sizeof( "PATCH" ) - 1 ) };
			static constexpr size_t MethodCount = sizeof( MethodNames ) / sizeof( std::pair<const char *, size_t> );

			enum ConnStatus { close, keep_alive };
			ConnStatus connStatus = ConnStatus::keep_alive;

			size_t contentLength = 0;

			typedef std::map<std::string, std::string> header_t; // so far good for both directions
			header_t header;

			// utils
			std::string makeLower( std::string& str ) // quick and dirty; TODO: revise (see, for instance, discussion at https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case)
			{
				std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
				return str;
			}

			void parseContentLength()
			{
				auto cl = header.find( "content-length" );
				if ( cl != header.end() )
					contentLength = ::atol( cl->second.c_str() ); // quick and dirty; TODO: revise
				contentLength = 0;
			}

			void parseConnStatus()
			{
				auto cs = header.find( "connection" );
				if ( cs != header.end() )
				{
					std::string val = cs->second.c_str();
					val = makeLower( val );
					if ( val == "keep alive" )
						connStatus = ConnStatus::keep_alive;
					else if ( val == "close" )
						connStatus = ConnStatus::close;
				}
				else
					connStatus = ConnStatus::close;
			}
		};



        class HttpSocketBase : public nodecpp::net::SocketBase
		{
			friend class IncomingHttpMessageAtServer;
			friend class OutgoingHttpMessageAtServer;

			// NOTE: private part is for future move to lib
			// NOTE: current implementation is anty-optimal; it's just a sketch of what could be in use
			class DummyBuffer
			{
				Buffer base;
				size_t currpos = 0;
			public:
				DummyBuffer() : base(0x10000) {}
				void pushFragment(const Buffer& b) { base.append( b ); }
				bool popLine(Buffer& b) 
				{ 
					for ( ; currpos<base.size(); ++currpos )
						if ( *(base.begin() + currpos) == '\n' )
						{
							b.clear();
							b.append( base, 0, currpos+1 );
							base.popFront( currpos+1 );
							currpos = 0;
							return true;
						}
					return false;
				}
			};
			DummyBuffer dbuf;

			size_t rqCnt = 0;

			nodecpp::handler_ret_type readLine(Buffer& lb)
			{
printf( "about to read line\n" );
				nodecpp::Buffer r_buff(0x200);
				while ( !dbuf.popLine( lb ) )
				{
					co_await a_read( r_buff, 2 );
printf( "a segment has been read\n" );
					dbuf.pushFragment( r_buff );
				}

				CO_RETURN;
			}

			nodecpp::handler_ret_type getRequest( IncomingHttpMessageAtServer& message );

			nodecpp::safememory::owning_ptr<IncomingHttpMessageAtServer> request;
			nodecpp::safememory::owning_ptr<OutgoingHttpMessageAtServer> response;
			awaitable_handle_t ahd_continueGetting = nullptr;

#ifndef NODECPP_NO_COROUTINES
			auto a_continueGetting() { 

				struct continue_getting_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpSocketBase& socket;

					continue_getting_awaiter(HttpSocketBase& socket_) : socket( socket_ ) {}

					continue_getting_awaiter(const continue_getting_awaiter &) = delete;
					continue_getting_awaiter &operator = (const continue_getting_awaiter &) = delete;

					~continue_getting_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						socket.ahd_continueGetting = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
printf( "resumed ahd_continueGetting\n" );
					}
				};
				return continue_getting_awaiter(*this);
			}

#endif // NODECPP_NO_COROUTINES

		public:
			HttpSocketBase();
			virtual ~HttpSocketBase() {}

			nodecpp::handler_ret_type run()
			{
				for(;;)
				{
printf( "about to get (next) request\n" );
					co_await getRequest( *request );
					auto cg = a_continueGetting();
//					request->dbgTrace();

					++rqCnt;

					nodecpp::safememory::soft_ptr_static_cast<HttpServerBase>(myServerSocket)->onNewRequest( request, response );
//					co_await cg;
				}
				CO_RETURN;
			}

			void proceedToNext()
			{
				if ( ahd_continueGetting != nullptr )
				{
					auto hr = ahd_continueGetting;
					ahd_continueGetting = nullptr;
printf( "about to resume ahd_continueGetting\n" );
					hr();
				}
			}

#ifndef NODECPP_NO_COROUTINES
			void forceReleasingAllCoroHandles()
			{
				if ( ahd_continueGetting != nullptr )
				{
					auto hr = ahd_continueGetting;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					ahd_continueGetting = nullptr;
					hr();
				}
			}
#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES

		};

		template<class DataParentT>
		class HttpSocket : public HttpSocketBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpSocket<DataParentT>() {};
			HttpSocket<DataParentT>(DataParentT* dataParent ) : HttpSocketBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~HttpSocket<DataParentT>() {}
		};

		template<>
		class HttpSocket<void> : public HttpSocketBase
		{
		public:
			using DataParentType = void;
			HttpSocket() {};
			virtual ~HttpSocket() {}
		};

		class IncomingHttpMessageAtServer : protected HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;

		private:
			struct Method // so far a struct
			{
				std::string name;
				std::string value;
				void clear() { name.clear(); value.clear(); }
			};
			Method method;

			nodecpp::Buffer body;
			enum ReadStatus { noinit, in_hdr, in_body, completed };
			ReadStatus readStatus = ReadStatus::noinit;
			size_t bodyBytesRetrieved = 0;

		private:

		public:
			IncomingHttpMessageAtServer() {}
			IncomingHttpMessageAtServer(const IncomingHttpMessageAtServer&) = delete;
			IncomingHttpMessageAtServer operator = (const IncomingHttpMessageAtServer&) = delete;
			IncomingHttpMessageAtServer(IncomingHttpMessageAtServer&& other)
			{
				method = std::move( other.method );
				header = std::move( other.header );
				readStatus = other.readStatus;
				contentLength = other.contentLength;
				other.readStatus = ReadStatus::noinit;
			}
			IncomingHttpMessageAtServer& operator = (IncomingHttpMessageAtServer&& other)
			{
				method = std::move( other.method );
				header = std::move( other.header );
				readStatus = other.readStatus;
				other.readStatus = ReadStatus::noinit;
				contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				method.clear();
				header.clear();
				body.clear();
				contentLength = 0;
				readStatus = ReadStatus::noinit;
				bodyBytesRetrieved = 0;
			}
			nodecpp::handler_ret_type a_readBody( Buffer& b )
			{
				if ( bodyBytesRetrieved < getContentLength() )
				{
					b.clear();
					co_await sock->a_read( b, getContentLength() - bodyBytesRetrieved );
					bodyBytesRetrieved += b.size();
				}
//				if ( bodyBytesRetrieved == getContentLength() )
//					sock->proceedToNext();

				CO_RETURN;
			}


			bool setMethod( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::noinit ); 
				size_t start = line.find_first_not_of( " \t" );
				if ( start == std::string::npos || line[start] == '\r' || line[start] == '\n' )
					return false;
				for ( size_t i=0; i<MethodCount; ++i )
					if ( line.size() > MethodNames[i].second + start && memcmp( line.c_str() + start, MethodNames[i].first, MethodNames[i].second ) == 0 && line.c_str()[ MethodNames[i].second] == ' ' ) // TODO: cthink about rfind(*,0)
					{
						method.name = MethodNames[i].first;
						start += MethodNames[i].second + 1;
						start = line.find_first_not_of( " \t", start );
						size_t end = line.find_last_not_of(" \t\r\n" );
						method.value = line.substr( start, end - start + 1 );
						readStatus = ReadStatus::in_hdr;
						return true;
					}
				return false;
			}

			bool addHeaderEntry( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::in_hdr ); 
				size_t end = line.find_last_not_of(" \t\r\n" );
				if ( end == std::string::npos )
				{
					if ( !( line.size() == 2 || line[0] == '\r' && line[1] == '\n' ) )
						return true;
					parseContentLength();
					readStatus = contentLength ? ReadStatus::in_body : ReadStatus::completed;
					return false;
				}
				size_t start = line.find_first_not_of( " \t" );
				size_t idx = line.find(':', start);
				if ( idx >= end )
					return false;
				size_t valStart = line.find_first_not_of( " \t", idx + 1 );
				std::string key = line.substr( start, idx-start );
				header.insert( std::make_pair( makeLower( key ), line.substr( valStart, end - valStart + 1 ) ));
				return true;
			}

			size_t getContentLength() const { return contentLength; }

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {} {}", method.name, method.value );
				for ( auto& entry : header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[CL = {}, Conn = {}]", getContentLength(), connStatus == ConnStatus::keep_alive ? "keep-alive" : "close" );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "" );
			}
		};

		class OutgoingHttpMessageAtServer : protected HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;

		private:
			typedef std::map<std::string, std::string> header_t;
			header_t header;
			size_t contentLength = 0;
			nodecpp::Buffer body;
			ConnStatus connStatus = ConnStatus::keep_alive;
			enum WriteStatus { notyet, hdr_flushed, in_body, completed };
			WriteStatus writeStatus = WriteStatus::notyet;

			std::string replyStatus;
			size_t bodyBytesWritten = 0;

		private:

		public:
			OutgoingHttpMessageAtServer() {}
			OutgoingHttpMessageAtServer(const OutgoingHttpMessageAtServer&) = delete;
			OutgoingHttpMessageAtServer operator = (const OutgoingHttpMessageAtServer&) = delete;
			OutgoingHttpMessageAtServer(OutgoingHttpMessageAtServer&& other)
			{
				replyStatus = std::move( other.replyStatus );
				header = std::move( other.header );
				contentLength = other.contentLength;
			}
			OutgoingHttpMessageAtServer& operator = (OutgoingHttpMessageAtServer&& other)
			{
				replyStatus = std::move( other.replyStatus );
				header = std::move( other.header );
				contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				replyStatus.clear();
				header.clear();
				body.clear();
				contentLength = 0;
				writeStatus = WriteStatus::notyet;
			}

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [<-] {}", replyStatus );
				for ( auto& entry : header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [<-] {}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "" );
			}

			void addHeader( std::string key, std::string value )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: sanitize
				header.insert( std::make_pair( key, value ) );
			}

			void setStatus( std::string status ) // temporary stub; TODO: ...
			{
				replyStatus = status;
			}

			nodecpp::handler_ret_type flushHeaders()
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: add real implementation
				std::string out = replyStatus;
				out += "\r\n";
				for ( auto h: header )
				{
					out += h.first;
					out += ": ";
					out += h.second;
					out += "\r\n";
				}
				out += "\r\n";

				Buffer b;
				b.append( out.c_str(), out.size() );
				parseContentLength();
				co_await sock->a_write( b );
				writeStatus = WriteStatus::hdr_flushed;
				parseConnStatus();
				header.clear();
				CO_RETURN;
			}

			nodecpp::handler_ret_type writeBodyPart(Buffer& b)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::hdr_flushed ); 
				// TODO: add real implementation
				co_await sock->a_write( b );
printf( "request has been sent\n" );
sock->request->clear();
				if ( connStatus != ConnStatus::keep_alive )
				{
printf( "socket has been ended\n" );
					sock->end();
					clear();
					CO_RETURN;
				}
//				else
					clear();
sock->proceedToNext();
printf( "getting next request has been allowed\n" );
				CO_RETURN;
			}
		};

		nodecpp::handler_ret_type HttpSocketBase::getRequest( IncomingHttpMessageAtServer& message )
		{
			bool ready = false;
			Buffer lb;
			co_await readLine(lb);
			lb.appendUint8( 0 );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
			if ( !message.setMethod( std::string( reinterpret_cast<char*>(lb.begin()) ) ) )
			{
				end();
//				co_await sendReply();
			}

			do
			{
				lb.clear();
				co_await readLine(lb);
				lb.appendUint8( 0 );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
			}
			while ( message.addHeaderEntry( std::string( reinterpret_cast<char*>(lb.begin()) ) ) );

			/*if ( message.getContentLength() )
			{
				lb.clear();
				lb.reserve( message.getContentLength() );
				co_await a_read( lb, message.getContentLength() );
			}*/

			CO_RETURN;
		}

		HttpSocketBase::HttpSocketBase() {
			request = nodecpp::safememory::make_owning<IncomingHttpMessageAtServer>();
			request->sock = myThis.getSoftPtr<HttpSocketBase>(this);
			response = nodecpp::safememory::make_owning<OutgoingHttpMessageAtServer>();
			response->sock = myThis.getSoftPtr<HttpSocketBase>(this);
			run(); // TODO: think about proper time for this call
		}

		template<class ServerT, class SocketT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			static_assert( std::is_base_of< HttpServerBase, ServerT >::value );
			static_assert( std::is_base_of< HttpSocketBase, SocketT >::value );
			return createServer<ServerT, HttpSocket, Types ...>(::std::forward<Types>(args)...);
		}

		template<class ServerT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			return createHttpServer<ServerT, HttpSocket, Types ...>(::std::forward<Types>(args)...);
		}

	} //namespace net
} //namespace nodecpp

#endif //HTTP_SERVER_H
