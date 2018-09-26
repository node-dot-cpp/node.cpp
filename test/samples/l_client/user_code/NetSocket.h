// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	net::Socket clientSock;

public:
	MySampleTNode()
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

		clientSock.on(event::connect, [this]() { 
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			clientSock.write(buf);
		});

		clientSock.on(event::data, [this](Buffer& buffer) { 
			++recvReplies;
			if ( ( recvReplies & 0xFFFF ) == 0 )
				printf( "[%zd] MySampleTNode::onData(), size = %zd\n", recvReplies, buffer.size() );
			recvSize += buffer.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			clientSock.write(buf);
		});

		clientSock.connect(2000, "127.0.0.1");
	}

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
