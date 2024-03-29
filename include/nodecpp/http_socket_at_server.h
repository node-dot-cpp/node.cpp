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
#include "socket_common.h"
#include "url.h"

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

#ifndef NODECPP_NO_COROUTINES
			static constexpr size_t maxHeaderSize = 0x4000;
			Buffer lineBuffer;
#endif // NODECPP_NO_COROUTINES

			::nodecpp::awaitable<CoroStandardOutcomes> getRequest( IncomingHttpMessageAtServer& message );

			struct RRPair
			{
				nodecpp::owning_ptr<IncomingHttpMessageAtServer> request;
				nodecpp::owning_ptr<HttpServerResponse> response;
				bool active = false;
			};

			template<size_t sizeExp>
			class RRQueue
			{
				RRPair* cbuff = nullptr;
				uint64_t head = 0;
				uint64_t tail = 0;
				size_t idxToStorageIdx(size_t idx ) { return idx & ((((size_t)1)<<sizeExp)-1); }
				size_t capacity() { return ((size_t)1)<<sizeExp; }
			public:
				RRQueue() {}
				~RRQueue() { 
					if ( cbuff != nullptr ) {
						size_t size = ((size_t)1 << sizeExp);
						nodecpp::dealloc( cbuff, size );
					}
				}
				void init( nodecpp::soft_ptr<HttpSocketBase> );
				bool canPush() { return head - tail < ((uint64_t)1<<sizeExp); }
				bool release( size_t idx )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx >= tail, "{} vs. {}", idx, tail );
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, idx < head, "{} vs. {}", idx, head );
					size_t storageIdx = idxToStorageIdx( idx );
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, storageIdx < capacity(), "{} vs. {}", storageIdx, capacity() );
					cbuff[storageIdx].active = false;
					if ( idx == tail )
					{
						do { ++tail; }
						while ( tail < head && !cbuff[idxToStorageIdx(tail)].active );
					}
					return canPush();
				}
				RRPair& getHead() {
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, canPush() );
					auto& ret = cbuff[idxToStorageIdx(head)];
					ret.active = true;
					ret.request->idx = head;
					ret.response->idx = head;
					++head;
					return ret;
				}
			};
			RRQueue<0> rrQueue;
			bool release( size_t idx ) { return rrQueue.release( idx ); }

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
						nodecpp::initCoroData(awaiting);
						socket.ahd_continueGetting = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isCoroException(myawaiting) )
							throw nodecpp::getCoroException(myawaiting);
					}
				};
				return continue_getting_awaiter(*this);
			}

			::nodecpp::awaitable<CoroStandardOutcomes> readLine(nodecpp::string& line)
			{
				lineBuffer.clear();
				CoroStandardOutcomes ret = co_await a_readUntil( lineBuffer, '\n' );
				if ( ret == CoroStandardOutcomes::ok )
				{
#ifdef NODECPP_USE_SAFE_MEMORY_CONTAINERS					
					line.assign_unsafe( (const char*)(lineBuffer.begin()), lineBuffer.size() );
#else
					line.assign( (const char*)(lineBuffer.begin()), lineBuffer.size() );
#endif // NODECPP_USE_SAFE_MEMORY_CONTAINERS					

					CO_RETURN CoroStandardOutcomes::ok;
				}
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, ret == CoroStandardOutcomes::insufficient_buffer ); 
				CO_RETURN ret;
			}

#endif // NODECPP_NO_COROUTINES

		public:
			HttpSocketBase();
			virtual ~HttpSocketBase() {}

#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type run()
			{
				for(;;)
				{
					// now we can reasonably expect a new request
					auto& rrPair = rrQueue.getHead();
					CoroStandardOutcomes status = co_await getRequest( *(rrPair.request) );

					if ( status == CoroStandardOutcomes::ok ) // the most likely outcome
					{
						soft_ptr_static_cast<HttpServerBase>(myServerSocket)->onNewRequest( rrPair.request, rrPair.response );
						if ( rrQueue.canPush() )
							continue;
						auto cg = a_continueGetting();
						co_await cg;
					}
					else
					{
						// TODO: switch status, report the other side an error, if applicable ( "413 Entity Too Large" for insufficient_buffer, for instance)
						end();
						CO_RETURN;
					}
				}
				CO_RETURN;
			}

			void proceedToNext()
			{
				if ( rrQueue.canPush() && ahd_continueGetting != nullptr )
				{
					auto hr = ahd_continueGetting;
					ahd_continueGetting = nullptr;
					hr();
				}
			}

			void forceReleasingAllCoroHandles()
			{
				if ( ahd_continueGetting != nullptr )
				{
					auto hr = ahd_continueGetting;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					ahd_continueGetting = nullptr;
					hr();
				}
			}
