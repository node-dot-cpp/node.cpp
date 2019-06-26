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

#ifndef SOCKET_TYPE_LIST_H
#define SOCKET_TYPE_LIST_H

#include "template_common.h"
#include "socket_common.h"

namespace nodecpp {

	namespace net {


		// "iteration" over a list of handling functions for an event

		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)();
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnConnectHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnConnectHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnAcceptedSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)();
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnAcceptedSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnAcceptedSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Buffer& b )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( b );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( b );
			}

			callOnDataHandlers<Node, Socket, args...>(nodePtr, socketPtr, b);
		}

		template<class Node, class Socket>
		void callOnDataHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Buffer& b )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnDrainSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)();
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnDrainSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnDrainSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, bool hadError )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( hadError );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( hadError );
			}

			callOnCloseSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr, hadError);
		}

		template<class Node, class Socket>
		void callOnCloseSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, bool hadError )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)();
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)();
			}

			callOnEndHandlers<Node, Socket, args...>(nodePtr, socketPtr);
		}

		template<class Node, class Socket>
		void callOnEndHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr )
		{
			return;
		}


		template<class Node, class Socket, class HandlerDataT, class ... args>
		void callOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> socketPtr, nodecpp::Error& e )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( e );
			else
			{
				static_assert (std::is_same< Socket, typename HandlerDataT::ObjT >::value );
				((&(*socketPtr))->*HandlerDataT::memberFn)( e );
			}

			callOnErrorSocketHandlers<Node, Socket, args...>(nodePtr, socketPtr, e);
		}

		template<class Node, class Socket>
		void callOnErrorSocketHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Socket> ptr, nodecpp::Error& e )
		{
			return;
		}



		template<class Socket, class ... args> // 'args' are HandlerData<...>
		struct SocketHandlerDataList
		{
			using SocketT = Socket;

			template<class Node>
			static void emitConnect( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				callOnConnectHandlers<Node, Socket, args...>(node, socket); 
			}
			template<class Node>
			static void emitAccepted( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) {
				callOnAcceptedSocketHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static void emitData( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Buffer& b ) { 
				callOnDataHandlers<Node, Socket, args...>(node, socket, b);
			}
			template<class Node>
			static void emitDrain( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				callOnDrainSocketHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static void emitClose( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, bool hadError ) {
				callOnCloseSocketHandlers<Node, Socket, args...>(node, socket, hadError);
			}
			template<class Node>
			static void emitEnd( Node* node, nodecpp::safememory::soft_ptr<Socket> socket ) { 
				callOnEndHandlers<Node, Socket, args...>(node, socket);
			}
			template<class Node>
			static void emitError( Node* node, nodecpp::safememory::soft_ptr<Socket> socket, nodecpp::Error& e ) { 
				callOnErrorSocketHandlers<Node, Socket, args...>(node, socket, e);
			}
		};


		template<class T>
		struct OnConnectT {};

		template<class T>
		struct OnAcceptedT {};

		template<class T>
		struct OnDataT {};

		template<class T>
		struct OnDrainT {};

		template<class T>
		struct OnEndT {};

		template<class T>
		struct OnCloseT {};

		template<class T>
		struct OnErrorT {};

		template<typename ... args>
		struct SocketHandlerDescriptorBase;

		//partial template specializations:
		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnConnectT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onConnectT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnAcceptedT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onAcceptedT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnDataT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onDataT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnDrainT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onDrainT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnCloseT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onCloseT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnEndT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onEndT = T;
		};

		template<class T, typename ... args>
		struct SocketHandlerDescriptorBase<OnErrorT<T>, args...>
			: public SocketHandlerDescriptorBase<args...> {
			using onErrorT = T;
		};

		template<> // forming template param list as onXxxST<HandlerDataList<...>>, ...
		struct SocketHandlerDescriptorBase<> {
			using onConnectT = void;
			using onAcceptedT = void;
			using onDataT = void;
			using onDrainT = void;
			using onCloseT = void;
			using onEndT = void;
			using onErrorT = void;
		};

		template<class SocketT, class HandlerDesciptorT>
		struct SocketHandlerDescriptor {
			using SocketType = SocketT;
			using HandlerDesciptorType = HandlerDesciptorT;
		};

		// type selection

		template<class Node, class T, class T1, class ... args>
		void callOnConnect( Node* nodePtr, T* ptr, int type )
		{
			assert(type!= -1);
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onConnectionT::emitConnection( nodePtr, socketTypedPtr );
			}
			else
				callOnConnect<Node, T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnConnect( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnAcceptedSocket( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onCloseT::emitCloseSocket( nodePtr, socketTypedPtr );
			}
			else
				callOnAcceptedSocket<T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnAcceptedSocket( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onListeningT::emitListening( nodePtr, socketTypedPtr, b );
			}
			else
				callOnData<T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class Node, class T>
		void callOnData( Node* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onErrorT::emitErrorSocket( nodePtr, socketTypedPtr );
			}
			else
				callOnDrainSocket<T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnDrainSocket( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onCloseT::emitCloseSocket( nodePtr, socketTypedPtr, hadError );
			}
			else
				callOnCloseSocket<T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class Node, class T>
		void callOnCloseSocket( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnEnd( Node* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onListeningT::emitEnd( nodePtr, socketTypedPtr );
			}
			else
				callOnEnd<T, args...>(nodePtr, ptr, type-1);
		}

		template<class Node, class T>
		void callOnEnd( Node* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				soft_ptr<typename T1::SocketType> socketTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(ptr->getPtr());
				T1::HandlerDesciptorType::onErrorT::emitErrorSocket( nodePtr, socketTypedPtr, e );
			}
			else
				callOnErrorSocket<T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class Node, class T>
		void callOnErrorSocket( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}



		template< class ... args >
		class SocketTEmitter
		{
		public:
			class Ptr
			{
				nodecpp::safememory::soft_ptr<SocketBase> ptr;
			public:
				Ptr( nodecpp::safememory::soft_ptr<SocketBase> ptr_ ) { ptr = ptr_; }
				nodecpp::safememory::soft_ptr<SocketBase> getPtr() const {return ptr;}
			};

		public:
			template<class Sock>
			static int getTypeIndex(Sock* s) { return ::getTypeIndex<Sock,args...>( s ); }
			template<class Sock>
			static int softGetTypeIndexIfTypeExists() { return ::softGetTypeIndexIfTypeExists<Sock,args...>(); }

			template<class Node>
			static Node* getThreadNode() { return static_cast<Node*>(thisThreadNode); }

			template<class Node>
			static void emitConnect( const OpaqueEmitter& emitter ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnConnect<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type); 
			}
			template<class Node>
			static void emitAccepted( const OpaqueEmitter& emitter ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnAcceptedSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnData<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, b);
			}
			template<class Node>
			static void emitDrain( const OpaqueEmitter& emitter ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnDrainSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnCloseSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, hadError);
			}
			template<class Node>
			static void emitEnd( const OpaqueEmitter& emitter ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnEnd<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type);
			}
			template<class Node>
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<SocketBase>(emitter.getClientSocketPtr()) ); 
				callOnErrorSocket<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, e);
			}
		};
	} // namespace net

} // namespace nodecpp

#endif // SOCKET_TYPE_LIST_H
