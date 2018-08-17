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

#ifndef SOCKET_O_H
#define SOCKET_O_H

#include <stdio.h>
#include "net_common.h"

namespace nodecpp {

	namespace net {

		class SocketO : public SocketBase {

		public:
			SocketO() {}

			SocketO(const SocketO&) = delete;
			SocketO& operator=(const SocketO&) = delete;

			SocketO(SocketO&&) = default;
			SocketO& operator=(SocketO&&) = default;

			virtual ~SocketO() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onError(Error& err) {}

			void connect(uint16_t port, const char* ip);
			SocketO& setNoDelay(bool noDelay = true);
			SocketO& setKeepAlive(bool enable = false);
		};

		
		template<auto x>
		struct OnClose {};

		template<auto x>
		struct OnConnect {};

		template<auto x>
		struct OnData {};

		template<auto x>
		struct OnDrain {};

		template<auto x>
		struct OnError {};

		template<auto x>
		struct OnEnd {};

		template<typename ... args>
		struct SocketOInitializer2;

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnClose<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onClose = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnConnect<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onConnect = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnData<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onData = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnDrain<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onDrain = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnError<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onError = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnEnd<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onEnd = F;
		};

		//create similar partial specializations for all the args

		//partial template specialiazation to end recursion
		template<>
		struct SocketOInitializer2<> {
			static constexpr auto onConnect = nullptr;
			static constexpr auto onClose = nullptr;
			static constexpr auto onData = nullptr;
			static constexpr auto onDrain = nullptr;
			static constexpr auto onError = nullptr;
			static constexpr auto onEnd = nullptr;
		};

	//	template <class T, class M> constexpr M get_member_type(M T:: *);
	//	#define GET_TYPE_OF(mem) decltype(get_member_type((mem)))

		template<class Node, class Initializer, class Extra>
		class SocketN2 : public SocketO
		{
	#if 0
			static constexpr bool is_void_extra = std::is_same< Extra, void >::value;
			static_assert( Initializer::onConnect != nullptr );
			static_assert( !is_void_extra );
			static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)() >::value );
			static_assert( Initializer::onConnect == nullptr || (is_void_extra && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value) || ( (!is_void_extra) && std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value ) );
	#endif
			Node* node;
			Extra extra;
		public:
			SocketN2(Node* node_) { node = node_;}
			Extra* getExtra() { return &extra; }

			void onConnect() override
			{ 
				if constexpr ( Initializer::onConnect != nullptr )
				{
	//				static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value );
					(node->*(Initializer::onConnect))(this->getExtra()); 
				}
				else
				{
	//		static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)() >::value );
					SocketO::onConnect();
				}
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(node->*(Initializer::onClose))(this->getExtra(),b); 
				else
					SocketO::onClose(b);
			}
			void onData(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onData != nullptr )
					(node->*(Initializer::onData))(this->getExtra(),b); 
				else
					SocketO::onData(b);
			}
			void onDrain() override
			{
				if constexpr ( Initializer::onDrain != nullptr )
					(node->*(Initializer::onDrain))(this->getExtra());
				else
					SocketO::onDrain();
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(node->*(Initializer::onError))(this->getExtra(),e);
				else
					SocketO::onError(e);
			}
			void onEnd() override
			{
				if constexpr ( Initializer::onEnd != nullptr )
					(node->*(Initializer::onEnd))(this->getExtra());
				else
					SocketO::onEnd();
			}
		};

		template<class Node, class Initializer>
		class SocketN2<Node, Initializer, void> : public SocketO
		{
			Node* node;
	//		static constexpr auto x = void (Node::*onConnect)(const void*);
//			typename std::remove_reference<decltype((Initializer::onConnect))>::type x;
	//		typename void (Node::*)(const void*) y;
	#if 1
			static_assert( Initializer::onConnect != nullptr );
	//		static_assert( std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype((Initializer::onConnect))>::type, typename std::remove_cv<void (Node::*)(const void*)>::type >::value );
	//		static_assert( std::is_same< typename std::remove_reference<typename std::remove_cv<decltype((Initializer::onConnect))>::type>::type, typename std::remove_reference<typename std::remove_cv<void (Node::*)(const void*)>::type>::type >::value );
	//		static_assert( std::is_same< typename std::remove_cv<decltype(x)>::type, typename std::remove_reference<decltype((Initializer::onConnect))>::type >::value );
	//		static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const void*) >::value );
	//		static_assert( Initializer::onConnect == nullptr || typeid(Initializer::onConnect).hash_code() == typeid(void (Node::*)(const void*)).hash_code() );
	#endif
		public:
			SocketN2(Node* node_) /*: x(nullptr), y(nullptr)*/ { node = node_;}
			void* getExtra() { return nullptr; }

			void onConnect() override
			{ 
	//assert( Initializer::onConnect == nullptr || typeid(Initializer::onConnect).hash_code() == typeid(void (Node::*)(const void*)).hash_code() );
				//printf("type 1 = \'%s\', hash = 0x%zx\n", typeid(x).name(), typeid(x).hash_code() );
				//printf("type 2 = \'%s\', hash = 0x%zx\n", typeid(void (Node::*)(const void*)).name(), typeid(x).hash_code() );
				//printf("type 3 = \'%s\', hash = 0x%zx\n", typeid(Initializer::onConnect).name(), typeid(x).hash_code() );
				if constexpr ( Initializer::onConnect != nullptr )
					(node->*(Initializer::onConnect))(this->getExtra()); 
				else
					SocketO::onConnect();
			}
			void onClose(bool b) override
			{ 
				if constexpr ( Initializer::onClose != nullptr )
					(node->*(Initializer::onClose))(this->getExtra(),b); 
				else
					SocketO::onClose(b);
			}
			void onData(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onData != nullptr )
					(node->*(Initializer::onData))(this->getExtra(),b); 
				else
					SocketO::onData(b);
			}
			void onDrain() override
			{
				if constexpr ( Initializer::onDrain != nullptr )
					(node->*(Initializer::onDrain))(this->getExtra());
				else
					SocketO::onDrain();
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
					(node->*(Initializer::onError))(this->getExtra(),e);
				else
					SocketO::onError(e);
			}
			void onEnd() override
			{
				if constexpr ( Initializer::onEnd != nullptr )
					(node->*(Initializer::onEnd))(this->getExtra());
				else
					SocketO::onEnd();
			}
		};


		template<class Node, class Extra, class ... Handlers>
		class SocketN : public SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>
		{
		public:
			SocketN(Node* node_) : SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>( node_ ) {}
		
		};

	} // namespace net

} // namespace nodecpp

#endif // SOCKET_O_H
