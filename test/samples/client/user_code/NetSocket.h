// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"

#include <functional>


using namespace std;
using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using SocketIdType = int;

public:
	MySampleTNode() : clientSock(this)
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

		*( clientSock.getExtra() ) = 17;
		clientSock.connect(2000, "127.0.0.1");
	}
	
	void onWhateverConnect(const SocketIdType* extra) 
	{
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		clientSock.write(buf);
	}
	void onWhateverData(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( extra != nullptr );
		++recvReplies;
		if ( ( recvReplies & 0xFFFF ) == 0 )
//			printf( "[%zd] MySampleTNode::onData(), size = %zd\n", recvReplies, buffer.size() );
			printf( "[%zd] MySampleTNode::onWhateverData(), extra = %d, size = %zd\n", recvReplies, *extra, buffer.size() );
		recvSize += buffer.size();
		buf.writeInt8( 2, 0 );
		buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
		clientSock.write(buf);
	}

	using ClientSockType = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData>
	>;
	ClientSockType clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, ClientSockType>;
};

#endif // NET_SOCKET_H
