// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include <fmt/format.h>
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	class MyListener : public SocketListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onConnect() override { myNode->onConnect(&id); }
		void onData(nodecpp::Buffer& buffer) override { myNode->onData(&id, buffer); }
	};

	nodecpp::safememory::owning_ptr<net::Socket> clientSock;

public:
	MySampleTNode()/* : myListener(this, 0)*/
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()\n" );

		clientSock = nodecpp::safememory::make_owning<net::Socket>();
		nodecpp::safememory::owning_ptr<SocketListener> myListener = nodecpp::safememory::make_owning<MyListener>(this, 0);
		clientSock->on( std::move(myListener) );
		clientSock->connect(2000, "127.0.0.1");
	}
	
	void onConnect(const int* extra) 
	{
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		clientSock->write(buf);
	}

	void onData(const int* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, extra != nullptr );
		++recvReplies;
		if ( ( recvReplies & 0xFFFF ) == 0 )
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), extra = {}, size = {}\n", recvReplies, *extra, buffer.size() );
		recvSize += buffer.size();
		buf.writeInt8( 2, 0 );
		buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
		clientSock->write(buf);
	}

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
};

#endif // NET_SOCKET_H
