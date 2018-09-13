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

#ifndef SOCKET_T_H
#define SOCKET_T_H

#include "template_common.h"
#include "net_common.h"
#include "socket_t_base.h"
#include "../../src/infrastructure.h"

namespace nodecpp {

	namespace net {

		template<auto x>
		struct OnCloseT {};

		template<auto x>
		struct OnConnectT {};

		template<auto x>
		struct OnDataT {};

		template<auto x>
		struct OnDrainT {};

		template<auto x>
		struct OnErrorT {};

		template<auto x>
		struct OnEndT {};

		template<auto x>
		struct OnAcceptedT {};

		template<typename ... args>
		struct SocketTInitializer;

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnCloseT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onClose = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnConnectT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onConnect = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnDataT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onData = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnDrainT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onDrain = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnErrorT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onError = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnEndT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onEnd = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketTInitializer<OnAcceptedT<F>, args...>
		: public SocketTInitializer<args...> {
			static constexpr auto onAccepted = F;
		};

		//partial template specialiazation to end recursion
		template<>
		struct SocketTInitializer<> {
			static constexpr auto onConnect = nullptr;
			static constexpr auto onClose = nullptr;
			static constexpr auto onData = nullptr;
			static constexpr auto onDrain = nullptr;
			static constexpr auto onError = nullptr;
			static constexpr auto onEnd = nullptr;
			static constexpr auto onAccepted = nullptr;
		};

		template<class Node, class Initializer, class Extra>
		class SocketT2 : public SocketTBase
		{
		public:
			using userIdType = Extra;
			using userNodeType = Node;
			using Handlers = Initializer;

		protected:
			Extra extra;

		public:
			SocketT2(Node* node_) : SocketTBase( node ) {}
			Extra* getExtra() { return &extra; }
			const Extra* getExtra() const { return &extra; }

		};

		template<class Node, class Initializer>
		class SocketT2<Node, Initializer, void> : public SocketTBase
		{
			public:
				using userIdType = void;
				using userNodeType = Node;
				using Handlers = Initializer;

		public:
			SocketT2(Node* node_) {this->node = node_;}
			SocketT2(Node* node_, OpaqueSocketData& sdata) {this->node = node_;}
			void* getExtra() { return nullptr; }
			const void* getExtra() const { return nullptr; }

		};


		template<class Node, class Extra, class ... Handlers>
		class SocketT : public SocketT2<Node, SocketTInitializer<Handlers...>, Extra>
		{
		public:
			SocketT(Node* node) : SocketT2<Node, SocketTInitializer<Handlers...>, Extra>(node) {int idType1 = Node::EmitterType::getTypeIndex( this ); NODECPP_ASSERT( this->node != nullptr ); registerWithInfraAndAcquireSocket(this->node, this, idType1); }
			SocketT(Node* node, OpaqueSocketData& sdata) : SocketT2<Node, SocketTInitializer<Handlers...>, Extra>(node) {int idType1 = Node::EmitterType::getTypeIndex( this ); NODECPP_ASSERT( node != nullptr ); registerWithInfraAndAssignSocket(node, this, idType1,sdata); }
			void connect(uint16_t port, const char* ip) {connectSocket(this, ip, port);}
			SocketT& setNoDelay(bool noDelay = true) { OSLayer::appSetNoDelay(this->dataForCommandProcessing, noDelay); return *this; }
			SocketT& setKeepAlive(bool enable = false) { OSLayer::appSetKeepAlive(this->dataForCommandProcessing, enable); return *this; }
		};

	} // namespace net

} // namespace nodecpp

#endif // SOCKET_T_H
