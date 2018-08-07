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

#ifndef SOCKET_H
#define SOCKET_H

#define USING_O_SOCKETS // TODO: make project-level define as soon as related dev and testing done
#define USING_L_SOCKETS // TODO: make project-level define as soon as related dev and testing done

#ifdef USING_O_SOCKETS
#include "socket_o.h"
#endif // USING_O_SOCKETS
#ifdef USING_L_SOCKETS
#include "socket_l.h"
#endif // USING_L_SOCKETS

namespace nodecpp {

	namespace net {

#if (defined USING_O_SOCKETS) && (defined USING_O_SOCKETS)

		class LOSocketEmitter
		{
			enum SocketType { Uninitialized, Lambda, Inheritance } st = Uninitialized;
			void* ptr = nullptr;
		public:
			LOSocketEmitter() {}
			LOSocketEmitter(net::Socket* s) { st = SocketType::Lambda; ptr = s; }
			LOSocketEmitter(net::SocketO* so) { st = SocketType::Inheritance; ptr = so; }
			void init(net::Socket* s) { NODECPP_ASSERT( st = SocketType::Uninitialized ); st = SocketType::Lambda; ptr = s; }
			void init(net::SocketO* so) { NODECPP_ASSERT( st = SocketType::Uninitialized ); st = SocketType::Lambda; ptr = so; }
		
			bool isValid() const { return ptr != nullptr; }
		
			void emitClose(bool hadError) const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitClose(hadError); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onClose(hadError); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
			void emitConnect() const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitConnect(); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onConnect(); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
			void emitData(Buffer& buffer) const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitData(buffer); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onData(buffer); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
			void emitDrain() const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitDrain(); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onDrain(); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
			void emitEnd() const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitEnd(); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onEnd(); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
			void emitError(Error& err) const
			{
				switch ( st )
				{
					case SocketType::Lambda: reinterpret_cast<net::Socket*>(ptr)->emitError(err); break;
					case SocketType::Inheritance: reinterpret_cast<net::SocketO*>(ptr)->onError(err); break;
					default: NODECPP_ASSERT( false ); break;
				}
			}
		};
		
		using SocketEmitter = LOSocketEmitter;
		
#elif defined USING_O_SOCKETS

		class LSocketEmitter
		{
			net::Socket* ptr = nullptr;
		public:
			LSocketEmitter() {}
			LSocketEmitter(net::Socket* s) { ptr = s; }
			void init(net::Socket* s) { ptr = s; }
		
			bool isValid() const { return ptr != nullptr; }
		
			void emitClose(bool hadError) const { return ptr->emitClose(hadError); }
			void emitConnect() const { return ptr->emitConnect(); }
			void emitData(Buffer& buffer) const { return ptr->emitData(buffer); }
			void emitDrain() const { return ptr->emitDrain(); }
			void emitEnd() const { return ptr->emitEnd(); }
			void emitError(Error& err) const { return ptr->emitError(); }
		};
		
		using SocketEmitter = LSocketEmitter;		

#elif defined USING_O_SOCKETS

		class OSocketEmitter
		{
			net::SocketO* ptr = nullptr;
		public:
			OSocketEmitter() {}
			OSocketEmitter(net::SocketO* s) { ptr = s; }
			void init(net::SocketO* s) { ptr = s; }
		
			bool isValid() const { return ptr != nullptr; }
		
			void emitClose(bool hadError) const { return ptr->onClose(hadError); }
			void emitConnect() const { return ptr->onConnect(); }
			void emitData(Buffer& buffer) const { return ptr->onData(buffer); }
			void emitDrain() const { return ptr->onDrain(); }
			void emitEnd() const { return ptr->onEnd(); }
			void emitError(Error& err) const { return ptr->onError(); }
		};
		
		using SocketEmitter = OSocketEmitter;
		
#endif // USING_O_SOCKETS, defined USING_O_SOCKETS


	} //namespace net

} //namespace nodecpp

#endif //SOCKET_H
