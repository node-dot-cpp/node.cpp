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

			nodecpp::handler_ret_type getRequest( IncomingHttpMessageAtServer& message );
			nodecpp::handler_ret_type getRequest2( IncomingHttpMessageAtServer& message );

			struct RRPair
			{
				nodecpp::safememory::owning_ptr<IncomingHttpMessageAtServer> request;
				nodecpp::safememory::owning_ptr<HttpServerResponse> response;
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
				void init( nodecpp::safememory::soft_ptr<HttpSocketBase> );
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
						nodecpp::setNoException(awaiting);
						socket.ahd_continueGetting = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
					}
				};
				return continue_getting_awaiter(*this);
			}

			auto a_readByte() { 

				struct read_byte {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;

					read_byte(SocketBase& socket_) : socket( socket_ ) {
					}

					read_byte(const read_byte &) = delete;
					read_byte &operator = (const read_byte &) = delete;
	
					~read_byte() {
					}

					bool await_ready() {
						return !socket.dataForCommandProcessing.readBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_byte_crh_except, nullptr, 0 );
								throw nodecpp::getException(myawaiting);
							}
							uint8_t ret = socket.dataForCommandProcessing.readBuffer.read_byte();
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_byte_crh_ok, &ret, sizeof( ret ) );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_byte_crh_except )
								throw nodecpp::getException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_byte_crh_ok )
							{
								NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size == 1, "indeed: {}", frame.size );
								return *(uint8_t*)(frame.ptr);
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
								throw nodecpp::getException(myawaiting);
							return socket.dataForCommandProcessing.readBuffer.read_byte();
						}
					}
				};
				return read_byte(*this);
			}

			auto a_dataAvailable( CircularByteBuffer::AvailableDataDescriptor& d, size_t minBytes = 1 ) { 

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					CircularByteBuffer::AvailableDataDescriptor& d;
					size_t minBytes;

					data_awaiter(SocketBase& socket_, CircularByteBuffer::AvailableDataDescriptor& d_, size_t minBytes_) : socket( socket_ ), d( d_ ), minBytes(minBytes_) {
					}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {
					}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() && socket.dataForCommandProcessing.readBuffer.used_size() >= minBytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = minBytes;
						nodecpp::setNoException(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						static_assert( sizeof( d.sz1 ) == sizeof( size_t ) );
						static_assert( sizeof( d.sz2 ) == sizeof( size_t ) );
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except, nullptr, 0 );
								throw nodecpp::getException(myawaiting);
							}
							socket.dataForCommandProcessing.readBuffer.get_available_data( d );
							::nodecpp::threadLocalData.binaryLog->startAddingFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_ok, &(d), sizeof( d ) );
							if ( d.ptr1 != nullptr && d.sz1 != 0 )
								::nodecpp::threadLocalData.binaryLog->continueAddingFrame( d.ptr1, d.sz1 );
							if ( d.ptr2 != nullptr && d.sz2 != 0 )
								::nodecpp::threadLocalData.binaryLog->continueAddingFrame( d.ptr2, d.sz2 );
							::nodecpp::threadLocalData.binaryLog->addingFrameDone();
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except )
								throw nodecpp::getException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_ok )
							{
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ), "{} vs. {}", frame.size, sizeof( d ) ); 
								uint8_t* buff = (uint8_t*)(frame.ptr);
								memcpy( &d, buff, sizeof( d ) );
								buff += sizeof( d );
								if ( d.ptr1 != nullptr && d.sz1 != 0 )
								{
									NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ) + d.sz1, "{} vs. {}", frame.size, sizeof( d ) + d.sz1 ); 
									d.ptr1 = buff;
									buff += d.sz1;
								}
								else
								{
									if ( d.ptr1 != nullptr )
									{
										d.ptr1 = buff;
										NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.sz1 == 0, "indeed: {}", d.sz1 ); 
									}
								}
								if ( d.ptr2 != nullptr && d.sz2 != 0 )
								{
									NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ) + d.sz1 + d.sz2, "{} vs. {}", frame.size, sizeof( d ) + d.sz1 + d.sz2 ); 
									d.ptr2 = buff;
									buff += d.sz2;
								}
								else
								{
									if ( d.ptr2 != nullptr )
									{
										d.ptr2 = buff;
										NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.sz2 == 0, "indeed: {}", d.sz2 ); 
									}
								}
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
								throw nodecpp::getException(myawaiting);
							socket.dataForCommandProcessing.readBuffer.get_available_data( d );
						}
					}
				};
				return data_awaiter(*this, d, minBytes);
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
#ifdef NODECPP_RECORD_AND_REPLAY
						static_assert( sizeof( d.sz1 ) == sizeof( size_t ) );
						static_assert( sizeof( d.sz2 ) == sizeof( size_t ) );
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							nodecpp::clearTimeout( to );
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except, nullptr, 0 );
								throw nodecpp::getException(myawaiting);
							}
							socket.dataForCommandProcessing.readBuffer.get_available_data( d );
							::nodecpp::threadLocalData.binaryLog->startAddingFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_ok, &(d), sizeof( d ) );
							if ( d.ptr1 != nullptr && d.sz1 != 0 )
								::nodecpp::threadLocalData.binaryLog->continueAddingFrame( d.ptr1, d.sz1 );
							if ( d.ptr2 != nullptr && d.sz2 != 0 )
								::nodecpp::threadLocalData.binaryLog->continueAddingFrame( d.ptr2, d.sz2 );
							::nodecpp::threadLocalData.binaryLog->addingFrameDone();
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							nodecpp::clearTimeout( to );
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except )
								throw nodecpp::getException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_ok )
							{
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ), "{} vs. {}", frame.size, sizeof( d ) ); 
								uint8_t* buff = (uint8_t*)(frame.ptr);
								memcpy( &d, buff, sizeof( d ) );
								buff += sizeof( d );
								if ( d.ptr1 != nullptr && d.sz1 != 0 )
								{
									NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ) + d.sz1, "{} vs. {}", frame.size, sizeof( d ) + d.sz1 ); 
									d.ptr1 = buff;
									buff += d.sz1;
								}
								else
								{
									if ( d.ptr1 != nullptr )
									{
										d.ptr1 = buff;
										NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.sz1 == 0, "indeed: {}", d.sz1 ); 
									}
								}
								if ( d.ptr2 != nullptr && d.sz2 != 0 )
								{
									NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.size >= sizeof( d ) + d.sz1 + d.sz2, "{} vs. {}", frame.size, sizeof( d ) + d.sz1 + d.sz2 ); 
									d.ptr2 = buff;
									buff += d.sz2;
								}
								else
								{
									if ( d.ptr2 != nullptr )
									{
										d.ptr2 = buff;
										NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, d.sz2 == 0, "indeed: {}", d.sz2 ); 
									}
								}
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							nodecpp::clearTimeout( to );
							if ( myawaiting != nullptr && nodecpp::isException(myawaiting) )
								throw nodecpp::getException(myawaiting);
							socket.dataForCommandProcessing.readBuffer.get_available_data( d );
						}
					}
				};
				return data_awaiter(*this, period, d);
			}

			nodecpp::handler_ret_type readLine(nodecpp::string& line)
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
					line += nodecpp::string( (const char*)(d.ptr1), pos );
					dataForCommandProcessing.readBuffer.skip_data( pos );
					pos = 0;
					if ( d.ptr2 && d.sz2 )
					{
						for ( ; pos<d.sz2; ++pos )
							if ( d.ptr2[pos] == '\n' )
							{
								line += nodecpp::string( (const char*)(d.ptr2), pos + 1 );
								dataForCommandProcessing.readBuffer.skip_data( pos + 1 );
								CO_RETURN;
							}
						line += nodecpp::string( (const char*)(d.ptr2), pos );
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
					// first test for continuation
					CircularByteBuffer::AvailableDataDescriptor d;
					co_await a_dataAvailable( d, 0 );
					if ( d.sz1 == 0 ) // no more data - no more requests
						CO_RETURN;

					// now we can reasonably expect a new request
					auto& rrPair = rrQueue.getHead();
					co_await getRequest( *(rrPair.request) );

					nodecpp::safememory::soft_ptr_static_cast<HttpServerBase>(myServerSocket)->onNewRequest( rrPair.request, rrPair.response );
					if ( rrQueue.canPush() )
						continue;
					auto cg = a_continueGetting();
					co_await cg;
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
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					ahd_continueGetting = nullptr;
					hr();
				}
			}
			::nodecpp::awaitable<char> skipSpaces(char ch);
			::nodecpp::awaitable<char> readToken( char firstCh, nodecpp::string& str );
			::nodecpp::awaitable<char> readTokenWithExpectedSeparator( char firstCh, char separator, nodecpp::string& str );
			::nodecpp::awaitable<char> readHeaderValueTokens( char firstCh, nodecpp::string& str );
			::nodecpp::awaitable<void> readEOL( char firstCh );
			nodecpp::handler_ret_type parseMethod( IncomingHttpMessageAtServer& message );
			nodecpp::handler_ret_type parseHeaderEntry( IncomingHttpMessageAtServer& message );
