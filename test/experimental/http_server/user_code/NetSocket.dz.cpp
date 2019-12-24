#line 1 "<built-in>"
#include <dezombiefy.h>
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code\\NetSocket.cpp"
// NetSocket.cpp : sample of user-defined code


#include <infrastructure.h>
#if 0 /* expanded below */
#include "NetSocket.h"
#endif /* expanded below */
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#if 0 /* expanded below */
#include "http_server.h"
#endif /* expanded below */
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server.h"
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

#if 0 /* expanded below */
#include "http_server_common.h"
#endif /* expanded below */
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server_common.h"
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

#ifndef HTTP_SERVER_COMMON_H
#define HTTP_SERVER_COMMON_H

#include <nodecpp/common.h>
#include <nodecpp/server_common.h>

#include <algorithm>
#include <cctype>
#include <string>

// NOTE: current implementation is anty-optimal; it's just a sketch of what could be in use

namespace nodecpp {

	namespace net {

		class HttpServerBase; // forward declaration
		class HttpSocketBase; // forward declaration

		class IncomingHttpMessageAtServer; // forward declaration
		class HttpServerResponse; // forward declaration

		class HttpServerBase : public nodecpp::net::ServerBase
		{
		public:
			using NodeType = void;
			using DataParentType = void;

		public:
			enum class Handler { IncomingRequest, Close, Error };
//		private:
			struct DataForHttpCommandProcessing
			{
				struct UserHandlersCommon
				{
				public:
					// originating from member functions of ServertBase-derived classes
					template<class T> using userIncomingRequestMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::safememory::soft_ptr<HttpServerResponse>);
					template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

					// originating from member functions of NodeBase-derived classes
					template<class T, class ServerT> using userIncomingRequestNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::safememory::soft_ptr<HttpServerResponse>);
					template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, bool);
					template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, Error&);

					using userDefIncomingRequestHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::safememory::soft_ptr<HttpServerResponse>);
					using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, bool);
					using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<HttpServerBase>, Error&);

					// originating from member functions of HttpServerBase-derived classes

					template<class ObjectT, userIncomingRequestMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type incomingRequestHandler( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<HttpServerResponse> response )
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
					static nodecpp::handler_ret_type incomingRequestHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<HttpServerBase> serverPtr, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<HttpServerResponse> response )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), request, response);
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
				};

				struct UserHandlersForDataCollecting : public UserHandlersCommon
				{
					UserDefHandlers<UserHandlersCommon::userDefIncomingRequestHandlerFnT> userDefIncomingRequestHandlers;
					UserDefHandlers<UserHandlersCommon::userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlers<UserHandlersCommon::userDefErrorHandlerFnT> userDefErrorHandlers;

					template<HttpServerBase::Handler handler, auto memmberFn, class ObjectT>
					void addHandler()
					{
						if constexpr (handler == Handler::IncomingRequest)
						{
							userDefIncomingRequestHandlers.add( &UserHandlersCommon::template incomingRequestHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(&UserHandlersCommon::template closeHandler<ObjectT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(&UserHandlersCommon::template errorHandler<ObjectT, memmberFn>);
						}
					}

					template<HttpServerBase::Handler handler, auto memmberFn, class ObjectT, class ServerT>
					void addHandlerFromNode(ObjectT* object)
					{
						if constexpr (handler == Handler::IncomingRequest)
						{
							userDefIncomingRequestHandlers.add(object, &UserHandlersCommon::template incomingRequestHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(object, &UserHandlersCommon::template closeHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(object, &UserHandlersCommon::template errorHandlerFromNode<ObjectT, ServerT, memmberFn>);
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
						if ( nodecpp::safememory::dezombiefy( this )->initialized )
							return;
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjPtr != nullptr);
						nodecpp::safememory::dezombiefy( this )->userDefIncomingRequestHandlers.from(nodecpp::safememory::dezombiefy( patternUH ).userDefIncomingRequestHandlers, nodecpp::safememory::dezombiefy( defaultObjPtr ));
						nodecpp::safememory::dezombiefy( this )->userDefCloseHandlers.from(nodecpp::safememory::dezombiefy( patternUH ).userDefCloseHandlers, nodecpp::safememory::dezombiefy( defaultObjPtr ));
						nodecpp::safememory::dezombiefy( this )->userDefErrorHandlers.from(nodecpp::safememory::dezombiefy( patternUH ).userDefErrorHandlers, nodecpp::safememory::dezombiefy( defaultObjPtr ));
						nodecpp::safememory::dezombiefy( this )->initialized = true;
					}
				};
				UserHandlers userHandlers;

				bool isIncomingRequesEventHandler() { return nodecpp::safememory::dezombiefy( this )->userHandlers.userDefIncomingRequestHandlers.willHandle(); }
				void handleIncomingRequesEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<HttpServerResponse> response) { nodecpp::safememory::dezombiefy( this )->userHandlers.userDefIncomingRequestHandlers.execute(server, request, response); }

				bool isCloseEventHandler() { return nodecpp::safememory::dezombiefy( this )->userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, bool hasError) { nodecpp::safememory::dezombiefy( this )->userHandlers.userDefCloseHandlers.execute(server, hasError); }

				bool isErrorEventHandler() { return nodecpp::safememory::dezombiefy( this )->userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(nodecpp::safememory::soft_ptr<HttpServerBase> server, Error& e) { nodecpp::safememory::dezombiefy( this )->userHandlers.userDefErrorHandlers.execute(server, nodecpp::safememory::dezombiefy( e )); }
			};
			DataForHttpCommandProcessing dataForHttpCommandProcessing;

		public:
			HttpServerBase() {}
			virtual ~HttpServerBase() {}

#ifndef NODECPP_NO_COROUTINES
			struct awaitable_request_data
			{
				awaitable_handle_t h = nullptr;
				nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request;
				nodecpp::safememory::soft_ptr<HttpServerResponse> response;
			};
			awaitable_request_data ahd_request;

			void forceReleasingAllCoroHandles()
			{
				if ( nodecpp::safememory::dezombiefy( this )->ahd_request.h != nullptr )
				{
					auto hr = nodecpp::safememory::dezombiefy( this )->ahd_request.h;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					nodecpp::safememory::dezombiefy( this )->ahd_request.h = nullptr;
					hr();
				}
			}

			void onNewRequest( nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<HttpServerResponse> response )
			{
//printf( "entering onNewRequest()  %s\n", ahd_request.h == nullptr ? "ahd_request.h is nullptr" : "" );
				if ( nodecpp::safememory::dezombiefy( this )->ahd_request.h != nullptr )
				{
					nodecpp::safememory::dezombiefy( this )->ahd_request.request = request;
					nodecpp::safememory::dezombiefy( this )->ahd_request.response = response;
					auto hr = nodecpp::safememory::dezombiefy( this )->ahd_request.h;
					ahd_request.h = nullptr;
//printf( "about to rezume ahd_request.h\n" );
					hr();
				}
				else if ( nodecpp::safememory::dezombiefy( this )->dataForHttpCommandProcessing.isIncomingRequesEventHandler() )
					nodecpp::safememory::dezombiefy( this )->dataForHttpCommandProcessing.handleIncomingRequesEvent( myThis.getSoftPtr<HttpServerBase>(this), request, response );
			}

			auto a_request(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::safememory::soft_ptr<HttpServerResponse>& response) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::safememory::soft_ptr<HttpServerResponse>& response;

					connection_awaiter(HttpServerBase& server_, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::safememory::soft_ptr<HttpServerResponse>& response_) : 
						server( nodecpp::safememory::dezombiefy( server_ ) ), request( nodecpp::safememory::dezombiefy( request_ ) ), response( nodecpp::safememory::dezombiefy( response_ )) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						nodecpp::safememory::dezombiefy( this )->server.ahd_request.h = awaiting;
						nodecpp::safememory::dezombiefy( this )->myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(nodecpp::safememory::dezombiefy( this )->myawaiting) )
							throw nodecpp::getException(nodecpp::safememory::dezombiefy( this )->myawaiting);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.request != nullptr ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.response != nullptr ); 
						nodecpp::safememory::dezombiefy( this )->request = server.ahd_request.request;
						nodecpp::safememory::dezombiefy( this )->response = server.ahd_request.response;
					}
				};
				return connection_awaiter(*nodecpp::safememory::dezombiefy( this ), nodecpp::safememory::dezombiefy( request ), nodecpp::safememory::dezombiefy( response ));
			}

			auto a_request(nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::safememory::soft_ptr<HttpServerResponse>& response, uint32_t period) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::safememory::soft_ptr<HttpServerResponse>& response;
					uint32_t period;
					nodecpp::Timeout to;

					connection_awaiter(HttpServerBase& server_, nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::safememory::soft_ptr<HttpServerResponse>& response_, uint32_t period_) : 
						server( nodecpp::safememory::dezombiefy( server_ ) ), request( nodecpp::safememory::dezombiefy( request_ ) ), response( nodecpp::safememory::dezombiefy( response_ )), period( period_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						nodecpp::safememory::dezombiefy( this )->server.ahd_request.h = awaiting;
						nodecpp::safememory::dezombiefy( this )->myawaiting = awaiting;
						nodecpp::safememory::dezombiefy( this )->to = nodecpp::setTimeoutForAction( server.ahd_request.h, period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( nodecpp::safememory::dezombiefy( this )->to );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(nodecpp::safememory::dezombiefy( this )->myawaiting) )
							throw nodecpp::getException(nodecpp::safememory::dezombiefy( this )->myawaiting);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.request != nullptr ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.response != nullptr ); 
						nodecpp::safememory::dezombiefy( this )->request = server.ahd_request.request;
						nodecpp::safememory::dezombiefy( this )->response = server.ahd_request.response;
					}
				};
				return connection_awaiter(*nodecpp::safememory::dezombiefy( this ), nodecpp::safememory::dezombiefy( request ), nodecpp::safememory::dezombiefy( response ), period);
			}

