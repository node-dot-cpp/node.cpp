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
#include "nls.h"

namespace nodecpp {

	class Cluster;

	namespace net {

		class ServerBase
		{
			friend class ::nodecpp::Cluster;
		public:
			using NodeType = void;
			using DataParentType = void;

		public:
			nodecpp::soft_this_ptr<ServerBase> myThis;
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


// end [perimeter calls for record and replay]
				void rrProcessListen( IPFAMILY family, nodecpp::Ip4 ip, uint16_t port, int backlog )
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
					{
						// TODO REPLAY: implement as a part of main replaying loop
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
					}
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::ServerListen edata;
						edata.ptr = (uintptr_t)(this);
						edata.ip = ip;
						edata.port = port;
						edata.backlog = backlog;
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_listen, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					refed = true;
					localAddress.ip = ip;
					localAddress.port = port;
					localAddress.family = family;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, index != 0 );
				}
// end [perimeter calls for record and replay]
				struct UserHandlersCommon
				{
				public:
					// originating from member functions of ServertBase-derived classes
					template<class T> using userListenMemberHandler = nodecpp::handler_ret_type (T::*)(size_t, nodecpp::net::Address);
					template<class T> using userConnectionMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<net::SocketBase>);
					template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

					// originating from member functions of NodeBase-derived classes
					template<class T, class ServerT> using userListenNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, size_t, nodecpp::net::Address);
					template<class T, class ServerT> using userConnectionNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, nodecpp::soft_ptr<net::SocketBase>);
					template<class T, class ServerT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, bool);
					template<class T, class ServerT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<ServerT>, Error&);

					using userDefListenHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<ServerBase>, size_t, nodecpp::net::Address);
					using userDefConnectionHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<ServerBase>, nodecpp::soft_ptr<net::SocketBase>);
					using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<ServerBase>, bool);
					using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<ServerBase>, Error&);

					// originating from member functions of ServerBase-derived classes

					template<class ObjectT, userListenMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type listenHandler( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(id, addr);
						CO_RETURN;
					}

					template<class ObjectT, userConnectionMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type connectionHandler( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, nodecpp::soft_ptr<net::SocketBase> socket )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(socket);
						CO_RETURN;
					}

					template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type closeHandler( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, bool hadError )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
						CO_RETURN;
					}

					template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type errorHandler( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, Error& e )
					{
						//NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == reinterpret_cast<ObjectT*>(serverPtr) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
						CO_RETURN;
					}

					// originating from member functions of NodeBase-derived classes

					template<class ObjectT, class ServerT, userListenNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type listenHandlerFromNode( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, size_t id, nodecpp::net::Address addr )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), id, addr);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userConnectionNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type connectionHandlerFromNode( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, nodecpp::soft_ptr<net::SocketBase> socket )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), socket);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userCloseNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type closeHandlerFromNode( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, bool hadError )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), hadError);
						CO_RETURN;
					}

					template<class ObjectT, class ServerT, userErrorNodeMemberHandler<ObjectT, ServerT> MemberFnT>
					static nodecpp::handler_ret_type errorHandlerFromNode( void* objPtr, nodecpp::soft_ptr<ServerBase> serverPtr, Error& e )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(nodecpp::soft_ptr_reinterpret_cast<ServerT>(serverPtr), e);
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
				void handleListenEvent(nodecpp::soft_ptr<ServerBase> server, size_t id, nodecpp::net::Address address) { userHandlers.userDefListenHandlers.execute(server, id, address); }

				bool isConnectionEventHandler() { return userHandlers.userDefConnectionHandlers.willHandle(); }
				void handleConnectionEvent(nodecpp::soft_ptr<ServerBase> server, nodecpp::soft_ptr<net::SocketBase> socket) { userHandlers.userDefConnectionHandlers.execute(server, socket); }

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(nodecpp::soft_ptr<ServerBase> server, bool hasError) { userHandlers.userDefCloseHandlers.execute(server, hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(nodecpp::soft_ptr<ServerBase> server, Error& e) { userHandlers.userDefErrorHandlers.execute(server, e); }
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

		public:
			void registerServer(soft_ptr<net::ServerBase> t);

		public:
			NodeBase* node = nullptr;

			using acceptedSocketCreationRoutineType = std::function<owning_ptr<SocketBase>(OpaqueSocketData&)>;
		private:
			acceptedSocketCreationRoutineType acceptedSocketCreationRoutine = nullptr;

		public:
			ServerBase();
			virtual ~ServerBase() {
				socketList.clear();
				reportBeingDestructed();
			}
			void setAcceptedSocketCreationRoutine(acceptedSocketCreationRoutineType socketCreationCB) { acceptedSocketCreationRoutine = std::move( socketCreationCB ); }
			void internalCleanupBeforeClosing()
			{
				NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, getSockCount() == 0 ); 
				dataForCommandProcessing.state = DataForCommandProcessing::State::Closed;
				forceReleasingAllCoroHandles();
//				dataForCommandProcessing.index = 0;
			}

			const Address& address() const { return dataForCommandProcessing.localAddress; }
			void close();

			bool listening() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Listening; }
			void ref();
			void unref();
			void reportBeingDestructed();

			void listen(uint16_t port, const char* ip, int backlog);
			void listen(uint16_t port, string ip, int backlog) { listen( port, ip.c_str(), backlog ); }
			void listen(uint16_t port, string_literal ip, int backlog) { listen( port, ip.c_str(), backlog ); };

