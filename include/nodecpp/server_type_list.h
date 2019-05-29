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
#include "net_common.h"
#include "server_t.h"
#include "server_o.h"
#include "server_l.h"

namespace nodecpp {

	namespace net {


		template<class T, class T1, class ... args>
		void callOnConnection( void* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<ServerO>(ptr->getPtr()))->onConnection(sock);
				else if constexpr (std::is_same< T1, Server >::value)
				{
					soft_ptr<net::Socket> s = soft_ptr_static_cast<net::Socket>(sock);
					(nodecpp::safememory::soft_ptr_static_cast<Server>(ptr->getPtr()))->emitConnection(s);
				}
				else
				{
					if constexpr ( T1::Handlers::onConnection != nullptr )
					{
						soft_ptr<typename T1::SocketType> s = nodecpp::safememory::soft_ptr_static_cast<typename T1::SocketType>(sock);
//						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnection)(static_cast<T1*>(ptr->getPtr()), static_cast<typename T1::SocketType*>(sock) );
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnection)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), s );
					}
				}
			}
			else
				callOnConnection<T, args...>(nodePtr, ptr, type-1, sock);
		}

		template<class T>
		void callOnConnection( void* nodePtr, T* ptr, int type, soft_ptr<SocketBase> sock )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnCloseServer( void* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<ServerO>(ptr->getPtr()))->onClose(hadError);
				else if constexpr (std::is_same< T1, Server >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Server>(ptr->getPtr()))->emitClose(hadError);
				else
				{
					if constexpr ( T1::Handlers::onClose != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onClose)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), hadError);
				}
			}
			else
				callOnCloseServer<T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class T>
		void callOnCloseServer( void* nodePtr, T* ptr, int type, bool hadError )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnListening( void* nodePtr, T* ptr, int type, size_t id, Address addr )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<ServerO>(ptr->getPtr()))->onListening(id, addr);
				else if constexpr (std::is_same< T1, Server >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Server>(ptr->getPtr()))->emitListening(id, addr);
				else
				{
					if constexpr ( T1::Handlers::onListening != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onListening)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), id, addr);
				}
			}
			else
				callOnListening<T, args...>(nodePtr, ptr, type-1, id, addr);
		}

		template<class T>
		void callOnListening( void* nodePtr, T* ptr, int type, size_t id, Address addr )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnErrorServer( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<ServerO>(ptr->getPtr()))->onError(e);
				else if constexpr (std::is_same< T1, Server >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Server>(ptr->getPtr()))->emitError(e);
				else
				{
					if constexpr ( T1::Handlers::onError != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onError)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), e);
				}
			}
			else
				callOnErrorServer<T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class T>
		void callOnErrorServer( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		soft_ptr<SocketBase> callMakeSocket( void* nodePtr, T* ptr, int type, OpaqueSocketData& sdata )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
				{
					//return (static_cast<ServerO*>(ptr->getPtr()))->makeSocket(sdata);
					//return nodecpp::safememory::soft_ptr_static_cast<SocketBase>( (static_cast<ServerO*>(ptr->getPtr()))->makeSocket(sdata) );
					//(static_cast<ServerO*>(ptr->getPtr()))->makeSocket(sdata);
					//assert( false );
					return (nodecpp::safememory::soft_ptr_static_cast<ServerO>(ptr->getPtr()))->makeSocket(sdata);
				}
				else if constexpr (std::is_same< T1, Server >::value)
				{
//					soft_ptr<nodecpp::net::Socket> p = (static_cast<Server*>(ptr->getPtr()))->makeSocket(sdata);
					soft_ptr<net::Socket> p = (nodecpp::safememory::soft_ptr_static_cast<Server>(ptr->getPtr()))->makeSocket(sdata);
					soft_ptr<nodecpp::net::SocketBase> p1 = nodecpp::safememory::soft_ptr_static_cast<SocketBase>(p);
					return p1;
//					return (static_cast<Server*>(ptr->getPtr()))->makeSocket(sdata);
//					return (static_cast<Server*>(ptr->getPtr()))->makeSocket(sdata);
				}
				else
				{
					auto p = nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr())->makeSocket(sdata);
					//soft_ptr<SocketBase> x;
					soft_ptr<nodecpp::net::SocketBase> p1 = nodecpp::safememory::soft_ptr_static_cast<SocketBase>(p);
					return p1;
				}
			}
			else
				return callMakeSocket<T, args...>(nodePtr, ptr, type-1, sdata);
		}

		template<class T>
		soft_ptr<SocketBase> callMakeSocket( void* nodePtr, T* ptr, int type, OpaqueSocketData& sdata )
		{
			assert( false );
			soft_ptr<SocketBase> ret;
			return ret;
		}



		template< class ... args >
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
			static int getTypeIndex(Server* s) { return ::getTypeIndex<Server,args...>( s ); }
			template<class Server>
			static int softGetTypeIndexIfTypeExists() { return ::softGetTypeIndexIfTypeExists<Server,args...>(); }
#if 0 // old version
			static void emitConnection( const OpaqueEmitterForServer& emitter, SocketBase* sock ) { Ptr emitter_ptr( emitter.ptr ); callOnConnection<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sock); }
			static void emitClose( const OpaqueEmitterForServer& emitter, bool hadError ) { Ptr emitter_ptr( emitter.ptr ); callOnCloseServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitListening( const OpaqueEmitterForServer& emitter, size_t id, Address addr ) { Ptr emitter_ptr( emitter.ptr ); callOnListening<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, id, addr); }
			static void emitError( const OpaqueEmitterForServer& emitter, nodecpp::Error& e ) { Ptr emitter_ptr( emitter.ptr ); callOnErrorServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }

			static SocketBase* makeSocket(const OpaqueEmitterForServer& emitter, OpaqueSocketData& sdata) { Ptr emitter_ptr( emitter.ptr ); return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sdata); }
#else

			static void emitConnection( const OpaqueEmitter& emitter, soft_ptr<SocketBase> sock ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnConnection<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sock); 
			}
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) {
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnCloseServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError);
			}
			static void emitListening( const OpaqueEmitter& emitter, size_t id, Address addr ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnListening<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, id, addr); 
			}
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				callOnErrorServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); 
			}

			static soft_ptr<SocketBase> makeSocket(const OpaqueEmitter& emitter, OpaqueSocketData& sdata) { 
				NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); 
				Ptr emitter_ptr( nodecpp::safememory::soft_ptr_static_cast<ServerBase>(emitter.getServerSocketPtr()) ); 
				return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sdata); 
			}
#endif // 0
		};
	} // namespace net

} // namespace nodecpp

#endif // SERVER_TYPE_LIST_H
