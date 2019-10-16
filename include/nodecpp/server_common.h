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

#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include "socket_common.h"

namespace nodecpp {

	namespace net {

		class ServerBase
		{
		public:
			using NodeType = void;
			using DataParentType = void;

		public:
			nodecpp::safememory::soft_this_ptr<ServerBase> myThis;
		public:
			class DataForCommandProcessing {
			public:
				size_t index;
				bool refed = false;
				short fdEvents = 0;
				//SOCKET osSocket = INVALID_SOCKET;
				unsigned long long osSocket = 0;

				enum State { Unused, Listening, BeingClosed, Closed }; // TODO: revise!
				State state = State::Unused;


				DataForCommandProcessing() {}
				DataForCommandProcessing(const DataForCommandProcessing& other) = delete;
				DataForCommandProcessing& operator=(const DataForCommandProcessing& other) = delete;

				DataForCommandProcessing(DataForCommandProcessing&& other) = default;
				DataForCommandProcessing& operator=(DataForCommandProcessing&& other) = default;

				Address localAddress;

				awaitable_handle_t ahd_listen = nullptr;
				struct awaitable_connection_handle_data
				{
					awaitable_handle_t h = nullptr;
					soft_ptr<SocketBase> sock;
				};
				awaitable_connection_handle_data ahd_connection;
				awaitable_handle_t ahd_close = nullptr;

				struct UserHandlersCommon
				{
				public:
					// originating from member functions of ServertBase-derived classes
					template<class T> using userListenMemberHandler = nodecpp::handler_ret_type (T::*)(size_t, nodecpp::net::Address);
					template<class T> using userConnectionMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<net::SocketBase>);
					template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