#else
#line 327 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server_common.h"
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES
#line 329 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server_common.h"

			template<class UserClass, HttpServerBase::Handler handler, auto memmberFn, class ObjectT>
			static void addHttpHandler(ObjectT* object)
			{
				DataForHttpCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().template addHandlerFromNode<handler, memmberFn, ObjectT, UserClass>(object);
			}
			template<class UserClass, HttpServerBase::Handler handler, auto memmberFn>
			static void addHttpHandler()
			{
				DataForHttpCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().template addHandler<handler, memmberFn, UserClass>();
			}
		};

		template<class DataParentT>
		class HttpServer : public HttpServerBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpServer<DataParentT>() {};
			HttpServer<DataParentT>(DataParentT* dataParent ) : HttpServerBase(), ::nodecpp::DataParent<DataParentT>( nodecpp::safememory::dezombiefy( dataParent ) ) {};
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
				std::transform(nodecpp::safememory::dezombiefy( str ).begin(), nodecpp::safememory::dezombiefy( str ).end(), nodecpp::safememory::dezombiefy( str ).begin(), [](unsigned char c){ return std::tolower(c); });
				return nodecpp::safememory::dezombiefy( str );
			}

			void parseContentLength()
			{
				auto cl = nodecpp::safememory::dezombiefy( this )->header.find( "content-length" );
				if ( cl != nodecpp::safememory::dezombiefy( this )->header.end() )
					nodecpp::safememory::dezombiefy( this )->contentLength = ::atol( cl->second.c_str() ); // quick and dirty; TODO: revise
				nodecpp::safememory::dezombiefy( this )->contentLength = 0;
			}

			void parseConnStatus()
			{
				auto cs = nodecpp::safememory::dezombiefy( this )->header.find( "connection" );
				if ( cs != nodecpp::safememory::dezombiefy( this )->header.end() )
				{
					std::string val = cs->second.c_str();
					val = nodecpp::safememory::dezombiefy( this )->makeLower( val );
					if ( val == "keep alive" )
						nodecpp::safememory::dezombiefy( this )->connStatus = ConnStatus::keep_alive;
					else if ( val == "close" )
						nodecpp::safememory::dezombiefy( this )->connStatus = ConnStatus::close;
				}
				else
					nodecpp::safememory::dezombiefy( this )->connStatus = ConnStatus::close;
			}
		};

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SERVER_COMMON_H
#line 424 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server_common.h"
#line 32 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server.h"
#if 0 /* expanded below */
#include "http_socket_at_server.h"
#endif /* expanded below */
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"
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

#ifndef HTTP_SOCKET_AT_SERVER_H
#define HTTP_SOCKET_AT_SERVER_H

#include "http_server_common.h"
#include <nodecpp/socket_common.h>

