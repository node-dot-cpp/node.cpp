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
		void callOnConnection( void* nodePtr, T* ptr, int type, SocketTBase* sock )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onConnection(sock);
				else if constexpr (std::is_same< T1, Server >::value)
				{
					net::Socket* s = static_cast<net::Socket*>(sock);
					(static_cast<Server*>(ptr->getPtr()))->emitConnection(s);
				}
				else
				{
					if constexpr ( T1::Handlers::onConnection != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnection)(static_cast<T1*>(ptr->getPtr())->getExtra(), static_cast<typename T1::SocketType*>(sock) );
				}
			}
			else
				callOnConnection<T, args...>(nodePtr, ptr, type-1, sock);
		}

		template<class T>
		void callOnConnection( void* nodePtr, T* ptr, int type, SocketTBase* sock )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnCloseServer( void* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onClose(hadError);
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitClose(hadError);
				else
				{
					if constexpr ( T1::Handlers::onClose != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onClose)(static_cast<T1*>(ptr->getPtr())->getExtra(), hadError);
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
					(static_cast<ServerO*>(ptr->getPtr()))->onListeningX(id, addr);
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitListening(id, addr);
				else
				{
					if constexpr ( T1::Handlers::onListening != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onListening)(static_cast<T1*>(ptr->getPtr())->getExtra(), id, addr);
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
					(static_cast<ServerO*>(ptr->getPtr()))->onError(e);
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitError(e);
				else
				{
					if constexpr ( T1::Handlers::onError != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onError)(static_cast<T1*>(ptr->getPtr())->getExtra(), e);
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
		SocketTBase* callMakeSocket( void* nodePtr, T* ptr, int type, OpaqueSocketData& sdata )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, ServerO >::value)
					return (static_cast<ServerO*>(ptr->getPtr()))->makeSocket(sdata);
				else if constexpr (std::is_same< T1, Server >::value)
					return (static_cast<Server*>(ptr->getPtr()))->makeSocket(sdata);
				else
					return static_cast<T1*>(ptr->getPtr())->makeSocket(sdata);
			}
			else
				return callMakeSocket<T, args...>(nodePtr, ptr, type-1, sdata);
		}

		template<class T>
		SocketTBase* callMakeSocket( void* nodePtr, T* ptr, int type, OpaqueSocketData& sdata )
		{
			assert( false );
			return nullptr;
		}



		template< class ... args >
		class ServerTEmitter
		{
		public:
			class Ptr
			{
				ServerTBase* ptr;
			public:
				Ptr( ServerTBase* ptr_ ) { ptr = ptr_; }
				ServerTBase* getPtr() const {return ptr;}
			};

		public:
			template<class Server>
			static int getTypeIndex(Server* s) { return ::getTypeIndex<Server,args...>( s ); }
			template<class Server>
			static int softGetTypeIndexIfTypeExists() { return ::softGetTypeIndexIfTypeExists<Server,args...>(); }
#if 0 // old version
			static void emitConnection( const OpaqueEmitterForServer& emitter, SocketTBase* sock ) { Ptr emitter_ptr( emitter.ptr ); callOnConnection<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sock); }
			static void emitClose( const OpaqueEmitterForServer& emitter, bool hadError ) { Ptr emitter_ptr( emitter.ptr ); callOnCloseServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitListening( const OpaqueEmitterForServer& emitter, size_t id, Address addr ) { Ptr emitter_ptr( emitter.ptr ); callOnListening<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, id, addr); }
			static void emitError( const OpaqueEmitterForServer& emitter, nodecpp::Error& e ) { Ptr emitter_ptr( emitter.ptr ); callOnErrorServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }

			static SocketTBase* makeSocket(const OpaqueEmitterForServer& emitter, OpaqueSocketData& sdata) { Ptr emitter_ptr( emitter.ptr ); return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sdata); }
#else

			static void emitConnection( const OpaqueEmitter& emitter, SocketTBase* sock ) { NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); Ptr emitter_ptr( static_cast<ServerTBase*>(emitter.getServerSocketPtr()) ); callOnConnection<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sock); }
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) { NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); Ptr emitter_ptr( static_cast<ServerTBase*>(emitter.getServerSocketPtr()) ); callOnCloseServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitListening( const OpaqueEmitter& emitter, size_t id, Address addr ) { NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); Ptr emitter_ptr( static_cast<ServerTBase*>(emitter.getServerSocketPtr()) ); callOnListening<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, id, addr); }
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); Ptr emitter_ptr( static_cast<ServerTBase*>(emitter.getServerSocketPtr()) ); callOnErrorServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }

			static SocketTBase* makeSocket(const OpaqueEmitter& emitter, OpaqueSocketData& sdata) { NODECPP_ASSERT( emitter.objectType == OpaqueEmitter::ObjectType::ServerSocket); Ptr emitter_ptr( static_cast<ServerTBase*>(emitter.getServerSocketPtr()) ); return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sdata); }
#endif // 0
		};
	} // namespace net

} // namespace nodecpp

#endif // SERVER_TYPE_LIST_H
