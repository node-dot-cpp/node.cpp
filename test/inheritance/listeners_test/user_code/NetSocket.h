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
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64;// * 1024;

	using SocketIdType = int;

	class MyListener : public SocketListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onConnect() override { myNode->onWhateverConnect(&id); }
		void onData(nodecpp::Buffer& buffer) override { myNode->onWhateverData(&id, buffer); }
	};
	MyListener myListener;

	net::Socket lsock;

public:
	MySampleTNode() : myListener(this, 0)
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

		lsock.addListener( &myListener );
		lsock.connect(2000, "127.0.0.1");
	}
	
	void onWhateverConnect(const SocketIdType* extra) 
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverConnect(), extra = %d\n", *extra );

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = 1;
		lsock.write(buff, 2);
	}

	void onWhateverData(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( extra != nullptr );
		++recvReplies;
		if ( ( recvReplies & 0xFFFF ) == 0 )
			printf( "[%zd] MySampleTNode::onWhateverData(), extra = %d, size = %zd\n", recvReplies, *extra, buffer.size() );
		recvSize += buffer.size();
		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = (uint8_t)recvReplies | 1;
		lsock.write(buff, 2);
	}

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
