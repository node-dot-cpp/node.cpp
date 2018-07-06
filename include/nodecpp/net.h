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
#include <cassert>

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
				size_t cp = std::max(sz, MIN_BUFFER);
				std::unique_ptr<uint8_t> tmp(static_cast<uint8_t*>(malloc(cp)));


				memcpy(tmp.get(), _data.get(), _size);
				memcpy(tmp.get() + _size, dt, sz);

				_size += sz;
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


		class Socket {
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

			void emitClose(bool hadError) {
				state = DESTROYED;
				this->id = 0;
				onClose(hadError);
			}

			// not in node.js
			void emitAccepted(size_t id) {
				this->id = id;
				state = CONNECTED;
			}

			void emitConnect() {
				state = CONNECTED;
				onConnect();
			}

			void emitData(Buffer buffer) {
				_bytesRead += buffer.size();
				onData(std::move(buffer));
			}

			void emitDrain() {
				onDrain();
			}

			void emitEnd() {
				onEnd();
			}

			void emitError() {
				state = DESTROYED;
				this->id = 0;
				onError();
			}


			virtual void onClose(bool hadError) {}
			virtual void onConnect() {}
			virtual void onData(Buffer buffer) {}
			virtual void onDrain() {}
			virtual void onEnd() {}
			virtual void onError() {}

			const Address& address() const { return _local; }

			size_t bufferSize() const;
			size_t bytesRead() const { return _bytesRead; }
			size_t bytesWritten() const { return _bytesWritten; }

			void connect(uint16_t port, const char* ip);

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

			void resume();

			Socket& setNoDelay(bool noDelay = true);
			Socket& setKeepAlive(bool enable = false);

			bool write(const uint8_t* data, uint32_t size);
		};

		class Server {
			Address localAddress;
			uint16_t localPort = 0;

			size_t index = 0;
			enum State { UNINITIALIZED = 0, LISTENING, CLOSED } state = UNINITIALIZED;

		public:
			Server() {}

			void emitClose(bool hadError) {
				state = CLOSED;
				index = 0;
				localAddress = Address();
				onClose(hadError);
			}

			void emitConnection(std::unique_ptr<Socket> socket) {
				onConnection(std::move(socket));
			}

			void emitListening(size_t id, Address addr) {
				index = id;
				localAddress = std::move(addr);
				state = LISTENING;
				onListening();
			}

			void emitError() {
				onError();
			}


			virtual void onClose(bool hadError) {}
			virtual void onConnection(std::unique_ptr<Socket> socket) {}
			virtual void onListening() {}
			virtual void onError() {}

			virtual std::unique_ptr<Socket> makeSocket() {
				return std::unique_ptr<Socket>(new Socket());
			}

			const Address& address() const { return localAddress; }
			void close();
			void listen(uint16_t port, const char* ip, int backlog);
			bool listening() const { return state == LISTENING; }

		};

		//TODO don't use naked pointers, think
		template<class T>
		T* createServer() {
			return new T();
		}

		template<class T>
		T* createConnection(uint16_t port, const char* host) {
			auto cli = new T();
			cli->connect(port, host);
			return cli;
		}


	} //namespace net
} //namespace nodecpp

#endif //NET_H
