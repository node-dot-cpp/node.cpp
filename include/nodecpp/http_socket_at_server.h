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
//printf( "resumed ahd_continueGetting\n" );
					}
				};
				return continue_getting_awaiter(*this);
			}

			auto a_dataAvailable( CircularByteBuffer::AvailableDataDescriptor& d ) { 

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					CircularByteBuffer::AvailableDataDescriptor& d;

					data_awaiter(SocketBase& socket_, CircularByteBuffer::AvailableDataDescriptor& d_) : socket( socket_ ), d( d_ ) {}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() != 0;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = 1;
						nodecpp::setNoException(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						socket.dataForCommandProcessing.readBuffer.get_available_data( d );
					}
				};
				return data_awaiter(*this, d);
			}

			auto a_dataAvailable( uint32_t period, CircularByteBuffer::AvailableDataDescriptor& d ) { 

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					CircularByteBuffer::AvailableDataDescriptor& d;
					uint32_t period;
					nodecpp::Timeout to;

					data_awaiter(SocketBase& socket_, uint32_t period_, CircularByteBuffer::AvailableDataDescriptor& d_) : socket( socket_ ), d( d_ ), period( period_ ) {}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() > 0;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = 1;
						nodecpp::setNoException(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( awaiting, period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( to );
						if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						socket.dataForCommandProcessing.readBuffer.get_available_data( d );
					}
				};
				return data_awaiter(*this, period, d);
			}

			nodecpp::handler_ret_type readLine(std::string& line)
			{
				size_t pos = 0;
				line.clear();
				CircularByteBuffer::AvailableDataDescriptor d;
				for(;;)
				{
					co_await a_dataAvailable( d );
					for ( ; pos<d.sz1; ++pos )
						if ( d.ptr1[pos] == '\n' )
						{
							line.append( (const char*)(d.ptr1), pos + 1 );
							dataForCommandProcessing.readBuffer.skip_data( pos + 1 );
							CO_RETURN;
						}
					line += std::string( (const char*)(d.ptr1), pos );
					dataForCommandProcessing.readBuffer.skip_data( pos );
					pos = 0;
					if ( d.ptr2 && d.sz2 )
					{
						for ( ; pos<d.sz2; ++pos )
							if ( d.ptr2[pos] == '\n' )
							{
								line += std::string( (const char*)(d.ptr2), pos + 1 );
								dataForCommandProcessing.readBuffer.skip_data( pos + 1 );
								CO_RETURN;
							}
						line += std::string( (const char*)(d.ptr2), pos );
						dataForCommandProcessing.readBuffer.skip_data( pos );
						pos = 0;
					}
				}

				CO_RETURN;
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
//printf( "about to get (next) request\n" );
					co_await getRequest( *request );
					auto cg = a_continueGetting();

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
//printf( "about to resume ahd_continueGetting\n" );
					hr();
				}
			}

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
				std::string url;
				std::string version;
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
//				if ( bodyBytesRetrieved == getContentLength() )
//					sock->proceedToNext();

				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES


			bool parseMethod( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::noinit ); 
				size_t start = line.find_first_not_of( " \t" );
				if ( start == std::string::npos || line[start] == '\r' || line[start] == '\n' )
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

			bool parseHeaderEntry( const std::string& line )
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, readStatus == ReadStatus::in_hdr ); 
				size_t end = line.find_last_not_of(" \t\r\n" );
				if ( end == std::string::npos )
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
				std::string key = line.substr( start, idx-start );
				header.insert( std::make_pair( makeLower( key ), line.substr( valStart, end - valStart + 1 ) ));
				return true;
			}

			const std::string& getMethod() { return method.name; }
			const std::string& getUrl() { return method.url; }
			const std::string& getHttpVersion() { return method.version; }

			size_t getContentLength() const { return contentLength; }

			void dbgTrace()
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {} {} HTTP/{}", method.name, method.url, method.version );
				for ( auto& entry : header )
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   [->] {}: {}", entry.first, entry.second );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[CL = {}, Conn = {}]", getContentLength(), connStatus == ConnStatus::keep_alive ? "keep-alive" : "close" );
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

#ifndef NODECPP_NO_COROUTINES
			nodecpp::handler_ret_type flushHeaders()
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::notyet ); 
				// TODO: add real implementation
				headerBuff.append( replyStatus.c_str(), replyStatus.size() );
				headerBuff.append( "\r\n", 2 );
				for ( auto h: header )
				{
					headerBuff.append( h.first.c_str(), h.first.size() );
					headerBuff.append( ": ", 2 );
					headerBuff.append( h.second.c_str(), h.second.size() );
					headerBuff.append( "\r\n", 2 );
				}
				headerBuff.append( "\r\n", 2 );

				parseContentLength();
				parseConnStatus();
//				co_await sock->a_write( headerBuff );
				writeStatus = WriteStatus::hdr_flushed;
				header.clear();
				CO_RETURN;
			}

			nodecpp::handler_ret_type writeBodyPart(Buffer& b)
			{
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, writeStatus == WriteStatus::hdr_flushed ); 
				// TODO: add real implementation
				try {
					if ( headerBuff.size() )
					{
						headerBuff.append( b );
						co_await sock->a_write( headerBuff );
						headerBuff.clear();
					}
					else
						co_await sock->a_write( headerBuff );
				} 
				catch(...) {
					sock->end();
					clear();
					sock->proceedToNext();
				}
//printf( "request has been sent\n" );
sock->request->clear();
				if ( connStatus != ConnStatus::keep_alive )
				{
//printf( "socket has been ended\n" );
					sock->end();
					clear();
					CO_RETURN;
				}
//				else
					clear();
sock->proceedToNext();
//printf( "getting next request has been allowed\n" );
				CO_RETURN;
			}
#endif // NODECPP_NO_COROUTINES
		};

		inline
		nodecpp::handler_ret_type HttpSocketBase::getRequest( IncomingHttpMessageAtServer& message )
		{
			bool ready = false;
			std::string line;
			co_await readLine(line);
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			if ( !message.parseMethod( line ) )
			{
				// TODO: report error
				end();
			}

			do
			{
				co_await readLine(line);
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			}
			while ( message.parseHeaderEntry( line ) );

			CO_RETURN;
		}

		inline
		HttpSocketBase::HttpSocketBase() {
			request = nodecpp::safememory::make_owning<IncomingHttpMessageAtServer>();
			request->sock = myThis.getSoftPtr<HttpSocketBase>(this);
			response = nodecpp::safememory::make_owning<HttpServerResponse>();
			response->sock = myThis.getSoftPtr<HttpSocketBase>(this);
			run(); // TODO: think about proper time for this call
		}

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SOCKET_AT_SERVER_H
