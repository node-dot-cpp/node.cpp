// TcpServerSample.h : sample of user-defined code

#ifndef TCP_SERVER_SAMPLE_H
#define TCP_SERVER_SAMPLE_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

using namespace nodecpp;

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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: closed!");
		});

		srv->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {

			socket->on( event::data, [this, socket](const Buffer& buffer) {
				if ( buffer.size() < 2 )
				{
					//log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = %d", *extra );
					socket->end();
					return;
				}
	
				if ( buffer.size() == 2 + 2 && buffer.readUInt8(2) == 0xfe && buffer.readUInt8(2) == 0xfe )
				{
					log::default_log::info( log::ModuleID(nodecpp_module_id), "Request to close server received" );
					srv->close();
					srvCtrl->close();
				}

				size_t receivedSz = buffer.readUInt8(0);
				if ( receivedSz != buffer.size() )
				{
					log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = [??]: received {}, expected: {} bytes", receivedSz, buffer.size() );
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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "ctrl server: closed!");
		});
		srvCtrl->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {
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

		CO_RETURN;
	}

	nodecpp::safememory::owning_ptr<nodecpp::net::ServerBase> srv;
	nodecpp::safememory::owning_ptr<nodecpp::net::ServerBase> srvCtrl;
};

#endif // TCP_SERVER_SAMPLE_H
