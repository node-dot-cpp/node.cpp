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

#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H

#include "net_common.h"
#include "event.h"

namespace nodecpp {

	namespace net {

		class SocketBase
		{
		public:
			nodecpp::safememory::soft_this_ptr<SocketBase> myThis;
		public:
			SocketBase* prev_;
			SocketBase* next_;

		public:
//			UserDefID userDefID;
			NodeBase* node = nullptr;

			public:
			class DataForCommandProcessing {
			public:
				enum State { Uninitialized, Connecting, Connected, LocalEnding, LocalEnded, Closing, ErrorClosing, Closed}
				state = Uninitialized;
				size_t index = 0;

				struct awaitable_read_handle_data : public awaitable_handle_data
				{
					size_t min_bytes;
				};
				struct awaitable_write_handle_data : public awaitable_handle_data
				{
					Buffer b;
				};

				// NOTE: make sure all of them are addressed at forceResumeWithThrowing()
				awaitable_handle_data ahd_connect;
				awaitable_handle_data ahd_accepted;
				awaitable_read_handle_data ahd_read;
				awaitable_write_handle_data ahd_write;
				awaitable_handle_data ahd_drain;

			//	bool connecting = false;
				bool remoteEnded = false;
				//bool localEnded = false;
				//bool pendingLocalEnd = false;
				bool paused = false;
				bool allowHalfOpen = true;

				bool refed = false;

				//Buffer writeBuffer = Buffer(64 * 1024);
				CircularByteBuffer writeBuffer = CircularByteBuffer( 16 );
				CircularByteBuffer readBuffer = CircularByteBuffer( 16 );
				Buffer recvBuffer = Buffer(64 * 1024);

				//SOCKET osSocket = INVALID_SOCKET;
				//UINT_PTR osSocket = INVALID_SOCKET;
				unsigned long long osSocket = 0;


				DataForCommandProcessing() {}
				DataForCommandProcessing(const DataForCommandProcessing& other) = delete;
				DataForCommandProcessing& operator=(const DataForCommandProcessing& other) = delete;

				DataForCommandProcessing(DataForCommandProcessing&& other) = default;
				DataForCommandProcessing& operator=(DataForCommandProcessing&& other) = default;

				bool isValid() const { return state != State::Uninitialized; }


				using userDefAcceptedHandlerFnT = nodecpp::awaitable<void> (*)(void*);
				using userDefConnectHandlerFnT = nodecpp::awaitable<void> (*)(void*);
				using userDefDataHandlerFnT = nodecpp::awaitable<void> (*)(void*, Buffer& buffer);
				using userDefDrainHandlerFnT = nodecpp::awaitable<void> (*)(void*);
				using userDefEndHandlerFnT = nodecpp::awaitable<void> (*)(void*);
				using userDefCloseHandlerFnT = nodecpp::awaitable<void> (*)(void*, bool);
				using userDefErrorHandlerFnT = nodecpp::awaitable<void> (*)(void*, Error&);

				UserDefHandlers<userDefAcceptedHandlerFnT> userDefAcceptedHandlers;
				UserDefHandlers<userDefConnectHandlerFnT> userDefConnectHandlers;
				UserDefHandlers<userDefDataHandlerFnT> userDefDataHandlers;
				UserDefHandlers<userDefDrainHandlerFnT> userDefDrainHandlers;
				UserDefHandlers<userDefEndHandlerFnT> userDefEndHandlers;
				UserDefHandlers<userDefCloseHandlerFnT> userDefCloseHandlers;
				UserDefHandlers<userDefErrorHandlerFnT> userDefErrorHandlers;

				bool isAcceptedEventHandler() { return userDefAcceptedHandlers.willHandle(); }
				void handleAcceptedEvent() { for (auto h : userDefAcceptedHandlers.handlers) h.handler(h.object); }

				bool isConnectEventHandler() { return userDefConnectHandlers.willHandle(); }
				void handleConnectEvent() { for (auto h : userDefConnectHandlers.handlers) h.handler(h.object); }

				bool isDataEventHandler() { return userDefDataHandlers.willHandle(); }
				void handleDataEvent(Buffer& buffer) { for (auto h : userDefDataHandlers.handlers) h.handler(h.object, buffer); }

				bool isDrainEventHandler() { return userDefDrainHandlers.willHandle(); }
				void handleDrainEvent() { for (auto h : userDefDrainHandlers.handlers) h.handler(h.object); }

				bool isEndEventHandler() { return userDefEndHandlers.willHandle(); }
				void handleEndEvent() { for (auto h : userDefEndHandlers.handlers) h.handler(h.object); }