#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES

		};

		template<class RequestT, class DataParentT>
		class HttpSocket : public HttpSocketBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpSocket() {};
			HttpSocket(DataParentT* dataParent ) : HttpSocketBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~HttpSocket() {}
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
				nodecpp::string name;
				nodecpp::string url;
				nodecpp::string version;
				void clear() { name.clear(); url.clear(); version.clear(); }
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
#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type a_readBody( Buffer& b )
			{
				if ( bodyBytesRetrieved < getContentLength() )
				{
					b.clear();
					co_await sock->a_read( b, getContentLength() - bodyBytesRetrieved );
					bodyBytesRetrieved += b.size();
				}
				if ( bodyBytesRetrieved == getContentLength() )
					sock->proceedToNext();

				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES


			bool parseMethod( const nodecpp::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::noinit ); 
				size_t start = line.find_first_not_of( " \t" );
				if ( start == nodecpp::string::npos || line[start] == '\r' || line[start] == '\n' )
					return false;
				bool found = false;
				// Method name
				for ( size_t i=0; i<MethodCount; ++i )
					if ( line.size() > MethodNames[i].second + start && memcmp( line.c_str() + start, MethodNames[i].first, MethodNames[i].second ) == 0 && line.c_str()[ MethodNames[i].second] == ' ' ) // TODO: cthink about rfind(*,0)
					{
						method.name = MethodNames[i].first;
						start += MethodNames[i].second + 1;
						start = line.find_first_not_of( " \t", start );
						found = true;
						break;
					}
				if ( !found )
					return false;
				// URI
				size_t endOfURI = line.find_first_of(" \t\r\n", start + 1 );
				method.url = line.substr( start, endOfURI - start );
				if ( method.url.size() == 0 )
					return false;
				start = line.find_first_not_of( " \t", endOfURI );
				// HTTP version
				size_t end = line.find_last_not_of(" \t\r\n" );
				if ( memcmp( line.c_str() + start, "HTTP/", 5 ) != 0 )
					return false;
				start += 5;
				method.version = line.substr( start, end - start + 1 );
				readStatus = ReadStatus::in_hdr;
				return true;
			}

			bool parseHeaderEntry( const nodecpp::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::in_hdr ); 
				size_t end = line.find_last_not_of(" \t\r\n" );
				if ( end == nodecpp::string::npos )
				{
					if ( !( line.size() == 2 && line[0] == '\r' && line[1] == '\n' ) ) // last empty line
						return true; // TODO: what should we do with this line of spaces? - just ignore or report a parsing error?
					parseContentLength();
					readStatus = contentLength ? ReadStatus::in_body : ReadStatus::completed;
					return false;
				}
				size_t start = line.find_first_not_of( " \t" );
				size_t idx = line.find(':', start);
				if ( idx >= end )
					return false;
				size_t valStart = line.find_first_not_of( " \t", idx + 1 );
				nodecpp::string key = line.substr( start, idx-start );
				header.insert( nodecpp::make_pair( makeLower( key ), line.substr( valStart, end - valStart + 1 ) ));
				return true;
			}

			const nodecpp::string& getMethod() { return method.name; }
			const nodecpp::string& getUrl() { return method.url; }
			const nodecpp::string& getHttpVersion() { return method.version; }

			size_t getContentLength() const { return contentLength; }

			void dbgTrace()
			{
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "   [->] {} {} HTTP/{}", method.name, method.url, method.version );
				for ( auto& entry : header )
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "   [->] {}: {}", entry.first, entry.second );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "[CL = {}, Conn = {}]", getContentLength(), connStatus == ConnStatus::keep_alive ? "keep-alive" : "close" );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "" );
			}
		};

		class HttpServerResponse : protected HttpMessageBase // TODO: candidate for being a part of lib
		{
			friend class HttpSocketBase;
			friend class RRQueue;
			Buffer headerBuff;

		private:
			nodecpp::soft_ptr<IncomingHttpMessageAtServer> myRequest;
			typedef nodecpp::map<nodecpp::string, nodecpp::string> header_t;
			header_t header;
			size_t contentLength = 0;
			nodecpp::Buffer body;
			ConnStatus connStatus = ConnStatus::keep_alive;
			enum WriteStatus { notyet, hdr_serialized, hdr_flushed, in_body, completed };
			WriteStatus writeStatus = WriteStatus::notyet;

			nodecpp::string replyStatus;
			//size_t bodyBytesWritten = 0;

		private:
			nodecpp::handler_ret_type serializeHeaders()
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: add real implementation
				if ( replyStatus.size() ) // NOTE: this makes sense only if no headers were added via writeHeader()
					headerBuff.appendString( replyStatus );
				headerBuff.append( "\r\n", 2 );
				for ( auto h: header )
				{
					headerBuff.appendString( h.first );
					headerBuff.append( ": ", 2 );
					headerBuff.appendString( h.second );
					headerBuff.append( "\r\n", 2 );
				}
				headerBuff.append( "\r\n", 2 );

				parseContentLength();
				parseConnStatus();
				writeStatus = WriteStatus::hdr_serialized;
				header.clear();
				CO_RETURN;
			}


		public:
			HttpServerResponse() : headerBuff(0x1000) {}
			HttpServerResponse(const HttpServerResponse&) = delete;
			HttpServerResponse operator = (const HttpServerResponse&) = delete;
			HttpServerResponse(HttpServerResponse&& other)
			{
				replyStatus = std::move( other.replyStatus );
				header = std::move( other.header );
				contentLength = other.contentLength;
				headerBuff = std::move( other.headerBuff );
			}
			HttpServerResponse& operator = (HttpServerResponse&& other)
			{
				replyStatus = std::move( other.replyStatus );
				header = std::move( other.header );
				headerBuff = std::move( other.headerBuff );
				contentLength = other.contentLength;
				other.contentLength = 0;
				return *this;
			}
			void clear() // TODO: ensure necessity (added for reuse purposes)
			{
				replyStatus.clear();
				header.clear();
				body.clear();
				headerBuff.clear();
				contentLength = 0;
				writeStatus = WriteStatus::notyet;
			}

			void dbgTrace()
			{
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "   [<-] {}", replyStatus );
				for ( auto& entry : header )
					nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "   [<-] {}: {}", entry.first, entry.second );
				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "" );
			}

			template< class Str1, class Str2, size_t N>
			void writeHead( size_t statusCode, Str1 statusMessage, nodecpp::pair<nodecpp::string, nodecpp::string> headers[N] )
			{
				replyStatus = statusCode;
				setStatus( nodecpp::format( "{} {} {}", myRequest->getHttpVersion(), statusCode, statusMessage ) ); 
				for ( size_t i=0; i<N; ++i ) {
					header.insert( headers[i] ); 
				}
			}

			struct HeaderHolder
			{
				const char* key = nullptr;
				enum ValType { undef, str, num };
				ValType valType = undef;
				const char* valStr = nullptr;
				size_t valNum = 0;
				HeaderHolder( const char* key_, const char* val ) { key = key_; valType = ValType::str; valStr = val; }
				HeaderHolder( nodecpp::string key_, const char* val ) { key = key_.c_str(); valType = ValType::str; valStr = val; }
				HeaderHolder( nodecpp::string_literal key_, const char* val ) { key = key_.c_str(); valType = ValType::str; valStr = val; }

				HeaderHolder( const char* key_, nodecpp::string val ) { key = key_; valType = ValType::str; valStr = val.c_str(); }
				HeaderHolder( nodecpp::string key_, nodecpp::string val ) { key = key_.c_str(); valType = ValType::str; valStr = val.c_str(); }
				HeaderHolder( nodecpp::string_literal key_, nodecpp::string val ) { key = key_.c_str(); valType = ValType::str; valStr = val.c_str(); }

				HeaderHolder( const char* key_, nodecpp::string_literal val ) { key = key_; valType = ValType::str; valStr = val.c_str(); }
				HeaderHolder( nodecpp::string key_, nodecpp::string_literal val ) { key = key_.c_str(); valType = ValType::str; valStr = val.c_str(); }
				HeaderHolder( nodecpp::string_literal key_, nodecpp::string_literal val ) { key = key_.c_str(); valType = ValType::str; valStr = val.c_str(); }

				HeaderHolder( const char* key_, size_t val ) { key = key_; valType = ValType::num; valNum = val; }
				HeaderHolder( nodecpp::string key_, size_t val ) { key = key_.c_str(); valType = ValType::num; valNum = val; }
				HeaderHolder( nodecpp::string_literal key_, size_t val ) { key = key_.c_str(); valType = ValType::num; valNum = val; }

				nodecpp::string toStr() const
				{
					switch ( valType )
					{
						case str: return nodecpp::format( "{}: {}\r\n", key, valStr );
						case num: return nodecpp::format( "{}: {}\r\n", key, valNum );
						default:
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "unexpected value {}", (size_t)valType ); 
							return nodecpp::string( "" );
					}
				}
				const char* getKey() { return key; } 
			};

			template< class Str1>
			void writeHead( size_t statusCode, Str1 statusMessage, std::initializer_list<HeaderHolder> headers )
			{
				setStatus( nodecpp::format( "HTTP/{} {} {}\r\n", myRequest->getHttpVersion(), statusCode, statusMessage ) ); 
				for ( auto& h : headers ) {
					if ( strncmp( h.key, "Content-Length", sizeof("Content-Length")-1) != 0 )
						replyStatus.append( h.toStr() );
				}
				replyStatus.erase( replyStatus.end() - 2, replyStatus.end() );
			}

			void writeHead( size_t statusCode, std::initializer_list<HeaderHolder> headers )
			{
				setStatus( nodecpp::format( "HTTP/{} {}\r\n", myRequest->getHttpVersion(), statusCode ) ); 
				for ( auto& h : headers ) {
					if ( strncmp( h.key, "Content-Length", sizeof("Content-Length")-1) != 0 )
						replyStatus.append( h.toStr() );
				}
				replyStatus.erase( replyStatus.end() - 2, replyStatus.end() );
			}

			template< class Str>
			void writeHead( size_t statusCode, Str statusMessage )
			{
				setStatus( nodecpp::format( "HTTP/{} {} {}", myRequest->getHttpVersion(), statusCode, statusMessage ) ); 
			}

			void writeHead( size_t statusCode )
			{
				setStatus( nodecpp::format( "HTTP/{} {}", myRequest->getHttpVersion(), statusCode ) ); 
			}

			void addHeader( nodecpp::string key, nodecpp::string value )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: sanitize
				if ( key != "Content-Length" )
					header.insert( nodecpp::make_pair( key, value ) );
			}

			void setStatus( nodecpp::string status ) // temporary stub; TODO: ...
			{
				replyStatus = status;
			}