#include <algorithm>
#include <cctype>
#include <string>


// NOTE: current implementation is anty-optimal; it's just a sketch of what could be in use

namespace nodecpp {

	namespace net {

		class IncomingHttpMessageAtServer; // forward declaration
		class HttpServerResponse; // forward declaration

        class HttpSocketBase : public nodecpp::net::SocketBase
		{
			friend class IncomingHttpMessageAtServer;
			friend class HttpServerResponse;

			nodecpp::handler_ret_type getRequest( IncomingHttpMessageAtServer& message );

			nodecpp::safememory::owning_ptr<IncomingHttpMessageAtServer> request;
			nodecpp::safememory::owning_ptr<HttpServerResponse> response;
			awaitable_handle_t ahd_continueGetting = nullptr;

#ifndef NODECPP_NO_COROUTINES
			auto a_continueGetting() { 

				struct continue_getting_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpSocketBase& socket;

					continue_getting_awaiter(HttpSocketBase& socket_) : socket( nodecpp::safememory::dezombiefy( socket_ ) ) {}

					continue_getting_awaiter(const continue_getting_awaiter &) = delete;
					continue_getting_awaiter &operator = (const continue_getting_awaiter &) = delete;

					~continue_getting_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						nodecpp::safememory::dezombiefy( this )->socket.ahd_continueGetting = awaiting;
						nodecpp::safememory::dezombiefy( this )->myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(nodecpp::safememory::dezombiefy( this )->myawaiting) )
							throw nodecpp::getException(nodecpp::safememory::dezombiefy( this )->myawaiting);
//printf( "resumed ahd_continueGetting\n" );
					}
				};
				return continue_getting_awaiter(*nodecpp::safememory::dezombiefy( this ));
			}

			auto a_dataAvailable( CircularByteBuffer::AvailableDataDescriptor& d ) { 

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					CircularByteBuffer::AvailableDataDescriptor& d;

					data_awaiter(SocketBase& socket_, CircularByteBuffer::AvailableDataDescriptor& d_) : socket( nodecpp::safememory::dezombiefy( socket_ ) ), d( nodecpp::safememory::dezombiefy( d_ ) ) {}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {}

					bool await_ready() {
						return nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.readBuffer.used_size() != 0;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.ahd_read.min_bytes = 1;
						nodecpp::setNoException(awaiting);
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.ahd_read.h = awaiting;
						nodecpp::safememory::dezombiefy( this )->myawaiting = awaiting;
					}

					auto await_resume() {
						if ( nodecpp::safememory::dezombiefy( this )->myawaiting != nullptr && nodecpp::isException(nodecpp::safememory::dezombiefy( this )->myawaiting) )
							throw nodecpp::getException(nodecpp::safememory::dezombiefy( this )->myawaiting);
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.readBuffer.get_available_data( d );
					}
				};
				return data_awaiter(*nodecpp::safememory::dezombiefy( this ), nodecpp::safememory::dezombiefy( d ));
			}

			auto a_dataAvailable( uint32_t period, CircularByteBuffer::AvailableDataDescriptor& d ) { 

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					CircularByteBuffer::AvailableDataDescriptor& d;
					uint32_t period;
					nodecpp::Timeout to;

					data_awaiter(SocketBase& socket_, uint32_t period_, CircularByteBuffer::AvailableDataDescriptor& d_) : socket( nodecpp::safememory::dezombiefy( socket_ ) ), d( nodecpp::safememory::dezombiefy( d_ ) ), period( period_ ) {}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {}

					bool await_ready() {
						return nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.readBuffer.used_size() > 0;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.ahd_read.min_bytes = 1;
						nodecpp::setNoException(awaiting);
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.ahd_read.h = awaiting;
						nodecpp::safememory::dezombiefy( this )->myawaiting = awaiting;
						nodecpp::safememory::dezombiefy( this )->to = nodecpp::setTimeoutForAction( awaiting, period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( nodecpp::safememory::dezombiefy( this )->to );
						if ( nodecpp::safememory::dezombiefy( this )->myawaiting != nullptr && nodecpp::isException(nodecpp::safememory::dezombiefy( this )->myawaiting) )
							throw nodecpp::getException(nodecpp::safememory::dezombiefy( this )->myawaiting);
						nodecpp::safememory::dezombiefy( this )->socket.dataForCommandProcessing.readBuffer.get_available_data( d );
					}
				};
				return data_awaiter(*nodecpp::safememory::dezombiefy( this ), period, nodecpp::safememory::dezombiefy( d ));
			}

			nodecpp::handler_ret_type readLine(std::string& line)
			{
				size_t pos = 0;
				nodecpp::safememory::dezombiefy( line ).clear();
				CircularByteBuffer::AvailableDataDescriptor d;
				for(;;)
				{
					co_await nodecpp::safememory::dezombiefy( this )->a_dataAvailable( d );
					for ( ; pos<d.sz1; ++pos )
						if ( d.ptr1[pos] == '\n' )
						{
							nodecpp::safememory::dezombiefy( line ).append( (const char*)(d.ptr1), pos + 1 );
							nodecpp::safememory::dezombiefy( this )->dataForCommandProcessing.readBuffer.skip_data( pos + 1 );
							CO_RETURN;
						}
					nodecpp::safememory::dezombiefy( line ) += std::string( (const char*)(d.ptr1), pos );
					nodecpp::safememory::dezombiefy( this )->dataForCommandProcessing.readBuffer.skip_data( pos );
					pos = 0;
					if ( d.ptr2 && d.sz2 )
					{
						for ( ; pos<d.sz2; ++pos )
							if ( d.ptr2[pos] == '\n' )
							{
								nodecpp::safememory::dezombiefy( line ) += std::string( (const char*)(d.ptr2), pos + 1 );
								nodecpp::safememory::dezombiefy( this )->dataForCommandProcessing.readBuffer.skip_data( pos + 1 );
								CO_RETURN;
							}
						nodecpp::safememory::dezombiefy( line ) += std::string( (const char*)(d.ptr2), pos );
						nodecpp::safememory::dezombiefy( this )->dataForCommandProcessing.readBuffer.skip_data( pos );
						pos = 0;
					}
				}

				CO_RETURN;
			}

#endif // NODECPP_NO_COROUTINES
#line 202 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"

		public:
			HttpSocketBase();
			virtual ~HttpSocketBase() {}