				bool isCloseEventHandler() { return userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(bool hasError) { for (auto h : userDefCloseHandlers.handlers) h.handler(h.object, hasError); }

				bool isErrorEventHandler() { return userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(Error& e) { for (auto h : userDefErrorHandlers.handlers) h.handler(h.object, e); }

				template<class T> using userAcceptedMemberHandler = nodecpp::awaitable<void> (T::*)();
				template<class T> using userConnectMemberHandler = nodecpp::awaitable<void> (T::*)();
				template<class T> using userDataMemberHandler = nodecpp::awaitable<void> (T::*)(Buffer&);
				template<class T> using userDrainMemberHandler = nodecpp::awaitable<void> (T::*)();
				template<class T> using userEndMemberHandler = nodecpp::awaitable<void> (T::*)();
				template<class T> using userCloseMemberHandler = nodecpp::awaitable<void> (T::*)(bool);
				template<class T> using userErrorMemberHandler = nodecpp::awaitable<void> (T::*)(Error&);

				template<class ObjectT, userAcceptedMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> acceptedHandler( void* objPtr )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
					co_return;
				}

				template<class ObjectT, userConnectMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> connectHandler( void* objPtr )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
					co_return;
				}

				template<class ObjectT, userDataMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> dataHandler( void* objPtr, Buffer& buffer )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(buffer);
					co_return;
				}

				template<class ObjectT, userDrainMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> drainHandler( void* objPtr )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
					co_return;
				}

				template<class ObjectT, userEndMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> endHandler( void* objPtr )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
					co_return;
				}

