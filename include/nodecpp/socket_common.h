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
#include "nls.h"

class OpaqueSocketData;

namespace nodecpp {

	namespace net {

		class ServerBase; // forward declaration
		class SocketBase
		{
		public:
			using NodeType = void;
			using DataParentType = void;

		public:
			nodecpp::soft_this_ptr<SocketBase> myThis;
		protected:
			friend class ServerBase;
			nodecpp::soft_ptr<ServerBase> myServerSocket = nullptr;
		public:
			void onFinalCleanup();


			public:
			class DataForCommandProcessing {
			public:
				enum State { Uninitialized, Connecting, Connected, LocalEnding, LocalEnded, Closing, ErrorClosing, Closed }
				state = Uninitialized;
				size_t index = 0;

				Address _local;
				Address _remote;

				struct awaitable_read_handle_data
				{
					awaitable_handle_t h = nullptr;
					size_t min_bytes;
				};
				struct awaitable_write_handle_data
				{
					awaitable_handle_t h = nullptr;
					Buffer b;
				};

				// NOTE: make sure all of them are addressed at forceResumeWithThrowing()
				awaitable_handle_t ahd_connect = nullptr;
				awaitable_handle_t ahd_accepted = nullptr;
				awaitable_read_handle_data ahd_read;
				awaitable_write_handle_data ahd_write;
				awaitable_handle_t ahd_drain = nullptr;

			//	bool connecting = false;
				bool remoteEnded = false;
				//bool localEnded = false;
				//bool pendingLocalEnd = false;
				bool paused = false;
				bool allowHalfOpen = false; // nodejs-inspired reasonable default

				bool refed = false;

