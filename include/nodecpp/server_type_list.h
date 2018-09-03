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
				/*if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onConnection();
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitConnection();
				else*/
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
				/*if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onClose(hadError);
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitClose(hadError);
				else*/
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
		void callOnListening( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				/*if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onListening();
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitListening();
				else*/
				{
					if constexpr ( T1::Handlers::onListening != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onListening)(static_cast<T1*>(ptr->getPtr())->getExtra());
				}
			}
			else
				callOnListening<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnListening( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnErrorServer( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				/*if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->onError(e);
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->emitError(e);
				else*/
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
		SocketTBase* callMakeSocket( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				/*if constexpr (std::is_same< T1, ServerO >::value)
					(static_cast<ServerO*>(ptr->getPtr()))->createSocket();
				else if constexpr (std::is_same< T1, Server >::value)
					(static_cast<Server*>(ptr->getPtr()))->createSocket();
				else*/
					return static_cast<T1*>(ptr->getPtr())->createSocket();
			}
			else
				return callMakeSocket<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		SocketTBase* callMakeSocket( void* nodePtr, T* ptr, int type )
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

			static void emitConnection( const OpaqueEmitterForServer& emitter, SocketTBase* sock ) { Ptr emitter_ptr( emitter.ptr ); callOnConnection<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, sock); }
			static void emitClose( const OpaqueEmitterForServer& emitter, bool hadError ) { Ptr emitter_ptr( emitter.ptr ); callOnCloseServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitListening( const OpaqueEmitterForServer& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnListening<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitError( const OpaqueEmitterForServer& emitter, nodecpp::Error& e ) { Ptr emitter_ptr( emitter.ptr ); callOnErrorServer<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }

			static SocketTBase* makeSocket(const OpaqueEmitterForServer& emitter) { Ptr emitter_ptr( emitter.ptr ); return callMakeSocket<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
		};
	} // namespace net

} // namespace nodecpp

#endif // SERVER_TYPE_LIST_H