				template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> closeHandler( void* objPtr, bool hadError )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
					co_return;
				}

				template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
				static nodecpp::awaitable<void> errorHandler( void* objPtr, Error& e )
				{
					((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
					co_return;
				}
			};
		//protected:
			DataForCommandProcessing dataForCommandProcessing;

		public:
			Address _local;
			Address _remote;
			//std::string _remoteAddress;
			//std::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			enum State { UNINITIALIZED = 0, CONNECTING, CONNECTED, DESTROYED } state = UNINITIALIZED;

			SocketBase(NodeBase* node_) {node = node_;}

			SocketBase(const SocketBase&) = delete;
			SocketBase& operator=(const SocketBase&) = delete;

			SocketBase(SocketBase&&) = default;
			SocketBase& operator=(SocketBase&&) = default;

			virtual ~SocketBase() {
				if (state == CONNECTING || state == CONNECTED) destroy();
				reportBeingDestructed(); 
				unref(); /*or assert that is must already be unrefed*/
			}

		public:

			const Address& address() const { return _local; }

			size_t bufferSize() const { return dataForCommandProcessing.writeBuffer.used_size(); }
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			bool connecting() const { return state == CONNECTING; }
			void destroy();
			bool destroyed() const { return state == DESTROYED; };
			void end();
			const std::string& localAddress() const { return _local.address; }
			uint16_t localPort() const { return _local.port; }


			const std::string& remoteAddress() const { return _remote.address; }
			const std::string& remoteFamily() const { return _remote.family; }
			uint16_t remotePort() const { return _remote.port; }

			void ref();
			void unref();
			void pause();
			void resume();
			void reportBeingDestructed();

			bool write(const uint8_t* data, uint32_t size);
			bool write(Buffer& buff) { return write( buff.begin(), (uint32_t)(buff.size()) ); }

			bool write2(Buffer& b);
			void connect(uint16_t port, const char* ip);


		//private:
			auto a_connect(uint16_t port, const char* ip) { 

				struct connect_awaiter {
					SocketBase& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					connect_awaiter(SocketBase& socket_) : socket( socket_ ) {}

					connect_awaiter(const connect_awaiter &) = delete;
					connect_awaiter &operator = (const connect_awaiter &) = delete;
	
					~connect_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						who_is_awaiting = awaiting;
						socket.dataForCommandProcessing.ahd_connect.h = who_is_awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_connect.is_exception )
						{
							socket.dataForCommandProcessing.ahd_connect.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_connect.exception;
						}
					}
				};
				connect( port, ip );
				return connect_awaiter(*this);
			}

			auto a_read( Buffer& buff, size_t min_bytes = 1 ) { 

				buff.clear();
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.capacity() >= min_bytes, "indeed: {} vs. {} bytes", buff.capacity(), min_bytes );

				struct read_data_awaiter {
					SocketBase& socket;
					Buffer& buff;
					size_t min_bytes;

					read_data_awaiter(SocketBase& socket_, Buffer& buff_, size_t min_bytes_) : socket( socket_ ), buff( buff_ ), min_bytes( min_bytes_ ) {}

					read_data_awaiter(const read_data_awaiter &) = delete;
					read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
					~read_data_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = min_bytes;
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_read.is_exception )
						{
							socket.dataForCommandProcessing.ahd_read.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_read.exception;
						}
						socket.dataForCommandProcessing.readBuffer.get_ready_data( buff );
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes);
					}
				};
				return read_data_awaiter(*this, buff, min_bytes);
			}

			auto a_write(Buffer& buff) { 

				struct write_data_awaiter {
					SocketBase& socket;
					Buffer& buff;
					bool write_ok = false;

					std::experimental::coroutine_handle<> who_is_awaiting;

					write_data_awaiter(SocketBase& socket_, Buffer& buff_) : socket( socket_ ), buff( buff_ )  {}

					write_data_awaiter(const write_data_awaiter &) = delete;
					write_data_awaiter &operator = (const write_data_awaiter &) = delete;
	
					~write_data_awaiter() {}

					bool await_ready() {
						write_ok = socket.write2( buff ); // so far we do it sync TODO: extend implementation for more complex (= requiring really async processing) cases
						return write_ok; // false means waiting (incl. exceptional cases)
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !write_ok ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_write.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_write.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_write.exception;
						}
					}
				};
				return write_data_awaiter(*this, buff);
			}

			auto a_drain() { 

				struct drain_awaiter {
					SocketBase& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					drain_awaiter(SocketBase& socket_) : socket( socket_ )  {}

					drain_awaiter(const drain_awaiter &) = delete;
					drain_awaiter &operator = (const drain_awaiter &) = delete;
	
					~drain_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.writeBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !socket.dataForCommandProcessing.writeBuffer.empty() ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_drain.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_drain.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_drain.exception;
						}
					}
				};
				return drain_awaiter(*this);
			}

			enum class Handler { Accepted, Connect, Data, Drain, End, Close, Error };
			template<Handler handler, auto memmberFn, class ObjectT>
			void addHandler(ObjectT* object )
			{
				if constexpr ( handler == Handler::Accepted )
				{
					dataForCommandProcessing.userDefAcceptedHandlers.add(object, &DataForCommandProcessing::acceptedHandler<ObjectT, memmberFn>);
				} 
				else if constexpr ( handler == Handler::Connect )
				{
					dataForCommandProcessing.userDefConnectHandlers.add(object, &DataForCommandProcessing::connectHandler<ObjectT, memmberFn>);
				}
				else if constexpr ( handler == Handler::Data )
				{
					dataForCommandProcessing.userDefDataHandlers.add(object, &DataForCommandProcessing::dataHandler<ObjectT, memmberFn>);
				}
				else if constexpr ( handler == Handler::Drain )
				{
					dataForCommandProcessing.userDefDrainHandlers.add(object, &DataForCommandProcessing::drainHandler<ObjectT, memmberFn>);
				} 
				else if constexpr ( handler == Handler::End )
				{
					dataForCommandProcessing.userDefEndHandlers.add(object, &DataForCommandProcessing::endHandler<ObjectT, memmberFn>);
				}
				else if constexpr ( handler == Handler::Close )
				{
					dataForCommandProcessing.userDefCloseHandlers.add(object, &DataForCommandProcessing::closeHandler<ObjectT, memmberFn>);
				}
				else
				{
					static_assert( handler == Handler::Error ); // the only remaining option
					dataForCommandProcessing.userDefErrorHandlers.add(object, &DataForCommandProcessing::errorHandler<ObjectT, memmberFn>);
				}
			}
			template<Handler handler, auto memmberFn, class ObjectT>
			void removeHandler(ObjectT* object)
			{
				if constexpr (handler == Handler::Accepted)
				{
					dataForCommandProcessing.userDefAcceptedHandlers.remove(object, &DataForCommandProcessing::acceptedHandler<ObjectT, memmberFn>);
				}
				else if constexpr (handler == Handler::Connect)
				{
					dataForCommandProcessing.userDefConnectHandlers.remove(object, &DataForCommandProcessing::connectHandler<ObjectT, memmberFn>);
				}
				else if constexpr (handler == Handler::Data)
				{
					dataForCommandProcessing.userDefDataHandlers.remove(object, &DataForCommandProcessing::dataHandler<ObjectT, memmberFn>);
				}
				if constexpr (handler == Handler::Drain)
				{
					dataForCommandProcessing.userDefDrainHandlers.remove(object, &DataForCommandProcessing::drainHandler<ObjectT, memmberFn>);
				}
				else if constexpr (handler == Handler::End)
				{
					dataForCommandProcessing.userDefEndHandlers.remove(object, &DataForCommandProcessing::endHandler<ObjectT, memmberFn>);
				}
				else if constexpr (handler == Handler::Close)
				{
					dataForCommandProcessing.userDefCloseHandlers.remove(object, &DataForCommandProcessing::closeHandler<ObjectT, memmberFn>);
				}
				else
				{
					static_assert(handler == Handler::Error); // the only remaining option
					dataForCommandProcessing.userDefErrorHandlers.remove(object, &DataForCommandProcessing::errorHandler<ObjectT, memmberFn>);
				}
			}
		};

	} //namespace net

} //namespace nodecpp

#endif //SOCKET_COMMON_H
