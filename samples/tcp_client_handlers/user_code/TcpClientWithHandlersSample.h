// TcpClientWithHandlersSample.h : sample of user-defined code

#ifndef TCP_CLIENT_WITH_HANDLERS_SAMPLE_H
#define TCP_CLIENT_WITH_HANDLERS_SAMPLE_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;
	uint64_t maxRequests = UINT64_MAX;
	static constexpr uint64_t autotestRespCnt = 30;

	using ClientSockType = net::SocketBase;

public:
	MySampleTNode() {}

	virtual handler_ret_type main()
	{
		net::SocketBase::addHandler<ClientSockType, net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onConnect>(this);
		net::SocketBase::addHandler<ClientSockType, net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onData>(this);

		clientSock = net::createSocket<ClientSockType>();
		clientSock->connect(2000, "127.0.0.1");

		for ( size_t i=1; i<argv.size(); ++i )
			if ( argv[i].size() > 12 && argv[i].substr(0,12) == "-autotest" )
			{
				maxRequests = autotestRespCnt;
				break;
			}

		CO_RETURN;
	}

	handler_ret_type onConnect(safememory::soft_ptr<net::SocketBase> socket) 
	{
			Buffer buf(2);
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			co_await socket->a_write(buf);
			CO_RETURN;
	}

	awaitable<void> onData(safememory::soft_ptr<net::SocketBase> socket, Buffer& r_buff )
	{
		++recvReplies;
		recvSize += r_buff.size();
		if ( ( recvReplies & 0xFFF ) == 0 )
			log::default_log::info( log::ModuleID(nodecpp_module_id), "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, r_buff.size(), recvSize );

		if ( recvReplies < maxRequests )
		{
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			co_await socket->a_write(buf);
		}
		else if ( recvReplies == maxRequests )
		{
			buf.writeInt8( 4, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			buf.writeInt8( 0xfe, 2 );
			buf.writeInt8( 0xfe, 3 );
			co_await socket->a_write(buf);
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Sending the last request in automated testing" );
		}
		else
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
			socket->end();
			socket->unref();
		}

		CO_RETURN;
	}

	safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // TCP_CLIENT_WITH_HANDLERS_SAMPLE_H
