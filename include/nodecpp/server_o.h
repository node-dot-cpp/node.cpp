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

#include "server_t_base.h"

namespace nodecpp {

	namespace net {

		class ServerO : public ServerTBase
		{
		public:
			ServerO();
			virtual ~ServerO() {}

			virtual void onClose(bool hadError) {}
			virtual void onConnection(SocketBase* socket) {} // NOTE: strange name is an MS compiler bug temporry workaround. TODO: go back to a reasonable nabe as soon as MS fixes its bug
			virtual void onListening(size_t id, Address addr) {} // NOTE: strange name is an MS compiler bug temporry workaround. TODO: go back to a reasonable nabe as soon as MS fixes its bug
			virtual void onError(Error& err) {}

			virtual soft_ptr<SocketBase> makeSocket(OpaqueSocketData& sdata) = 0;
			void listen(uint16_t port, const char* ip, int backlog);
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

			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(this->node->*(Initializer::onClose))(nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>>(this),b); 
				else
					ServerO::onClose(b);
			}
			void onConnection(SocketBase* socket) override
			{ 
				if constexpr ( Initializer::onConnection != nullptr )
					(this->node->*(Initializer::onConnection))(nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>>(this), socket); 
				else
					ServerO::onConnection(socket);
			}
			void onListening(size_t id, Address addr) override
			{ 
				if constexpr ( Initializer::onListening != nullptr )
					(this->node->*(Initializer::onListening))(nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>>(this), id, addr); 
				else
					ServerO::onListening(id, addr);
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(this->node->*(Initializer::onError))(nodecpp::safememory::soft_ptr<ServerOUserBase<Node, Extra>>(this),e);
				else
					ServerO::onError(e);
			}
		};

		/*template<class Node, class Initializer>
		class ServerN2<Node, Initializer, void> : public ServerO
		{
			Node* node;
			static_assert( Initializer::onConnection != nullptr );
		public:
			ServerN2(Node* node_) { node = node_;}
			void* getExtra() { return nullptr; }

			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(node->*(Initializer::onClose))(this->getExtra(),b); 
				else
					ServerO::onClose(b);
			}
#if 1 //[+++] revision required
			void onConnectionX(SocketBase* socket) override
			{ 
				if constexpr ( Initializer::onConnection != nullptr )
					(node->*(Initializer::onConnection))(this->getExtra()); 
				else
					ServerO::onConnectionX(socket);
			}
			void onListeningX(size_t id, Address addr) override
			{ 
				if constexpr ( Initializer::onListening != nullptr )
					(node->*(Initializer::onListening))(this->getExtra()); 
				else
					ServerO::onListeningX(id, addr);
			}
#endif
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(node->*(Initializer::onError))(this->getExtra(),e);
				else
					ServerO::onError(e);
			}
		};*/


		template<class Node, class Socket, class Extra, class ... Handlers>
		class ServerN : public ServerN2<Node, ServerOInitializer<Handlers...>, Extra>
		{
			protected:
				MultiOwner<typename Socket::StorableType> socketList;
		public:
			ServerN(Node* node_) : ServerN2<Node, ServerOInitializer<Handlers...>, Extra>( node_ ) {}
			soft_ptr<SocketBase> makeSocket(OpaqueSocketData& sdata) { 
				//return new Socket( static_cast<Node*>(this->node), sdata ); 
				owning_ptr<Socket> sock_ = make_owning<Socket>(static_cast<Node*>(this->node), sdata);
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