				CircularByteBuffer writeBuffer = CircularByteBuffer( 12 );
				CircularByteBuffer readBuffer = CircularByteBuffer( 12 );

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
					// originating from member functions of SocketBase-derived classes
					template<class T> using userAcceptedMemberHandler = nodecpp::handler_ret_type (T::*)();
					template<class T> using userConnectMemberHandler = nodecpp::handler_ret_type (T::*)();
					template<class T> using userDataMemberHandler = nodecpp::handler_ret_type (T::*)(Buffer&);
					template<class T> using userDrainMemberHandler = nodecpp::handler_ret_type (T::*)();
					template<class T> using userEndMemberHandler = nodecpp::handler_ret_type (T::*)();
					template<class T> using userCloseMemberHandler = nodecpp::handler_ret_type (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::handler_ret_type (T::*)(Error&);

					// originating from member functions of NodeBase-derived classes
					template<class T, class SocketT> using userAcceptedNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>);
					template<class T, class SocketT> using userConnectNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>);
					template<class T, class SocketT> using userDataNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>, Buffer&);
					template<class T, class SocketT> using userDrainNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>);
					template<class T, class SocketT> using userEndNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>);
					template<class T, class SocketT> using userCloseNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>, bool);
					template<class T, class SocketT> using userErrorNodeMemberHandler = nodecpp::handler_ret_type (T::*)(nodecpp::soft_ptr<SocketT>, Error&);

					using userDefAcceptedHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>);
					using userDefConnectHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>);
					using userDefDataHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>, Buffer& buffer);
					using userDefDrainHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>);
					using userDefEndHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>);
					using userDefCloseHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>, bool);
					using userDefErrorHandlerFnT = nodecpp::handler_ret_type (*)(void*, nodecpp::soft_ptr<SocketBase>, Error&);

					// originating from member functions of SocketBase-derived classes

					template<class ObjectT, userAcceptedMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type acceptedHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						CO_RETURN;
					}

					template<class ObjectT, userConnectMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type connectHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						CO_RETURN;
					}

					template<class ObjectT, userDataMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type dataHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, Buffer& buffer)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(buffer);
						CO_RETURN;
					}

					template<class ObjectT, userDrainMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type drainHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						CO_RETURN;
					}

					template<class ObjectT, userEndMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type endHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)();
						CO_RETURN;
					}

					template<class ObjectT, userCloseMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type closeHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, bool hadError)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(hadError);
						CO_RETURN;
					}

					template<class ObjectT, userErrorMemberHandler<ObjectT> MemberFnT>
					static nodecpp::handler_ret_type errorHandler(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, Error& e)
					{
						NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, reinterpret_cast<ObjectT*>(objPtr) == &(*(nodecpp::soft_ptr_reinterpret_cast<ObjectT>(sockPtr))) ); 
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(e);
						CO_RETURN;
					}


					// originating from member functions of NodeBase-derived classes

					template<class ObjectT, class SocketT, userAcceptedNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type acceptedHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr) );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userConnectNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type connectHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr) );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userDataNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type dataHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, Buffer& buffer)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr), buffer );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userDrainNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type drainHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr) );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userEndNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type endHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr) );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userCloseNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type closeHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, bool hadError)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr), hadError );
						CO_RETURN;
					}

					template<class ObjectT, class SocketT, userErrorNodeMemberHandler<ObjectT, SocketT> MemberFnT>
					static nodecpp::handler_ret_type errorHandlerFromNode(void* objPtr, nodecpp::soft_ptr<SocketBase> sockPtr, Error& e)
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)( nodecpp::soft_ptr_reinterpret_cast<SocketT>(sockPtr), e );
						CO_RETURN;
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
					void addHandler()
					{
						if constexpr (handler == Handler::Accepted)
						{
							userDefAcceptedHandlers.add(&DataForCommandProcessing::UserHandlers::acceptedHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connect)
						{
							userDefConnectHandlers.add(&DataForCommandProcessing::UserHandlers::connectHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Data)
						{
							userDefDataHandlers.add(&DataForCommandProcessing::UserHandlers::dataHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Drain)
						{
							userDefDrainHandlers.add(&DataForCommandProcessing::UserHandlers::drainHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::End)
						{
							userDefEndHandlers.add(&DataForCommandProcessing::UserHandlers::endHandler<ObjectT, memmberFn>);
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
					template<Handler handler, auto memmberFn, class ObjectT, class SocketT>
					void addHandlerFromNode(ObjectT* object)
					{
						if constexpr (handler == Handler::Accepted)
						{
							userDefAcceptedHandlers.add(object, &DataForCommandProcessing::UserHandlers::acceptedHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connect)
						{
							userDefConnectHandlers.add(object, &DataForCommandProcessing::UserHandlers::connectHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else if constexpr (handler == Handler::Data)
						{
							userDefDataHandlers.add(object, &DataForCommandProcessing::UserHandlers::dataHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else if constexpr (handler == Handler::Drain)
						{
							userDefDrainHandlers.add(object, &DataForCommandProcessing::UserHandlers::drainHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else if constexpr (handler == Handler::End)
						{
							userDefEndHandlers.add(object, &DataForCommandProcessing::UserHandlers::endHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else if constexpr (handler == Handler::Close)
						{
							userDefCloseHandlers.add(object, &DataForCommandProcessing::UserHandlers::closeHandlerFromNode<ObjectT, SocketT, memmberFn>);
						}
						else
						{
							static_assert(handler == Handler::Error); // the only remaining option
							userDefErrorHandlers.add(object, &DataForCommandProcessing::UserHandlers::errorHandlerFromNode<ObjectT, SocketT, memmberFn>);
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
				void handleAcceptedEvent(nodecpp::soft_ptr<SocketBase> socket) { userHandlers.userDefAcceptedHandlers.execute(socket); }

				bool isConnectEventHandler() { return userHandlers.userDefConnectHandlers.willHandle(); }
				void handleConnectEvent(nodecpp::soft_ptr<SocketBase> socket) { userHandlers.userDefConnectHandlers.execute(socket); }

				bool isDataEventHandler() { return userHandlers.userDefDataHandlers.willHandle(); }
				void handleDataEvent(nodecpp::soft_ptr<SocketBase> socket, Buffer& buffer) { userHandlers.userDefDataHandlers.execute(socket, buffer); }

				bool isDrainEventHandler() { return userHandlers.userDefDrainHandlers.willHandle(); }
				void handleDrainEvent(nodecpp::soft_ptr<SocketBase> socket) { userHandlers.userDefDrainHandlers.execute(socket); }

				bool isEndEventHandler() { return userHandlers.userDefEndHandlers.willHandle(); }
				void handleEndEvent(nodecpp::soft_ptr<SocketBase> socket) { userHandlers.userDefEndHandlers.execute(socket); }

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(nodecpp::soft_ptr<SocketBase> socket, bool hasError) { userHandlers.userDefCloseHandlers.execute(socket, hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(nodecpp::soft_ptr<SocketBase> socket, Error& e) { userHandlers.userDefErrorHandlers.execute(socket, e); }
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

//		private:
		public:
			void registerMeAndAcquireSocket();
			void registerMeAndAssignSocket(OpaqueSocketData& sdata);

		public:
			//nodecpp::string _remoteAddress;
			//nodecpp::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			SocketBase() {}

			SocketBase(const SocketBase&) = delete;
			SocketBase& operator=(const SocketBase&) = delete;

			SocketBase(SocketBase&&) = default;
			SocketBase& operator=(SocketBase&&) = default;

			virtual ~SocketBase() {
				if ( dataForCommandProcessing.state == DataForCommandProcessing::State::Connecting || dataForCommandProcessing.state == DataForCommandProcessing::State::Connected ) destroy();
				unref(); /*or assert that is must already be unrefed*/
				reportBeingDestructed(); 
			}

		public:


			size_t bufferSize() const { return dataForCommandProcessing.writeBuffer.used_size(); }
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			bool connecting() const { return dataForCommandProcessing.state == DataForCommandProcessing::State::Connecting; }
			void destroy();
			bool destroyed() const { return !(dataForCommandProcessing.state == DataForCommandProcessing::State::Connecting || dataForCommandProcessing.state == DataForCommandProcessing::State::Connected); };
			void end();

			const Address& localAddress() const { return dataForCommandProcessing._local; }
			nodecpp::string localIP() const { return dataForCommandProcessing._local.ip.toStr(); }
			uint16_t localPort() const { return dataForCommandProcessing._local.port; }


			const Address& remoteAddress() const { return dataForCommandProcessing._remote; }
			nodecpp::string remoteIP() const { return dataForCommandProcessing._remote.ip.toStr(); }
			const nodecpp::IPFAMILY remoteFamily() const { return dataForCommandProcessing._remote.family; }
			uint16_t remotePort() const { return dataForCommandProcessing._remote.port; }

			void ref();
			void unref();
			void pause();
			void resume();
			void reportBeingDestructed();

		private:
			bool write(const uint8_t* data, uint32_t size);
			bool write2(Buffer& b);

		public:
			void connect(uint16_t port, const char* ip);
			void connect(uint16_t port, string ip) { connect( port, ip.c_str() ); }
			void connect(uint16_t port, string_literal ip) { connect( port, ip.c_str() ); };
			void connect(uint16_t port, string ip, event::Connect::callback cb NODECPP_MAY_EXTEND_TO_THIS) { connect( port, ip.c_str(), std::move(cb) ); }
			void connect(uint16_t port, string_literal ip, event::Connect::callback cb NODECPP_MAY_EXTEND_TO_THIS) { connect( port, ip.c_str(), std::move(cb) ); };

			bool write(Buffer& buff) { return write( buff.begin(), (uint32_t)(buff.size()) ); }

			SocketBase& setNoDelay(bool noDelay = true);
			SocketBase& setKeepAlive(bool enable = false);

// begin [perimeter calls for record and replay]
			void rrOnAccepted()
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
				{
					record_and_replay_impl::BinaryLog::SocketEvent edata;
					edata.ptr = (uintptr_t)(this);
					::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_accepted, &edata, sizeof( edata ) );
				}
				else if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
#endif // NODECPP_RECORD_AND_REPLAY
				auto hr = dataForCommandProcessing.ahd_accepted;
				if ( hr )
				{
					dataForCommandProcessing.ahd_accepted = nullptr;
					hr();
				}
				else // TODO: make sure we never have both cases in the same time
				{
					emitAccepted();
					if (dataForCommandProcessing.isAcceptedEventHandler())
					{
						soft_ptr<net::SocketBase> sockSoftPtr = myThis.getSoftPtr<net::SocketBase>(this);
						dataForCommandProcessing.handleAcceptedEvent(sockSoftPtr);
					}
				}
			}

			void rrOnRemoteEnded()
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
#endif // NODECPP_RECORD_AND_REPLAY
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
				{
					record_and_replay_impl::BinaryLog::SocketEvent edata;
					edata.ptr = (uintptr_t)(&(*entry.getClientSocket()));
					::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_remote_ended_call, &edata, sizeof( edata ) );
				}
#endif // NODECPP_RECORD_AND_REPLAY
				dataForCommandProcessing.remoteEnded = true;

				emitEnd();
				if (dataForCommandProcessing.isEndEventHandler())
				{
					soft_ptr<net::SocketBase> sockSoftPtr = myThis.getSoftPtr<net::SocketBase>(this);
					dataForCommandProcessing.handleEndEvent(sockSoftPtr);
				}
			}

			void rrOnReadHandler( Buffer& recvBuffer )
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
				{
					record_and_replay_impl::BinaryLog::SocketEvent edata;
					edata.ptr = (uintptr_t)(this);
					::nodecpp::threadLocalData.binaryLog->startAddingFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_event_call, &edata, sizeof( edata ) );
					::nodecpp::threadLocalData.binaryLog->continueAddingFrame( recvBuffer.begin(), recvBuffer.size() );
					::nodecpp::threadLocalData.binaryLog->addingFrameDone();
				}
#endif // NODECPP_RECORD_AND_REPLAY
				emitData( recvBuffer );
				if (dataForCommandProcessing.isDataEventHandler())
				{
					soft_ptr<net::SocketBase> sockSoftPtr = myThis.getSoftPtr<net::SocketBase>(this);
					dataForCommandProcessing.handleDataEvent(sockSoftPtr, recvBuffer);
				}
			}

			void rrUpdateState( DataForCommandProcessing::State state )
			{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
					{
						record_and_replay_impl::BinaryLog::SocketUpdateState edata;
						edata.ptr = (uintptr_t)(this);
						edata.state = net::SocketBase::DataForCommandProcessing::LocalEnding;
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_update_state, &edata, sizeof( edata ) );
					}
#endif // NODECPP_RECORD_AND_REPLAY
					dataForCommandProcessing.state = state;
			}

			void rrOnConnected()
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
				record_and_replay_impl::BinaryLog::SocketEvent edata;
				edata.ptr = (uintptr_t)(&dataForCommandProcessing);
#endif // NODECPP_RECORD_AND_REPLAY
				auto hr = dataForCommandProcessing.ahd_connect;
				if ( hr )
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_event_crh, &edata, sizeof( edata ) );
#endif // NODECPP_RECORD_AND_REPLAY
					dataForCommandProcessing.ahd_connect = nullptr;
					hr();
				}
				else
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_event_call, &edata, sizeof( edata ) );
#endif // NODECPP_RECORD_AND_REPLAY
					emitConnect();
					if (dataForCommandProcessing.isConnectEventHandler())
					{
						soft_ptr<net::SocketBase> sockSoftPtr = myThis.getSoftPtr<net::SocketBase>(this);
						dataForCommandProcessing.handleConnectEvent(sockSoftPtr);
					}
				}
			}

			void rrOnDrain()
			{
#ifdef NODECPP_RECORD_AND_REPLAY
				if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
				{
					// TODO REPLAY: implement as a part of main replaying loop
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "must be implemented as a part of main replaying loop" );
				}
				record_and_replay_impl::BinaryLog::SocketEvent edata;
				edata.ptr = (uintptr_t)(&dataForCommandProcessing);
#endif // NODECPP_RECORD_AND_REPLAY
				auto hr = dataForCommandProcessing.ahd_drain;
				if ( hr )
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_event_crh, &edata, sizeof( edata ) );
#endif // NODECPP_RECORD_AND_REPLAY
					dataForCommandProcessing.ahd_drain = nullptr;
					hr();
				}
				else // TODO: make sure we never have both cases in the same time
				{
#ifdef NODECPP_RECORD_AND_REPLAY
					if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_event_call, &edata, sizeof( edata ) );
#endif // NODECPP_RECORD_AND_REPLAY
					emitDrain();
					if (dataForCommandProcessing.isDrainEventHandler())
					{
						soft_ptr<net::SocketBase> sockSoftPtr = myThis.getSoftPtr<net::SocketBase>(this);
						dataForCommandProcessing.handleDrainEvent(sockSoftPtr);
					}
				}
			}
// end [perimeter calls for record and replay]


#ifndef NODECPP_NO_COROUTINES

			void forceReleasingAllCoroHandles()
			{
				if ( dataForCommandProcessing.ahd_accepted != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_accepted;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_accepted = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_connect != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_connect;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_connect = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_read.h != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_read.h;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_read.h = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_write.h != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_write.h;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_write.h = nullptr;
					hr();
				}
				if ( dataForCommandProcessing.ahd_drain != nullptr )
				{
					auto hr = dataForCommandProcessing.ahd_drain;
					nodecpp::setCoroException(hr, std::exception()); // TODO: switch to our exceptions ASAP!
					dataForCommandProcessing.ahd_drain = nullptr;
					hr();
				}
			}

			auto a_connect(uint16_t port, const char* ip) { 

				struct connect_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;

					connect_awaiter(SocketBase& socket_) : socket( socket_ ) {}

					connect_awaiter(const connect_awaiter &) = delete;
					connect_awaiter &operator = (const connect_awaiter &) = delete;
	
					~connect_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_connect = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, myawaiting != nullptr ); 
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_ok, nullptr, 0 );
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_ok )
							{
								return;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
						}
					}
				};
				connect( port, ip );
				return connect_awaiter(*this);
			}

			auto a_connect(uint16_t port, const char* ip, uint32_t period) { 

				struct connect_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					uint32_t period;
					nodecpp::Timeout to;

					connect_awaiter(SocketBase& socket_, uint32_t period_) : socket( socket_ ), period( period_ ) {}

					connect_awaiter(const connect_awaiter &) = delete;
					connect_awaiter &operator = (const connect_awaiter &) = delete;
	
					~connect_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_connect = awaiting;
						to = nodecpp::setTimeoutForAction( awaiting, period );
						myawaiting = awaiting;
					}

					::nodecpp::awaitable<::nodecpp::CoroStandardOutcomes> await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_timeout, nullptr, 0 );
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_ok, nullptr, 0 );
							CO_RETURN CoroStandardOutcomes::ok;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_connect_crh_ok )
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
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							CO_RETURN CoroStandardOutcomes::ok;
						}
					}
				};
				connect( port, ip );
				return connect_awaiter(*this, period);
			}

			auto a_read( Buffer& buff, size_t min_bytes = 1, size_t max_bytes = SIZE_MAX ) { 

				buff.clear();
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.capacity() >= min_bytes, "indeed: {} vs. {} bytes", buff.capacity(), min_bytes );
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, min_bytes <= max_bytes, "indeed: {} vs. {} bytes", min_bytes, max_bytes );
				if ( max_bytes > buff.capacity() )
					max_bytes = buff.capacity();

				struct read_data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					Buffer& buff;
					size_t min_bytes;
					size_t max_bytes;

					read_data_awaiter(SocketBase& socket_, Buffer& buff_, size_t min_bytes_, size_t max_bytes_) : socket( socket_ ), buff( buff_ ), min_bytes( min_bytes_ ), max_bytes( max_bytes_ ) {}

					read_data_awaiter(const read_data_awaiter &) = delete;
					read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
					~read_data_awaiter() {}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						return socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = min_bytes;
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							socket.dataForCommandProcessing.readBuffer.get_ready_data( buff, max_bytes );
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_ok, buff.begin(), buff.size() );
							NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_ok )
							{
								buff.append( frame.ptr, frame.size );
								NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							socket.dataForCommandProcessing.readBuffer.get_ready_data( buff, max_bytes );
							NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
						}
					}
				};
				return read_data_awaiter(*this, buff, min_bytes, max_bytes);
			}

			auto a_read( uint32_t period, Buffer& buff, size_t min_bytes = 1, size_t max_bytes = SIZE_MAX ) { 

				buff.clear();
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.capacity() >= min_bytes, "indeed: {} vs. {} bytes", buff.capacity(), min_bytes );
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, min_bytes <= max_bytes, "indeed: {} vs. {} bytes", min_bytes, max_bytes );
				if ( max_bytes > buff.capacity() )
					max_bytes = buff.capacity();

				struct read_data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					Buffer& buff;
					size_t min_bytes;
					size_t max_bytes;
					uint32_t period;
					nodecpp::Timeout to;

					read_data_awaiter(SocketBase& socket_, uint32_t period_, Buffer& buff_, size_t min_bytes_, size_t max_bytes_) : socket( socket_ ), buff( buff_ ), min_bytes( min_bytes_ ), max_bytes( max_bytes_ ), period( period_ ) {}

					read_data_awaiter(const read_data_awaiter &) = delete;
					read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
					~read_data_awaiter() {}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						return socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = min_bytes;
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( awaiting, period );
					}

					::nodecpp::awaitable<::nodecpp::CoroStandardOutcomes> await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							nodecpp::clearTimeout( to );
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							else if ( myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_timeout, nullptr, 0 );
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							socket.dataForCommandProcessing.readBuffer.get_ready_data( buff, max_bytes );
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_ok, buff.begin(), buff.size() );
							NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
							CO_RETURN CoroStandardOutcomes::ok;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							nodecpp::clearTimeout( to );
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_read_crh_ok )
							{
								buff.append( frame.ptr, frame.size );
								NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
								CO_RETURN CoroStandardOutcomes::ok;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							nodecpp::clearTimeout( to );
							// TODO: revise order of checking conditions to favour success
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							socket.dataForCommandProcessing.readBuffer.get_ready_data( buff, max_bytes );
							NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes, "{} vs. {}", buff.size(), min_bytes);
							CO_RETURN CoroStandardOutcomes::ok;
						}
					}
				};
				return read_data_awaiter(*this, period, buff, min_bytes, max_bytes);
			}

			auto a_someDataAvailable() { // resumed on data availablity or throws

				struct data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;

					data_awaiter(SocketBase& socket_) : socket( socket_ ) {
					}

					data_awaiter(const data_awaiter &) = delete;
					data_awaiter &operator = (const data_awaiter &) = delete;
	
					~data_awaiter() {
					}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.dataForCommandProcessing.readBuffer.used_size() >= 1;
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						return socket.dataForCommandProcessing.readBuffer.used_size() >= 1;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = 1;
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
						myawaiting = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							bool ret = socket.dataForCommandProcessing.readBuffer.used_size() >= 1;
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
//							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::http_sock_read_data_crh_ok )
							{
								auto frame = threadLocalData.binaryLog->readNextFrame();
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
//								return *(reinterpret_cast<bool*>(frame.ptr));
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
						}
					}
				};
				return data_awaiter(*this);
			}

			::nodecpp::awaitable<CoroStandardOutcomes> a_readUntil( Buffer& b, uint8_t what ) { 
				CoroStandardOutcomes rus = dataForCommandProcessing.readBuffer.read_ready_data_until( b, what );
				if ( rus == CoroStandardOutcomes::ok )
					CO_RETURN rus;
				while ( rus == CoroStandardOutcomes::in_progress )
				{ 
					co_await a_someDataAvailable();
					bool isData = dataForCommandProcessing.readBuffer.used_size() >= 1;
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, isData ); // if we're here
					rus = dataForCommandProcessing.readBuffer.read_ready_data_until( b, what );
					if ( rus == CoroStandardOutcomes::ok )
						CO_RETURN rus;
				}
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, rus == CoroStandardOutcomes::insufficient_buffer, "indeed: {}", (int)rus ); 
				CO_RETURN rus;
			}