#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES

		};

		template<class RequestT, class DataParentT>
		class HttpSocket : public HttpSocketBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			HttpSocket<RequestT, DataParentT>() {};
			HttpSocket<RequestT, DataParentT>(DataParentT* dataParent ) : HttpSocketBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
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
				header.insert( std::make_pair( makeLower( key ), line.substr( valStart, end - valStart + 1 ) ));
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
			nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> myRequest;
			typedef std::map<nodecpp::string, nodecpp::string> header_t;
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
			void writeHead( size_t statusCode, Str1 statusMessage, std::pair<nodecpp::string, nodecpp::string> headers[N] )
			{
				replyStatus = statusCode;
				setStatus( nodecpp::format( "{} {} {}", myRequest->getHttpVersion(), statusCode, statusMessage ) ); 
				for ( size_t i=0; i<N; ++i ) {
					header.insert( headers[i] ); 
				}
			}

			struct HeaderHolder
			{
				const char* key;
				enum ValType { undef, str, num };
				ValType valType;
				const char* valStr;
				size_t valNum;
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
					header.insert( std::make_pair( key, value ) );
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
					header.insert( std::make_pair( "Content-Length", format( "{}", b.size() ) ) );
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
		void HttpServerBase::onNewRequest( nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<HttpServerResponse> response )
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
		nodecpp::handler_ret_type HttpSocketBase::getRequest( IncomingHttpMessageAtServer& message )
		{
			nodecpp::string line;
			co_await readLine(line);
//			nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			if ( !message.parseMethod( line ) )
			{
				// TODO: report error
				end();
				CO_RETURN;
			}

			do
			{
				co_await readLine(line);
//				nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "line [{} bytes]: {}", lb.size() - 1, reinterpret_cast<char*>(lb.begin()) );
			}
			while ( message.parseHeaderEntry( line ) );

			CO_RETURN;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		inline
		::nodecpp::awaitable<char> HttpSocketBase::skipSpaces(char ch)
		{
			while ( ch == ' ' || ch == '\t' )
				ch = co_await a_readByte();
			CO_RETURN ch;
		}

		inline
		::nodecpp::awaitable<char> HttpSocketBase::readToken( char firstCh, nodecpp::string& str )
		{
			while ( !(firstCh == ' ' || firstCh == '\t' || firstCh == '\r' || firstCh == '\n') )
			{
				str += firstCh;
				firstCh = co_await a_readByte();
			}
			CO_RETURN firstCh;
		}

		inline
		::nodecpp::awaitable<char> HttpSocketBase::readHeaderValueTokens( char firstCh, nodecpp::string& str )
		{
			firstCh = co_await skipSpaces( firstCh );
			firstCh = co_await readToken( firstCh, str );
			while ( firstCh != '\r' && firstCh != '\n' )
			{
				str += ' ';
				firstCh = co_await skipSpaces( firstCh );
				firstCh = co_await readToken( firstCh, str );
			}
			CO_RETURN firstCh;
		}

		inline
		::nodecpp::awaitable<char> HttpSocketBase::readTokenWithExpectedSeparator( char firstCh, char separator, nodecpp::string& str )
		{
			while ( !(firstCh == separator || firstCh == ' ' || firstCh == '\t' || firstCh == '\r' || firstCh == '\n') )
			{
				str += firstCh;
				firstCh = co_await a_readByte();
			}
			if ( firstCh == separator )
				firstCh = co_await a_readByte();
			else
				throw Error();
			CO_RETURN firstCh;
		}

		inline
		::nodecpp::awaitable<void> HttpSocketBase::readEOL( char firstCh )
		{
			if ( firstCh == '\r' )
				firstCh = co_await a_readByte();
			if ( firstCh != '\n' )
				throw Error();
			CO_RETURN;
		}

		inline
		nodecpp::handler_ret_type HttpSocketBase::parseMethod( IncomingHttpMessageAtServer& message )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, message.readStatus == IncomingHttpMessageAtServer::ReadStatus::noinit ); 
			char ch = co_await a_readByte();

			ch = co_await readToken( ch, message.method.name );
			if ( ch == '\r' || ch == '\n' )
				throw Error();
			ch = co_await skipSpaces( ch );

			ch = co_await readToken( ch, message.method.url );
			if ( ch == '\r' || ch == '\n' )
				throw Error();
			ch = co_await skipSpaces( ch );

			ch = co_await readToken( ch, message.method.version );
			ch = co_await skipSpaces( ch );
			if ( memcmp( message.method.version.c_str(), "HTTP/", 5 ) == 0 )
				message.method.version = message.method.version.substr( 5 );

			co_await readEOL( ch );
			message.readStatus = IncomingHttpMessageAtServer::ReadStatus::in_hdr;
			CO_RETURN;
		}

		inline
		nodecpp::handler_ret_type HttpSocketBase::parseHeaderEntry( IncomingHttpMessageAtServer& message )
		{
			// for details see https://tools.ietf.org/html/rfc2616#page-17
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, message.readStatus == IncomingHttpMessageAtServer::ReadStatus::in_hdr ); 
			nodecpp::string key;
			nodecpp::string value;
			char ch = co_await a_readByte();
			if ( ch == '\r' || ch == '\n' )
			{
				co_await readEOL( ch );
				message.readStatus = IncomingHttpMessageAtServer::ReadStatus::in_body;
				CO_RETURN;
			}
			else if ( ch == ' ' || ch == '\t' )
			{
				// TODO: it's a continuation of a header - implement!
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "parsing header continuation is not implemented" );
			}

			ch = co_await readTokenWithExpectedSeparator( ch, ':', key );
			if ( ch == '\r' || ch == '\n' )
			{
				co_await readEOL( ch );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, key.size() );
				message.header.insert( std::make_pair( message.makeLower( key ), "" ));
				CO_RETURN;
			}
			ch = co_await skipSpaces( ch );

			ch = co_await readHeaderValueTokens( ch, value );
			message.header.insert( std::make_pair( message.makeLower( key ), message.makeLower( value ) ));

			co_await readEOL( ch );
		}

		inline
		nodecpp::handler_ret_type HttpSocketBase::getRequest2( IncomingHttpMessageAtServer& message )
		{
			try
			{
				co_await parseMethod( message );
				do
				{
					co_await parseHeaderEntry( message );
				}
				while ( message.readStatus == IncomingHttpMessageAtServer::ReadStatus::in_hdr );
			}
			catch (...)
			{
				// TODO: report/process error
				end();
			}
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::pedantic, message.readStatus == IncomingHttpMessageAtServer::ReadStatus::in_body, "indeed {}", message.readStatus );
			message.parseContentLength();
			message.readStatus = message.contentLength ? IncomingHttpMessageAtServer::ReadStatus::in_body : IncomingHttpMessageAtServer::ReadStatus::completed;
			message.parseConnStatus();

			CO_RETURN;
		}

		inline
		HttpSocketBase::HttpSocketBase() {
			rrQueue.init( myThis.getSoftPtr<HttpSocketBase>(this) );
			run(); // TODO: think about proper time for this call
		}

		template<size_t sizeExp>
		void HttpSocketBase::RRQueue<sizeExp>::init( nodecpp::safememory::soft_ptr<HttpSocketBase> socket ) {
			size_t size = ((size_t)1 << sizeExp);
			cbuff = nodecpp::alloc<RRPair>( size ); // TODO: use nodecpp::a
			//cbuff = new RRPair [size];
			for ( size_t i=0; i<size; ++i )
			{
				cbuff[i].request = nodecpp::safememory::make_owning<IncomingHttpMessageAtServer>();
				cbuff[i].response = nodecpp::safememory::make_owning<HttpServerResponse>();
				nodecpp::safememory::soft_ptr<IncomingHttpMessageAtServer> tmprq = (cbuff[i].request);
				nodecpp::safememory::soft_ptr<HttpServerResponse> tmrsp = (cbuff[i].response);
				cbuff[i].response->counterpart = nodecpp::safememory::soft_ptr_reinterpret_cast<HttpMessageBase>(tmprq);
				cbuff[i].request->counterpart = nodecpp::safememory::soft_ptr_reinterpret_cast<HttpMessageBase>(tmrsp);
				cbuff[i].request->sock = socket;
				cbuff[i].response->sock = socket;
				cbuff[i].response->myRequest = cbuff[i].request;
			}
		}	

	} //namespace net
} //namespace nodecpp

#endif // HTTP_SOCKET_AT_SERVER_H
