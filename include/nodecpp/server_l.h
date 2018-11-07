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

#ifndef SERVER_L_H
#define SERVER_L_H

#include "server_t_base.h"
#include "socket_l.h"

namespace nodecpp {

	namespace net {

		class Server : public ServerTBase
		{
			// support for a list of accepted sockets
			class SocketList;
			class SocketForserver : public Socket
			{
				friend class Server;
				friend class SocketList;
				SocketForserver* _prevSock;
				SocketForserver* _nextSock;
			public:
				SocketForserver() : Socket() {}
				SocketForserver(OpaqueSocketData& sdata) : Socket( sdata ) {}

				SocketForserver(const SocketForserver&) = delete;
				SocketForserver& operator=(const SocketForserver&) = delete;

				SocketForserver(SocketForserver&&) = default;
				SocketForserver& operator=(SocketForserver&&) = default;
			};

			class SocketList
			{
				size_t _size = 0;
				SocketForserver* _begin = nullptr;
			public:
				SocketForserver* getNewSocket(OpaqueSocketData& sdata)
				{
					SocketForserver* sock = new SocketForserver(sdata);
					NODECPP_ASSERT( sock != nullptr );
					NODECPP_ASSERT( _begin == nullptr || _begin->_prevSock == nullptr );
					if ( _begin == nullptr )
					{
						_begin = sock;
						sock->_nextSock = nullptr;
						sock->_prevSock = nullptr;
					}
					else
					{
						sock->_nextSock = _begin;
						_begin->_prevSock = sock;
						sock->_prevSock = nullptr;
						_begin = sock;
					}
					NODECPP_ASSERT( _begin == nullptr || _begin->_prevSock == nullptr );
					++_size;
					return sock;
				}
				void removeAndDelete( SocketForserver* sock )
				{
					NODECPP_ASSERT( sock != nullptr );
					NODECPP_ASSERT( _begin == nullptr || _begin->_prevSock == nullptr );
					if ( _begin == sock )
					{
						NODECPP_ASSERT( sock == _begin );
						_begin = sock->_nextSock;
					}
					if ( sock->_prevSock )
						sock->_prevSock->_nextSock = sock->_nextSock;
					if ( sock->_nextSock )
						sock->_nextSock->_prevSock = sock->_prevSock;
					--_size;
					NODECPP_ASSERT( _begin == nullptr || _begin->_prevSock == nullptr );
					delete sock;
				}
				void clear()
				{
					while ( _begin )
					{
						SocketForserver* tmp = _begin;
						delete _begin;
						_begin = tmp;

					}
				}
				size_t getServerSockCount() { return _size; }
			};
			SocketList socketList;

			// event emitters
			EventEmitterSupportingListeners<event::Close, ServerListener, &ServerListener::onClose> eClose;
			EventEmitterSupportingListeners<event::Connection, ServerListener, &ServerListener::onConnection> eConnection;
			EventEmitterSupportingListeners<event::Listening, ServerListener, &ServerListener::onListening> eListening;
			EventEmitterSupportingListeners<event::Error, ServerListener, &ServerListener::onError> eError;

			std::vector<std::unique_ptr<ServerListener>> ownedListeners;

		public:
			Server();
			~Server() {socketList.clear();}

			void emitClose(bool hadError) {
				state = CLOSED;
				//id = 0;
				dataForCommandProcessing.index = 0;
				eClose.emit(hadError);
			}

			void emitConnection(Socket* socket) {
				eConnection.emit(socket);
			}

			void emitListening(size_t id, Address addr) {
				//this->id = id;
				this->dataForCommandProcessing.index = id;
				localAddress = std::move(addr);
				state = LISTENING;
				eListening.emit(id, addr);
			}

			void emitError(Error& err) {
				eError.emit(err);
			}