// begin [perimeter calls for record and replay]
			void rrOnListening( size_t idx)
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
#endif // NODECPP_RECORD_AND_REPLAY
				dataForCommandProcessing.state = nodecpp::net::ServerBase::DataForCommandProcessing::State::Listening;
				auto hr = dataForCommandProcessing.ahd_listen;
				if ( hr != nullptr )
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::SocketEvent edata;
						edata.ptr = (uintptr_t)(this);
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_listening_event_crh, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					dataForCommandProcessing.ahd_listen = nullptr;
					hr();
				}
				else
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::SocketEvent edata;
						edata.ptr = (uintptr_t)(this);
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_listening_event_call, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					// TODO: revise (for the sournecessity of supplying id and addr here: index has already been assign while registering server)
					emitListening(idx, dataForCommandProcessing.localAddress);
					if (dataForCommandProcessing.isListenEventHandler() )
					{
						nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = myThis.getSoftPtr<net::ServerBase>(this);
						dataForCommandProcessing.handleListenEvent(serverSoftPtr, idx, dataForCommandProcessing.localAddress);
					}
				}
			}

			void rrOnClose( bool isError )
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
#endif // NODECPP_RECORD_AND_REPLAY
				NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, dataForCommandProcessing.state == nodecpp::net::ServerBase::DataForCommandProcessing::State::BeingClosed, "indeed: {}", (size_t)(dataForCommandProcessing.state) ); 
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
				{
					record_and_replay_impl::BinaryLog::SocketEvent edata;
					edata.ptr = (uintptr_t)(this);
					::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_event_1, &edata, sizeof( edata ) );
				}
#endif // NODECPP_RECORD_AND_REPLAY
				internalCleanupBeforeClosing();
				auto hr = dataForCommandProcessing.ahd_close;
				if ( hr != nullptr )
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::ServerCloseEvent_2 edata;
						edata.ptr = (uintptr_t)(this);
						edata.err = isError;
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_event_2_crh, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					dataForCommandProcessing.ahd_close = nullptr;
					if (isError)
						nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					hr();
				}
				else
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::ServerCloseEvent_2 edata;
						edata.ptr = (uintptr_t)(this);
						edata.err = isError;
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_event_2_call, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					emitClose( isError );
					if (dataForCommandProcessing.isCloseEventHandler())
					{
						nodecpp::safememory::soft_ptr<net::ServerBase> serverSoftPtr = myThis.getSoftPtr<net::ServerBase>(this);
						dataForCommandProcessing.handleCloseEvent(serverSoftPtr, isError);
					}
					// TODO: what should we do with this event, if, at present, nobody is willing to process it?
				}
			}

// end [perimeter calls for record and replay]