#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type run()
			{
				for(;;)
				{
//printf( "about to get (next) request\n" );
					co_await nodecpp::safememory::dezombiefy( this )->getRequest( *request );
					auto cg = nodecpp::safememory::dezombiefy( this )->a_continueGetting();

					nodecpp::safememory::soft_ptr_static_cast<HttpServerBase>(nodecpp::safememory::dezombiefy( this )->myServerSocket)->onNewRequest( nodecpp::safememory::dezombiefy( this )->request, response );
//					co_await cg;
				}
				CO_RETURN;
			}

			void proceedToNext()
			{
				if ( nodecpp::safememory::dezombiefy( this )->ahd_continueGetting != nullptr )
				{
					auto hr = nodecpp::safememory::dezombiefy( this )->ahd_continueGetting;
					ahd_continueGetting = nullptr;
//printf( "about to resume ahd_continueGetting\n" );
					hr();
				}
			}

			void forceReleasingAllCoroHandles()
			{
				if ( nodecpp::safememory::dezombiefy( this )->ahd_continueGetting != nullptr )
				{
					auto hr = nodecpp::safememory::dezombiefy( this )->ahd_continueGetting;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					nodecpp::safememory::dezombiefy( this )->ahd_continueGetting = nullptr;
					hr();
				}
			}
#else
#line 244 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES
#line 246 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"

		};

		template<class RequestT, class DataParentT>
		class HttpSocket : public HttpSocketBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpSocket<RequestT, DataParentT>() {};
			HttpSocket<RequestT, DataParentT>(DataParentT* dataParent ) : HttpSocketBase(), ::nodecpp::DataParent<DataParentT>( nodecpp::safememory::dezombiefy( dataParent ) ) {};
			virtual ~HttpSocket<RequestT, DataParentT>() {}
		};

		template<class RequestT>
		class HttpSocket<RequestT, void> : public HttpSocketBase
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
				std::string url;
				std::string version;
				void clear() { nodecpp::safememory::dezombiefy( this )->name.clear(); nodecpp::safememory::dezombiefy( this )->url.clear(); nodecpp::safememory::dezombiefy( this )->version.clear(); }
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
				nodecpp::safememory::dezombiefy( this )->method = std::move( other.method );
				nodecpp::safememory::dezombiefy( this )->header = std::move( other.header );
				nodecpp::safememory::dezombiefy( this )->readStatus = other.readStatus;
				contentLength = other.contentLength;
				other.readStatus = ReadStatus::noinit;
			}
			IncomingHttpMessageAtServer& operator = (IncomingHttpMessageAtServer&& other)
			{
				nodecpp::safememory::dezombiefy( this )->method = std::move( other.method );
				nodecpp::safememory::dezombiefy( this )->header = std::move( other.header );
				nodecpp::safememory::dezombiefy( this )->readStatus = other.readStatus;
				other.readStatus = ReadStatus::noinit;
				contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				nodecpp::safememory::dezombiefy( this )->method.clear();
				nodecpp::safememory::dezombiefy( this )->header.clear();
				nodecpp::safememory::dezombiefy( this )->body.clear();
				nodecpp::safememory::dezombiefy( this )->contentLength = 0;
				readStatus = ReadStatus::noinit;
				bodyBytesRetrieved = 0;
			}
