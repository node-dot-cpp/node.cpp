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
#include "socket_t_base.h"

namespace nodecpp {

	namespace net {

		class Socket : public SocketBase {
			class EventEmitterSupportingListeners<event::Close, SocketListener, &SocketListener::onClose> eClose;
			class EventEmitterSupportingListeners<event::Connect, SocketListener, &SocketListener::onConnect> eConnect;
			class EventEmitterSupportingListeners<event::Data, SocketListener, &SocketListener::onData> eData;
			class EventEmitterSupportingListeners<event::Drain, SocketListener, &SocketListener::onDrain> eDrain;
			class EventEmitterSupportingListeners<event::End, SocketListener, &SocketListener::onEnd> eEnd;
			class EventEmitterSupportingListeners<event::Error, SocketListener, &SocketListener::onError> eError;
			class EventEmitterSupportingListeners<event::Accepted, SocketListener, &SocketListener::onAccepted> eAccepted;

			std::vector<nodecpp::safememory::owning_ptr<SocketListener>> ownedListeners;

			void registerMeAndAcquireSocket();
			void registerMeAndAssignSocket(OpaqueSocketData& sdata);

		public:
			Socket() : SocketBase( nullptr ) {registerMeAndAcquireSocket();}
			Socket(OpaqueSocketData& sdata) : SocketBase( nullptr ) {registerMeAndAssignSocket(sdata);}

			Socket(const Socket&) = delete;
			Socket& operator=(const Socket&) = delete;

			Socket(Socket&&) = default;
			Socket& operator=(Socket&&) = default;

			void emitClose(bool hadError) {
				state = DESTROYED;
				unref();
				//this->dataForCommandProcessing.id = 0;
				//handler may release, put virtual onClose first.
				eClose.emit(hadError);
			}

			// not in node.js
			void emitAccepted() {
				state = CONNECTED;
				eAccepted.emit();
			}

			void emitConnect() {
				state = CONNECTED;
				eConnect.emit();
			}

			void emitData(Buffer& buffer) {
				_bytesRead += buffer.size();
				eData.emit(std::ref(buffer));
			}

			void emitDrain() {
				eDrain.emit();
			}

			void emitEnd() {
				eEnd.emit();
			}

			void emitError(Error& err) {
				state = DESTROYED;
				//this->dataForCommandProcessing.id = 0;
				eError.emit(err);
			}

			void connect(uint16_t port, const char* ip, std::function<void()> cb) {
				once(event::connect, std::move(cb));
				connect(port, ip);
			}

			bool write(Buffer& buff) { return SocketBase::write( buff ); }
			bool write(const uint8_t* data, uint32_t size) { return SocketBase::write( data, size ); }
			bool write(const uint8_t* data, uint32_t size, std::function<void()> cb) {
				bool b = SocketBase::write(data, size);
				if(!b)
					once(event::drain, std::move(cb));

				return b;
			}

			void connect(uint16_t port, const char* ip);
			Socket& setNoDelay(bool noDelay = true) { OSLayer::appSetNoDelay(dataForCommandProcessing, noDelay); return *this; }
			Socket& setKeepAlive(bool enable = false) { OSLayer::appSetKeepAlive(dataForCommandProcessing, enable); return *this; }



			void on( nodecpp::safememory::owning_ptr<SocketListener> l) {
				eClose.on(std::move(l));
				eConnect.on(std::move(l));
				eData.on(std::move(l));
				eDrain.on(std::move(l));
				eError.on(std::move(l));
				eEnd.on(std::move(l));
				eAccepted.on(std::move(l));
			}

			void once( nodecpp::safememory::owning_ptr<SocketListener> l) {
				eClose.once(std::move(l));
				eConnect.once(std::move(l));
				eData.once(std::move(l));
				eDrain.once(std::move(l));
				eError.once(std::move(l));
				eEnd.once(std::move(l));
				eAccepted.once(std::move(l));
			}

			void on( nodecpp::safememory::owning_ptr<SocketListener>& l) {
				ownedListeners.emplace_back( std::move( l ) );
				on( std::move(l) );
			}

			void once( nodecpp::safememory::owning_ptr<SocketListener>& l) {
				ownedListeners.emplace_back( std::move( l ) );
				once( std::move(l) );
			}

			void on( std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.on(std::move(cb));
			}
			void on( std::string name, event::Data::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.on(std::move(cb));
			}
			void on(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.on(std::move(cb));
			}
			void on( std::string name, event::Connect::callback cb [[nodecpp::may_extend_to_this]]) {
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
				else if (name == event::Accepted::name)
					eAccepted.on(std::move(cb));
				else
					assert(false);
			}

			void once( std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Close::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Data::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Close::callback, event::Error::callback >::value );
				assert( name == event::Close::name );
				eClose.once(std::move(cb));
			}
			void once( std::string name, event::Data::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert( !std::is_same< event::Data::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Connect::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Drain::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::End::callback >::value );
				static_assert( !std::is_same< event::Data::callback, event::Error::callback >::value );
				assert( name == event::Data::name );
				eData.once(std::move(cb));
			}
			void once(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connect::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Drain::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::End::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Data::callback >::value);
				assert(name == event::Error::name);
				eError.once(std::move(cb));
			}
			void once( std::string name, event::Connect::callback cb [[nodecpp::may_extend_to_this]]) {
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
				else if (name == event::Accepted::name)
					eAccepted.once(std::move(cb));
				else
					assert(false);
			}

			template<class EV>
			void on( EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.on(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.on(std::move(cb)); }
				else assert(false);
			}

			template<class EV>
			void once( EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr ( std::is_same< EV, event::Close >::value ) { eClose.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Connect >::value ) { eConnect.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Data >::value ) { eData.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Drain >::value ) { eDrain.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::End >::value ) { eEnd.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Error >::value ) { eError.once(std::move(cb)); }
				else if constexpr ( std::is_same< EV, event::Accepted >::value ) { eAccepted.once(std::move(cb)); }
				else assert(false);
			}
		};

	} // namespace net

} // namespace nodecpp

#endif // SOCKET_L_H