			SocketTBase* makeSocket(OpaqueSocketData& sdata) {
				SocketForserver* sock = socketList.getNewSocket(sdata);
				return sock;
			}
			void removeSocket( net::Socket* sock ) {
				socketList.removeAndDelete( static_cast<SocketForserver*>(sock) );
			}
			size_t getSockCount() {return socketList.getServerSockCount();}

			void close(std::function<void(bool)> cb) {
				once(event::close, std::move(cb));
				ServerBase::close();
			}

			void listen(uint16_t port, const char* ip, int backlog, std::function<void(size_t, net::Address)> cb) {
				once(event::listening, std::move(cb));
				ServerTBase::listen(port, ip, backlog);
			}

			void on( ServerListener* l) {
				eClose.on(l);
				eConnection.on(l);
				eListening.on(l);
				eError.on(l);
			}

			void once( ServerListener* l) {
				eClose.once(l);
				eConnection.once(l);
				eListening.once(l);
				eError.once(l);
			}

			void on( std::unique_ptr<ServerListener>& l) {
				ownedListeners.emplace_back( std::move( l ) );
				on( l.get() );
			}

			void once( std::unique_ptr<ServerListener>& l) {
				ownedListeners.emplace_back( std::move( l ) );
				once( l.get() );
			}

			void on(std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				assert(name == event::Close::name);
				eClose.on(std::move(cb));
			}

			void on(std::string name, event::Connection::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				assert(name == event::Connection::name);
				eConnection.on(std::move(cb));
			}

			void on(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				assert(name == event::Error::name);
				eError.on(std::move(cb));
			}

			void on(std::string name, event::Listening::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				assert(name == event::Listening::name);
				eListening.on(std::move(cb));
			}

			void once(std::string name, event::Close::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Close::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Close::callback, event::Error::callback >::value);
				assert(name == event::Close::name);
				eClose.once(std::move(cb));
			}

			void once(std::string name, event::Connection::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Connection::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Connection::callback, event::Error::callback >::value);
				assert(name == event::Connection::name);
				eConnection.once(std::move(cb));
			}

			void once(std::string name, event::Error::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Error::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Listening::callback >::value);
				static_assert(!std::is_same< event::Error::callback, event::Connection::callback >::value);
				assert(name == event::Error::name);
				eError.once(std::move(cb));
			}

			void once(std::string name, event::Listening::callback cb [[nodecpp::may_extend_to_this]]) {
				static_assert(!std::is_same< event::Listening::callback, event::Close::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Connection::callback >::value);
				static_assert(!std::is_same< event::Listening::callback, event::Error::callback >::value);
				assert(name == event::Listening::name);
				eListening.once(std::move(cb));
			}


			template<class EV>
			void on(EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.on(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.on(std::move(cb)); }
				else assert(false);
			}

			template<class EV>
			void once(EV, typename EV::callback cb [[nodecpp::may_extend_to_this]]) {
				if constexpr (std::is_same< EV, event::Close >::value) { eClose.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Connection >::value) { eConnection.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Listening >::value) { eListening.once(std::move(cb)); }
				else if constexpr (std::is_same< EV, event::Error >::value) { eError.once(std::move(cb)); }
				else assert(false);
			}
		};

		//TODO don't use naked pointers, think
		//template<class T>
		//T* createServer() {
		//	return new T();
		//}
		//template<class T>
		//T* createServer(std::function<void()> cb) {
		//	auto svr = new T();
		//	svr->on<event::Connect>(cb);
		//	return svr;
		//}

		//template<class T>
		//T* createConnection(uint16_t port, const char* host) {
		//	auto cli = new T();
		//	cli->appConnect(port, host);
		//	return cli;
		//}

		//template<class T>
		//T* createConnection(uint16_t port, const char* host, std::function<void()> cb) {
		//	auto cli = new T();
		//	cli->appConnect(port, host, std::move(cb));
		//	return cli;
		//}

	} //namespace net
} //namespace nodecpp

#endif //SERVER_L_H
