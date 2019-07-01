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

#ifndef SERVER_TYPE_LIST_H
#define SERVER_TYPE_LIST_H

#include "template_common.h"
#include "socket_common.h"

extern thread_local NodeBase* thisThreadNode;

namespace nodecpp {

	namespace net {


		// "iteration" over a list of handling functions for an event

		template<class Node, class Server, class HandlerDataT, class ... args>
		void checkIsOnConnectionHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, soft_ptr<SocketBase> sock )
		{
			return true;
		}

		template<class Node, class Server>
		void checkIsOnConnectionHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, soft_ptr<SocketBase> sock )
		{
			return false;
		}

		template<class Node, class Server, class HandlerDataT, class ... args>
		void callOnConnectionHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, soft_ptr<SocketBase> sock )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( sock );
			else
			{
				static_assert (std::is_same< Server, typename HandlerDataT::ObjT >::value );
				((&(*serverPtr))->*HandlerDataT::memberFn)( sock );
			}

			callOnConnectionHandlers<Node, Server, args...>(nodePtr, serverPtr, sock);
		}

		template<class Node, class Server>
		void callOnConnectionHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, soft_ptr<SocketBase> sock )
		{
			return;
		}


		template<class Node, class Server, class HandlerDataT, class ... args>
		void callOnCloseServerHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, bool hadError )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( hadError );
			else
			{
				static_assert (std::is_same< Server, typename HandlerDataT::ObjT >::value );
				((&(*serverPtr))->*HandlerDataT::memberFn)( hadError );
			}

			callOnCloseServerHandlers<Node, Server, args...>(nodePtr, serverPtr, hadError);
		}

		template<class Node, class Server>
		void callOnCloseServerHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> ptr, bool hadError )
		{
			return;
		}


		template<class Node, class Server, class HandlerDataT, class ... args>
		void callOnListeningHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, size_t id, Address addr )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( id, addr );
			else
			{
				static_assert (std::is_same< Server, typename HandlerDataT::ObjT >::value );
				((&(*serverPtr))->*HandlerDataT::memberFn)( id, addr );
			}

			callOnListeningHandlers<Node, Server, args...>(nodePtr, serverPtr, id, addr);
		}

		template<class Node, class Server>
		void callOnListeningHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> ptr, size_t id, Address addr )
		{
			return;
		}


		template<class Node, class Server, class HandlerDataT, class ... args>
		void callOnErrorServerHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> serverPtr, nodecpp::Error& e )
		{
			if constexpr (std::is_same< Node, typename HandlerDataT::ObjT >::value)
				(nodePtr->*HandlerDataT::memberFn)( e );
			else
			{
				static_assert (std::is_same< Server, typename HandlerDataT::ObjT >::value );
				((&(*serverPtr))->*HandlerDataT::memberFn)( e );
			}

			callOnErrorServerHandlers<Node, Server, args...>(nodePtr, serverPtr, e);
		}

		template<class Node, class Server>
		void callOnErrorServerHandlers( Node* nodePtr, nodecpp::safememory::soft_ptr<Server> ptr, nodecpp::Error& e )
		{
			return;
		}



		template<class Server, class ... args> // 'args' are HandlerData<...>
		struct ServerHandlerDataList
		{
			using ServerT = Server;

			template<class Node>
			static bool isConnectionEmitter( Node* node, nodecpp::safememory::soft_ptr<Server> server, soft_ptr<SocketBase> sock ) {
				checkIsOnConnectionHandlers<Node, Server, args...>(node, server, sock); 
			}

			template<class Node>
			static void emitConnection( Node* node, nodecpp::safememory::soft_ptr<Server> server, soft_ptr<SocketBase> sock ) {
				callOnConnectionHandlers<Node, Server, args...>(node, server, sock); 
			}
			template<class Node>
			static void emitClose( Node* node, nodecpp::safememory::soft_ptr<Server> server, bool hadError ) {
				callOnCloseServerHandlers<Node, Server, args...>(node, server, hadError);
			}
			template<class Node>
			static void emitListening( Node* node, nodecpp::safememory::soft_ptr<Server> server, size_t id, Address addr ) { 
				callOnListeningHandlers<Node, Server, args...>(node, server, id, addr);
			}
			template<class Node>
			static void emitError( Node* node, nodecpp::safememory::soft_ptr<Server> server, nodecpp::Error& e ) { 
				callOnErrorServerHandlers<Node, Server, args...>(node, server, e);
			}
		};


		template<class T>
		struct OnCloseST {};

		template<class T>
		struct OnConnectionST {};

		template<class T>
		struct OnListeningST {};

		template<class T>
		struct OnErrorST {};

		template<typename ... args>
		struct ServerHandlerDescriptorBase;

		//partial template specializations:
		template<class T, typename ... args>
		struct ServerHandlerDescriptorBase<OnCloseST<T>, args...>
			: public ServerHandlerDescriptorBase<args...> {
			using onCloseT = T;
		};

		template<class T, typename ... args>
		struct ServerHandlerDescriptorBase<OnConnectionST<T>, args...>
			: public ServerHandlerDescriptorBase<args...> {
			using onConnectionT = T;
		};

		template<class T, typename ... args>
		struct ServerHandlerDescriptorBase<OnListeningST<T>, args...>
			: public ServerHandlerDescriptorBase<args...> {
			using onListeningT = T;
		};

		template<class T, typename ... args>
		struct ServerHandlerDescriptorBase<OnErrorST<T>, args...>
			: public ServerHandlerDescriptorBase<args...> {
			using onErrorT = T;
		};

		template<> // forming template param list as onXxxST<HandlerDataList<...>>, ...
		struct ServerHandlerDescriptorBase<> {
			using onConnectionT = void;
			using onCloseT = void;
			using onListeningT = void;
			using onErrorT = void;
		};

		template<class ServerT, class HandlerDesciptorT>
		struct ServerHandlerDescriptor {
			using ServerType = ServerT;
			using HandlerDesciptorType = HandlerDesciptorT;
		};


		// type selection (for checkIsXXX() )

		template<class Node, class T, class T1, class ... args>
		bool checkIsOnConnection( Node* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			if ( type == 0 )
			{
				if constexpr ( std::is_same< typename T1::HandlerDesciptorType::onConnectionT, void >::value )
					return false;
				else
				{
					soft_ptr<typename T1::ServerType> serverTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::ServerType>(ptr->getPtr());
					return T1::HandlerDesciptorType::onConnectionT::isConnectionEmitters( nodePtr, serverTypedPtr, sock );
				}
			}
			return checkIsOnConnection<Node, T, args...>(nodePtr, ptr, type-1, sock);
		}
		template<class Node, class T>
		bool checkIsOnConnection( Node* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			return false;
		}



		// type selection (for callXXX() )

		template<class Node, class T, class T1, class ... args>
		void callOnConnection( Node* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			assert(type!= -1);
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onConnectionT, void >::value )
				{
					soft_ptr<typename T1::ServerType> serverTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::ServerType>(ptr->getPtr());
					T1::HandlerDesciptorType::onConnectionT::emitConnection( nodePtr, serverTypedPtr, sock );
				}
			}
			else
				callOnConnection<Node, T, args...>(nodePtr, ptr, type-1, sock);
		}

		template<class Node, class T>
		void callOnConnection( Node* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnCloseServer( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onCloseT, void >::value )
				{
					soft_ptr<typename T1::ServerType> serverTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::ServerType>(ptr->getPtr());
					T1::HandlerDesciptorType::onCloseT::emitCloseServer( nodePtr, serverTypedPtr, hadError );
				}
			}
			else
				callOnCloseServer<Node, T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class Node, class T>
		void callOnCloseServer( Node* nodePtr, T* ptr, int type, bool hadError )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnListening( Node* nodePtr, T* ptr, int type, size_t id, Address addr )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onListeningT, void >::value )
				{
					soft_ptr<typename T1::ServerType> serverTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::ServerType>(ptr->getPtr());
					T1::HandlerDesciptorType::onListeningT::emitListening( nodePtr, serverTypedPtr, id, addr );
				}
			}
			else
				callOnListening<Node, T, args...>(nodePtr, ptr, type-1, id, addr);
		}

		template<class Node, class T>
		void callOnListening( Node* nodePtr, T* ptr, int type, size_t id, Address addr )
		{
			assert( false );
		}


		template<class Node, class T, class T1, class ... args>
		void callOnErrorServer( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				if constexpr ( !std::is_same< typename T1::HandlerDesciptorType::onErrorT, void >::value )
				{
					soft_ptr<typename T1::ServerType> serverTypedPtr = nodecpp::safememory::soft_ptr_static_cast<typename T1::ServerType>(ptr->getPtr());
					T1::HandlerDesciptorType::onErrorT::emitErrorServer( nodePtr, serverTypedPtr, e );
				}
			}
			else
				callOnErrorServer<Node, T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class Node, class T>
		void callOnErrorServer( Node* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}


		template< class ... args > // are a list of ServerHandlerDescriptor<...>
		class ServerTEmitter
		{
		public:
			class Ptr
			{
				nodecpp::safememory::soft_ptr<ServerBase> ptr;
			public:
				Ptr( nodecpp::safememory::soft_ptr<ServerBase> ptr_ ) { ptr = ptr_; }
				nodecpp::safememory::soft_ptr<ServerBase> getPtr() const {return ptr;}
			};

		public:
			template<class Server>
			static int getTypeIndex(Server* s) { 
				static_assert( !std::is_same<typename Server::NodeType, void>::value );
				return ::getTypeIndex<Server, typename args::ServerType...>( s ); 
			}
			template<class Server>
			static int softGetTypeIndexIfTypeExists() { return ::softGetTypeIndexIfTypeExists<Server, typename args::ServerType...>(); }

			template<class Node>
			static Node* getThreadNode() { return static_cast<Node*>(thisThreadNode); }

			// checking presense of an emitter

			template<class Node>
			static bool isConnectionEmitter( const OpaqueEmitter& emitter, soft_ptr<SocketBase> sock ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				if ( emitter.type == -1 ) return false;
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				return checkIsOnConnection<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, sock); 
			}
			/*template<class Node>
			static bool isCloseEmitter( const OpaqueEmitter& emitter, bool hadError ) {
				if ( emitter.type == -1 ) return false;
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				checkIsOnCloseServer<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, hadError);
			}
			template<class Node>
			static bool isListeningEmitter( const OpaqueEmitter& emitter, size_t id, Address addr ) { 
				if ( emitter.type == -1 ) return false;
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				checkIsOnListening<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, id, addr);
			}
			template<class Node>
			static bool isErrorEmitter( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				if ( emitter.type == -1 ) return false;
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				checkIsOnErrorServer<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, e);
			}*/

			// emiitting

			template<class Node>
			static void emitConnection( const OpaqueEmitter& emitter, soft_ptr<SocketBase> sock ) {
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnConnection<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, sock); 
			}
			template<class Node>
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) {
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnCloseServer<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, hadError);
			}
			template<class Node>
			static void emitListening( const OpaqueEmitter& emitter, size_t id, Address addr ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnListening<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, id, addr);
			}
			template<class Node>
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				if ( emitter.type == -1 ) return; // replace by assert(); should be checked externally before making this call
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnErrorServer<Node, Ptr, args...>(getThreadNode<Node>(), &emitter_ptr, emitter.type, e);
			}

			static soft_ptr<SocketBase> makeSocket(const OpaqueEmitter& emitter, OpaqueSocketData& sdata) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
//				return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr/*, emitter.type*/, sdata); 
				auto p = ptr.getPtr()->makeSocket(sdata);
				//soft_ptr<SocketBase> x;
				soft_ptr<nodecpp::net::SocketBase> p1 = nodecpp::safememory::soft_ptr_static_cast<SocketBase>(p);
				return p1;
			}
		};
	} // namespace net

} // namespace nodecpp

#endif // SERVER_TYPE_LIST_H