					// originating from member functions of NodeBase-derived classes
					template<class T, class ServerT> using userListenNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, size_t, nodecpp::net::Address);
					template<class T, class ServerT> using userConnectionNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, nodecpp::safememory::soft_ptr<net::SocketBase>);
					template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, bool);
					template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::safememory::soft_ptr<ServerT>, Error&);

					using userDefListenHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, size_t, nodecpp::net::Address);
					using userDefConnectionHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, nodecpp::safememory::soft_ptr<net::SocketBase>);
					using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, bool);
					using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::safememory::soft_ptr<ServerBase>, Error&);

					// originating from member functions of ServerBase-derived classes

					template<class ObjectT, userListenMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type listenHandler( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(id, addr);
						CO_RETURN;
					}

					template<class ObjectT, userConnectionMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type connectionHandler( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, nodecpp::safememory::soft_ptr<net::SocketBase> socket )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(socket);
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

					template<class ObjectT, class ServerT, userListenNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type listenHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), id, addr);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userConnectionNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type connectionHandlerFromNode( void* objPtr, nodecpp::safememory::soft_ptr<ServerBase> serverPtr, nodecpp::safememory::soft_ptr<net::SocketBase> socket )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::safememory::soft_ptr_reinterpret_cast<ServerT>(serverPtr), socket);
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

					enum class Handler { Listen, Connection, Close, Error };
				};

				struct UserHandlersForDataCollecting : public UserHandlersCommon
				{
					UserDefHandlers<userDefListenHandlerFnT> userDefListenHandlers;
					UserDefHandlers<userDefConnectionHandlerFnT> userDefConnectionHandlers;
					UserDefHandlers<userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlers<userDefErrorHandlerFnT> userDefErrorHandlers;

					template<Handler handler, auto memmberFn, class ObjectT>
					void addHandler()
					{
						if constexpr (handler == Handler::Listen)
						{
							userDefListenHandlers.add( &DataForCommandProcessing::UserHandlers::listenHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connection)
						{
							userDefConnectionHandlers.add(&DataForCommandProcessing::UserHandlers::connectionHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(&DataForCommandProcessing::UserHandlers::closeHandler<ObjectT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(&DataForCommandProcessing::UserHandlers::errorHandler<ObjectT, memmberFn>);
						}
					}

					template<Handler handler, auto memmberFn, class ObjectT, class ServerT>
					void addHandlerFromNode(ObjectT* object)
					{
						if constexpr (handler == Handler::Listen)
						{
							userDefListenHandlers.add(object, &DataForCommandProcessing::UserHandlers::listenHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connection)
						{
							userDefConnectionHandlers.add(object, &DataForCommandProcessing::UserHandlers::connectionHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(object, &DataForCommandProcessing::UserHandlers::closeHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(object, &DataForCommandProcessing::UserHandlers::errorHandlerFromNode<ObjectT, ServerT, memmberFn>);
						}
					}
				};
				thread_local static UserHandlerClassPatterns<UserHandlersForDataCollecting> userHandlerClassPattern; // TODO: consider using thread-local allocator

				struct UserHandlers : public UserHandlersCommon
				{
					bool initialized = false;
				public:
					UserDefHandlersWithOptimizedStorage<userDefListenHandlerFnT> userDefListenHandlers;
					UserDefHandlersWithOptimizedStorage<userDefConnectionHandlerFnT> userDefConnectionHandlers;
					UserDefHandlersWithOptimizedStorage<userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlersWithOptimizedStorage<userDefErrorHandlerFnT> userDefErrorHandlers;

					void from(const UserHandlersForDataCollecting& patternUH, void* defaultObjPtr)
					{
						if ( initialized )
							return;
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjPtr != nullptr);
						userDefListenHandlers.from(patternUH.userDefListenHandlers, defaultObjPtr);
						userDefConnectionHandlers.from(patternUH.userDefConnectionHandlers, defaultObjPtr);
						userDefCloseHandlers.from(patternUH.userDefCloseHandlers, defaultObjPtr);
						userDefErrorHandlers.from(patternUH.userDefErrorHandlers, defaultObjPtr);
						initialized = true;
					}
				};
				UserHandlers userHandlers;

				bool isListenEventHandler() { return userHandlers.userDefListenHandlers.willHandle(); }
				void handleListenEvent(nodecpp::safememory::soft_ptr<ServerBase> server, size_t id, nodecpp::net::Address address) { userHandlers.userDefListenHandlers.execute(server, id, address); }

				bool isConnectionEventHandler() { return userHandlers.userDefConnectionHandlers.willHandle(); }
				void handleConnectionEvent(nodecpp::safememory::soft_ptr<ServerBase> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { userHandlers.userDefConnectionHandlers.execute(server, socket); }

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(nodecpp::safememory::soft_ptr<ServerBase> server, bool hasError) { userHandlers.userDefCloseHandlers.execute(server, hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(nodecpp::safememory::soft_ptr<ServerBase> server, Error& e) { userHandlers.userDefErrorHandlers.execute(server, e); }
			};
			DataForCommandProcessing dataForCommandProcessing;

			template<class UserClass, DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn, class ObjectT>
			static void addHandler(ObjectT* object)
			{
				DataForCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().template addHandlerFromNode<handler, memmberFn, ObjectT, UserClass>(object);
			}
			template<class UserClass, DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn>
			static void addHandler()
			{
				DataForCommandProcessing::userHandlerClassPattern.getPatternForUpdate<UserClass>().template addHandler<handler, memmberFn, UserClass>();
			}

		protected:
//			uint16_t localPort = 0;

//			size_t id = 0;
//			enum State { UNINITIALIZED = 0, LISTENING, CLOSED } state = UNINITIALIZED;

		//protected:
		public:
			void registerServerByID(/*NodeBase* node, */soft_ptr<net::ServerBase> t, int typeId);
			void registerServer(/*NodeBase* node, */soft_ptr<net::ServerBase> t) {registerServerByID(/*node, */t, -1);}
			template<class Node, class DerivedServer>
			void registerServer(soft_ptr<DerivedServer> t) {
				int id = -1;
				if constexpr ( !std::is_same< typename Node::EmitterTypeForServer, void>::value )
					id = Node::EmitterTypeForServer::template getTypeIndex<DerivedServer>( &(*t));
				registerServerByID(/*nullptr, */t, id);
			}

		public:
			NodeBase* node = nullptr;

			using acceptedSocketCreationRoutineType = std::function<owning_ptr<SocketBase>(OpaqueSocketData&)>;
		private:
			acceptedSocketCreationRoutineType acceptedSocketCreationRoutine = nullptr;

		public:
			ServerBase();
			//ServerBase(int typeID);
//			ServerBase(acceptedSocketCreationRoutineType socketCreationCB);
			//ServerBase(int typeID, acceptedSocketCreationRoutineType socketCreationCB);
			virtual ~ServerBase() {
				socketList.clear();
				reportBeingDestructed();
			}
			void setAcceptedSocketCreationRoutine(acceptedSocketCreationRoutineType socketCreationCB) { acceptedSocketCreationRoutine = std::move( socketCreationCB ); }
			void internalCleanupBeforeClosing()
			{
				NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getSockCount() == 0 ); 
				dataForCommandProcessing.state = DataForCommandProcessing::State::Closed;
				dataForCommandProcessing.index = 0;
				forceReleasingAllCoroHandles();
			}

			const Address& address() const { return dataForCommandProcessing.localAddress; }
			void close();

			bool listening() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Listening; }
			void ref();
			void unref();
			void reportBeingDestructed();

			void listen(uint16_t port, const char* ip, int backlog);

#ifndef NODECPP_NO_COROUTINES
			void forceReleasingAllCoroHandles()
			{
				if ( dataForCommandProcessing.ahd_listen != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_listen;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_listen = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_connection.h != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_connection.h;
					nodecpp::setException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_connection.h = nullptr;
					hr();
				}
			}


			auto a_listen(uint16_t port, const char* ip, int backlog) { 

				struct listen_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;

					listen_awaiter(ServerBase& server_) : server( server_ ) {}

					listen_awaiter(const listen_awaiter &) = delete;
					listen_awaiter &operator = (const listen_awaiter &) = delete;

					~listen_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.dataForCommandProcessing.ahd_listen = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
					}
				};
				listen( port, ip, backlog );
				return listen_awaiter(*this);
			}

			template<class SocketT>
			auto a_connection(nodecpp::safememory::soft_ptr<SocketT>& socket) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;
					nodecpp::safememory::soft_ptr<SocketT>& socket;

					connection_awaiter(ServerBase& server_, nodecpp::safememory::soft_ptr<SocketT>& socket_) : server( server_ ), socket( socket_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.dataForCommandProcessing.ahd_connection.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						if constexpr ( std::is_same<SocketT, SocketBase>::value )
							socket = server.dataForCommandProcessing.ahd_connection.sock;
						else
							socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
					}
				};
				return connection_awaiter(*this, socket);
			}

			template<class SocketT>
			auto a_connection(nodecpp::safememory::soft_ptr<SocketT>& socket, uint32_t period) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;
					nodecpp::safememory::soft_ptr<SocketT>& socket;
					uint32_t period;
					nodecpp::Timeout to;

					connection_awaiter(ServerBase& server_, nodecpp::safememory::soft_ptr<SocketT>& socket_, uint32_t period_) : server( server_ ), socket( socket_ ), period( period_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.dataForCommandProcessing.ahd_connection.h = awaiting;
						myawaiting = awaiting;
						/*to = std::move( nodecpp::setTimeout( [this](){
								auto h = server.dataForCommandProcessing.ahd_connection.h;
								server.dataForCommandProcessing.ahd_connection.h = nullptr;
								server.dataForCommandProcessing.ahd_connection.is_exception = true;
								server.dataForCommandProcessing.ahd_connection.exception = std::exception(); // TODO: switch to our exceptions ASAP!
								h();
							}, 
							period ) );*/
						to = nodecpp::setTimeoutForAction( &(server.dataForCommandProcessing.ahd_connection), period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( to );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
						if constexpr ( std::is_same<SocketT, SocketBase>::value )
							socket = server.dataForCommandProcessing.ahd_connection.sock;
						else
							socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
					}
				};
				return connection_awaiter(*this, socket, period);
			}

			template<class SocketT>
			auto a_close() { 

				struct close_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;

					close_awaiter(ServerBase& server_) : server( server_ ) {}

					close_awaiter(const close_awaiter &) = delete;
					close_awaiter &operator = (const close_awaiter &) = delete;

					~close_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						server.dataForCommandProcessing.ahd_close = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
					}
				};
				close();
				return close_awaiter(*this);
			}

			template<class SocketT>
			auto a_close(uint32_t period) { 

				struct close_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;
					uint32_t period;
					nodecpp::Timeout to;

					close_awaiter(ServerBase& server_, uint32_t period_) : server( server_ ), period( period_ ) {}

					close_awaiter(const close_awaiter &) = delete;
					close_awaiter &operator = (const close_awaiter &) = delete;

					~close_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::setNoException(awaiting);
						myawaiting = awaiting;
						server.dataForCommandProcessing.ahd_close = awaiting;
						to = nodecpp::setTimeoutForAction( &(server.dataForCommandProcessing.ahd_close), period );
					}

					auto await_resume() {
						nodecpp::clearTimeout( to );
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
						if ( nodecpp::isException(myawaiting) )
							throw nodecpp::getException(myawaiting);
					}
				};
				close();
				return close_awaiter(*this, period);
			}

