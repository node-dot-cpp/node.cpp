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

#ifndef SOCKET_T_BASE_H
#define SOCKET_T_BASE_H

#include <stdio.h>
#include "template_common.h"
#include "net_common.h"
#include "../../src/tcp_socket/tcp_socket_base.h"

namespace nodecpp {

	namespace net {

		//class SocketBase; // forward declaration

		class ServerTBase; // forward declaration

	} // namespace net


	class OpaqueEmitter
	{
		//using PtrType = void*;
		//PtrType ptr = nullptr;
		//using UnderlyingType = nodecpp::net::SocketBase;
		using UnderlyingType = void;
		nodecpp::safememory::soft_ptr<UnderlyingType> ptr;
	public:
	//	using PtrType = nodecpp::net::SocketBase*;
		enum ObjectType { Undefined, ClientSocket, ServerSocket };
		ObjectType objectType;
		int type = -1;
		NodeBase* nodePtr = nullptr;
		OpaqueEmitter() : /*ptr( nullptr), */type(-1), objectType(ObjectType::Undefined) {}
//		OpaqueEmitter( ObjectType ot, NodeBase* node, nodecpp::safememory::soft_ptr<net::SocketBase> ptr_, int type_ ) : objectType(ot), ptr( nodecpp::safememory::soft_ptr_static_cast<UnderlyingType>(ptr_)), type(type_), nodePtr( node ) {}
		OpaqueEmitter( ObjectType ot, NodeBase* node, nodecpp::safememory::soft_ptr<net::SocketBase> ptr_, int type_ ) : objectType(ot), ptr( ptr_), type(type_), nodePtr( node ) {}
//		OpaqueEmitter( ObjectType ot, NodeBase* node, nodecpp::safememory::soft_ptr<net::ServerTBase> ptr_, int type_ ) : objectType(ot), ptr( nodecpp::safememory::soft_ptr_static_cast<UnderlyingType>(ptr_)), type(type_), nodePtr( node ) {}
		OpaqueEmitter( ObjectType ot, NodeBase* node, nodecpp::safememory::soft_ptr<net::ServerTBase> ptr_, int type_ ) : objectType(ot), ptr( ptr_), type(type_), nodePtr( node ) {}
		bool isValid() const { return (bool)ptr; }
		nodecpp::safememory::soft_ptr<net::SocketBase> getClientSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ClientSocket ); return nodecpp::safememory::soft_ptr_static_cast<net::SocketBase>(ptr); }
//		const net::SocketBase* getClientSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ClientSocket ); return static_cast<const net::SocketBase*>(ptr); }
		nodecpp::safememory::soft_ptr<net::ServerTBase> getServerSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ServerSocket ); return nodecpp::safememory::soft_ptr_static_cast<net::ServerTBase>(ptr); }
//		const net::ServerTBase* getServerSocketPtr() const { NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, objectType == ObjectType::ServerSocket ); return static_cast<const net::ServerTBase*>(ptr); }
	};
} // namespace nodecpp

#endif // SOCKET_T_BASE_H
