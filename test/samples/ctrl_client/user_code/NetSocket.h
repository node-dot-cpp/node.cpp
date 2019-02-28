// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t.h"
/*#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"

#include <functional>*/


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
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}, {}, {}, {}, {}", infraGetCurrentTime(), stats.connCnt, stats.recvSize, stats.sentSize, stats.rqCnt );
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
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual void main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		*( clientSock.getExtra() ) = 17;
		clientSock.connect(2001, "127.0.0.1");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));
	}
	
	void onWhateverConnect(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) 
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverConnect(), extra = {}", *(clientSock.getExtra()) );

		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = 1;
		clientSock.write(buff, 2);
	}
	void onWhateverClose(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, bool)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverClose(), extra = {}", *(clientSock.getExtra()) );
	}
	void onWhateverData(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Buffer& buffer)
	{
		if ( buffer.size() < sizeof( Stats ) )
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "{}, Failure (expected {} bytes, received {} bytes", infraGetCurrentTime(), sizeof( Stats ), buffer.size() );
		else
			printStats( *reinterpret_cast<Stats*>( buffer.begin() ) );
		
		getchar();

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		++recvReplies;
		recvSize += buffer.size();
		uint8_t* buff = ptr.get();
		buff[0] = 2;
		buff[1] = (uint8_t)recvReplies | 1;
		clientSock.write(buff, 2);
	}
	void onWhateverDrain(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		if ( letOnDrain )
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverDrain(), extra = {}", *(clientSock.getExtra()) );
	}
	void onWhateverError(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Error&)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverError(), extra = {}", *(clientSock.getExtra()) );
	}
	void onWhateverEnd(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket)
	{
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::onWhateverEnd(), extra = {}", *(clientSock.getExtra()) );
	}

	using ClientSockType = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnCloseT<&MySampleTNode::onWhateverClose>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData>,
		nodecpp::net::OnDrainT<&MySampleTNode::onWhateverDrain>,
		nodecpp::net::OnErrorT<&MySampleTNode::onWhateverError>,
		nodecpp::net::OnEndT<&MySampleTNode::onWhateverEnd>
	>;
	//nodecpp::safememory::owning_ptr<ClientSockType> clientSock;
	ClientSockType clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, ClientSockType>;
};

#endif // NET_SOCKET_H
