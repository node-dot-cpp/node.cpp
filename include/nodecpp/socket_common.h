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

class OpaqueSocketData;

namespace nodecpp {

	namespace net {

		class [[nodecpp::owning_only]] SocketBase
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
				enum State { Uninitialized, Connecting, Connected, LocalEnding, LocalEnded, Closing, ErrorClosing, Closed }
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


				struct UserHandlersCommon
				{
				public:
					using userDefAcceptedHandlerFnT = nodecpp::awaitable<void> (*)(void*);
					using userDefConnectHandlerFnT = nodecpp::awaitable<void> (*)(void*);
					using userDefDataHandlerFnT = nodecpp::awaitable<void> (*)(void*, Buffer& buffer);
					using userDefDrainHandlerFnT = nodecpp::awaitable<void> (*)(void*);
					using userDefEndHandlerFnT = nodecpp::awaitable<void> (*)(void*);
					using userDefCloseHandlerFnT = nodecpp::awaitable<void> (*)(void*, bool);
					using userDefErrorHandlerFnT = nodecpp::awaitable<void> (*)(void*, Error&);

					template<class T> using userAcceptedMemberHandler = nodecpp::awaitable<void> (T::*)();
					template<class T> using userConnectMemberHandler = nodecpp::awaitable<void> (T::*)();
					template<class T> using userDataMemberHandler = nodecpp::awaitable<void> (T::*)(Buffer&);
					template<class T> using userDrainMemberHandler = nodecpp::awaitable<void> (T::*)();
					template<class T> using userEndMemberHandler = nodecpp::awaitable<void> (T::*)();
					template<class T> using userCloseMemberHandler = nodecpp::awaitable<void> (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::awaitable<void> (T::*)(Error&);

					template<class ObjectT, userAcceptedMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> acceptedHandler(void* objPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						co_return;
					}

					template<class ObjectT, userConnectMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> connectHandler(void* objPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						co_return;
					}

					template<class ObjectT, userDataMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> dataHandler(void* objPtr, Buffer& buffer)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(buffer);
						co_return;
					}

					template<class ObjectT, userDrainMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> drainHandler(void* objPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						co_return;
					}

					template<class ObjectT, userEndMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> endHandler(void* objPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						co_return;
					}

					template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> closeHandler(void* objPtr, bool hadError)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
						co_return;
					}

					template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> errorHandler(void* objPtr, Error& e)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
						co_return;
					}

