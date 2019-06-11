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
				nodecpp::safememory::soft_this_ptr<ServerBase> myThis;
			public:
			class DataForCommandProcessing {
			public:
				size_t index;
				bool refed = false;
				short fdEvents = 0;
				//SOCKET osSocket = INVALID_SOCKET;
				unsigned long long osSocket = 0;

				DataForCommandProcessing() {}
				DataForCommandProcessing(const DataForCommandProcessing& other) = delete;
				DataForCommandProcessing& operator=(const DataForCommandProcessing& other) = delete;

				DataForCommandProcessing(DataForCommandProcessing&& other) = default;
				DataForCommandProcessing& operator=(DataForCommandProcessing&& other) = default;

				Address localAddress;

				awaitable_handle_data ahd_listen;
				struct awaitable_connection_handle_data : public awaitable_handle_data
				{
					soft_ptr<SocketBase> sock;
				};
				awaitable_connection_handle_data ahd_connection;

				struct UserHandlers
				{
				private:
					void* defaultObjectPtr = nullptr;

				public:
					using userDefListenHandlerFnT = nodecpp::awaitable<void> (*)(void*, size_t, nodecpp::net::Address);
					using userDefConnectionHandlerFnT = nodecpp::awaitable<void> (*)(void*, nodecpp::safememory::soft_ptr<net::SocketBase>);
					using userDefCloseHandlerFnT = nodecpp::awaitable<void> (*)(void*, bool);
					using userDefErrorHandlerFnT = nodecpp::awaitable<void> (*)(void*, Error&);

					UserDefHandlers<userDefListenHandlerFnT> userDefListenHandlers;
					UserDefHandlers<userDefConnectionHandlerFnT> userDefConnectionHandlers;
					UserDefHandlers<userDefCloseHandlerFnT> userDefCloseHandlers;
					UserDefHandlers<userDefErrorHandlerFnT> userDefErrorHandlers;

					template<class T> using userListenMemberHandler = nodecpp::awaitable<void> (T::*)(size_t, nodecpp::net::Address);
					template<class T> using userConnectionMemberHandler = nodecpp::awaitable<void> (T::*)(nodecpp::safememory::soft_ptr<net::SocketBase>);
					template<class T> using userCloseMemberHandler = nodecpp::awaitable<void> (T::*)(bool);
					template<class T> using userErrorMemberHandler = nodecpp::awaitable<void> (T::*)(Error&);

					template<class ObjectT, userListenMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> listenHandler( void* objPtr, size_t id, nodecpp::net::Address addr )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(id, addr);
						co_return;
					}

					template<class ObjectT, userConnectionMemberHandler<ObjectT> MemberFnT>
					static nodecpp::awaitable<void> connectionHandler( void* objPtr, nodecpp::safememory::soft_ptr<net::SocketBase> socket )
					{
						((reinterpret_cast<ObjectT*>(objPtr))->*MemberFnT)(socket);
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

					template<class T>
					void setPointerToMe(T* objPtr)
					{
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objPtr != nullptr);
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjectPtr == nullptr);
						defaultObjectPtr = objPtr;
					}
					void* ptrToMe()
					{
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjectPtr != nullptr);
						return defaultObjectPtr;
					}

					enum class Handler { Listen, Connection, Close, Error };
					template<Handler handler, auto memmberFn, class ObjectT>
					void addHandler(ObjectT* object = nullptr)
					{
						if constexpr (handler == Handler::Listen)
						{
							userDefListenHandlers.add(object, &DataForCommandProcessing::UserHandlers::listenHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connection)
						{
							userDefConnectionHandlers.add(object, &DataForCommandProcessing::UserHandlers::connectionHandler<ObjectT, memmberFn>);
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
					void removeHandler(ObjectT* object = nullptr)
					{
						if constexpr (handler == Handler::Listen)
						{
							userDefListenHandlers.remove(object, &DataForCommandProcessing::UserHandlers::listenHandler<ObjectT, memmberFn>);
						}
						else if constexpr (handler == Handler::Connection)
						{
							userDefConnectionHandlers.remove(object, &DataForCommandProcessing::UserHandlers::connectionHandler<ObjectT, memmberFn>);
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

					void from( const UserHandlers& patternUH, void* defaultObjPtr )
					{
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjectPtr == nullptr);
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, defaultObjPtr != nullptr);
						userDefListenHandlers.from(patternUH.userDefListenHandlers, defaultObjPtr);
						userDefConnectionHandlers.from(patternUH.userDefConnectionHandlers, defaultObjPtr);
						userDefCloseHandlers.from(patternUH.userDefCloseHandlers, defaultObjPtr);
						userDefErrorHandlers.from(patternUH.userDefErrorHandlers, defaultObjPtr);
					}
				};
				UserHandlers userHandlers;
				UserHandlers* userHandlersPtr;
				thread_local static std::map<std::type_info, UserHandlers> userHandlerClassPattern; // TODO: consider using thread-local allocator

				/*bool isListenEventHandler() { return userHandlers.userDefListenHandlers.willHandle(); }
				void handleListenEvent(size_t id, nodecpp::net::Address address) { 
					for (auto h : userHandlers.userDefListenHandlers.handlers) 
						if (h.object)
							h.handler(h.object, id, address);
						else
							h.handler(userHandlers.ptrToMe(), id, address);
				}

				bool isConnectionEventHandler() { return userHandlers.userDefConnectionHandlers.willHandle(); }
				void handleConnectionEvent(nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
					for (auto h : userHandlers.userDefConnectionHandlers.handlers) 
						if ( h.object )
							h.handler(h.object, socket);
						else
							h.handler(userHandlers.ptrToMe(), socket);
				}*/
				bool isListenEventHandler() { return userHandlersPtr->userDefListenHandlers.willHandle(); }
				void handleListenEvent(size_t id, nodecpp::net::Address address) {
					for (auto h : userHandlersPtr->userDefListenHandlers.handlers)
						if (h.object)
							h.handler(h.object, id, address);
						else
							h.handler(userHandlers.ptrToMe(), id, address);
				}

				bool isConnectionEventHandler() { return userHandlersPtr->userDefConnectionHandlers.willHandle(); }
				void handleConnectionEvent(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
					for (auto h : userHandlersPtr->userDefConnectionHandlers.handlers)
						if (h.object)
							h.handler(h.object, socket);
						else
							h.handler(userHandlers.ptrToMe(), socket);
				}

				bool isCloseEventHandler() { return userHandlers.userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(bool hasError) { for (auto h : userHandlers.userDefCloseHandlers.handlers) h.handler(h.object, hasError); }

				bool isErrorEventHandler() { return userHandlers.userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(Error& e) { for (auto h : userHandlers.userDefErrorHandlers.handlers) h.handler(h.object, e); }
			};
			DataForCommandProcessing dataForCommandProcessing;

		protected:
//			uint16_t localPort = 0;

//			size_t id = 0;
			enum State { UNINITIALIZED = 0, LISTENING, CLOSED } state = UNINITIALIZED;

		protected:
			void registerServerByID(NodeBase* node, soft_ptr<net::ServerBase> t, int typeId);

		public:
			NodeBase* node = nullptr;

		public:
			ServerBase() {}
			~ServerBase() {reportBeingDestructed();}

			const Address& address() const { return dataForCommandProcessing.localAddress; }
			void close();

			bool listening() const { return state == LISTENING; }
			void ref();
			void unref();
			void reportBeingDestructed();

			void listen(uint16_t port, const char* ip, int backlog);

			template<DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn, class ObjectT>
			void addHandler(ObjectT* object )
			{
				dataForCommandProcessing.userHandlers.addHandler<handler, memmberFn, ObjectT>(object);
			}
			template<DataForCommandProcessing::UserHandlers::Handler handler, auto memmberFn, class ObjectT>
			void removeHandler(ObjectT* object)
			{
				dataForCommandProcessing.userHandlers.removeHandler<handler, memmberFn, ObjectT>(object);
			}

		};

		//TODO don't use naked pointers, think
		//template<class T>
		//T* createServer() {
		//	return new T();
		//}
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