#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type a_readBody( Buffer& b )
			{
				auto& nodecpp_0 = nodecpp::safememory::dezombiefy( this )->bodyBytesRetrieved; auto nodecpp_1 = getContentLength(); if ( nodecpp::safememory::dezombiefy( nodecpp_0 ) < nodecpp_1 )
				{
					nodecpp::safememory::dezombiefy( b ).clear();
					co_await nodecpp::safememory::dezombiefy( this )->sock->a_read( nodecpp::safememory::dezombiefy( b ), nodecpp::safememory::dezombiefy( this )->getContentLength() - nodecpp::safememory::dezombiefy( this )->bodyBytesRetrieved );
					nodecpp::safememory::dezombiefy( this )->bodyBytesRetrieved += nodecpp::safememory::dezombiefy( b ).size();
				}
//				if ( bodyBytesRetrieved == getContentLength() )
//					sock->proceedToNext();

				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES
#line 335 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"


			bool parseMethod( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::noinit ); 
				size_t start = nodecpp::safememory::dezombiefy( line ).find_first_not_of( " \t" );
				if ( start == std::string::npos || nodecpp::safememory::dezombiefy( line )[start] == '\r' || nodecpp::safememory::dezombiefy( line )[start] == '\n' )
					return false;
				bool found = false;
				// Method name
				for ( size_t i=0; i<MethodCount; ++i )
					{ auto& nodecpp_0 = MethodNames[i].second; auto nodecpp_1 = nodecpp::safememory::dezombiefy( line ).size(); auto nodecpp_2 = nodecpp::safememory::dezombiefy( nodecpp_0 ) + start; auto nodecpp_3 = nodecpp::safememory::dezombiefy( line ).c_str(); auto nodecpp_4 = nodecpp_3 + start; auto& nodecpp_5 = MethodNames[i].first; auto& nodecpp_6 = MethodNames[i].second; auto nodecpp_7 = memcmp( nodecpp_4, nodecpp_5, nodecpp_6 ); auto& nodecpp_8 = nodecpp::safememory::dezombiefy( line ).c_str()[ MethodNames[i].second]; auto nodecpp_9 = ' '; if ( nodecpp_1 > nodecpp_2 && nodecpp_7 == 0 && nodecpp_8 == nodecpp_9 ) // TODO: cthink about rfind(*,0)
					{
						nodecpp::safememory::dezombiefy( this )->method.name = MethodNames[i].first;
						start += MethodNames[i].second + 1;
						start = nodecpp::safememory::dezombiefy( line ).find_first_not_of( " \t", start );
						found = true;
						break;
					} }
				if ( !found )
					return false;
				// URI
				size_t endOfURI = nodecpp::safememory::dezombiefy( line ).find_first_of(" \t\r\n", start + 1 );
				nodecpp::safememory::dezombiefy( this )->method.url = nodecpp::safememory::dezombiefy( line ).substr( start, endOfURI - start );
				if ( nodecpp::safememory::dezombiefy( this )->method.url.size() == 0 )
					return false;
				start = nodecpp::safememory::dezombiefy( line ).find_first_not_of( " \t", endOfURI );
				// HTTP version
				size_t end = nodecpp::safememory::dezombiefy( line ).find_last_not_of(" \t\r\n" );
				if ( memcmp( nodecpp::safememory::dezombiefy( line ).c_str() + start, "HTTP/", 5 ) != 0 )
					return false;
				start += 5;
				nodecpp::safememory::dezombiefy( this )->method.version = nodecpp::safememory::dezombiefy( line ).substr( start, end - start + 1 );
				nodecpp::safememory::dezombiefy( this )->readStatus = ReadStatus::in_hdr;
				return true;
			}

			bool parseHeaderEntry( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::in_hdr ); 
				size_t end = nodecpp::safememory::dezombiefy( line ).find_last_not_of(" \t\r\n" );
				if ( end == std::string::npos )
				{
					if ( !( nodecpp::safememory::dezombiefy( line ).size() == 2 && nodecpp::safememory::dezombiefy( line )[0] == '\r' && nodecpp::safememory::dezombiefy( line )[1] == '\n' ) ) // last empty line
						return true; // TODO: what should we do with this line of spaces? - just ignore or report a parsing error?
					nodecpp::safememory::dezombiefy( this )->parseContentLength();
					nodecpp::safememory::dezombiefy( this )->readStatus = nodecpp::safememory::dezombiefy( this )->contentLength ? ReadStatus::in_body : ReadStatus::completed;
					return false;
				}
				size_t start = nodecpp::safememory::dezombiefy( line ).find_first_not_of( " \t" );
				size_t idx = nodecpp::safememory::dezombiefy( line ).find(':', start);
				if ( idx >= end )
					return false;
				size_t valStart = nodecpp::safememory::dezombiefy( line ).find_first_not_of( " \t", idx + 1 );
				std::string key = nodecpp::safememory::dezombiefy( line ).substr( start, idx-start );
				nodecpp::safememory::dezombiefy( this )->header.insert( std::make_pair( makeLower( key ), nodecpp::safememory::dezombiefy( line ).substr( valStart, end - valStart + 1 ) ));
				return true;
			}

			const std::string& getMethod() { return nodecpp::safememory::dezombiefy( this )->method.name; }
			const std::string& getUrl() { return nodecpp::safememory::dezombiefy( this )->method.url; }
			const std::string& getHttpVersion() { return nodecpp::safememory::dezombiefy( this )->method.version; }

			size_t getContentLength() const { return nodecpp::safememory::dezombiefy( this )->contentLength; }

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {} {} HTTP/{}", nodecpp::safememory::dezombiefy( this )->method.name, method.url, method.version );
				for ( auto& entry : nodecpp::safememory::dezombiefy( this )->header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {}: {}", entry.first, entry.second );
				{ auto& nodecpp_0 = nodecpp::safememory::dezombiefy( this )->connStatus; auto nodecpp_1 = getContentLength(); auto nodecpp_2 = nodecpp::safememory::dezombiefy( nodecpp_0 ) == ConnStatus::keep_alive ? "keep-alive" : "close"; nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[CL = {}, Conn = {}]", nodecpp_1, nodecpp_2 ); };
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "" );
			}
		};

		class HttpServerResponse : protected HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;
			Buffer headerBuff;

		private:
			typedef std::map<std::string, std::string> header_t;
			header_t header;
			size_t contentLength = 0;
			nodecpp::Buffer body;
			ConnStatus connStatus = ConnStatus::keep_alive;
			enum WriteStatus { notyet, hdr_flushed, in_body, completed };
			WriteStatus writeStatus = WriteStatus::notyet;

			std::string replyStatus;
			//size_t bodyBytesWritten = 0;

		private:

		public:
			HttpServerResponse() : headerBuff(0x1000) {}
			HttpServerResponse(const HttpServerResponse&) = delete;
			HttpServerResponse operator = (const HttpServerResponse&) = delete;
			HttpServerResponse(HttpServerResponse&& other)
			{
				nodecpp::safememory::dezombiefy( this )->replyStatus = std::move( other.replyStatus );
				nodecpp::safememory::dezombiefy( this )->header = std::move( other.header );
				nodecpp::safememory::dezombiefy( this )->contentLength = other.contentLength;
				headerBuff = std::move( other.headerBuff );
			}
			HttpServerResponse& operator = (HttpServerResponse&& other)
			{
				nodecpp::safememory::dezombiefy( this )->replyStatus = std::move( other.replyStatus );
				nodecpp::safememory::dezombiefy( this )->header = std::move( other.header );
				nodecpp::safememory::dezombiefy( this )->headerBuff = std::move( other.headerBuff );
				nodecpp::safememory::dezombiefy( this )->contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				nodecpp::safememory::dezombiefy( this )->replyStatus.clear();
				nodecpp::safememory::dezombiefy( this )->header.clear();
				nodecpp::safememory::dezombiefy( this )->body.clear();
				nodecpp::safememory::dezombiefy( this )->headerBuff.clear();
				nodecpp::safememory::dezombiefy( this )->contentLength = 0;
				writeStatus = WriteStatus::notyet;
			}

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [<-] {}", nodecpp::safememory::dezombiefy( this )->replyStatus );
				for ( auto& entry : nodecpp::safememory::dezombiefy( this )->header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [<-] {}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "" );
			}

			void addHeader( std::string key, std::string value )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: sanitize
				nodecpp::safememory::dezombiefy( this )->header.insert( std::make_pair( key, value ) );
			}

			void setStatus( std::string status ) // temporary stub; TODO: ...
			{
				nodecpp::safememory::dezombiefy( this )->replyStatus = status;
			}