#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES

			protected:
				MultiOwner<SocketBase> socketList;
		public:
			soft_ptr<SocketBase> makeSocket(OpaqueSocketData& sdata) { 
				owning_ptr<SocketBase> sock_;
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, acceptedSocketCreationRoutine != nullptr);
				sock_ = acceptedSocketCreationRoutine( sdata );
				nodecpp::safememory::soft_ptr<ServerBase> myPtr = myThis.getSoftPtr<ServerBase>(this);
				sock_->myServerSocket = myPtr;
				sock_->dataForCommandProcessing._local = dataForCommandProcessing.localAddress;
				soft_ptr<SocketBase> retSock( sock_ );
				socketList.add( std::move(sock_) );
				return retSock;
			}	
		private:
			friend class SocketBase;
			void reportAllAceptedConnectionsEnded();
			void removeSocket( soft_ptr<SocketBase> sock ) {
				socketList.removeAndDelete( sock );
				if ( dataForCommandProcessing.state == DataForCommandProcessing::State::BeingClosed && socketList.getCount() == 0 )
				{
					//dataForCommandProcessing.state = DataForCommandProcessing::State::Closed;
					//nodecpp::setInmediate( [this]() {} );
					reportAllAceptedConnectionsEnded();
				}
			}
		public:
			size_t getSockCount() {return this->socketList.getCount();}

			/////////////////////////////////////////////////////////////////

			// event emitters
