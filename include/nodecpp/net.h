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

#ifndef NET_H
#define NET_H

#include "common.h"
#include "event.h"

namespace nodecpp {

	//TODO quick and temp implementation
	static constexpr size_t MIN_BUFFER = 1024;

	class Buffer {
	private:
		size_t _size = 0;
		size_t _capacity = 0;
		std::unique_ptr<uint8_t> _data;

	public:
		Buffer() {}
		Buffer(size_t res) { reserve(res); }
		Buffer(Buffer&& p) {
			std::swap(_size, p._size);
			std::swap(_capacity, p._capacity);
			std::swap(_data, p._data);
		}
		Buffer& operator = (Buffer&& p) {
			std::swap(_size, p._size);
			std::swap(_capacity, p._capacity);
			std::swap(_data, p._data);
			return *this;
		}
		Buffer(const Buffer&) = delete;
		Buffer& operator = (const Buffer& p) = delete;

		Buffer clone() {
			Buffer cp(size());
			cp.append(begin(), size());
			return cp;
		}

		void reserve(size_t sz) {
			assert(_size == 0);
			assert(_capacity == 0);
			assert(_data == nullptr);

			size_t cp = std::max(sz, MIN_BUFFER);
			std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));

			_capacity = cp;
			_data = std::move(tmp);
		}

		void append(const uint8_t* dt, size_t sz) { // NOTE: may invalidate pointers
			if (_data == nullptr) {
				reserve(sz);
				memcpy(end(), dt, sz);
				_size += sz;
			}
			else if (_size + sz <= _capacity) {
				memcpy(end(), dt, sz);
				_size += sz;
			}
			else {
				size_t cp = std::max(sz + _size, MIN_BUFFER);
				std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));


				memcpy(tmp.get(), _data.get(), _size);
				memcpy(tmp.get() + _size, dt, sz);

				_size = sz + _size;
				_capacity = cp;
				_data = std::move(tmp);
			}
		}

		void trim(size_t sz) { // NOTE: keeps pointers
			assert(sz <= _size);
			assert(_data != nullptr);
			_size -= sz;
		}

		void clear() {
			trim(size());
		}

		size_t size() const { return _size; }
		bool empty() const { return _size == 0; }
		size_t capacity() const { return _capacity; }
		uint8_t* begin() { return _data.get(); }
		const uint8_t* begin() const { return _data.get(); }
		uint8_t* end() { return _data.get() + _size; }
		const uint8_t* end() const { return _data.get() + _size; }

		//TODO mb: to avoid memmove things around, popFront and pushFront
		// may be implemented by always
		// reserving some bytes at front, and using an extra header pointer
		// like linux kernel skbuff does.
		void popFront(size_t sz) {
			assert(sz <= size());
			size_t remaining = size() - sz;
			memmove(begin(), begin() + sz, remaining);
			trim(sz);
		}

	};



	namespace net {


		struct Address {
			uint16_t port;
			std::string family;
			std::string address;
		};


		class Socket /*:EventEmitter<event::Close>, EventEmitter<event::Connect> ,
			EventEmitter<event::Data> , EventEmitter<event::Drain> ,
			EventEmitter<event::End>, EventEmitter<event::Error>*/ {
			EventEmitter<event::Close> eClose;
			EventEmitter<event::Connect> eConnect;
			EventEmitter<event::Data> eData;
			EventEmitter<event::Drain> eDrain;
			EventEmitter<event::End> eEnd;
			EventEmitter<event::Error> eError;

			size_t recvSize = 0;
			size_t sentSize = 0;
			std::unique_ptr<uint8_t> ptr;
			size_t size = 64 * 1024;

			size_t id = 0;
			Address _local;
			Address _remote;
			//std::string _remoteAddress;
			//std::string _remoteFamily;
			//uint16_t _remotePort = 0;
			size_t _bytesRead = 0;
			size_t _bytesWritten = 0;

			enum State { UNINITIALIZED = 0, CONNECTING, CONNECTED, DESTROYED } state = UNINITIALIZED;

		public:
			Socket() {}

			Socket(const Socket&) = delete;
			Socket& operator=(const Socket&) = delete;

			Socket(Socket&&) = default;
			Socket& operator=(Socket&&) = default;

			virtual ~Socket() { if (state == CONNECTING || state == CONNECTED) destroy(); }

			void emitClose(bool hadError) {
				state = DESTROYED;
				this->id = 0;
//				EventEmitter<event::Close>::emit(hadError);
				eClose.emit(hadError);
				onClose(hadError);
			}

			// not in node.js
			void emitAccepted(size_t id) {
				this->id = id;
				state = CONNECTED;
			}

			void emitConnect() {
				state = CONNECTED;
//				EventEmitter<event::Connect>::emit();
				eConnect.emit();
				onConnect();
			}

			void emitData(Buffer& buffer) {
				_bytesRead += buffer.size();
//				EventEmitter<event::Data>::emit(std::ref(buffer));
				eData.emit(std::ref(buffer));
				onData(buffer);
			}

			void emitDrain() {
//				EventEmitter<event::Drain>::emit();
				eDrain.emit();
				onDrain();
			}

			void emitEnd() {
				EventEmitter<event::End>::emit();
				onEnd();
			}

			void emitError() {
				state = DESTROYED;
				this->id = 0;
				EventEmitter<event::Error>::emit();
				onError();
			}


/*			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer& buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onError() {}*/

			virtual void onClose(bool hadError) {
				fmt::print("onClose!\n");
			}

			virtual void onConnect() {
				fmt::print("onConnect!\n");
				ptr.reset(static_cast<uint8_t*>(malloc(size)));

				bool ok = true;
				while (ok) {
					ok = write(ptr.get(), size);
					sentSize += size;
				}
			}

			virtual void onData(Buffer& buffer) {
				fmt::print("onData!\n");
				recvSize += buffer.size();

				if (recvSize >= sentSize)
					end();
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
					ok = write(ptr.get(), size, [] { fmt::print("onDrain!\n"); });
					sentSize += size;
				}
			}

			void didData(Buffer& buffer) {
				fmt::print("onData!\n");
				recvSize += buffer.size();

				if (recvSize >= sentSize)
					end();
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

			const Address& address() const { return _local; }

			size_t bufferSize() const;
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			void connect(uint16_t port, const char* ip);
			void connect(uint16_t port, const char* ip, std::function<void()> cb) {
				once<event::Connect>(std::move(cb));
				connect(port, ip);
			}

			bool connecting() const { return state == CONNECTING; }
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

			Socket& setNoDelay(bool noDelay = true);
			Socket& setKeepAlive(bool enable = false);

			void unref();
			bool write(const uint8_t* data, uint32_t size);
			bool write(const uint8_t* data, uint32_t size, std::function<void()> cb) {
				bool b = write(data, size);
				if(!b)
					once<event::Drain>(std::move(cb));

				return b;
			}



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
			void on( std::string name, event::Connect::callback cb) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::Error::callback >::value );
				if ( name == event::Drain::name )
					eDrain.on(std::move(cb));
				else if ( name == event::Connect::name )
					eConnect.on(std::move(cb));
				else if ( name == event::End::name )
					eEnd.on(std::move(cb));
				else if ( name == event::Error::name )
					eError.on(std::move(cb));
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
			void once( std::string name, event::Connect::callback cb) {
				static_assert( !std::is_same< event::Connect::callback, event::Close::callback >::value );
				static_assert( !std::is_same< event::Connect::callback, event::Data::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::Drain::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::End::callback >::value );
				static_assert( std::is_same< event::Connect::callback, event::Error::callback >::value );
				if ( name == event::Drain::name )
					eDrain.once(std::move(cb));
				else if ( name == event::Connect::name )
					eConnect.once(std::move(cb));
				else if ( name == event::End::name )
					eEnd.once(std::move(cb));
				else if ( name == event::Error::name )
					eError.once(std::move(cb));
			}

			template<class EV>
			void on( typename EV, typename EV::callback cb) {
				if constexpr ( (int)(EV::type) == (int)(event::EventType::Close) ) { eClose.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Connect) ) { eConnect.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Data) ) { eData.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Drain) ) { eDrain.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::End) ) { eEnd.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Error) ) { eError.on(std::move(cb)); }
			}

			template<class EV>
			void once( typename EV, typename EV::callback cb) {
				if constexpr ( (int)(EV::type) == (int)(event::EventType::Close) ) { eClose.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Connect) ) { eConnect.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Data) ) { eData.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Drain) ) { eDrain.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::End) ) { eEnd.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Error) ) { eError.once(std::move(cb)); }
			}

			template<class EV>
			void on(typename EV::callback cb) {
				if constexpr ( (int)(EV::type) == (int)(event::EventType::Close) ) { eClose.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Connect) ) { eConnect.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Data) ) { eData.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Drain) ) { eDrain.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::End) ) { eEnd.on(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Error) ) { eError.on(std::move(cb)); }
			}

			template<class EV>
			void once(typename EV::callback cb) {
				if constexpr ( (int)(EV::type) == (int)(event::EventType::Close) ) { eClose.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Connect) ) { eConnect.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Data) ) { eData.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Drain) ) { eDrain.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::End) ) { eEnd.once(std::move(cb)); }
				else if constexpr ( (int)(EV::type) == (int)(event::EventType::Error) ) { eError.once(std::move(cb)); }
			}