#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type flushHeaders()
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: add real implementation
				{ auto& nodecpp_0 = nodecpp::safememory::dezombiefy( this )->replyStatus; auto& nodecpp_1 = replyStatus; auto& nodecpp_2 = headerBuff; auto nodecpp_3 = nodecpp_0.c_str(); auto nodecpp_4 = nodecpp::safememory::dezombiefy( nodecpp_1 ).size(); nodecpp::safememory::dezombiefy( nodecpp_2 ).append( nodecpp::safememory::dezombiefy( nodecpp_3 ), nodecpp_4 ); };
				nodecpp::safememory::dezombiefy( this )->headerBuff.append( "\r\n", 2 );
				for ( auto h: nodecpp::safememory::dezombiefy( this )->header )
				{
					{ auto& nodecpp_0 = h.first; auto& nodecpp_1 = h.first; auto& nodecpp_2 = nodecpp::safememory::dezombiefy( this )->headerBuff; auto nodecpp_3 = nodecpp_0.c_str(); auto nodecpp_4 = nodecpp::safememory::dezombiefy( nodecpp_1 ).size(); nodecpp::safememory::dezombiefy( nodecpp_2 ).append( nodecpp::safememory::dezombiefy( nodecpp_3 ), nodecpp_4 ); };
					nodecpp::safememory::dezombiefy( this )->headerBuff.append( ": ", 2 );
					{ auto& nodecpp_5 = h.second; auto& nodecpp_6 = h.second; auto& nodecpp_7 = nodecpp::safememory::dezombiefy( this )->headerBuff; auto nodecpp_8 = nodecpp_5.c_str(); auto nodecpp_9 = nodecpp::safememory::dezombiefy( nodecpp_6 ).size(); nodecpp::safememory::dezombiefy( nodecpp_7 ).append( nodecpp::safememory::dezombiefy( nodecpp_8 ), nodecpp_9 ); };
					nodecpp::safememory::dezombiefy( this )->headerBuff.append( "\r\n", 2 );
				}
				nodecpp::safememory::dezombiefy( this )->headerBuff.append( "\r\n", 2 );

				nodecpp::safememory::dezombiefy( this )->parseContentLength();
				nodecpp::safememory::dezombiefy( this )->parseConnStatus();
//				co_await sock->a_write( headerBuff );
				nodecpp::safememory::dezombiefy( this )->writeStatus = WriteStatus::hdr_flushed;
				header.clear();
				CO_RETURN;
			}

			nodecpp::handler_ret_type writeBodyPart(Buffer& b)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::hdr_flushed ); 
				// TODO: add real implementation
				try {
					if ( nodecpp::safememory::dezombiefy( this )->headerBuff.size() )
					{
						nodecpp::safememory::dezombiefy( this )->headerBuff.append( nodecpp::safememory::dezombiefy( b ) );
						co_await nodecpp::safememory::dezombiefy( this )->sock->a_write( nodecpp::safememory::dezombiefy( this )->headerBuff );
						nodecpp::safememory::dezombiefy( this )->headerBuff.clear();
					}
					else
						co_await nodecpp::safememory::dezombiefy( this )->sock->a_write( nodecpp::safememory::dezombiefy( this )->headerBuff );
				} 
				catch(...) {
					nodecpp::safememory::dezombiefy( this )->sock->end();
					nodecpp::safememory::dezombiefy( this )->clear();
					nodecpp::safememory::dezombiefy( this )->sock->proceedToNext();
				}
//printf( "request has been sent\n" );
nodecpp::safememory::dezombiefy( this )->sock->request->clear();
				if ( nodecpp::safememory::dezombiefy( this )->connStatus != ConnStatus::keep_alive )
				{
//printf( "socket has been ended\n" );
					sock->end();
					nodecpp::safememory::dezombiefy( this )->clear();
					CO_RETURN;
				}
//				else
					nodecpp::safememory::dezombiefy( this )->clear();
nodecpp::safememory::dezombiefy( this )->sock->proceedToNext();
//printf( "getting next request has been allowed\n" );
				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES
#line 538 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"
		};

		inline
		nodecpp::handler_ret_type HttpSocketBase::getRequest( IncomingHttpMessageAtServer& message )
		{
			bool ready = false;
			std::string line;
			co_await nodecpp::safememory::dezombiefy( this )->readLine(line);
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			if ( !nodecpp::safememory::dezombiefy( message ).parseMethod( line ) )
			{
				// TODO: report error
				nodecpp::safememory::dezombiefy( this )->end();
			}

			do
			{
				co_await nodecpp::safememory::dezombiefy( this )->readLine(line);
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			}
			while ( nodecpp::safememory::dezombiefy( message ).parseHeaderEntry( line ) );

			CO_RETURN;
		}

		inline
		HttpSocketBase::HttpSocketBase() {
			nodecpp::safememory::dezombiefy( this )->request = nodecpp::safememory::make_owning<IncomingHttpMessageAtServer>();
			{ auto nodecpp_0 = &*(nodecpp::safememory::dezombiefy( this )->request); auto& nodecpp_1 = nodecpp::safememory::dezombiefy( this )->myThis; auto& nodecpp_2 = nodecpp_0->sock; auto&& nodecpp_3 = nodecpp_1.getSoftPtr<HttpSocketBase>(this); nodecpp::safememory::dezombiefy( nodecpp_2 ) = nodecpp_3; };
			nodecpp::safememory::dezombiefy( this )->response = nodecpp::safememory::make_owning<HttpServerResponse>();
			{ auto nodecpp_4 = &*(nodecpp::safememory::dezombiefy( this )->response); auto& nodecpp_5 = nodecpp::safememory::dezombiefy( this )->myThis; auto& nodecpp_6 = nodecpp_4->sock; auto&& nodecpp_7 = nodecpp_5.getSoftPtr<HttpSocketBase>(this); nodecpp::safememory::dezombiefy( nodecpp_6 ) = nodecpp_7; };
			nodecpp::safememory::dezombiefy( this )->run(); // TODO: think about proper time for this call
		}

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SOCKET_AT_SERVER_H
#line 576 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_socket_at_server.h"
#line 33 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server.h"

// NOTE: current implementation is anty-optimal; it's just a sketch of what could be in use

namespace nodecpp {

	namespace net {

		class HttpServerBase; // forward declaration
		class HttpSocketBase; // forward declaration

		class IncomingHttpMessageAtServer; // forward declaration
		class HttpServerResponse; // forward declaration



