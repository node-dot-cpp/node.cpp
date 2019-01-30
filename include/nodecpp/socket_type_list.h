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

#ifndef SOCKET_TYPE_LIST_H
#define SOCKET_TYPE_LIST_H

#include "template_common.h"
#include "net_common.h"
#include "socket_t.h"
#include "socket_o.h"
#include "socket_l.h"

namespace nodecpp {

	namespace net {


		template<class T, class T1, class ... args>
		void callOnConnect( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onConnect();
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitConnect();
				else 
				{
					if constexpr( T1::Handlers::onConnect != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onConnect)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()));
				}
			}
			else
				callOnConnect<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnConnect( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnClose( void* nodePtr, T* ptr, int type, bool hadError )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onClose(hadError);
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitClose(hadError);
				else 
				{
					if constexpr( T1::Handlers::onClose != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onClose)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), hadError);
				}
			}
			else
				callOnClose<T, args...>(nodePtr, ptr, type-1, hadError);
		}

		template<class T>
		void callOnClose( void* nodePtr, T* ptr, int type, bool hadError )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnData( void* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onData(b);
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitData(b);
				else 
				{
					if constexpr( T1::Handlers::onData != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onData)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), b);
				}
			}
			else
				callOnData<T, args...>(nodePtr, ptr, type-1, b);
		}

		template<class T>
		void callOnData( void* nodePtr, T* ptr, int type, nodecpp::Buffer& b )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnDrain( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onDrain();
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitDrain();
				else 
				{
					if constexpr( T1::Handlers::onDrain != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onDrain)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()));
				}
			}
			else
				callOnDrain<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnDrain( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnError( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onError(e);
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitError(e);
				else 
				{
					if constexpr( T1::Handlers::onError != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onError)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()), e);
				}
			}
			else
				callOnError<T, args...>(nodePtr, ptr, type-1, e);
		}

		template<class T>
		void callOnError( void* nodePtr, T* ptr, int type, nodecpp::Error& e )
		{
			assert( false );
		}


		template<class T, class T1, class ... args>
		void callOnEnd( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onEnd();
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitEnd();
				else 
				{
					if constexpr( T1::Handlers::onEnd != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onEnd)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()));
				}
			}
			else
				callOnEnd<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnEnd( void* nodePtr, T* ptr, int type )
		{
			assert( false );
		}

		template<class T, class T1, class ... args>
		void callOnAccepted( void* nodePtr, T* ptr, int type )
		{
			if ( type == 0 )
			{
				if constexpr (std::is_same< T1, SocketO >::value)
					(nodecpp::safememory::soft_ptr_static_cast<SocketO>(ptr->getPtr()))->onAccepted();
				else if constexpr (std::is_same< T1, Socket >::value)
					(nodecpp::safememory::soft_ptr_static_cast<Socket>(ptr->getPtr()))->emitAccepted();
				else 
				{
					if constexpr( T1::Handlers::onAccepted != nullptr )
						(static_cast<typename T1::userNodeType*>(nodePtr)->*T1::Handlers::onAccepted)(nodecpp::safememory::soft_ptr_static_cast<T1>(ptr->getPtr()));
				}
			}
			else
				callOnAccepted<T, args...>(nodePtr, ptr, type-1);
		}

		template<class T>
		void callOnAccepted( void* nodePtr, T* ptr, int type )
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

#if 0 // old version
			static void emitConnect( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnConnect<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) { Ptr emitter_ptr( emitter.ptr ); callOnClose<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { Ptr emitter_ptr( emitter.ptr ); callOnData<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, b); }
			static void emitDrain( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnDrain<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { Ptr emitter_ptr( emitter.ptr ); callOnError<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }
			static void emitEnd( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnEnd<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitAccepted( const OpaqueEmitter& emitter ) { Ptr emitter_ptr( emitter.ptr ); callOnAccepted<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
#elif 0
			static void emitConnect( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnConnect<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnClose<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnData<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, b); }
			static void emitDrain( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnDrain<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnError<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }
			static void emitEnd( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnEnd<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitAccepted( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( static_cast<SocketBase*>(emitter.getClientSocketPtr()) ); callOnAccepted<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
#else
			static void emitConnect( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnConnect<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitClose( const OpaqueEmitter& emitter, bool hadError ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnClose<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, hadError); }
			static void emitData( const OpaqueEmitter& emitter, nodecpp::Buffer& b ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnData<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, b); }
			static void emitDrain( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnDrain<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitError( const OpaqueEmitter& emitter, nodecpp::Error& e ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnError<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type, e); }
			static void emitEnd( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnEnd<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
			static void emitAccepted( const OpaqueEmitter& emitter ) { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, emitter.objectType == OpaqueEmitter::ObjectType::ClientSocket); Ptr emitter_ptr( emitter.getClientSocketPtr() ); callOnAccepted<Ptr, args...>(emitter.nodePtr, &emitter_ptr, emitter.type); }
#endif // 0
		};
	} // namespace net

} // namespace nodecpp

#endif // SOCKET_TYPE_LIST_H
