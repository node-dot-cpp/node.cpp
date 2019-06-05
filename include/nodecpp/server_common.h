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

#include "common.h"
#include "event.h"
#include "net_common.h"

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


				using userDefListenHandlerFnT = nodecpp::awaitable<void> (*)(void*, size_t, nodecpp::net::Address);
				using userDefConnectionHandlerFnT = nodecpp::awaitable<void> (*)(void*, nodecpp::safememory::soft_ptr<net::SocketBase>);
				using userDefCloseHandlerFnT = nodecpp::awaitable<void> (*)(void*, bool);
				using userDefErrorHandlerFnT = nodecpp::awaitable<void> (*)(void*, Error&);

				template<class FnT>
				struct UserDefHandlers
				{
					friend void handleListenEvent(size_t, nodecpp::net::Address);
					struct HandlerInstance
					{
						FnT handler = nullptr;
						void *object = nullptr;
					};
					std::vector<HandlerInstance> handlers;

					bool willHandle() { return handlers.size(); }
					template<class ObjectT>
					void add( ObjectT* object, FnT handler )
					{
						for (auto h : handlers)
							NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, h.handler != handler || h.object != object, "already added" );
						HandlerInstance inst;
						inst.object = object;
						inst.handler = handler;
						handlers.push_back(inst);
					}
					template<class ObjectT>
					void remove(ObjectT* object, FnT handler)
					{
						for (size_t i=0; i<handlers.size(); ++i)
							if (handlers[i].handler == handler && handlers[i].object == object)
							{
								handlers.erase(handlers.begin() + i);
								return;
							}
					}
				};

				UserDefHandlers<userDefListenHandlerFnT> userDefListenHandlers;
				UserDefHandlers<userDefConnectionHandlerFnT> userDefConnectionHandlers;
				UserDefHandlers<userDefCloseHandlerFnT> userDefCloseHandlers;
				UserDefHandlers<userDefErrorHandlerFnT> userDefErrorHandlers;

				bool isListenEventHandler() { return userDefListenHandlers.willHandle(); }
				void handleListenEvent(size_t id, nodecpp::net::Address address) { for (auto h : userDefListenHandlers.handlers) h.handler(h.object, id, address); }

				bool isConnectionEventHandler() { return userDefConnectionHandlers.willHandle(); }
				void handleConnectionEvent(nodecpp::safememory::soft_ptr<net::SocketBase> socket) { for (auto h : userDefConnectionHandlers.handlers) h.handler(h.object, socket); }

				bool isCloseEventHandler() { return userDefCloseHandlers.willHandle(); }
				void handleCloseEvent(bool hasError) { for (auto h : userDefCloseHandlers.handlers) h.handler(h.object, hasError); }

				bool isErrorEventHandler() { return userDefErrorHandlers.willHandle(); }
				void handleErrorEvent(Error& e) { for (auto h : userDefErrorHandlers.handlers) h.handler(h.object, e); }

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

			enum class Handler { Listen, Connection, Close, Error };
			template<Handler handler, auto memmberFn, class ObjectT>
			void addHandler(ObjectT* object )
			{
				if constexpr ( handler == Handler::Listen )
				{
					dataForCommandProcessing.userDefListenHandlers.add(object, &DataForCommandProcessing::listenHandler<ObjectT, memmberFn>);
				} 
				else if constexpr ( handler == Handler::Connection )
				{
					dataForCommandProcessing.userDefConnectionHandlers.add(object, &DataForCommandProcessing::connectionHandler<ObjectT, memmberFn>);
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
				if constexpr (handler == Handler::Listen)
				{
					dataForCommandProcessing.userDefListenHandlers.remove(object, &DataForCommandProcessing::listenHandler<ObjectT, memmberFn>);
				}
				else if constexpr (handler == Handler::Connection)
				{
					dataForCommandProcessing.userDefConnectionHandlers.remove(object, &DataForCommandProcessing::connectionHandler<ObjectT, memmberFn>);
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
