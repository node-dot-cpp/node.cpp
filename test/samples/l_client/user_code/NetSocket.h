// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"


using namespace std;
using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64;// * 1024;

	net::Socket clientSock;

public:
	MySampleTNode()
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

		clientSock.once(event::connect, [this]() { 
			ptr.reset(static_cast<uint8_t*>(malloc(size)));

			uint8_t* buff = ptr.get();
			buff[0] = 2;
			buff[1] = 1;
			clientSock.write(buff, 2);
		});

		clientSock.on(event::data, [this](Buffer& buffer) { 
			++recvReplies;
			if ( ( recvReplies & 0xFFFF ) == 0 )
				printf( "[%zd] MySampleTNode::onData(), size = %zd\n", recvReplies, buffer.size() );
			recvSize += buffer.size();
			uint8_t* buff = ptr.get();
			buff[0] = 2;
			buff[1] = (uint8_t)recvReplies | 1;
			clientSock.write(buff, 2);
		});
		clientSock.connect(2000, "127.0.0.1");
	}

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
