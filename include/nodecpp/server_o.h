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

#ifndef SERVER_O_H
#define SERVER_O_H

#include "server_common.h"

namespace nodecpp {

	namespace net {

		class ServerO : public ServerBase
		{
		public:
			ServerO();
			virtual ~ServerO() {}

			virtual void onClose(bool hadError) {}
			virtual nodecpp::awaitable<void> onConnection(nodecpp::safememory::soft_ptr<SocketBase> socket) {co_return;}
			virtual nodecpp::awaitable<void> onListening(size_t id, Address addr) {co_return;}
			virtual void onError(Error& err) {}

			virtual soft_ptr<SocketBase> makeSocket(OpaqueSocketData& sdata) = 0;
			void listen(uint16_t port, const char* ip, int backlog);

			auto a_listen(uint16_t port, const char* ip, int backlog) { 

				struct listen_awaiter {
					ServerO& server;

					std::experimental::coroutine_handle<> who_is_awaiting;

					listen_awaiter(ServerO& server_) : server( server_ ) {}

					listen_awaiter(const listen_awaiter &) = delete;
					listen_awaiter &operator = (const listen_awaiter &) = delete;
	
					~listen_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						who_is_awaiting = awaiting;
						server.dataForCommandProcessing.ahd_listen.h = who_is_awaiting;
					}

					auto await_resume() {
						if ( server.dataForCommandProcessing.ahd_listen.is_exception )
						{
							server.dataForCommandProcessing.ahd_listen.is_exception = false; // now we will throw it and that's it
							throw server.dataForCommandProcessing.ahd_listen.exception;
						}
					}
				};
				listen( port, ip, backlog );
				return listen_awaiter(*this);
			}

			template<class SocketT>
			auto a_connection(nodecpp::safememory::soft_ptr<SocketT>& socket) { 

				struct connection_awaiter {
					ServerO& server;
					nodecpp::safememory::soft_ptr<SocketT>& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					connection_awaiter(ServerO& server_, nodecpp::safememory::soft_ptr<SocketT>& socket_) : server( server_ ), socket( socket_ ) {}

					connection_awaiter(const connection_awaiter &) = delete;
					connection_awaiter &operator = (const connection_awaiter &) = delete;
	
					~connection_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						who_is_awaiting = awaiting;
						server.dataForCommandProcessing.ahd_connection.h = who_is_awaiting;
					}

					auto await_resume() {
						if ( server.dataForCommandProcessing.ahd_connection.is_exception )
						{
							server.dataForCommandProcessing.ahd_connection.is_exception = false; // now we will throw it and that's it
							throw server.dataForCommandProcessing.ahd_connection.exception;
						}
						if constexpr ( std::is_same<SocketT, SocketBase>::value )
							socket = server.dataForCommandProcessing.ahd_connection.sock;
						else
							socket = nodecpp::safememory::soft_ptr_reinterpret_cast<SocketT>(server.dataForCommandProcessing.ahd_connection.sock);
					}
				};
				return connection_awaiter(*this, socket);
			}

		};
		
		template<auto x>
		struct OnCloseSO {};

		template<auto x>
		struct OnConnectionSO {};

		template<auto x>
		struct OnListeningSO {};

		template<auto x>
		struct OnErrorSO {};

		template<typename ... args>
		struct ServerOInitializer;

		//partial template specializations:
		template<auto F, typename ... args>
		struct ServerOInitializer<OnCloseSO<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onClose = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnConnectionSO<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onConnection = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnListeningSO<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onListening = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnErrorSO<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onError = F;
		};

		template<>
		struct ServerOInitializer<> {
			static constexpr auto onConnection = nullptr;
			static constexpr auto onClose = nullptr;
			static constexpr auto onListening = nullptr;
			static constexpr auto onError = nullptr;
		};


		template<class Node, class Extra>
		class ServerOUserBase : public ServerO
		{
		protected:
			Node* node;
			Extra extra;
		public:
			ServerOUserBase(Node* node_) { node = node_;}
			virtual ~ServerOUserBase() {}
			Extra* getExtra() { return &extra; }
		};


		template<class Node>
		class ServerOUserBase<Node, void> : public ServerO
		{
			Node* node;
		public:
			ServerOUserBase(Node* node_) { node = node_;}
			void* getExtra() { return nullptr; }
		};
		template<class Node, class Initializer, class Extra>
		class ServerN2 : public ServerOUserBase<Node, Extra>
		{
		public:
			ServerN2(Node* node_) : ServerOUserBase<Node, Extra>(node_) {}
			virtual ~ServerN2() {}

			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
				{
					nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<ServerOUserBase<Node, Extra>>(this);
					(this->node->*(Initializer::onClose))(ptr2this,b); 
				}
				else
					ServerO::onClose(b);
			}
			nodecpp::awaitable<void> onConnection(nodecpp::safememory::soft_ptr<SocketBase> socket) override
			{ 
				if constexpr ( Initializer::onConnection != nullptr )
				{
					nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<ServerOUserBase<Node, Extra>>(this);
					(this->node->*(Initializer::onConnection))(ptr2this, socket); 
				}
				else
					ServerO::onConnection(socket);
				co_return;
			}
			nodecpp::awaitable<void> onListening(size_t id, Address addr) override
			{ 
				if constexpr ( Initializer::onListening != nullptr )
				{
					nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<ServerOUserBase<Node, Extra>>(this);
					(this->node->*(Initializer::onListening))(ptr2this, id, addr); 
				}
				else
					ServerO::onListening(id, addr);
				co_return;
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
				{
					nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<ServerOUserBase<Node, Extra>>(this);
					(this->node->*(Initializer::onError))(ptr2this,e);
				}
				else
					ServerO::onError(e);
			}
		};


		template<class Node, class Socket, class Extra, class ... Handlers>
		class ServerN : public ServerN2<Node, ServerOInitializer<Handlers...>, Extra>
		{
			protected:
				MultiOwner<typename Socket::StorableType> socketList;
		public:
			ServerN(Node* node_) : ServerN2<Node, ServerOInitializer<Handlers...>, Extra>( node_ ) {}
			virtual ~ServerN() {}
			soft_ptr<SocketBase> makeSocket(OpaqueSocketData& sdata) { 
				//return new Socket( static_cast<Node*>(this->node), sdata ); 
				owning_ptr<Socket> sock_ = nodecpp::net::createSocket<Socket>(static_cast<Node*>(this->node), sdata);
				soft_ptr<Socket> retSock( sock_ );
				this->socketList.add( std::move(sock_) );
				return retSock;
			}		
			void removeSocket( soft_ptr<typename Socket::StorableType> sock ) {
				this->socketList.removeAndDelete( sock );
			}
			size_t getSockCount() {return this->socketList.getCount();}
		};
	} //namespace net
} //namespace nodecpp

#endif //SERVER_O_H
