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

#ifndef SOCKET_O_H
#define SOCKET_O_H

#include "net_common.h"
#include "socket_t_base.h"

namespace nodecpp {

	namespace net {

		class [[nodecpp::owning_only]] SocketO : public SocketBase {

		private:
			void registerMeAndAcquireSocket();
			void registerMeAndAssignSocket(OpaqueSocketData& sdata);

		public:
			SocketO(NodeBase* node) : SocketBase( node ) {registerMeAndAcquireSocket();}
			SocketO(NodeBase* node, OpaqueSocketData& sdata) : SocketBase( node ) {registerMeAndAssignSocket(sdata);}

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
			virtual void onAccepted() {}
			virtual void onError(Error& err) {}

			nodecpp::awaitable<void> onConnectAwaitor;

			void connect(uint16_t port, const char* ip);
			SocketO& setNoDelay(bool noDelay = true) { OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay); return *this; }
			SocketO& setKeepAlive(bool enable = false) { OSLayer::appSetKeepAlive(dataForCommandProcessing, enable); return *this; }

		//private:
			auto a_connect_core(DataForCommandProcessing::awaitable_handle_data& ahd_conn) { 

				struct connect_awaiter {
					DataForCommandProcessing::awaitable_handle_data& ahd_conn;

					std::experimental::coroutine_handle<> who_is_awaiting;

					connect_awaiter(DataForCommandProcessing::awaitable_handle_data& ahd_conn_) : ahd_conn( ahd_conn_ ) {}

					connect_awaiter(const connect_awaiter &) = delete;
					connect_awaiter &operator = (const connect_awaiter &) = delete;
	
					~connect_awaiter() {}

					bool await_ready() {
						return false;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						who_is_awaiting = awaiting;
						ahd_conn.h = who_is_awaiting;
					}

					auto await_resume() {
						if ( ahd_conn.is_exception )
						{
							ahd_conn.is_exception = false; // now we will throw it and that's it
							ahd_conn.exception;
						}
					}
				};
				return connect_awaiter(ahd_conn);
			}
			auto a_connect(uint16_t port, const char* ip)
			{
				connect( port, ip );
				return a_connect_core(dataForCommandProcessing.ahd_connect);
			}
			auto a_read( Buffer& buff, size_t min_bytes = 1 ) { 

				buff.clear();
				NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.capacity() >= min_bytes, "indeed: {} vs. {} bytes", buff.capacity(), min_bytes );

				struct read_data_awaiter {
					SocketO& socket;
					Buffer& buff;
					size_t min_bytes;

					read_data_awaiter(SocketO& socket_, Buffer& buff_, size_t min_bytes_) : socket( socket_ ), buff( buff_ ), min_bytes( min_bytes_ ) {}

					read_data_awaiter(const read_data_awaiter &) = delete;
					read_data_awaiter &operator = (const read_data_awaiter &) = delete;
	
