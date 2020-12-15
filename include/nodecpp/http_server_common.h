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

#include "common.h"
#include "server_common.h"

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
					template<class T> using userIncomingRequestMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::soft_ptr<HttpServerResponse>);
					template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

					// originating from member functions of NodeBase-derived classes
					template<class T, class ServerT> using userIncomingRequestNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, nodecpp::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::soft_ptr<HttpServerResponse>);
					template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, bool);
					template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, Error&);

					using userDefIncomingRequestHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<HttpServerBase>, nodecpp::soft_ptr<IncomingHttpMessageAtServer>, nodecpp::soft_ptr<HttpServerResponse>);
					using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<HttpServerBase>, bool);
					using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<HttpServerBase>, Error&);

					// originating from member functions of HttpServerBase-derived classes

					template<class ObjectT, userIncomingRequestMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type incomingRequestHandler( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, nodecpp::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::soft_ptr<HttpServerResponse> response )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(request);
						CO_RETURN;
					}

					template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type closeHandler( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, bool hadError )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
						CO_RETURN;
					}

					template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type errorHandler( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, Error& e )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
						CO_RETURN;
					}

					// originating from member functions of NodeBase-derived classes

					template<class ObjectT, class ServerT, userIncomingRequestNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type incomingRequestHandlerFromNode( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, nodecpp::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::soft_ptr<HttpServerResponse> response )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), request, response);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userCloseNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type closeHandlerFromNode( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, bool hadError )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), hadError);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userErrorNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type errorHandlerFromNode( void* objPtr, nodecpp::soft_ptr<HttpServerBase> serverPtr, Error& e )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), e);
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
				void handleIncomingRequesEvent(nodecpp::soft_ptr<HttpServerBase> server, nodecpp::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::soft_ptr<HttpServerResponse> response) { userHandlers.userDefIncomingRequestHandlers.execute(server, request, response); }

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(nodecpp::soft_ptr<HttpServerBase> server, bool hasError) { userHandlers.userDefCloseHandlers.execute(server, hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(nodecpp::soft_ptr<HttpServerBase> server, Error& e) { userHandlers.userDefErrorHandlers.execute(server, e); }
			};
			DataForHttpCommandProcessing dataForHttpCommandProcessing;

		public:
			HttpServerBase() {}
			HttpServerBase(event::HttpRequest::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				eHttpRequest.on(std::move(cb));
			}
			virtual ~HttpServerBase() {}

#ifndef NODECPP_NO_COROUTINES
			struct awaitable_request_data
			{
				awaitable_handle_t h = nullptr;
				nodecpp::soft_ptr<IncomingHttpMessageAtServer> request;
				nodecpp::soft_ptr<HttpServerResponse> response;
			};
			awaitable_request_data ahd_request;

			void forceReleasingAllCoroHandles()
			{
				if ( ahd_request.h != nullptr )
				{
					auto hr = ahd_request.h;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					ahd_request.h = nullptr;
					hr();
				}
			}

			void onNewRequest( nodecpp::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::soft_ptr<HttpServerResponse> response );

			auto a_request(nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::soft_ptr<HttpServerResponse>& response) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::soft_ptr<HttpServerResponse>& response;

					connection_awaiter(HttpServerBase& server_, nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::soft_ptr<HttpServerResponse>& response_) : 
						server( server_ ), request( request_ ), response( response_) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						server.ahd_request.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isCoroException(myawaiting) )
							throw nodecpp::getCoroException(myawaiting);
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.request != nullptr ); 
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, server.ahd_request.response != nullptr ); 
						request = server.ahd_request.request;
						response = server.ahd_request.response;
					}
				};
				return connection_awaiter(*this, request, response);
			}

			auto a_request(nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request, nodecpp::soft_ptr<HttpServerResponse>& response, uint32_t period) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					HttpServerBase& server;
					nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request;
					nodecpp::soft_ptr<HttpServerResponse>& response;
					uint32_t period;
					nodecpp::Timeout to;

					connection_awaiter(HttpServerBase& server_, nodecpp::soft_ptr<IncomingHttpMessageAtServer>& request_, nodecpp::soft_ptr<HttpServerResponse>& response_, uint32_t period_) : 
						server( server_ ), request( request_ ), response( response_), period( period_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						server.ahd_request.h = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( server.ahd_request.h, period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( to );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isCoroException(myawaiting) )
							throw nodecpp::getCoroException(myawaiting);
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

			EventEmitter<event::HttpRequest> eHttpRequest;
			void on(nodecpp::string_literal name, event::HttpRequest::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::HttpRequest::name);
				eHttpRequest.on(std::move(cb));
			}

		};

		template<class DataParentT>
		class HttpServer : public HttpServerBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpServer<DataParentT>() {};
			HttpServer(event::HttpRequest::callback cb NODECPP_MAY_EXTEND_TO_THIS) : HttpServerBase(std::move(cb)) {}
			HttpServer<DataParentT>(DataParentT* dataParent ) : HttpServerBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			HttpServer<DataParentT>(DataParentT* dataParent, event::HttpRequest::callback cb NODECPP_MAY_EXTEND_TO_THIS ) : HttpServerBase(std::move(cb)), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~HttpServer<DataParentT>() {}
		};

		template<>
		class HttpServer<void> : public HttpServerBase
		{
		public:
			using DataParentType = void;
			HttpServer() {};
			HttpServer(event::HttpRequest::callback cb NODECPP_MAY_EXTEND_TO_THIS) : HttpServerBase(std::move(cb)) {}
			virtual ~HttpServer() {}
		};

		class HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;

		protected:
			nodecpp::soft_ptr<HttpSocketBase> sock;
			uint64_t idx = (uint64_t)(-1);
			nodecpp::soft_ptr<HttpMessageBase> counterpart;

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

			typedef nodecpp::map<nodecpp::string, nodecpp::string> header_t; // so far good for both directions
			header_t header;

			// utils
			nodecpp::string makeLower( nodecpp::string& str ) // quick and dirty; TODO: revise (see, for instance, discussion at https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case)
			{
				std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
				return str;
			}

			void parseContentLength()
			{
				auto cl = header.find( nodecpp::string_literal("content-length") );
				if ( cl != header.end() )
					contentLength = ::atol( cl->second.c_str() ); // quick and dirty; TODO: revise
				contentLength = 0;
			}

			void parseConnStatus()
			{
				auto cs = header.find( nodecpp::string_literal("connection") );
				if ( cs != header.end() )
				{
					nodecpp::string val = cs->second;
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

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SERVER_COMMON_H
