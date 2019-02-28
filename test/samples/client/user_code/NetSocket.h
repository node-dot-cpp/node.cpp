// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t.h"


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
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual void main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		*( clientSock.getExtra() ) = 17;
		clientSock.connect(2000, "127.0.0.1");
	}
	
	void onWhateverConnect(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) 
	{
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		clientSock.write(buf);
	}
	void onWhateverData(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		++recvReplies;
		if ( ( recvReplies & 0xFFFF ) == 0 )
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), extra = {}, size = {}", recvReplies, *(socket->getExtra()), buffer.size() );
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
