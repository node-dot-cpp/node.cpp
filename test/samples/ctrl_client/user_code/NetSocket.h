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
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	void printStats(const Stats& stats)
	{
		printf( "%lld, %lld, %lld, %lld, %lld\n", infraGetCurrentTime(), stats.connCnt, stats.recvSize, stats.sentSize, stats.rqCnt );
	}

	size_t recvSize = 0;
	size_t recvReplies = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64;// * 1024;
	bool letOnDrain = false;

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
		clientSock.connect(2001, "127.0.0.1");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));
	}
	
	void onWhateverConnect(const SocketIdType* extra) 
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverConnect(), extra = %d\n", *extra );

		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = 1;
		clientSock.write(buff, 2);
	}
	void onWhateverClose(const SocketIdType* extra, bool)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverClose(), extra = %d\n", *extra );
	}
	void onWhateverData(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		if ( buffer.size() < sizeof( Stats ) )
			printf( "%lld, Failure (expected %zd bytes, received %zd bytes\n", infraGetCurrentTime(), buffer.size(), sizeof( Stats ) );
		else
			printStats( *reinterpret_cast<Stats*>( buffer.begin() ) );
		
		getchar();

		NODECPP_ASSERT( extra != nullptr );
		++recvReplies;
		recvSize += buffer.size();
		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = (uint8_t)recvReplies | 1;
		clientSock.write(buff, 2);
	}
	void onWhateverDrain(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		if ( letOnDrain )
			printf( "MySampleTNode::onWhateverDrain(), extra = %d\n", *extra );
	}
	void onWhateverError(const SocketIdType* extra, nodecpp::Error&)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverError(), extra = %d\n", *extra );
	}
	void onWhateverEnd(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverEnd(), extra = %d\n", *extra );
	}

	using ClientSockType = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnCloseT<&MySampleTNode::onWhateverClose>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData>,
		nodecpp::net::OnDrainT<&MySampleTNode::onWhateverDrain>,
		nodecpp::net::OnErrorT<&MySampleTNode::onWhateverError>,
		nodecpp::net::OnEndT<&MySampleTNode::onWhateverEnd>
	>;
	ClientSockType clientSock;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, ClientSockType>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
