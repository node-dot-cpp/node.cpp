// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/socket_t_base.h>


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using SocketIdType = int;

public:
	MySampleTNode() : clientSock( this )
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		*( clientSock.getExtra() ) = 17;
		clientSock.connect(2000, "127.0.0.1");
		
		co_return;
	}
	
	void onWhateverConnect(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) 
	{
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		socket->write(buf);
	}

	void onWhateverData(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		++recvReplies;
		if ( ( recvReplies & 0xFFFF ) == 0 )
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), extra = {}, size = {}", recvReplies, *(socket->getExtra()), buffer.size() );
		recvSize += buffer.size();
		buf.writeInt8( 2, 0 );
		buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
		socket->write(buf);
	}

	using ClientSockType = nodecpp::net::SocketN<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnData<&MySampleTNode::onWhateverData>
	>;
	ClientSockType clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
};

#endif // NET_SOCKET_H
