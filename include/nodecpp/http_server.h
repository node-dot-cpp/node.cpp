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

		class HttpMessageBase // TODO: candidate for being a part of lib
		{
		public:
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

			struct Method // so far a struct
			{
				std::string name;
				std::string value;
				void clear() { name.clear(); value.clear(); }
			};
			Method method;

		public:
			// utils
			std::string makeLower( std::string& str ) // quick and dirty; TODO: revise (see, for instance, discussion at https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case)
			{
				std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
				return str;
			}
		};

		class IncomingHttpMessageAtServer : protected HttpMessageBase // TODO: candidate for being a part of lib
		{
		private:
			typedef std::map<std::string, std::string> header_t;
			header_t header;
			size_t contentLength = 0;
			nodecpp::Buffer body;
			enum ConnStatus { close, keep_alive };
			ConnStatus connStatus = ConnStatus::keep_alive;
			enum ReadStatus { noinit, in_hdr, in_body, completed };
			ReadStatus readStatus = ReadStatus::noinit;

		private:
			void setCL()
			{
				auto cl = header.find( "content-length" );
				if ( cl != header.end() )
					contentLength = ::atol( cl->second.c_str() ); // quick and dirty; TODO: revise
				contentLength = 0;
			}

			void setConnStatus()
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
				contentLength = 0;
			}

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
					setCL();
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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{} {}", method.name, method.value );
				for ( auto& entry : header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[CL = {}, Conn = {}]", getContentLength(), connStatus == ConnStatus::keep_alive ? "keep-alive" : "close" );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "" );
			}
		};

		class HttpServerBase : public nodecpp::net::ServerBase
		{
		public:
			HttpServerBase() {}
			virtual ~HttpServerBase() {}

			struct UserHandlersCommon
			{
			public:
				// originating from member functions of ServertBase-derived classes
				template<class T> using userIncomingRequestMemberHandler = nodecpp::handler_ret_type (T::*)(/*...*/);
				template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
				template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

				// originating from member functions of NodeBase-derived classes
				template<class T, class ServerT> using userIncomingRequestNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>/*, ...*/);
				template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, bool);
				template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, Error&);

				using userDefIncomingRequestHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>/*, ...*/);
				using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, bool);
				using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, Error&);

				// originating from member functions of ServerBase-derived classes

				template<class ObjectT, userIncomingRequestMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type incomingRequestHandler( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(id, addr);
					CO_RETURN;
				}

				template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type closeHandler( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, bool hadError )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
					CO_RETURN;
				}

				template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
				static nodecpp::handler_ret_type errorHandler( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, Error& e )
				{
					//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
					CO_RETURN;
				}

				// originating from member functions of NodeBase-derived classes

				template<class ObjectT, class ServerT, userIncomingRequestNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type incomingRequestHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), id, addr);
					CO_RETURN;
				}

				template<class ObjectT, class ServerT, userCloseNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type closeHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, bool hadError )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), hadError);
					CO_RETURN;
				}

				template<class ObjectT, class ServerT, userErrorNodeMemberHandler<ObjectT, ServerT> MemberFnT>
				static nodecpp::handler_ret_type errorHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, Error& e )
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
			void handleIncomingRequesEvent(nodecpp::safememory::soft_ptr<ServerBase> server, size_t id, nodecpp::net::Address address) { userHandlers.userDefIncomingRequestHandlers.execute(server, id, address); }

			bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
			void handleCloseEvent(nodecpp::safememory::soft_ptr<ServerBase> server, bool hasError) { userHandlers.userDefCloseHandlers.execute(server, hasError); }

			bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
			void handleErrorEvent(nodecpp::safememory::soft_ptr<ServerBase> server, Error& e) { userHandlers.userDefErrorHandlers.execute(server, e); }
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


		template<class DP>
		class HttpSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<DP>
		{
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
				nodecpp::Buffer r_buff(0x200);
				while ( !dbuf.popLine( lb ) )
				{
					co_await a_read( r_buff, 2 );
					dbuf.pushFragment( r_buff );
				}

				CO_RETURN;
			}

			nodecpp::handler_ret_type getRequest( IncomingHttpMessageAtServer& message )
			{
				bool ready = false;
				Buffer lb;
				co_await readLine(lb);
				lb.appendUint8( 0 );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
				if ( !message.setMethod( std::string( reinterpret_cast<char*>(lb.begin()) ) ) )
				{
					end();
					co_await sendReply();
				}

				do
				{
					lb.clear();
					co_await readLine(lb);
					lb.appendUint8( 0 );
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line: {}", reinterpret_cast<char*>(lb.begin()) );
				}
				while ( message.addHeaderEntry( std::string( reinterpret_cast<char*>(lb.begin()) ) ) );

				if ( message.getContentLength() )
				{
					lb.clear();
					lb.reserve( message.getContentLength() );
					co_await a_read( lb, message.getContentLength() );
				}

				CO_RETURN;
			}

		public:
			using NodeType = DP;

		public:
			HttpSocket() {}
			HttpSocket(DP* node) : nodecpp::net::SocketBase(), ::nodecpp::DataParent<DP>(node) {}
			virtual ~HttpSocket() {}

			nodecpp::handler_ret_type sendReply2()
			{
				Buffer reply;
				std::string replyHeadFormat = "HTTP/1.1 200 OK\r\n"
					"Content-Length: {}\r\n"
					"Content-Type: text/html\r\n"
					"Connection: keep-alive\r\n"
					"\r\n";
				std::string replyHtmlFormat = "<html>\r\n"
					"<body>\r\n"
					"<h1>Get reply! (# {})</h1>\r\n"
					"</body>\r\n"
					"</html>\r\n";
				std::string replyHtml = fmt::format( replyHtmlFormat.c_str(), this->getDataParent()->stats.rqCnt + 1 );
				std::string replyHead = fmt::format( replyHeadFormat.c_str(), replyHtml.size() );

				std::string r = replyHead + replyHtml;
				reply.append( r.c_str(), r.size() );
				write(reply);
		//			end();

				++(this->getDataParent()->stats.rqCnt);

				CO_RETURN;
			}

			nodecpp::handler_ret_type sendReply()
			{
				Buffer reply;
				std::string replyBegin = "HTTP/1.1 200 OK\r\n"
				"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
				"Server: Apache/2.2.14 (Win32)\r\n"
				"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
				"Content-Length: 88\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
				"\r\n\r\n"
				"<html>\r\n"
				"<body>\r\n";
							std::string replyEnd = "</body>\r\n"
				"</html>\r\n"
				"\r\n\r\n";
				std::string replyBody = fmt::format( "<h1>Get reply! (# {})</h1>\r\n", this->getDataParent()->stats.rqCnt + 1 );
				std::string r = replyBegin + replyBody + replyEnd;
				reply.append( r.c_str(), r.size() );
				write(reply);
				end();

				++(this->getDataParent()->stats.rqCnt);

				CO_RETURN;
			}

			nodecpp::handler_ret_type processRequest()
			{
				IncomingHttpMessageAtServer message;
				co_await getRequest( message );
				message.dbgTrace();

				++rqCnt;
				co_await sendReply();

				CO_RETURN;
			}

		};

	} //namespace net
} //namespace nodecpp

#endif //HTTP_SERVER_H
