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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "http_server_common.h"
#include "http_socket_at_server.h"

// NOTE: current implementation is anty-optimal; it's just a sketch of what could be in use

namespace nodecpp {

	namespace net {

		class HttpServerBase; // forward declaration
		class HttpSocketBase; // forward declaration

		class IncomingHttpMessageAtServer; // forward declaration
		class HttpServerResponse; // forward declaration



		template<class ServerT, class RequestT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			static_assert( std::is_base_of< HttpServerBase, ServerT >::value );
			static_assert( std::is_base_of< IncomingHttpMessageAtServer, RequestT >::value );
			auto retServer = createServer<ServerT, HttpSocketBase, Types ...>(::std::forward<Types>(args)...);
			retServer->dataForHttpCommandProcessing.userHandlers.from(HttpServerBase::DataForHttpCommandProcessing::userHandlerClassPattern.getPatternForApplying<ServerT>(), &(*retServer));
			return retServer;
		}

		template<class ServerT, class ... Types>
		static
		nodecpp::safememory::owning_ptr<ServerT> createHttpServer(Types&& ... args) {
			return createHttpServer<ServerT, IncomingHttpMessageAtServer, Types ...>(::std::forward<Types>(args)...);
		}

	} //namespace net
} //namespace nodecpp

#endif //HTTP_SERVER_H
