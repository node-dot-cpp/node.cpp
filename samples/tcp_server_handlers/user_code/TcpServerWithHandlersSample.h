// TcpServerWithHandlersSample.h : sample of user-defined code

#ifndef TCP_SERVER_WITH_HANDLERS_SAMPLE_H
#define TCP_SERVER_WITH_HANDLERS_SAMPLE_H

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

	class MyWorkingSocket : public nodecpp::net::SocketBase {};
	class MyStatSocket : public nodecpp::net::SocketBase {};
	class MyWorkingServer : public nodecpp::net::ServerBase {};
	class MyStatServer : public nodecpp::net::ServerBase {};

public:
	MySampleTNode() {}

	virtual nodecpp::handler_ret_type main()
	{
		// registering handlers
		nodecpp::net::ServerBase::addHandler<MyWorkingServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Close, &MySampleTNode::onCloseWorkingServer>(this);
		nodecpp::net::ServerBase::addHandler<MyStatServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Close, &MySampleTNode::onCloseStatServer>(this);

		nodecpp::net::SocketBase::addHandler<MyWorkingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onWorkingData>(this);
		nodecpp::net::SocketBase::addHandler<MyStatSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onStatData>(this);
		nodecpp::net::SocketBase::addHandler<MyWorkingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::End, &MySampleTNode::onWorkingEnd>(this);
		nodecpp::net::SocketBase::addHandler<MyStatSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::End, &MySampleTNode::onStatEnd>(this);

		srv = nodecpp::net::createServer<MyWorkingServer, MyWorkingSocket>();
		srvCtrl = nodecpp::net::createServer<MyStatServer, MyStatSocket>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

		CO_RETURN;
	}

public:
	// server
	nodecpp::handler_ret_type onCloseWorkingServer(nodecpp::safememory::soft_ptr<MyWorkingServer> server, bool haserror) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "working server: closed!");
		CO_RETURN;
	}

	nodecpp::handler_ret_type onWorkingData(nodecpp::safememory::soft_ptr<MyWorkingSocket> socket, Buffer& buffer)
	{
		if ( buffer.size() < 2 )
		{
			//log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = %d", *extra );
			socket->end();
			CO_RETURN;
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
			CO_RETURN;
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

		CO_RETURN;
	}

	nodecpp::handler_ret_type onWorkingEnd(nodecpp::safememory::soft_ptr<MyWorkingSocket> socket)
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "working server socket: onEnd!");
		Buffer b;
		b.appendString( nodecpp::string_literal( "goodbye!" ) );
		socket->write( b );
		socket->end();
		CO_RETURN;
	}

	// ctrl server
	nodecpp::handler_ret_type onCloseStatServer(nodecpp::safememory::soft_ptr<MyStatServer> server, bool hasError) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "stat server: closed!");
		CO_RETURN;
	}

	nodecpp::handler_ret_type onStatData(nodecpp::safememory::soft_ptr<MyStatSocket> socket, Buffer& buffer)
	{
		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			stats.connCnt = srv->getSockCount();
			Buffer reply(sizeof(stats));
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz);
			co_await socket->a_write(reply);
		}
	}

	nodecpp::handler_ret_type onStatEnd(nodecpp::safememory::soft_ptr<MyStatSocket> socket)
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "stat server socket: onEnd!");
		Buffer b;
		b.appendString( nodecpp::string_literal( "goodbye!" ) );
		socket->write( b );
		socket->end();
		CO_RETURN;
	}

	nodecpp::safememory::owning_ptr<MyWorkingServer> srv;
	nodecpp::safememory::owning_ptr<MyStatServer> srvCtrl;
};

#endif // TCP_SERVER_WITH_HANDLERS_SAMPLE_H