#if 0
			::nodecpp::awaitable<bool> a_readUntil( uint32_t period, Buffer& b, uint8_t what ) { 
				CircularByteBuffer::ReadUntilStatus rus = dataForCommandProcessing.readBuffer.read_ready_data_until( b, what ); //;
				while ( rus == CircularByteBuffer::ReadUntilStatus::waiting )
				{ 
					co_await a_someDataAvailable(period);
					bool isData = dataForCommandProcessing.readBuffer.used_size() >= 1;
					if ( isData )
					{
						rus = dataForCommandProcessing.readBuffer.read_ready_data_until( b, what );
						if ( rus == CircularByteBuffer::ReadUntilStatus::done )
							CO_RETURN true;
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, rus != CircularByteBuffer::ReadUntilStatus::insufficient_buffer ); 
					}
					else
						CO_RETURN false;
				}
				CO_RETURN rus == CircularByteBuffer::ReadUntilStatus::done;
			}
#endif // 0
			auto a_write(Buffer& buff) { 

				struct write_data_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					Buffer& buff;
					bool write_ok = false;

					write_data_awaiter(SocketBase& socket_, Buffer& buff_) : socket( socket_ ), buff( buff_ )  {}

					write_data_awaiter(const write_data_awaiter &) = delete;
					write_data_awaiter &operator = (const write_data_awaiter &) = delete;
	
					~write_data_awaiter() {}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.write2( buff );
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							write_ok = socket.write2( buff ); // so far we do it sync TODO: extend implementation for more complex (= requiring really async processing) cases
							return write_ok; // false means waiting (incl. exceptional cases)
						}
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !write_ok ); // otherwise, why are we here?
						nodecpp::initCoroData(awaiting);
						myawaiting = awaiting;
						socket.dataForCommandProcessing.ahd_write.h = awaiting;
					}

					auto await_resume() {
						if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
							throw nodecpp::getCoroException(myawaiting);
					}
				};
				return write_data_awaiter(*this, buff);
			}

			auto a_drain() { 

				struct drain_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;

					drain_awaiter(SocketBase& socket_) : socket( socket_ )  {}

					drain_awaiter(const drain_awaiter &) = delete;
					drain_awaiter &operator = (const drain_awaiter &) = delete;
	
					~drain_awaiter() {}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.dataForCommandProcessing.writeBuffer.empty();
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						return socket.dataForCommandProcessing.writeBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !socket.dataForCommandProcessing.writeBuffer.empty() ); // otherwise, why are we here?
						nodecpp::initCoroData(awaiting);
						myawaiting = awaiting;
						socket.dataForCommandProcessing.ahd_drain = awaiting;
					}

					auto await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_ok, nullptr, 0 );
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_ok )
							{
								return;
							}
							else
								NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false, "UNEXPECTED FRAME TYPE {}", frame.type ); 
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						{
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
						}
					}
				};
				return drain_awaiter(*this);
			}

			auto a_drain(uint32_t period) { 

				struct drain_awaiter {
					std::experimental::coroutine_handle<> myawaiting = nullptr;
					SocketBase& socket;
					uint32_t period;
					nodecpp::Timeout to;

					drain_awaiter(SocketBase& socket_, uint32_t period_) : socket( socket_ ), period( period_ )  {}

					drain_awaiter(const drain_awaiter &) = delete;
					drain_awaiter &operator = (const drain_awaiter &) = delete;
	
					~drain_awaiter() {}

					bool await_ready() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							bool ret = socket.dataForCommandProcessing.writeBuffer.empty();
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, &ret, 1 );
							return ret;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, frame.type == record_and_replay_impl::BinaryLog::FrameType::coro_await_ready_res, "UNEXPECTED FRAME TYPE {}", frame.type ); 
							return *(reinterpret_cast<bool*>(frame.ptr));
						}
						else