		template<class ServerT, class RequestT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			static_assert( std::is_base_of< HttpServerBase, ServerT >::value );
			static_assert( std::is_base_of< IncomingHttpMessageAtServer, RequestT >::value );
			nodecpp::safememory::owning_ptr<ServerT> retServer = nodecpp::safememory::make_owning<ServerT>(::std::forward<Types>(args)...);
			if constexpr ( !std::is_same<typename ServerT::NodeType, void>::value )
			{
				static_assert( std::is_base_of< NodeBase, typename ServerT::NodeType >::value );
				retServer->template registerServer<typename ServerT::NodeType, ServerT>(retServer);
			}
			else
			{
				retServer->registerServer(retServer);
			}
			if constexpr ( std::is_same< typename ServerT::DataParentType, void >::value )
			{
				using SocketT = HttpSocket< RequestT, void>;
				retServer->setAcceptedSocketCreationRoutine( [](OpaqueSocketData& sdata) {
						nodecpp::safememory::owning_ptr<SocketT> ret = nodecpp::safememory::make_owning<SocketT>();
						ret->registerMeByIDAndAssignSocket(sdata, -1);
						return ret;
					} );
			}
			else
			{
				auto myDataParent = retServer->getDataParent();
				retServer->setAcceptedSocketCreationRoutine( [myDataParent](OpaqueSocketData& sdata) {
						using SocketT = HttpSocket<RequestT, typename ServerT::DataParentType>;
						nodecpp::safememory::owning_ptr<SocketT> retSock;
						if constexpr ( std::is_base_of< NodeBase, typename ServerT::DataParentType >::value )
						{
							retSock = nodecpp::safememory::make_owning<SocketT>(nodecpp::safememory::dezombiefy( myDataParent ));
						}
						else
						{
							retSock = nodecpp::safememory::make_owning<SocketT>();
						}
						retSock->registerMeByIDAndAssignSocket(nodecpp::safememory::dezombiefy( sdata ), -1);
						return retSock;
					} );
			}
			{ auto nodecpp_0 = &*(retServer); auto& nodecpp_1 = nodecpp_0->dataForCommandProcessing.userHandlers; auto& nodecpp_2 = ServerBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<ServerT>(); auto nodecpp_3 = &(*retServer); nodecpp::safememory::dezombiefy( nodecpp_1 ).from(nodecpp::safememory::dezombiefy( nodecpp_2 ), nodecpp_3); };
			retServer->dataForHttpCommandProcessing.userHandlers.from(HttpServerBase::DataForHttpCommandProcessing::userHandlerClassPattern.getPatternForApplying<ServerT>(), &(*retServer));
			return retServer;
		}

		template<class ServerT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			return createHttpServer<ServerT, IncomingHttpMessageAtServer, Types ...>(::std::forward<Types>(args)...);
		}

	} //namespace net
} //namespace nodecpp

#endif //HTTP_SERVER_H
#line 105 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/http_server.h"
#line 11 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
#ifdef NODECPP_ENABLE_CLUSTERING
#include <nodecpp/cluster.h>
#endif // NODECPP_ENABLE_CLUSTERING
#line 14 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

using namespace std;
using namespace nodecpp;
using namespace fmt;

//#define IMPL_VERSION 1 // main() is a single coro
#define IMPL_VERSION 12 // main() is a single coro (with clustering)
//#define IMPL_VERSION 2 // onRequest is a coro

class MySampleTNode : public NodeBase
{
public: // just temporarily
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	Buffer ctrlReplyBuff;

public:

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif
#line 44 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	class MyHttpServer : public nodecpp::net::HttpServer<MySampleTNode>
	{
	public:
		MyHttpServer() {}
		MyHttpServer(MySampleTNode* node) : HttpServer<MySampleTNode>(nodecpp::safememory::dezombiefy( node )) {}
		virtual ~MyHttpServer() {}
	};

	class CtrlServer : public nodecpp::net::ServerBase
	{
	public:
		CtrlServer() {}
		virtual ~CtrlServer() {}
	};

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyHttpServer;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = CtrlServer;
	nodecpp::safememory::owning_ptr<CtrlServerType>  srvCtrl;

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

#if IMPL_VERSION == 1
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

//		srv = nodecpp::net::createServer<ServerType, HttpSock>(this);
		srv = nodecpp::net::createHttpServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5000);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif
#line 98 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
		try { 
			for(;;) { 
				co_await srv->a_request(request, response); 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				++(stats.rqCnt);
//				request->dbgTrace();

//				simpleProcessing( request, response );
				yetSimpleProcessing( request, response );
			} 
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}

#elif IMPL_VERSION == 12
#line 120 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(nodecpp::safememory::dezombiefy( this ));

#ifdef NODECPP_ENABLE_CLUSTERING
		if ( getCluster().isMaster() )
		{
			size_t coreCnt = 1;
			auto argv = getArgv();
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 9 && argv[i].substr(0,9) == "numcores=" )
					coreCnt = atol(argv[i].c_str() + 9);
			}

			for ( size_t i=0; i<coreCnt; ++i )
				getCluster().fork();
		}
		else
#endif // NODECPP_ENABLE_CLUSTERING
#line 143 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
		{
			nodecpp::safememory::dezombiefy( this )->srv = nodecpp::net::createHttpServer<ServerType>();
			nodecpp::safememory::dezombiefy( this )->srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

			auto argv = getArgv();
			std::string ip = "0.0.0.0";
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 3 && argv[i].substr(0,9) == "ip=" )
					ip = argv[i].substr(3);
			}
			{ auto nodecpp_0 = &*(nodecpp::safememory::dezombiefy( this )->srv); auto nodecpp_1 = ip.c_str(); nodecpp::safememory::dezombiefy( nodecpp_0 )->listen(2000, nodecpp_1, 5000); };
			nodecpp::safememory::dezombiefy( this )->srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
			to = std::move( nodecpp::setTimeout(  [this]() { 
				srv->close();
				srv->unref();
				srvCtrl->close();
				srvCtrl->unref();
				stopAccepting = true;
				to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
			}, 3000 ) );
#endif
#line 167 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

			nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
			nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
			try { 
				for(;;) { 
					co_await nodecpp::safememory::dezombiefy( this )->srv->a_request(request, response); 
					Buffer b1(0x1000);
					co_await request->a_readBody( b1 );
					++(nodecpp::safememory::dezombiefy( this )->stats.rqCnt);
//					request->dbgTrace();

	//				simpleProcessing( request, response );
					yetSimpleProcessing( request, response );
				} 
			} 
			catch (...) { // TODO: what?
			}
		}

		CO_RETURN;
	}
#elif IMPL_VERSION == 2
#line 189 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::HttpServerBase::addHttpHandler<ServerType, nodecpp::net::HttpServerBase::Handler::IncomingRequest, &MySampleTNode::onRequest>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

//		srv = nodecpp::net::createHttpServer<ServerType, HttpSock>(this);
//		srv = nodecpp::net::createHttpServer<ServerType, nodecpp::net::IncomingHttpMessageAtServer>(this);
		srv = nodecpp::net::createHttpServer<ServerType, nodecpp::net::IncomingHttpMessageAtServer>();