#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type flushHeaders()
			{
				if ( writeStatus == WriteStatus::notyet )
					serializeHeaders();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::hdr_serialized ); 
				try {
					co_await sock->a_write( headerBuff );
					headerBuff.clear();
					writeStatus = WriteStatus::hdr_flushed;
				} 
				catch(...) {
					// TODO: revise!!! should we close the socket? what should be done with other pipelined requests (if any)?
					sock->end();
					clear();
					sock->release( idx );
					sock->proceedToNext();
				}
				CO_RETURN;
			}

			nodecpp::handler_ret_type writeBodyPart(Buffer& b)
			{
				if ( writeStatus == WriteStatus::notyet )
					serializeHeaders();
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::hdr_serialized || writeStatus == WriteStatus::hdr_flushed ); 
				try {
					if ( writeStatus == WriteStatus::hdr_serialized )
					{
						headerBuff.append( b );
						co_await sock->a_write( headerBuff );
						headerBuff.clear();
						writeStatus = WriteStatus::hdr_flushed;
					}
					else
						co_await sock->a_write( b );
					writeStatus = WriteStatus::in_body;
				} 
				catch(...) {
					// TODO: revise!!! should we close the socket? what should be done with other pipelined requests (if any)?
					sock->end();
					clear();
					sock->release( idx );
					sock->proceedToNext();
				}
				CO_RETURN;
			}

			NODECPP_NO_AWAIT
			nodecpp::handler_ret_type end(Buffer& b)
			{
				if ( writeStatus != WriteStatus::in_body )
				{
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
					header.insert( nodecpp::make_pair( nodecpp::string("Content-Length"), format( "{}", b.size() ) ) );
				}
//dbgTrace();
				co_await writeBodyPart(b);

				myRequest->clear();
				if ( connStatus != ConnStatus::keep_alive )
				{
					sock->end();
					clear();
					CO_RETURN;
				}

				clear();
				sock->release( idx );
				sock->proceedToNext();
				CO_RETURN;
			}

			NODECPP_NO_AWAIT
			nodecpp::handler_ret_type end(const char* s)
			{
				// TODO: potentially, quite temporary implementation
				size_t ln = strlen( s );
				Buffer b( ln );
				b.append( s, ln );
				co_await end( b );
				CO_RETURN;
			}

			NODECPP_NO_AWAIT
			nodecpp::handler_ret_type end(nodecpp::string s)
			{
				// TODO: potentially, quite temporary implementation
				Buffer b;
				b.appendString( s );
				co_await end( b );
			}

			NODECPP_NO_AWAIT
			nodecpp::handler_ret_type end(nodecpp::string_literal s)
			{
				// TODO: potentially, quite temporary implementation
				Buffer b;
				b.appendString( s );
				co_await end( b );
			}

			NODECPP_NO_AWAIT
			nodecpp::handler_ret_type end()
			{
				if ( writeStatus != WriteStatus::hdr_flushed )
					co_await flushHeaders();
				myRequest->clear();
				if ( connStatus != ConnStatus::keep_alive )
				{
					sock->end();
					clear();
					CO_RETURN;
				}
//dbgTrace();
				clear();
				sock->release( idx );
				sock->proceedToNext();
				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES
		};

		inline
		void HttpServerBase::onNewRequest( nodecpp::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::soft_ptr<HttpServerResponse> response )
		{
//printf( "entering onNewRequest()  %s\n", ahd_request.h == nullptr ? "ahd_request.h is nullptr" : "" );
			if ( ahd_request.h != nullptr )
			{
				ahd_request.request = request;
				ahd_request.response = response;
				auto hr = ahd_request.h;
				ahd_request.h = nullptr;
//printf( "about to rezume ahd_request.h\n" );
				hr();
			}
			else if ( dataForHttpCommandProcessing.isIncomingRequesEventHandler() )
				dataForHttpCommandProcessing.handleIncomingRequesEvent( myThis.getSoftPtr<HttpServerBase>(this), request, response );
			else if ( eHttpRequest.listenerCount() )
				eHttpRequest.emit( *request, *response );
		}

		inline
		::nodecpp::awaitable<CoroStandardOutcomes> HttpSocketBase::getRequest( IncomingHttpMessageAtServer& message )
		{
			nodecpp::string line;
			CoroStandardOutcomes status = co_await readLine(line);
			if ( status == CoroStandardOutcomes::ok )
			{
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
				if ( !message.parseMethod( line ) )
				{
					// TODO: report error
					CO_RETURN CoroStandardOutcomes::failed;
				}
			}
			else
				CO_RETURN status;

			do
			{
				status = co_await readLine(line);
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			}
			while ( status == CoroStandardOutcomes::ok && message.parseHeaderEntry( line ) );

			CO_RETURN message.readStatus == IncomingHttpMessageAtServer::ReadStatus::in_body || message.readStatus == IncomingHttpMessageAtServer::ReadStatus::completed ? CoroStandardOutcomes::ok : CoroStandardOutcomes::failed;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		inline
		HttpSocketBase::HttpSocketBase() {
#ifndef NODECPP_NO_COROUTINES
			lineBuffer.reserve(maxHeaderSize);
#endif // NODECPP_NO_COROUTINES
			rrQueue.init( myThis.getSoftPtr<HttpSocketBase>(this) );
			run(); // TODO: think about proper time for this call
		}

		template<size_t sizeExp>
		void HttpSocketBase::RRQueue<sizeExp>::init( nodecpp::soft_ptr<HttpSocketBase> socket ) {
			size_t size = ((size_t)1 << sizeExp);
			cbuff = nodecpp::alloc<RRPair>( size ); // TODO: use nodecpp::a
			//cbuff = new RRPair [size];
			for ( size_t i=0; i<size; ++i )
			{
				cbuff[i].request = nodecpp::make_owning<IncomingHttpMessageAtServer>();
				cbuff[i].response = nodecpp::make_owning<HttpServerResponse>();
				nodecpp::soft_ptr<IncomingHttpMessageAtServer> tmprq = (cbuff[i].request);
				nodecpp::soft_ptr<HttpServerResponse> tmrsp = (cbuff[i].response);
				cbuff[i].response->counterpart = nodecpp::soft_ptr_reinterpret_cast<HttpMessageBase>(tmprq);
				cbuff[i].request->counterpart = nodecpp::soft_ptr_reinterpret_cast<HttpMessageBase>(tmrsp);
				cbuff[i].request->sock = socket;
				cbuff[i].response->sock = socket;
				cbuff[i].response->myRequest = cbuff[i].request;
			}
		}	

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SOCKET_AT_SERVER_H