/*			template<class EV>
			void on(typename EV::callback cb) {
				EventEmitter<EV>::once(std::move(cb));

			template<class EV>
			void once(typename EV::callback cb) {
				EventEmitter<EV>::once(std::move(cb));
			}*/
		};

		class Server :EventEmitter<event::Close>, EventEmitter<event::Connection>,
			EventEmitter<event::Listening>, EventEmitter<event::Error> {
			Address localAddress;
			uint16_t localPort = 0;

			size_t id = 0;
			enum State { UNINITIALIZED = 0, LISTENING, CLOSED } state = UNINITIALIZED;

		public:
			Server() {}
			virtual ~Server() {}

			void emitClose(bool hadError) {
				state = CLOSED;
				id = 0;
				EventEmitter<event::Close>::emit(hadError);
				onClose(hadError);
			}

			void emitConnection(Socket* socket) {
				EventEmitter<event::Connection>::emit(socket);
				onConnection(socket);
			}

			void emitListening(size_t id, Address addr) {
				this->id = id;
				localAddress = std::move(addr);
				state = LISTENING;
				EventEmitter<event::Listening>::emit();
				onListening();
			}

			void emitError() {
				EventEmitter<event::Error>::emit();
				onError();
			}


			virtual void onClose(bool hadError) {}
			virtual void onConnection(Socket* socket) {}
			virtual void onListening() {}
			virtual void onError() {}

			virtual Socket* makeSocket() {
				return new Socket();
			}

			const Address& address() const { return localAddress; }
			void close();
			void close(std::function<void(bool)> cb) {
				once<event::Close>(std::move(cb));
				close();
			}

			void listen(uint16_t port, const char* ip, int backlog);
			void listen(uint16_t port, const char* ip, int backlog, std::function<void()> cb) {
				once<event::Listening>(std::move(cb));
				listen(port, ip, backlog);
			}
			bool listening() const { return state == LISTENING; }
			void ref();
			void unref();

			template<class EV>
			void on(typename EV::callback cb) {
				EventEmitter<EV>::on(std::move(cb));
			}

			template<class EV>
			void once(typename EV::callback cb) {
				EventEmitter<EV>::once(std::move(cb));
			}

		};

		//TODO don't use naked pointers, think
		template<class T>
		T* createServer() {
			return new T();
		}
		template<class T>
		T* createServer(std::function<void()> cb) {
			auto svr = new T();
			svr->on<event::Connect>(cb);
			return svr;
		}

		template<class T>
		T* createConnection(uint16_t port, const char* host) {
			auto cli = new T();
			cli->connect(port, host);
			return cli;
		}

		template<class T>
		T* createConnection(uint16_t port, const char* host, std::function<void()> cb) {
			auto cli = new T();
			cli->connect(port, host, std::move(cb));
			return cli;
		}

	} //namespace net
} //namespace nodecpp

#endif //NET_H
