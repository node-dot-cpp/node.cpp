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

#ifndef SOCKET_L_H
#define SOCKET_L_H

#include "net_common.h"

namespace nodecpp {

	namespace net {

		class Socket : public SocketBase {
			EventEmitter<event::Close> eClose;
			EventEmitter<event::Connect> eConnect;
			EventEmitter<event::Data> eData;
			EventEmitter<event::Drain> eDrain;
			EventEmitter<event::End> eEnd;
			EventEmitter<event::Error> eError;

/*			//size_t recvSize = 0;
			//size_t sentSize = 0;
			//std::unique_ptr<uint8_t> ptr;
			//size_t size = 64 * 1024;

			size_t id = 0;
			Address _local;
			Address _remote;
			//std::string _remoteAddress;
			//std::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			enum State { UNINITIALIZED = 0, CONNECTING, CONNECTED, DESTROYED } state = UNINITIALIZED;*/

		public:
			Socket() {}

			Socket(const Socket&) = delete;
			Socket& operator=(const Socket&) = delete;

			Socket(Socket&&) = default;
			Socket& operator=(Socket&&) = default;

//			virtual ~Socket() { if (state == CONNECTING || state == CONNECTED) destroy(); }
			~Socket() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			void emitClose(bool hadError) {
				state = DESTROYED;
				this->dataForCommandProcessing.id = 0;
				//handler may release, put virtual onClose first.
//				onClose(hadError);
				eClose.emit(hadError);
			}

			// not in node.js
			void emitAccepted(size_t id) {
				this->dataForCommandProcessing.id = id;
				state = CONNECTED;
			}

			void emitConnect() {
				state = CONNECTED;
//				EventEmitter<event::Connect>::emit();
				eConnect.emit();
//				onConnect();
			}

			void emitData(Buffer& buffer) {
				_bytesRead += buffer.size();
//				EventEmitter<event::Data>::emit(std::appRef(buffer));
				eData.emit(std::ref(buffer));
//				onData(buffer);
			}

			void emitDrain() {
//				EventEmitter<event::Drain>::emit();
				eDrain.emit();
//				onDrain();
			}

			void emitEnd() {
				//EventEmitter<event::End>::emit();
				eEnd.emit();
//				onEnd();
			}

			void emitError(Error& err) {
				state = DESTROYED;
				this->dataForCommandProcessing.id = 0;
				//EventEmitter<event::Error>::emit();
				eError.emit(err);
//				onError(err);
			}


/*			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onError(Error& err) {}*/
/*
			virtual void onClose(bool hadError) {
				fmt::print("onClose!\n");
			}

			virtual void onConnect() {
				fmt::print("onConnect!\n");
				ptr.reset(static_cast<uint8_t*>(malloc(size)));

				bool ok = true;
				while (ok) {
					ok = appWrite(ptr.get(), size);
					sentSize += size;
				}
			}

			virtual void onData(Buffer& buffer) {
				fmt::print("onData!\n");
				recvSize += buffer.size();

				if (recvSize >= sentSize)
					appEnd();
			}

			virtual void onDrain() {
				fmt::print("onDrain!\n");
			}

			virtual void onEnd() {
				fmt::print("onEnd!\n");
			}

			virtual void onError() {
				fmt::print("onError!\n");
			}

			void didClose(bool hadError) {
				fmt::print("onClose!\n");
			}

			void didConnect() {
				fmt::print("onConnect!\n");
				ptr.reset(static_cast<uint8_t*>(malloc(size)));

				bool ok = true;
				while (ok) {
					ok = appWrite(ptr.get(), size, [] { fmt::print("onDrain!\n"); });
					sentSize += size;
				}
			}

			void didData(Buffer& buffer) {
				fmt::print("onData!\n");
				recvSize += buffer.size();

				if (recvSize >= sentSize)
					appEnd();
			}

			//void didDrain() {
			//	print("onDrain!\n");
			//}

			void didEnd() {
				fmt::print("onEnd!\n");
			}

			void didError() {
				fmt::print("onError!\n");
			}
			*/
/*			const Address& address() const { return _local; }

			size_t bufferSize() const;
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			void connect(uint16_t port, const char* ip);*/
			void connect(uint16_t port, const char* ip, std::function<void()> cb) {
				once(event::connect, std::move(cb));
				connect(port, ip);
			}

/*			bool connecting() const { return state == CONNECTING; }
			void destroy();
			bool destroyed() const { return state == DESTROYED; };
			void end();

			const std::string& localAddress() const { return _local.address; }
			uint16_t localPort() const { return _local.port; }

			void pause();

			const std::string& remoteAddress() const { return _remote.address; }
			const std::string& remoteFamily() const { return _remote.family; }
			uint16_t remotePort() const { return _remote.port; }

			void ref();
			void resume();

			void unref();*/
			bool write(const uint8_t* data, uint32_t size) { return SocketBase::write( data, size ); }
			bool write(const uint8_t* data, uint32_t size, std::function<void()> cb) {
				bool b = SocketBase::write(data, size);
				if(!b)
					once(event::drain, std::move(cb));

				return b;
			}

			void connect(uint16_t port, const char* ip);
			Socket& setNoDelay(bool noDelay = true);
			Socket& setKeepAlive(bool enable = false);



			void on( std::string name, event::Close::callback cb) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.on(std::move(cb));
			}
			void on( std::string name, event::Data::callback cb) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.on(std::move(cb));
			}
			void on(std::string name, event::Error::callback cb) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.on(std::move(cb));
			}
			void on( std::string name, event::Connect::callback cb) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if (name == event::Drain::name)
					eDrain.on(std::move(cb));
				else if (name == event::Connect::name)
					eConnect.on(std::move(cb));
				else if (name == event::End::name)
					eEnd.on(std::move(cb));
				else
					assert(false);
			}

			void once( std::string name, event::Close::callback cb) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.once(std::move(cb));
			}
			void once( std::string name, event::Data::callback cb) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.once(std::move(cb));
			}
			void once(std::string name, event::Error::callback cb) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.once(std::move(cb));
			}
			void once( std::string name, event::Connect::callback cb) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Error::callback >::value);
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				if (name == event::Drain::name)
					eDrain.once(std::move(cb));
				else if (name == event::Connect::name)
					eConnect.once(std::move(cb));
				else if (name == event::End::name)
					eEnd.once(std::move(cb));
				else
					assert(false);
			}

			template<class EV>
			void on( EV, typename EV::callback cb) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.on(std::move(cb)); }
				else assert(false);
			}

			template<class EV>
			void once( EV, typename EV::callback cb) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.once(std::move(cb)); }
				else assert(false);
			}
		};

	} // namespace net

} // namespace nodecpp

#endif // SOCKET_L_H
