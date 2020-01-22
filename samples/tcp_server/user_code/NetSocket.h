// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

using namespace std;
using namespace nodecpp;
using namespace fmt;

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

class MySampleTNode : public NodeBase
{
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	Buffer replyBuff;

public:
	virtual nodecpp::handler_ret_type main()
	{
		srv = nodecpp::net::createServer<net::ServerBase>();
		srvCtrl = nodecpp::net::createServer<net::ServerBase>();

		srv->on( event::close, [this](bool hadError) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onCloseServer()!");
		});
		srv->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {
#ifdef AUTOMATED_TESTING_ONLY
			nodecpp::setTimeout(  [this, socket]() { 
				socket->end();
				socket->unref();
				}, 3000 );
#endif
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
			//srv->unref();
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
			socket->on( event::close, [this, socket](bool hadError) {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
				socket->unref();
			});

			socket->on( event::data, [this, socket](const Buffer& buffer) {
				if ( buffer.size() < 2 )
				{
					//log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = %d", *extra );
					socket->end();
					return;
				}
	
				size_t receivedSz = buffer.readUInt8(0);
				if ( receivedSz != buffer.size() )
				{
//					log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes", *extra, receivedSz, buffer.size() );
					log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = [??]: received %zd, expected: %zd bytes", receivedSz, buffer.size() );
					socket->unref();
					return;
				}
	
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(requestedSz);
					for ( size_t i=0; i<(uint8_t)requestedSz; ++i )
						reply.appendUint8( 0 );
					socket->write(reply);
				}
	
				stats.recvSize += receivedSz;
				stats.sentSize += requestedSz;
				++(stats.rqCnt);
			});
			socket->on( event::end, [this, socket]() {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
				Buffer b;
				b.appendString( nodecpp::string_literal( "goodbye!" ) );
				socket->write( b );
				socket->end();
			});

		});

		srvCtrl->on( event::close, [this](bool hadError) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onCloseServerCtrl()!");
		});
		srvCtrl->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
#ifdef AUTOMATED_TESTING_ONLY
			nodecpp::setTimeout(  [this, socket]() { 
				socket->end();
				socket->unref();
				}, 3000 );
#endif
			socket->on( event::close, [this, socket](bool hadError) {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
			});
			socket->on( event::data, [this, socket](const Buffer& buffer) {
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(sizeof(stats));
					stats.connCnt = srv->getSockCount();
					size_t replySz = sizeof(Stats);
					reply.append( &stats, replySz );
					socket->write(reply);
				}
			});
			socket->on( event::end, [this, socket]() {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
				Buffer b;
				b.appendString( nodecpp::string_literal( "goodbye!" ) );
				socket->write( b );
			});
		});

		srv->listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl->listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});

#ifdef AUTOMATED_TESTING_ONLY
		nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
		}, 3000 );
#endif

		CO_RETURN;
	}

	nodecpp::safememory::owning_ptr<nodecpp::net::ServerBase> srv;
	nodecpp::safememory::owning_ptr<nodecpp::net::ServerBase> srvCtrl;

	nodecpp::handler_ret_type onDataServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
//			log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data: received {} bytes", buffer.size() );
			socket->unref();
			CO_RETURN;
		}
//		log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
//			log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			CO_RETURN;
		}

		uint32_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			reply.set_size(requestedSz);
			socket->write(reply);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
		/*if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by count)" );
			socket->end();
			socket->unref();
		}*/
#endif
		CO_RETURN;
	}
};

#endif // NET_SOCKET_H