					~read_data_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.readBuffer.used_size() >= min_bytes;
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						socket.dataForCommandProcessing.ahd_read.min_bytes = min_bytes;
						socket.dataForCommandProcessing.ahd_read.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_read.is_exception )
						{
							socket.dataForCommandProcessing.ahd_read.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_read.exception;
						}
						socket.dataForCommandProcessing.readBuffer.get_ready_data( buff );
						NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, buff.size() >= min_bytes);
					}
				};
				return read_data_awaiter(*this, buff, min_bytes);
			}

			auto a_write(Buffer& buff) { 

				struct write_data_awaiter {
					SocketO& socket;
					Buffer& buff;
					bool write_ok = false;

					std::experimental::coroutine_handle<> who_is_awaiting;

					write_data_awaiter(SocketO& socket_, Buffer& buff_) : socket( socket_ ), buff( buff_ )  {}

					write_data_awaiter(const write_data_awaiter &) = delete;
					write_data_awaiter &operator = (const write_data_awaiter &) = delete;
	
					~write_data_awaiter() {}

					bool await_ready() {
						write_ok = socket.write2( buff ); // so far we do it sync TODO: extend implementation for more complex (= requiring really async processing) cases
						return write_ok; // false means waiting (incl. exceptional cases)
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !write_ok ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_write.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_write.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_write.exception;
						}
					}
				};
				return write_data_awaiter(*this, buff);
			}

			auto a_drain() { 

				struct drain_awaiter {
					SocketO& socket;

					std::experimental::coroutine_handle<> who_is_awaiting;

					drain_awaiter(SocketO& socket_) : socket( socket_ )  {}

					drain_awaiter(const drain_awaiter &) = delete;
					drain_awaiter &operator = (const drain_awaiter &) = delete;
	
					~drain_awaiter() {}

					bool await_ready() {
						return socket.dataForCommandProcessing.writeBuffer.empty();
					}

					void await_suspend(std::experimental::coroutine_handle<> awaiting) {
						NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, !socket.dataForCommandProcessing.writeBuffer.empty() ); // otherwise, why are we here?
						socket.dataForCommandProcessing.ahd_drain.h = awaiting;
					}

					auto await_resume() {
						if ( socket.dataForCommandProcessing.ahd_write.is_exception )
						{
							socket.dataForCommandProcessing.ahd_drain.is_exception = false; // now we will throw it and that's it
							throw socket.dataForCommandProcessing.ahd_drain.exception;
						}
					}
				};
				return drain_awaiter(*this);
			}
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
		struct OnAccepted {};

		template<auto x>
		struct OnEnd {};

		template<auto x>
		struct OnConnectA {};

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

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnAccepted<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onAccepted = F;
		};

		//partial template specialization:
		template<auto F, typename ... args>
		struct SocketOInitializer2<OnConnectA<F>, args...>
		: public SocketOInitializer2<args...> {
			static constexpr auto onConnectA = F;
		};

		//partial template specialiazation to end recursion
		template<>
		struct SocketOInitializer2<> {
			static constexpr auto onConnect = nullptr;
			static constexpr auto onClose = nullptr;
			static constexpr auto onData = nullptr;
			static constexpr auto onDrain = nullptr;
			static constexpr auto onError = nullptr;
			static constexpr auto onEnd = nullptr;
			static constexpr auto onAccepted = nullptr;
			static constexpr auto onConnectA = nullptr;
		};

		template<class Node, class Extra>
		class SocketOUserBase : public SocketO
		{
			Extra extra;
		public:
			SocketOUserBase(Node* node) : SocketO( node ) {}
			SocketOUserBase(Node* node, OpaqueSocketData& sdata) : SocketO( node, sdata ) {}
			Extra* getExtra() { return &extra; }
		};

		template<class Node>
		class SocketOUserBase<Node, void> : public SocketO
		{
		public:
			SocketOUserBase(Node* node) : SocketO( node ) {}
			SocketOUserBase(Node* node, OpaqueSocketData& sdata) : SocketO( node, sdata ) {}
			void* getExtra() { return nullptr; }
		};

		template<class Node, class Initializer, class Extra>
		class SocketN2 : public SocketOUserBase<Node, Extra>
		{
		public:
			using StorableType = SocketOUserBase<Node, Extra>;

			nodecpp::awaitable<void> a_connect_handler_entry_point(nodecpp::safememory::soft_ptr<nodecpp::awaitable<void>> me) 
			{
				if constexpr ( Initializer::onConnectA != nullptr )
				{
					printf( "OnConnectA is present\n" ); 
					co_await this->a_connect_core(this->dataForCommandProcessing.ahd_connect_2);
					startNewConnectHandlerInstance();
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					co_await ((static_cast<Node*>(this->node))->*(Initializer::onConnectA))(ptr2this); 
				}
				co_await this->dataForCommandProcessing.handlerAwaiterList.cleanup(); // IMPORTANT: do it before (!!!) adding me
				this->dataForCommandProcessing.handlerAwaiterList.mark_done( std::move( me ) );
				co_return;
			}
			void startNewConnectHandlerInstance()
			{
				nodecpp::safememory::owning_ptr<nodecpp::awaitable<void>> awaitor = nodecpp::safememory::make_owning<nodecpp::awaitable<void>>();
				nodecpp::safememory::soft_ptr<nodecpp::awaitable<void>> soft_awaitor = awaitor;
				*awaitor = std::move( this->a_connect_handler_entry_point(soft_awaitor) );
				this->dataForCommandProcessing.handlerAwaiterList.add( std::move( awaitor ) );
			}

		public:
			SocketN2(Node* node) : SocketOUserBase<Node, Extra>( node )
			{
				if constexpr ( Initializer::onConnectA != nullptr )
					startNewConnectHandlerInstance();
			}
			SocketN2(Node* node, OpaqueSocketData& sdata) : SocketOUserBase<Node, Extra>( node, sdata )
			{
				if constexpr ( Initializer::onConnectA != nullptr )
					startNewConnectHandlerInstance();
			}

			void onConnect() override
			{ 
				if constexpr ( Initializer::onConnect != nullptr )
				{
	//				static_assert( Initializer::onConnect == nullptr || std::is_same< decltype(Initializer::onConnect), void (Node::*)(const Extra*) >::value );
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onConnect))(ptr2this); 
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
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onClose))(ptr2this,b); 
				}
				else
					SocketO::onClose(b);
			}
			void onData(nodecpp::Buffer& b) override
			{ 
				if constexpr ( Initializer::onData != nullptr )
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onData))(ptr2this, b); 
				}
				else
					SocketO::onData(b);
			}
			void onDrain() override
			{
				if constexpr ( Initializer::onDrain != nullptr )
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onDrain))(ptr2this);
				}
				else
					SocketO::onDrain();
			}
			void onError(nodecpp::Error& e) override
			{
				if constexpr ( Initializer::onError != nullptr )
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onError))(ptr2this,e);
				}
				else
					SocketO::onError(e);
			}
			void onEnd() override
			{
				if constexpr ( Initializer::onEnd != nullptr )
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onEnd))(ptr2this);
				}
				else
					SocketO::onEnd();
			}
			void onAccepted() override
			{
				if constexpr ( Initializer::onAccepted != nullptr )
				{
					nodecpp::safememory::soft_ptr<SocketOUserBase<Node, Extra>> ptr2this = this->myThis.template getSoftPtr<SocketOUserBase<Node, Extra>>(this);
					((static_cast<Node*>(this->node))->*(Initializer::onAccepted))(ptr2this);
				}
				else
					SocketO::onAccepted();
			}
		};

		template<class Node, class Extra, class ... Handlers>
		class SocketN : public SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>
		{
		public:
			SocketN(Node* node) : SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>( node ) {}
			SocketN(Node* node, OpaqueSocketData& sdata) : SocketN2<Node, SocketOInitializer2<Handlers...>, Extra>( node, sdata ) {}
		
		};


	} // namespace net

} // namespace nodecpp

#endif // SOCKET_O_H
