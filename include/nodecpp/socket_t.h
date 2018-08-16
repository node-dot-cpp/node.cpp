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

#ifndef SOCKET_T_H
#define SOCKET_T_H

#include <stdio.h>
#include "template_common.h"
#include "net_common.h"

namespace nodecpp {

	namespace net {

		template<class UserDefID>
		class SocketT : public SocketBase {
		
		public:
			UserDefID userDefID;

		public:
			SocketT() {}

			SocketT(const SocketT&) = delete;
			SocketT& operator=(const SocketT&) = delete;

			SocketT(SocketT&&) = default;
			SocketT& operator=(SocketT&&) = default;

			~SocketT() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			void connect(uint16_t port, const char* ip);
			SocketT& setNoDelay(bool noDelay = true);
			SocketT& setKeepAlive(bool enable = false);
		};

#if 0
		template<class T, class T1, class Extra, class ... args>
		void emitConnect( T* ptr, int type, Extra* extra, bool ok )
		{
			if ( type == 0 )
			{
				reinterpret_cast<T1*>(ptr)->doOne(ok);
			}
			else
				callDoOne<T, args...>(ptr, type-1, ok);
		}

		template<class T>
		void emitConnect( T* ptr, int type, Extra* extra, bool ok )
		{
			assert( false );
		}

		template< class ... args >
		class Emitter
		{
		public:
			class Ptr
			{
				void* ptr;
			public:
				Ptr() {}
				void init( void* ptr_ ) { ptr = ptr_; }
				void* getPtr() {return ptr;}
			};

		protected:
			Ptr ptr;
			int type = -1;

		public:
			Emitter() {}

			template<class Sock>
			void init(Sock* s)
			{ 
				ptr.init(s);
				type = getTypeIndex<Sock,args...>( s );
			}

			void emitOne( bool ok ) { callDoOne<Ptr, args...>(&ptr, this->type, ok); }
			void emitTwo( bool ok ) { callDoTwo<Ptr, args...>(&ptr, this->type, ok); }
			void emitThree() { callDoThree<Ptr, args...>(&ptr, this->type); }
		};
#endif // 0

	} // namespace net

} // namespace nodecpp

#endif // SOCKET_T_H