//		srv = nodecpp::net::createHttpServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5000);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif
#line 215 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

		CO_RETURN;
	}
	virtual nodecpp::handler_ret_type onRequest(nodecpp::safememory::soft_ptr<ServerType> server, nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response)
	{
//		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
//		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
		try { 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				++(stats.rqCnt);
//				request->dbgTrace();

//				simpleProcessing( request, response );
				yetSimpleProcessing( request, response );
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}
#else
#line 237 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
#error
#endif // IMPL_VERSION
#line 239 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	nodecpp::handler_ret_type simpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
//			response->dbgTrace();
			co_await response->flushHeaders();
			CO_RETURN;
		}

		bool dataAvailable = false;
		std::string replyHtml;
		if ( request->getUrl() == "/" )
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>HOME</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">another_page.html</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = true;
		}
		else if ( request->getUrl() == "/another_page.html" )
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>Another Page</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">this page</a> again, or back to <a href=\"/\">HOME</a><br/>"
				"( or try to access <a href=\"x.html\">inexistent</a> page to (hopefully) see 404 error"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = true;
		}
		else // not found (body for 404)
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>404 Not Found</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"/\">HOME</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = false;
		}

		if ( dataAvailable )
		{
			response->setStatus( "HTTP/1.1 200 OK" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", fmt::format( "{}", replyHtml.size()) );

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			{ auto nodecpp_0 = replyHtml.c_str(); auto nodecpp_1 = replyHtml.size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_0 ), nodecpp_1 ); };
			co_await response->writeBodyPart(b);
		}
		else
		{
			response->setStatus( "HTTP/1.1 404 Not Found" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
//					response->addHeader( "Content-Length", "0" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Content-Length", fmt::format( "{}", replyHtml.size()) );

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			{ auto nodecpp_2 = replyHtml.c_str(); auto nodecpp_3 = replyHtml.size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_2 ), nodecpp_3 ); };
			co_await response->writeBodyPart(b);
		}
		co_await response->end();

		// TODO: co_await for msg body, if any
		// TODO: form and send response

		CO_RETURN;
	}
	void parseUrlQueryString(const std::string& query, std::vector<std::pair<std::string, std::string>>& queryValues )
	{
		size_t start = 0;

		for(;;)
		{
			size_t endEq = nodecpp::safememory::dezombiefy( query ).find_first_of( "=", start );
			if ( endEq == std::string::npos )
			{
				{ auto nodecpp_0 = endEq-start; auto&& nodecpp_1 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_0 ); auto&& nodecpp_2 = std::make_pair( nodecpp_1, "" ); auto&& nodecpp_3 = nodecpp_2; nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_3 ); };
				break;
			}
			size_t endAmp = nodecpp::safememory::dezombiefy( query ).find_first_of( "&", endEq+ 1  );
			if ( endAmp == std::string::npos )
			{
				{ auto nodecpp_4 = endEq-start; auto nodecpp_5 = endEq + 1; auto&& nodecpp_6 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_4 ); auto&& nodecpp_7 = nodecpp::safememory::dezombiefy( query ).substr( nodecpp_5 ); auto&& nodecpp_8 = std::make_pair( nodecpp_6, nodecpp_7 ); nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_8 ); };
				break;
			}
			else
			{
				{ auto nodecpp_9 = endEq-start; auto nodecpp_10 = endAmp - endEq; auto nodecpp_11 = endEq + 1; auto nodecpp_12 = nodecpp_10 - 1; auto&& nodecpp_13 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_9 ); auto&& nodecpp_14 = nodecpp::safememory::dezombiefy( query ).substr( nodecpp_11, nodecpp_12 ); auto&& nodecpp_15 = std::make_pair( nodecpp_13, nodecpp_14 ); nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_15 ); };
				start = endAmp + 1;
			}
		}
	}

	nodecpp::handler_ret_type yetSimpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
//			response->dbgTrace();
			co_await response->flushHeaders();
			co_await response->end();
			CO_RETURN;
		}

		const auto & url = request->getUrl();
		size_t start = url.find_first_of( "?" );
		response->setStatus( "HTTP/1.1 200 OK" );
		response->addHeader( "Content-Type", "text/xml" );
		Buffer b;
		if ( start == std::string::npos )
		{
			b.append( "no value specified", sizeof( "undefined" ) - 1 );
			response->addHeader( "Connection", "close" );
		}
		else
		{
			std::vector<std::pair<std::string, std::string>> queryValues;
			nodecpp::safememory::dezombiefy( this )->parseUrlQueryString(nodecpp::safememory::dezombiefy( url ).substr(start+1), queryValues );
			for ( auto entry: queryValues )
				if ( entry.first == "value" )
				{
					{ auto& nodecpp_0 = entry.second; auto& nodecpp_1 = entry.second; auto nodecpp_2 = nodecpp_0.c_str(); auto nodecpp_3 = nodecpp::safememory::dezombiefy( nodecpp_1 ).size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_2 ), nodecpp_3 ); };
					b.appendUint8( ',' );
				}
			if ( b.size() )
			{
				b.trim( 1 );
				response->addHeader( "Connection", "keep-alive" );
			}
			else
			{
				b.append( "no value specified", sizeof( "no value specified" ) - 1 );
				response->addHeader( "Connection", "close" );
			}
		}
		response->addHeader( "Content-Length", fmt::format( "{}", b.size() ) );
//		response->dbgTrace();
		co_await response->flushHeaders();
		co_await response->writeBodyPart(b);
		co_await response->end();

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<CtrlServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
#line 425 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
			co_await socket->a_read( r_buff, 2 );
			co_await nodecpp::safememory::dezombiefy( this )->onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = nodecpp::safememory::dezombiefy( buffer ).begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(nodecpp::safememory::dezombiefy( this )->stats));
			nodecpp::safememory::dezombiefy( this )->stats.connCnt = nodecpp::safememory::dezombiefy( this )->srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			ctrlReplyBuff.clear();
			nodecpp::safememory::dezombiefy( this )->ctrlReplyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(nodecpp::safememory::dezombiefy( this )->ctrlReplyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H
#line 452 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
#line 6 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code\\NetSocket.cpp"

static NodeRegistrator<Runnable<MySampleTNode>> noname( "MySampleTemplateNode" );