#ifndef NODECPP_MSVC_BUG_379712_WORKAROUND_NO_LISTENER
			EventEmitterSupportingListeners<event::Close, ServerListener, &ServerListener::onClose> eClose;
			EventEmitterSupportingListeners<event::Connection, ServerListener, &ServerListener::onConnection> eConnection;
			EventEmitterSupportingListeners<event::Listening, ServerListener, &ServerListener::onListening> eListening;
			EventEmitterSupportingListeners<event::Error, ServerListener, &ServerListener::onError> eError;

			std::vector<nodecpp::safememory::owning_ptr<ServerListener>> ownedListeners;
#else
			EventEmitter<event::Close> eClose;
			EventEmitter<event::Connection> eConnection;
			EventEmitter<event::Listening> eListening;
			EventEmitter<event::Error> eError;
#endif

		public:
			void emitClose(bool hadError) {
//				state = CLOSED;
				//id = 0;
				eClose.emit(hadError);
			}

			void emitConnection(soft_ptr<SocketBase> socket) {
				eConnection.emit(socket);
			}

			void emitListening(size_t id, Address addr) {
				//this->id = id;
				this->dataForCommandProcessing.index = id;
				this->dataForCommandProcessing.localAddress = std::move(addr);
//				state = LISTENING;
				eListening.emit(id, addr);
			}

			void emitError(Error& err) {
				eError.emit(err);
			}

			void close(std::function<void(bool)> cb) {
				once(event::close, std::move(cb));
				ServerBase::close();
			}

			void listen(uint16_t port, const char* ip, int backlog, std::function<void(size_t, net::Address)> cb) {
				once(event::listening, std::move(cb));
				ServerBase::listen(port, ip, backlog);
			}

#ifndef NODECPP_MSVC_BUG_379712_WORKAROUND_NO_LISTENER
			void on( nodecpp::safememory::soft_ptr<ServerListener> l) {
				eClose.on(l);
				eConnection.on(l);
				eListening.on(l);
				eError.on(l);
			}

			void once( nodecpp::safememory::soft_ptr<ServerListener> l) {
				eClose.once(l);
				eConnection.once(l);
				eListening.once(l);
				eError.once(l);
			}

			void on( nodecpp::safememory::owning_ptr<ServerListener>& l) {
				nodecpp::safememory::soft_ptr<ServerListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				on( sl );
			}

			void once( nodecpp::safememory::owning_ptr<ServerListener>& l) {
				nodecpp::safememory::soft_ptr<ServerListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				once( sl );
			}
