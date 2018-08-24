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

#include <stdio.h>
#include "template_common.h"
#include "net_common.h"

namespace nodecpp {

	namespace net {

		class SocketTBase : public SocketBase {
		
		public:
//			UserDefID userDefID;
			NodeBase* node = nullptr;

		public:
			SocketTBase() {}

			SocketTBase(const SocketTBase&) = delete;
			SocketTBase& operator=(const SocketTBase&) = delete;

			SocketTBase(SocketTBase&&) = default;
			SocketTBase& operator=(SocketTBase&&) = default;

			~SocketTBase() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			void connect(uint16_t port, const char* ip);
			SocketTBase& setNoDelay(bool noDelay = true);
			SocketTBase& setKeepAlive(bool enable = false);
		};

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

		//create similar partial specializations for all the args

		//partial template specialiazation to end recursion
		template<>
		struct SocketTInitializer<> {
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
		class SocketT2 : public SocketTBase
		{
		public:
			using userIdType = Extra;
			using userNodeType = Node;
			using Handlers = Initializer;

		protected:
			Extra extra;
		public:
//			Node* node;

			SocketT2(Node* node_) {this->node = node_;}
			Extra* getExtra() { return &extra; }
			const Extra* getExtra() const { return &extra; }

		};

		template<class Node, class Initializer>
		class SocketT2<Node, Initializer, void> : public SocketBase
		{
			public:
				using userIdType = void;
				using userNodeType = Node;
				using Handlers = Initializer;

		public:
			SocketT2() {}
			void* getExtra() { return nullptr; }
			const void* getExtra() const { return nullptr; }

		};


		template<class Node, class Extra, class ... Handlers>
		class SocketT : public SocketT2<Node, SocketTInitializer<Handlers...>, Extra>
		{
			int idType1; // we will try to get rid of it later
		public:
			SocketT(Node* node) : SocketT2<Node, SocketTInitializer<Handlers...>, Extra>(node) {idType1 = Node::EmitterType::getTypeIndex( this );}
			void connect(uint16_t port, const char* ip) {connectToInfra(this->node, this, idType1, ip, port);}
//			void connect(uint16_t port, const char* ip) {connectToInfra2<Node>(this, idType1, ip, port);}
		
		};


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		template<class T, class T1, class ... args>
		void callOnConnect( void* nodePtr, const T* ptr, int type )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnect)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnConnect<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnConnect( void* nodePtr, const T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnClose( void* nodePtr, const T* ptr, int type, bool ok )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onClose)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra(), ok);
			}
			else
				callOnClose<T, args...>(nodePtr, ptr, type-1, ok);
		}

		template<class T>
		void callOnClose( void* nodePtr, const T* ptr, int type, bool ok )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnData( void* nodePtr, const T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onData)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra(), b);
			}
			else
				callOnData<T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class T>
		void callOnData( void* nodePtr, const T* ptr, int type, nodecpp::Buffer& b )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnDrain( void* nodePtr, const T* ptr, int type )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onDrain)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnDrain<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnDrain( void* nodePtr, const T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnError( void* nodePtr, const T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onError)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra(), e);
			}
			else
				callOnError<T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class T>
		void callOnError( void* nodePtr, const T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnEnd( void* nodePtr, const T* ptr, int type )
		{
			if ( type == 0 )
			{
				(reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onEnd)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnEnd<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnEnd( void* nodePtr, const T* ptr, int type )
		{
			assert( false );
		}





		template< class ... args >
		class SocketTEmitter
		{
		public:
			class Ptr
			{
				void* ptr = nullptr;
			public:
				Ptr() {}
				void init( void* ptr_ ) { ptr = ptr_; }
				void* getPtr() const {return ptr;}
				bool isValid() const { return ptr != nullptr; }
			};

		protected:
			Ptr ptr;
			void* nodePtr = nullptr;
			int type = -1;

		public:
			SocketTEmitter() {}
			template<class Sock>
			SocketTEmitter(Sock* s) {ptr.init(s); nodePtr = s->node; type = getTypeIndex<Sock,args...>( s );}
			SocketTEmitter(SocketTBase* ptr_, int type_) {ptr.init(ptr_); nodePtr = ptr_->node; type = type_;}

			template<class Sock>
			static int getTypeIndex(Sock* s) { return ::getTypeIndex<Sock,args...>( s ); } // we may need it externally

			template<class Sock>
			void init(Sock* s)
			{ 
				ptr.init(s);
				nodePtr = s->node;
				type = getTypeIndex<Sock,args...>( s );
			}
			bool isValid() const { return ptr.isValid(); }

			void emitConnect() const { callOnConnect<Ptr, args...>(nodePtr, &ptr, this->type); }
			void emitClose( bool ok ) const { callOnClose<Ptr, args...>(nodePtr, &ptr, this->type, ok); }
			void emitData( nodecpp::Buffer& b ) const { callOnData<Ptr, args...>(nodePtr, &ptr, this->type, b); }
			void emitDrain() const { callOnDrain<Ptr, args...>(nodePtr, &ptr, this->type); }
			void emitError( nodecpp::Error& e ) const { callOnError<Ptr, args...>(nodePtr, &ptr, this->type, e); }
			void emitEnd() const { callOnEnd<Ptr, args...>(nodePtr, &ptr, this->type); }
		};



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		template<class T, class T1, class ... args>
		void callOnConnect2( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnect)(static_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnConnect2<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnConnect2( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnClose2( void* nodePtr, T* ptr, int type, bool ok )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onClose)(static_cast<T1*>(ptr->getPtr())->getExtra(), ok);
			}
			else
				callOnClose2<T, args...>(nodePtr, ptr, type-1, ok);
		}

		template<class T>
		void callOnClose2( void* nodePtr, T* ptr, int type, bool ok )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnData2( void* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onData)(static_cast<T1*>(ptr->getPtr())->getExtra(), b);
			}
			else
				callOnData2<T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class T>
		void callOnData2( void* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnDrain2( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onDrain)(static_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnDrain2<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnDrain2( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnError2( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onError)(static_cast<T1*>(ptr->getPtr())->getExtra(), e);
			}
			else
				callOnError2<T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class T>
		void callOnError2( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnEnd2( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onEnd)(static_cast<T1*>(ptr->getPtr())->getExtra());
			}
			else
				callOnEnd2<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnEnd2( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}





		template< class ... args >
		class SocketTEmitter2
		{
		public:
			class Ptr
			{
				void* ptr;
			public:
				Ptr( void* ptr_ ) { ptr = ptr_; }
				void* getPtr() const {return ptr;}
			};

		public:
			template<class Sock>
			static int getTypeIndex(Sock* s) { return ::getTypeIndex<Sock,args...>( s ); }

			static void emitConnect( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnConnect<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitClose( const OpaqueEmitter& emitter, bool ok ) { Ptr emitter_ptr( emitter.ptr ); callOnClose2<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, ok); }
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { Ptr emitter_ptr( emitter.ptr ); callOnData2<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, b); }
			static void emitDrain( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnDrain2<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { Ptr emitter_ptr( emitter.ptr ); callOnError2<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }
			static void emitEnd( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnEnd2<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
		};
	} // namespace net

} // namespace nodecpp

#endif // SOCKET_T_H
