// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>

using namespace nodecpp;
using namespace fmt;

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

class MySampleTNode : public NodeBase
{
public:
	MySampleTNode() {}

	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using ClientSockType = net::SocketBase;

	virtual awaitable<void> main()
	{
		clientSock = net::createSocket();

		clientSock->on(event::connect, [this]() { 
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			clientSock->write(buf);
		});

		clientSock->on(event::data, [this](const Buffer& buffer) { 
			++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
				clientSock->end();
				clientSock->unref();
			}
#endif
			if ( ( recvReplies & 0xFFF ) == 0 )
				log::default_log::info( log::ModuleID(nodecpp_module_id), "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );
			recvSize += buffer.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			clientSock->write(buf);
		});

		clientSock->connect(2000, "127.0.0.1");
		
		CO_RETURN;
	}

	safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // NET_SOCKET_H