#endif
			void on(std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				assert(name == event::Close::name);
				eClose.on(std::move(cb));
			}

			void on(std::string name, event::Connection::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				assert(name == event::Connection::name);
				eConnection.on(std::move(cb));
			}

			void on(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				assert(name == event::Error::name);
				eError.on(std::move(cb));
			}

			void on(std::string name, event::Listening::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				assert(name == event::Listening::name);
				eListening.on(std::move(cb));
			}

			void once(std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				assert(name == event::Close::name);
				eClose.once(std::move(cb));
			}

			void once(std::string name, event::Connection::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				assert(name == event::Connection::name);
				eConnection.once(std::move(cb));
			}

			void once(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				assert(name == event::Error::name);
				eError.once(std::move(cb));
			}

			void once(std::string name, event::Listening::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				assert(name == event::Listening::name);
				eListening.once(std::move(cb));
			}


			template<class EV>
			void on(EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.on(std::move(cb)); }
				else assert(false);
			}

			template<class EV>
			void once(EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.once(std::move(cb)); }
				else assert(false);
			}
		};

		inline
		void SocketBase::onFinalCleanup()
		{ 
			forceReleasingAllCoroHandles();
			if ( myServerSocket != nullptr ) 
			{
				nodecpp::safememory::soft_ptr<SocketBase> myPtr = myThis.getSoftPtr<SocketBase>(this);
				myServerSocket->removeSocket(myPtr);
			}
		}

		template<class DataParentT>
		class ServerSocket : public ServerBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			ServerSocket<DataParentT>() {};
			ServerSocket<DataParentT>(DataParentT* dataParent ) : ServerBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~ServerSocket<DataParentT>() {}
		};

		template<>
		class ServerSocket<void> : public ServerBase
		{
		public:
			using DataParentType = void;
			ServerSocket() {};
			virtual ~ServerSocket() {}
		};

		template<class ServerT, class SocketT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createServer(Types&& ... args) {
			static_assert( std::is_base_of< ServerBase, ServerT >::value );
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
				retServer->setAcceptedSocketCreationRoutine( [](OpaqueSocketData& sdata) {
						nodecpp::safememory::owning_ptr<SocketT> ret = nodecpp::safememory::make_owning<SocketT>();
						ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*ret));
						if constexpr ( !std::is_same<typename SocketT::NodeType, void>::value )
						{
							static_assert( std::is_base_of< NodeBase, typename SocketT::NodeType >::value );
							int id = -1;
							if constexpr ( !std::is_same< typename SocketT::NodeType::EmitterType, void>::value )
								id = SocketT::NodeType::EmitterType::template softGetTypeIndexIfTypeExists<SocketT>();
							ret->registerMeByIDAndAssignSocket(sdata, id);
						}
						else
						{
							ret->registerMeByIDAndAssignSocket(sdata, -1);
						}
						return ret;
					} );
			}
			else
			{
				auto myDataParent = retServer->getDataParent();
				retServer->setAcceptedSocketCreationRoutine( [myDataParent](OpaqueSocketData& sdata) {
						nodecpp::safememory::owning_ptr<SocketT> retSock;
						if constexpr ( std::is_same<typename SocketT::DataParentType, typename ServerT::DataParentType >::value )
						{
							retSock = nodecpp::safememory::make_owning<SocketT>(myDataParent);
						}
						else
						{
							retSock = nodecpp::safememory::make_owning<SocketT>();
						}
						retSock->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*retSock));
						if constexpr ( !std::is_same<typename SocketT::NodeType, void>::value )
						{
							static_assert( std::is_base_of< NodeBase, typename SocketT::NodeType >::value );
							int id = -1;
							if constexpr ( !std::is_same< typename SocketT::NodeType::EmitterType, void>::value )
								id = SocketT::NodeType::EmitterType::template softGetTypeIndexIfTypeExists<SocketT>();
							retSock->registerMeByIDAndAssignSocket(sdata, id);
						}
						else
						{
							retSock->registerMeByIDAndAssignSocket(sdata, -1);
						}
						return retSock;
					} );
			}
			retServer->dataForCommandProcessing.userHandlers.from(ServerBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<ServerT>(), &(*retServer));
			return retServer;
		}

		template<class ServerT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createServer(Types&& ... args) {
			return createServer<ServerT, SocketBase>(::std::forward<Types>(args)...);
		}

		//template<class T>
		//T* createServer(std::function<void()> cb) {
		//	auto svr = new T();
		//	svr->on<event::Connect>(cb);
		//	return svr;
		//}

		//template<class T>
		//T* createConnection(uint16_t port, const char* host) {
		//	auto cli = new T();
		//	cli->appConnect(port, host);
		//	return cli;
		//}

		//template<class T>
		//T* createConnection(uint16_t port, const char* host, std::function<void()> cb) {
		//	auto cli = new T();
		//	cli->appConnect(port, host, std::move(cb));
		//	return cli;
		//}

	} //namespace net
} //namespace nodecpp

#endif //SERVER_COMMON_H