					enum class Handler { Accepted, Connect, Data, Drain, End, Close, Error };
				};


				struct UserHandlersForDataCollecting : public UserHandlersCommon
				{
					UserDefHandlers<userDefAcceptedHandlerFnT> userDefAcceptedHandlers;
					UserDefHandlers<userDefConnectHandlerFnT> userDefConnectHandlers;
					UserDefHandlers<userDefDataHandlerFnT> userDefDataHandlers;
					UserDefHandlers<userDefDrainHandlerFnT> userDefDrainHandlers;
					UserDefHandlers<userDefEndHandlerFnT> userDefEndHandlers;
					UserDefHandlers<userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlers<userDefErrorHandlerFnT> userDefErrorHandlers;
					template<Handler handler, auto memmberFn, class ObjectT>
					void addHandler(ObjectT* object)
					{
						if constexpr (handler == Handler::Accepted)
						{
							userDefAcceptedHandlers.add(object, &DataForCommandProcessing::UserHandlers::acceptedHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connect)
						{
							userDefConnectHandlers.add(object, &DataForCommandProcessing::UserHandlers::connectHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Data)
						{
							userDefDataHandlers.add(object, &DataForCommandProcessing::UserHandlers::dataHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Drain)
						{
							userDefDrainHandlers.add(object, &DataForCommandProcessing::UserHandlers::drainHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::End)
						{
							userDefEndHandlers.add(object, &DataForCommandProcessing::UserHandlers::endHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(object, &DataForCommandProcessing::UserHandlers::closeHandler<ObjectT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(object, &DataForCommandProcessing::UserHandlers::errorHandler<ObjectT, memmberFn>);
						}
					}
					template<Handler handler, auto memmberFn, class ObjectT>
					void removeHandler(ObjectT* object)
					{
						if constexpr (handler == Handler::Accepted)
						{
							userDefAcceptedHandlers.remove(object, &DataForCommandProcessing::UserHandlers::acceptedHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connect)
						{
							userDefConnectHandlers.remove(object, &DataForCommandProcessing::UserHandlers::connectHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Data)
						{
							userDefDataHandlers.remove(object, &DataForCommandProcessing::UserHandlers::dataHandler<ObjectT, memmberFn>);
						}
						if constexpr (handler == Handler::Drain)
						{
							userDefDrainHandlers.remove(object, &DataForCommandProcessing::UserHandlers::drainHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::End)
						{
							userDefEndHandlers.remove(object, &DataForCommandProcessing::UserHandlers::endHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.remove(object, &DataForCommandProcessing::UserHandlers::closeHandler<ObjectT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.remove(object, &DataForCommandProcessing::UserHandlers::errorHandler<ObjectT, memmberFn>);
						}
					}
				};
				thread_local static UserHandlerClassPatterns<UserHandlersForDataCollecting> userHandlerClassPattern; // TODO: consider using thread-local allocator

				struct UserHandlers : public UserHandlersCommon
				{
					bool initialized = false;

					UserDefHandlersWithOptimizedStorage<userDefAcceptedHandlerFnT> userDefAcceptedHandlers;
					UserDefHandlersWithOptimizedStorage<userDefConnectHandlerFnT> userDefConnectHandlers;
					UserDefHandlersWithOptimizedStorage<userDefDataHandlerFnT> userDefDataHandlers;
					UserDefHandlersWithOptimizedStorage<userDefDrainHandlerFnT> userDefDrainHandlers;
					UserDefHandlersWithOptimizedStorage<userDefEndHandlerFnT> userDefEndHandlers;
					UserDefHandlersWithOptimizedStorage<userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlersWithOptimizedStorage<userDefErrorHandlerFnT> userDefErrorHandlers;


					void from(const UserHandlersForDataCollecting& patternUH, void* defaultObjPtr)
					{
						if ( initialized )
							return;
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjPtr != nullptr);
						userDefAcceptedHandlers.from(patternUH.userDefAcceptedHandlers, defaultObjPtr);
						userDefConnectHandlers.from(patternUH.userDefConnectHandlers, defaultObjPtr);
						userDefDataHandlers.from(patternUH.userDefDataHandlers, defaultObjPtr);
						userDefDrainHandlers.from(patternUH.userDefDrainHandlers, defaultObjPtr);
						userDefEndHandlers.from(patternUH.userDefEndHandlers, defaultObjPtr);
						userDefCloseHandlers.from(patternUH.userDefCloseHandlers, defaultObjPtr);
						userDefErrorHandlers.from(patternUH.userDefErrorHandlers, defaultObjPtr);
						initialized = true;
					}
				};
				UserHandlers userHandlers;

				bool isAcceptedEventHandler() { return userHandlers.userDefAcceptedHandlers.willHandle(); }
				void handleAcceptedEvent() { userHandlers.userDefAcceptedHandlers.execute(); }

				bool isConnectEventHandler() { return userHandlers.userDefConnectHandlers.willHandle(); }
				void handleConnectEvent() { userHandlers.userDefConnectHandlers.execute(); }

				bool isDataEventHandler() { return userHandlers.userDefDataHandlers.willHandle(); }
				void handleDataEvent(Buffer& buffer) { userHandlers.userDefDataHandlers.execute(buffer); }

				bool isDrainEventHandler() { return userHandlers.userDefDrainHandlers.willHandle(); }
				void handleDrainEvent() { userHandlers.userDefDrainHandlers.execute(); }

				bool isEndEventHandler() { return userHandlers.userDefEndHandlers.willHandle(); }
				void handleEndEvent() { userHandlers.userDefEndHandlers.execute(); }

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(bool hasError) { userHandlers.userDefCloseHandlers.execute(hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(Error& e) { userHandlers.userDefErrorHandlers.execute(e); }
			};
			DataForCommandProcessing dataForCommandProcessing;

			template<class UserClass, DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn, class ObjectT>			
			static void addHandler(ObjectT* object)
			{
				DataForCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().addHandler<handler, memmberFn, ObjectT>(object);
			}
			template<class UserClass, DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn>			
			static void addHandler()
			{
				DataForCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().addHandler<handler, memmberFn, UserClass>(nullptr);
			}

		private:
			void registerMeAndAcquireSocket();
			void registerMeAndAssignSocket(OpaqueSocketData& sdata);

		public:
			Address _local;
			Address _remote;
			//std::string _remoteAddress;
			//std::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			enum State { UNINITIALIZED = 0, CONNECTING, CONNECTED, DESTROYED } state = UNINITIALIZED;

			SocketBase(NodeBase* node_) {node = node_; registerMeAndAcquireSocket();}
			SocketBase(NodeBase* node_, OpaqueSocketData& sdata);

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

			SocketBase& setNoDelay(bool noDelay = true);
			SocketBase& setKeepAlive(bool enable = false);


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

			///////////////////////////////////////////////////////////////////
		private:
			class EventEmitterSupportingListeners<event::Close, SocketListener, &SocketListener::onClose> eClose;
			class EventEmitterSupportingListeners<event::Connect, SocketListener, &SocketListener::onConnect> eConnect;
			class EventEmitterSupportingListeners<event::Data, SocketListener, &SocketListener::onData> eData;
			class EventEmitterSupportingListeners<event::Drain, SocketListener, &SocketListener::onDrain> eDrain;
			class EventEmitterSupportingListeners<event::End, SocketListener, &SocketListener::onEnd> eEnd;
			class EventEmitterSupportingListeners<event::Error, SocketListener, &SocketListener::onError> eError;
			class EventEmitterSupportingListeners<event::Accepted, SocketListener, &SocketListener::onAccepted> eAccepted;

			std::vector<nodecpp::safememory::owning_ptr<SocketListener>> ownedListeners;

		public:
			void emitClose(bool hadError) {
				state = DESTROYED;
				unref();
				//this->dataForCommandProcessing.id = 0;
				//handler may release, put virtual onClose first.
				eClose.emit(hadError);
			}

			// not in node.js
			void emitAccepted() {
				state = CONNECTED;
				eAccepted.emit();
			}

			void emitConnect() {
				state = CONNECTED;
				eConnect.emit();
			}

			void emitData(Buffer& buffer) {
				_bytesRead += buffer.size();
				eData.emit(std::ref(buffer));
			}

			void emitDrain() {
				eDrain.emit();
			}

			void emitEnd() {
				eEnd.emit();
			}

			void emitError(Error& err) {
				state = DESTROYED;
				//this->dataForCommandProcessing.id = 0;
				eError.emit(err);
			}

			void connect(uint16_t port, const char* ip, std::function<void()> cb) {
				once(event::connect, std::move(cb));
				connect(port, ip);
			}

			bool write(const uint8_t* data, uint32_t size, std::function<void()> cb) {
				bool b = write(data, size);
				if(!b)
					once(event::drain, std::move(cb));

				return b;
			}



			void on_( nodecpp::safememory::soft_ptr<SocketListener> l) {
				eClose.on(l);
				eConnect.on(l);
				eData.on(l);
				eDrain.on(l);
				eError.on(l);
				eEnd.on(l);
				eAccepted.on(l);
			}

			void once_( nodecpp::safememory::soft_ptr<SocketListener> l) {
				eClose.once(l);
				eConnect.once(l);
				eData.once(l);
				eDrain.once(l);
				eError.once(l);
				eEnd.once(l);
				eAccepted.once(l);
			}

			void on( nodecpp::safememory::owning_ptr<SocketListener> l) {
				nodecpp::safememory::soft_ptr<SocketListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				on_( std::move(sl) );
			}

			void once( nodecpp::safememory::owning_ptr<SocketListener> l) {
				nodecpp::safememory::soft_ptr<SocketListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				once_( std::move(sl) );
			}

			void on( std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.on(std::move(cb));
			}
			void on( std::string name, event::Data::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.on(std::move(cb));
			}
			void on(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.on(std::move(cb));
			}
			void on( std::string name, event::Connect::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if (name == event::Drain::name)
					eDrain.on(std::move(cb));
				else if (name == event::Connect::name)
					eConnect.on(std::move(cb));
				else if (name == event::End::name)
					eEnd.on(std::move(cb));
				else if (name == event::Accepted::name)
					eAccepted.on(std::move(cb));
				else
					assert(false);
			}

			void once( std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.once(std::move(cb));
			}
			void once( std::string name, event::Data::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.once(std::move(cb));
			}
			void once(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.once(std::move(cb));
			}
			void once( std::string name, event::Connect::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if (name == event::Drain::name)
					eDrain.once(std::move(cb));
				else if (name == event::Connect::name)
					eConnect.once(std::move(cb));
				else if (name == event::End::name)
					eEnd.once(std::move(cb));
				else if (name == event::Accepted::name)
					eAccepted.once(std::move(cb));
				else
					assert(false);
			}

			template<class EV>
			void on( EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.on(std::move(cb)); }
				else assert(false);
			}

			template<class EV>
			void once( EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.once(std::move(cb)); }
				else assert(false);
			}

		};

		template<class T, class ... Types>
		static
			nodecpp::safememory::owning_ptr<T> createSocket(Types&& ... args) {
			static_assert( std::is_base_of< SocketBase, T >::value );
			nodecpp::safememory::owning_ptr<T> ret = nodecpp::safememory::make_owning<T>(::std::forward<Types>(args)...);
			ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<T>(), &(*ret));
			return ret;
		}

	} //namespace net

} //namespace nodecpp

#endif //SOCKET_COMMON_H