#endif // NODECPP_RECORD_AND_REPLAY
						return socket.dataForCommandProcessing.writeBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !socket.dataForCommandProcessing.writeBuffer.empty() ); // otherwise, why are we here?
						nodecpp::initCoroData(awaiting);
						socket.dataForCommandProcessing.ahd_drain = awaiting;
						myawaiting = awaiting;
						to = nodecpp::setTimeoutForAction( awaiting, period );
					}

					::nodecpp::awaitable<::nodecpp::CoroStandardOutcomes> await_resume() {
#ifdef NODECPP_RECORD_AND_REPLAY
						if ( ::nodecpp::threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::recording )
						{
							if ( nodecpp::isCoroException(myawaiting) )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_except, nullptr, 0 );
								throw nodecpp::getCoroException(myawaiting);
							}
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_event_crh_timeout, nullptr, 0 );
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							::nodecpp::threadLocalData.binaryLog->addFrame( record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_ok, nullptr, 0 );
							CO_RETURN CoroStandardOutcomes::ok;
						}
						else if ( threadLocalData.binaryLog != nullptr && threadLocalData.binaryLog->mode() == record_and_replay_impl::BinaryLog::Mode::replaying )
						{
							auto frame = threadLocalData.binaryLog->readNextFrame();
							if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_except )
								throw nodecpp::getCoroException(myawaiting);
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							else if ( frame.type == record_and_replay_impl::BinaryLog::FrameType::sock_drain_crh_ok )
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
							// TODO: revise order of checking conditions to favour success
							if ( myawaiting != nullptr && nodecpp::isCoroException(myawaiting) )
								throw nodecpp::getCoroException(myawaiting);
							if (  myawaiting != nullptr && nodecpp::getCoroStatus(myawaiting) == CoroStandardOutcomes::timeout )
							{
								//std::exception e;
								//throw e; // TODO: switch to nodecpp exceptions
								CO_RETURN CoroStandardOutcomes::timeout;
							}
							CO_RETURN CoroStandardOutcomes::ok;
						}
					}
				};
				return drain_awaiter(*this, period);
			}