#ifndef NODECPP_NO_COROUTINES
			void forceReleasingAllCoroHandles()
			{
				if ( dataForCommandProcessing.ahd_listen != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_listen;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_listen = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_connection.h != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_connection.h;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
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
						nodecpp::initCoroData(awaiting);
						server.dataForCommandProcessing.ahd_listen = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_listen_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_listen_crh_ok, nullptr, 0 );
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_listen_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_listen_crh_ok )
							{
								return;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
						}
					}
				};
				listen( port, ip, backlog );
				return listen_awaiter(*this);
			}

			template<class SocketT>
			auto a_connection(nodecpp::soft_ptr<SocketT>& socket) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;
					nodecpp::soft_ptr<SocketT>& socket;

					connection_awaiter(ServerBase& server_, nodecpp::soft_ptr<SocketT>& socket_) : server( server_ ), socket( socket_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						server.dataForCommandProcessing.ahd_connection.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						// TODO: rework with IDs
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_ok, &(*(server.dataForCommandProcessing.ahd_connection.sock)), sizeof( void* ) );
							if constexpr ( std::is_same<SocketT, SocketBase>::value )
								socket = server.dataForCommandProcessing.ahd_connection.sock;
							else
								socket = nodecpp::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_ok )
							{
								if constexpr ( std::is_same<SocketT, SocketBase>::value )
									socket = server.dataForCommandProcessing.ahd_connection.sock;
								else
									socket = nodecpp::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if constexpr ( std::is_same<SocketT, SocketBase>::value )
								socket = server.dataForCommandProcessing.ahd_connection.sock;
							else
								socket = nodecpp::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
						}
					}
				};
				return connection_awaiter(*this, socket);
			}

			template<class SocketT>
			::nodecpp::awaitable<::nodecpp::CoroStandardOutcomes> a_connection(nodecpp::safememory::soft_ptr<SocketT>& socket, uint32_t period) { 

				struct connection_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					ServerBase& server;
					nodecpp::soft_ptr<SocketT>& socket;
					uint32_t period;
					nodecpp::Timeout to;

					connection_awaiter(ServerBase& server_, nodecpp::soft_ptr<SocketT>& socket_, uint32_t period_) : server( server_ ), socket( socket_ ), period( period_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;

					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						server.dataForCommandProcessing.ahd_connection.h = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( server.dataForCommandProcessing.ahd_connection.h, period );
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						// TODO: rework with IDs
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							nodecpp::clearTimeout( to ); // TODO: consider adding respective control frame
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_timeout, nullptr, 0 );
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_ok, &(*(server.dataForCommandProcessing.ahd_connection.sock)), sizeof( void* ) );
							if constexpr ( std::is_same<SocketT, SocketBase>::value )
								socket = server.dataForCommandProcessing.ahd_connection.sock;
							else
								socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
							CO_RETURN CoroStandardOutcomes::ok;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							nodecpp::clearTimeout( to ); // TODO: consider getting and checking respective control frame
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_conn_crh_ok )
							{
								if constexpr ( std::is_same<SocketT, SocketBase>::value )
									socket = server.dataForCommandProcessing.ahd_connection.sock;
								else
									socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
								CO_RETURN CoroStandardOutcomes::ok;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							nodecpp::clearTimeout( to );
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							// TODO: revise order of checking conditions to favour success
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if ( nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							if constexpr ( std::is_same<SocketT, SocketBase>::value )
								socket = server.dataForCommandProcessing.ahd_connection.sock;
							else
								socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
							CO_RETURN CoroStandardOutcomes::ok;
						}
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
						nodecpp::initCoroData(awaiting);
						server.dataForCommandProcessing.ahd_close = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_crh_ok, nullptr, 0 );
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_close_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_close_crh_ok )
							{
								return;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
						}
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
						nodecpp::initCoroData(awaiting);
						myawaiting = awaiting;
						server.dataForCommandProcessing.ahd_close = awaiting;
						to = nodecpp::setTimeoutForAction( &(server.dataForCommandProcessing.ahd_close), period );
					}

					::nodecpp::awaitable<::nodecpp::CoroStandardOutcomes> await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_crh_timeout, nullptr, 0 );
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::server_close_crh_ok, nullptr, 0 );
							CO_RETURN CoroStandardOutcomes::ok;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_close_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_close_crh_timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::server_close_crh_ok )
							{
								CO_RETURN CoroStandardOutcomes::ok;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							nodecpp::clearTimeout( to );
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
							// TODO: revise order of checking conditions to favour success
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if (  nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							CO_RETURN CoroStandardOutcomes::ok;
						}
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
				nodecpp::soft_ptr<ServerBase> myPtr = myThis.getSoftPtr<ServerBase>(this);
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
					reportAllAceptedConnectionsEnded(); // posts close event
			}

			void closingProcedure();
#ifdef NODECPP_ENABLE_CLUSTERING
			void closeByWorkingCluster() {closingProcedure();}
#endif // NODECPP_ENABLE_CLUSTERING
		public:
			size_t getSockCount() {return this->socketList.getCount();}

			/////////////////////////////////////////////////////////////////

			// event emitters
