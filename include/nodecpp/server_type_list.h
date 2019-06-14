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

namespace nodecpp {

	namespace net {

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
