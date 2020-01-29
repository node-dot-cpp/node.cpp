// TcpClientSample.h : sample of user-defined code

#ifndef TCP_CLIENT_SAMPLE_H
#define TCP_CLIENT_SAMPLE_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
public:
	MySampleTNode() {}

	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;
	uint64_t maxRequests = UINT64_MAX;
	static constexpr uint64_t autotestRespCnt = 30;

	using ClientSockType = net::SocketBase;

	virtual awaitable<void> main()
	{
		auto argv = getArgv();
		for ( size_t i=1; i<argv.size(); ++i )
			if ( argv[i].size() >= 9 && argv[i].substr(0,9) == "-autotest" )
			{
				maxRequests = autotestRespCnt;
				break;
			}

		clientSock = net::createSocket();

		clientSock->on(event::connect, [this]() { 
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			clientSock->write(buf);
		});

		clientSock->on(event::data, [this](const Buffer& buffer) { 
			++recvReplies;
			recvSize += buffer.size();

			if ( ( recvReplies & 0xFFF ) == 0 )
				log::default_log::info( log::ModuleID(nodecpp_module_id), "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );

			if ( recvReplies < maxRequests )
			{
				buf.writeInt8( 2, 0 );
				buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
				clientSock->write(buf);
			}
			else if ( recvReplies == maxRequests )
			{
				buf.writeInt8( 4, 0 );
				buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
				buf.writeInt8( (uint8_t)0xfe, 2 );
				buf.writeInt8( (uint8_t)0xfe, 3 );
				clientSock->write(buf);
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Sending the last request in automated testing" );
			}
			else
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
				clientSock->end();
				clientSock->unref();
			}

		});

		clientSock->connect(2000, "127.0.0.1");
		
		CO_RETURN;
	}

	safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // TCP_CLIENT_SAMPLE_H