#ifndef NODECPP_MSVC_BUG_379712_WORKAROUND_NO_LISTENER
			EventEmitterSupportingListeners<event::Close, ServerListener, &ServerListener::onClose> eClose;
			EventEmitterSupportingListeners<event::Connection, ServerListener, &ServerListener::onConnection> eConnection;
			EventEmitterSupportingListeners<event::Listening, ServerListener, &ServerListener::onListening> eListening;
			EventEmitterSupportingListeners<event::Error, ServerListener, &ServerListener::onError> eError;

			nodecpp::vector<nodecpp::owning_ptr<ServerListener>> ownedListeners;
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
				// TODO: revise together with a point of call (for the source of id and addr)
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
			void on( nodecpp::soft_ptr<ServerListener> l) {
				eClose.on(l);
				eConnection.on(l);
				eListening.on(l);
				eError.on(l);
			}

			void once( nodecpp::soft_ptr<ServerListener> l) {
				eClose.once(l);
				eConnection.once(l);
				eListening.once(l);
				eError.once(l);
			}

			void on( nodecpp::owning_ptr<ServerListener>& l) {
				nodecpp::soft_ptr<ServerListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				on( sl );
			}

			void once( nodecpp::owning_ptr<ServerListener>& l) {
				nodecpp::soft_ptr<ServerListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				once( sl );
			}
#endif
			void on(nodecpp::string_literal name, event::Close::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Close::name);
				eClose.on(std::move(cb));
			}

			void on(nodecpp::string_literal name, event::Connection::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Connection::name);
				eConnection.on(std::move(cb));
			}

			void on(nodecpp::string_literal name, event::Error::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Error::name);
				eError.on(std::move(cb));
			}

			void on(nodecpp::string_literal name, event::Listening::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Listening::name);
				eListening.on(std::move(cb));
			}

			void once(nodecpp::string_literal name, event::Close::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Close::name);
				eClose.once(std::move(cb));
			}

			void once(nodecpp::string_literal name, event::Connection::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Connection::name);
				eConnection.once(std::move(cb));
			}

			void once(nodecpp::string_literal name, event::Error::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Error::name);
				eError.once(std::move(cb));
			}

			void once(nodecpp::string_literal name, event::Listening::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == event::Listening::name);
				eListening.once(std::move(cb));
			}


			template<class EV>
			void on(EV, typename EV::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.on(std::move(cb)); }
				else NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false);
			}

			template<class EV>
			void once(EV, typename EV::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.once(std::move(cb)); }
				else NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false);
			}
		};

		inline
		void SocketBase::onFinalCleanup()
		{ 
			forceReleasingAllCoroHandles();
			if ( myServerSocket != nullptr ) 
			{
				nodecpp::soft_ptr<SocketBase> myPtr = myThis.getSoftPtr<SocketBase>(this);
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
		nodecpp::owning_ptr<ServerT> createServer(Types&& ... args) {
			static_assert( std::is_base_of< ServerBase, ServerT >::value );
			nodecpp::owning_ptr<ServerT> retServer = nodecpp::make_owning<ServerT>(::std::forward<Types>(args)...);
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
						nodecpp::owning_ptr<SocketT> ret = nodecpp::make_owning<SocketT>();
						ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*ret));
						ret->registerMeAndAssignSocket(sdata);
						return ret;
					} );
			}
			else
			{
				auto myDataParent = retServer->getDataParent();
				retServer->setAcceptedSocketCreationRoutine( [myDataParent](OpaqueSocketData& sdata) {
						nodecpp::owning_ptr<SocketT> retSock;
						if constexpr ( std::is_same<typename SocketT::DataParentType, typename ServerT::DataParentType >::value )
						{
							retSock = nodecpp::make_owning<SocketT>(myDataParent);
						}
						else
						{
							retSock = nodecpp::make_owning<SocketT>();
						}
						retSock->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*retSock));
						retSock->registerMeAndAssignSocket(sdata);
						return retSock;
					} );
			}
			retServer->dataForCommandProcessing.userHandlers.from(ServerBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<ServerT>(), &(*retServer));
			return retServer;
		}

		template<class ServerT, class ... Types>
		static
		nodecpp::owning_ptr<ServerT> createServer(Types&& ... args) {
			return createServer<ServerT, SocketBase>(::std::forward<Types>(args)...);
		}

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