#else
			void forceReleasingAllCoroHandles() {}
#endif // NODECPP_NO_COROUTINES


			///////////////////////////////////////////////////////////////////
		private:
			class EventEmitterSupportingListeners<event::Close, SocketListener, &SocketListener::onClose> eClose;
			class EventEmitterSupportingListeners<event::Connect, SocketListener, &SocketListener::onConnect> eConnect;
			class EventEmitterSupportingListeners<event::Data, SocketListener, &SocketListener::onData> eData;
			class EventEmitterSupportingListeners<event::Drain, SocketListener, &SocketListener::onDrain> eDrain;
			class EventEmitterSupportingListeners<event::End, SocketListener, &SocketListener::onEnd> eEnd;
			class EventEmitterSupportingListeners<event::Error, SocketListener, &SocketListener::onError> eError;
			class EventEmitterSupportingListeners<event::Accepted, SocketListener, &SocketListener::onAccepted> eAccepted;

			nodecpp::vector<nodecpp::owning_ptr<SocketListener>> ownedListeners;

		public:
			void emitClose(bool hadError) {
				unref();
				//this->dataForCommandProcessing.id = 0;
				//handler may release, put virtual onClose first.
				eClose.emit(hadError);
			}

			// not in node.js
			void emitAccepted() {
				eAccepted.emit();
			}

			void emitConnect() {
				eConnect.emit();
			}

			void emitData(const Buffer& buffer) {
				_bytesRead += buffer.size();
				eData.emit<const Buffer&>(buffer);
			}

			void emitDrain() {
				eDrain.emit();
			}

			void emitEnd() {
				eEnd.emit();
			}

			void emitError(Error& err) {
				//this->dataForCommandProcessing.id = 0;
				eError.emit(err);
			}

			void connect(uint16_t port, const char* ip, std::function<void()> cb) {
				once(event::connect, std::move(cb));
				connect(port, ip);
			}

			bool write(Buffer& buff, std::function<void()> cb) {
				bool b = write(buff);
				if(!b)
					once(event::drain, std::move(cb));

				return b;
			}



			void on_( nodecpp::soft_ptr<SocketListener> l) {
				eClose.on(l);
				eConnect.on(l);
				eData.on(l);
				eDrain.on(l);
				eError.on(l);
				eEnd.on(l);
				eAccepted.on(l);
			}

			void once_( nodecpp::soft_ptr<SocketListener> l) {
				eClose.once(l);
				eConnect.once(l);
				eData.once(l);
				eDrain.once(l);
				eError.once(l);
				eEnd.once(l);
				eAccepted.once(l);
			}

			void on( nodecpp::owning_ptr<SocketListener> l) {
				nodecpp::soft_ptr<SocketListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				on_( std::move(sl) );
			}

			void once( nodecpp::owning_ptr<SocketListener> l) {
				nodecpp::soft_ptr<SocketListener> sl( l );
				ownedListeners.emplace_back( std::move( l ) );
				once_( std::move(sl) );
			}

			void on( string_literal name, event::Close::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,  name == string_literal(event::Close::name) );
				eClose.on(std::move(cb));
			}
			void on( string_literal name, event::Data::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,  name == string_literal(event::Data::name) );
				eData.on(std::move(cb));
			}
			void on(string_literal name, event::Error::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,  name == string_literal(event::Error::name) );
				eError.on(std::move(cb));
			}
			void on( string_literal name, event::Connect::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if ( name == string_literal(event::Drain::name) )
					eDrain.on(std::move(cb));
				else if ( name == string_literal(event::Connect::name) )
					eConnect.on(std::move(cb));
				else if ( name == string_literal(event::End::name) )
					eEnd.on(std::move(cb));
				else if ( name == string_literal(event::Accepted::name) )
					eAccepted.on(std::move(cb));
				else
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false);
			}

			void once( string_literal name, event::Close::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,  name == string_literal(event::Close::name) );
				eClose.once(std::move(cb));
			}
			void once( string_literal name, event::Data::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical,  name == string_literal(event::Data::name) );
				eData.once(std::move(cb));
			}
			void once(string_literal name, event::Error::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, name == string_literal(event::Error::name));
				eError.once(std::move(cb));
			}
			void once( string_literal name, event::Connect::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if (name == string_literal(event::Drain::name))
					eDrain.once(std::move(cb));
				else if (name == string_literal(event::Connect::name))
					eConnect.once(std::move(cb));
				else if (name == string_literal(event::End::name))
					eEnd.once(std::move(cb));
				else if (name == string_literal(event::Accepted::name))
					eAccepted.once(std::move(cb));
				else
					NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false);
			}

			template<class EV>
			void on( EV, typename EV::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.on(std::move(cb)); }
				else NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false);
			}

			template<class EV>
			void once( EV, typename EV::callback cb NODECPP_MAY_EXTEND_TO_THIS) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.once(std::move(cb)); }
				else NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, false );
			}

		};

		template<class DataParentT>
		class Socket : public SocketBase, public ::nodecpp::DataParent<DataParentT>
		{
		public:
			using DataParentType = DataParentT;
			Socket() {};
			Socket(DataParentT* dataParent ) : SocketBase(), ::nodecpp::DataParent<DataParentT>( dataParent ) {};
			virtual ~Socket() {}
		};

		template<>
		class Socket<void> : public SocketBase
		{
		public:
			using DataParentType = void;
			Socket() {};
			virtual ~Socket() {}
		};

		/*template<class T, class ... Types>
		static
			nodecpp::owning_ptr<T> createSocket(Types&& ... args) {
			static_assert( std::is_base_of< SocketBase, T >::value );
			nodecpp::owning_ptr<T> ret = nodecpp::make_owning<T>(::std::forward<Types>(args)...);
			ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<T>(), &(*ret));
			return ret;
		}*/

		/*template<class Node, class SocketT, class ... Types>
		static
			nodecpp::owning_ptr<SocketT> createSocket(Types&& ... args) {
			static_assert( std::is_base_of< SocketBase, SocketT >::value );
			nodecpp::owning_ptr<SocketT> ret = nodecpp::make_owning<SocketT>(::std::forward<Types>(args)...);
			ret->registerMeAndAcquireSocket<Node, SocketT>(ret);
			ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*ret));
			return ret;
		}*/

		template<class SocketT = SocketBase, class ... Types>
		static
			nodecpp::owning_ptr<SocketT> createSocket(Types&& ... args) {
			static_assert( std::is_base_of< SocketBase, SocketT >::value );
			nodecpp::owning_ptr<SocketT> ret = nodecpp::make_owning<SocketT>(::std::forward<Types>(args)...);
			ret->registerMeAndAcquireSocket();
			ret->dataForCommandProcessing.userHandlers.from(SocketBase::DataForCommandProcessing::userHandlerClassPattern.getPatternForApplying<SocketT>(), &(*ret));
			return ret;
		}

	} //namespace net

} //namespace nodecpp

#endif //SOCKET_COMMON_H
