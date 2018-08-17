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
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
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
			ServerO() {}
			virtual ~ServerO() {}

			virtual void onClose(bool hadError) {}
			virtual void onConnection(Socket* socket) {}
			virtual void onListening() {}
			virtual void onError(Error& err) {}

			virtual Socket* makeSocket() {
				return new Socket();
			}

			void listen(uint16_t port, const char* ip, int backlog);

		};

		
		template<auto x>
		struct OnClose {};

		template<auto x>
		struct OnConnection {};

		template<auto x>
		struct OnListening {};

		template<auto x>
		struct OnError {};

		template<typename ... args>
		struct ServerOInitializer;

		//partial template specializations:
		template<auto F, typename ... args>
		struct ServerOInitializer<OnClose<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onClose = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnConnection<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onConnection = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnListening<F>, args...>
		: public ServerOInitializer<args...> {
			static constexpr auto onListening = F;
		};

		template<auto F, typename ... args>
		struct ServerOInitializer<OnError<F>, args...>
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


		template<class Node, class Initializer, class Extra>
		class ServerN2 : public ServerO
		{
			Node* node;
			Extra extra;
		public:
			ServerN2(Node* node_) { node = node_;}
			Extra* getExtra() { return &extra; }

			void onConnection() override
			{ 
				if constexpr ( Initializer::onConnection != nullptr )
					(node->*(Initializer::onConnection))(this->getExtra()); 
				else
					ServerO::onConnection();
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(node->*(Initializer::onClose))(this->getExtra(),b); 
				else
					ServerO::onClose(b);
			}
			void onListening(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onListening != nullptr )
					(node->*(Initializer::onListening))(this->getExtra(),b); 
				else
					ServerO::onListening(b);
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(node->*(Initializer::onError))(this->getExtra(),e);
				else
					ServerO::onError(e);
			}
		};

		template<class Node, class Initializer>
		class ServerN2<Node, Initializer, void> : public ServerO
		{
			Node* node;
			static_assert( Initializer::onConnection != nullptr );
		public:
			ServerN2(Node* node_) { node = node_;}
			void* getExtra() { return nullptr; }

			void onConnection() override
			{ 
				if constexpr ( Initializer::onConnection != nullptr )
					(node->*(Initializer::onConnection))(this->getExtra()); 
				else
					ServerO::onConnection();
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(node->*(Initializer::onClose))(this->getExtra(),b); 
				else
					ServerO::onClose(b);
			}
			void onListening(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onListening != nullptr )
					(node->*(Initializer::onListening))(this->getExtra(),b); 
				else
					ServerO::onListening(b);
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(node->*(Initializer::onError))(this->getExtra(),e);
				else
					ServerO::onError(e);
			}
		};


		template<class Node, class Extra, class ... Handlers>
		class ServerN : public ServerN2<Node, ServerOInitializer<Handlers...>, Extra>
		{
		public:
			ServerN(Node* node_) : ServerN2<Node, ServerOInitializer<Handlers...>, Extra>( node_ ) {}
		
		};
	} //namespace net
} //namespace nodecpp

#endif //SERVER_O_H
