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

#ifndef SERVER_T_BASE_H
#define SERVER_T_BASE_H

#include "template_common.h"
#include "server_common.h"
#include "../../src/tcp_socket/tcp_socket_base.h"

namespace nodecpp {

	namespace net {

		class ServerTBase : public ServerBase {
		
		public:
//			UserDefID userDefID;
			NodeBase* node = nullptr;

		public:
			ServerTBase() {}

			ServerTBase(const ServerTBase&) = delete;
			ServerTBase& operator=(const ServerTBase&) = delete;

			ServerTBase(ServerTBase&&) = default;
			ServerTBase& operator=(ServerTBase&&) = default;

			~ServerTBase() {}
		};


		struct OpaqueEmitterForServer
		{
			using PtrType = nodecpp::net::ServerTBase*;
			PtrType ptr = nullptr;
			int type = -1;
			NodeBase* nodePtr = nullptr;
			OpaqueEmitterForServer() : ptr( nullptr), type(-1) {}
			OpaqueEmitterForServer( NodeBase* node, PtrType ptr_, int type_ ) : ptr( ptr_), type(type_), nodePtr( node ) {}
			bool isValid() const { return ptr != nullptr; }
		};

	} // namespace net

} // namespace nodecpp

#endif // SERVER_T_BASE_H
