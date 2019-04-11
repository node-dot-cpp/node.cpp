// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include <common.h>


#include <fmt/format.h>
#include <socket_type_list.h>
#include <socket_t_base.h>


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using SocketIdType = int;
	awaitable<void> dataProcessorcallRet;

public:
	MySampleTNode() : clientSock( this )
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		*( clientSock.getExtra() ) = 17;
		//clientSock.connect(2000, "127.0.0.1");
		co_await clientSock.a_connect(2000, "127.0.0.1");
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		bool write_ok = co_await clientSock.a_write(buf);
		// TODO: address failure
		co_await doWhateverWithIncomingData();
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

	awaitable<void> incomingDataLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket)
	{
		for (;;)
		{
			nodecpp::Buffer r_buff;
			try
			{
				r_buff = std::move( co_await socket->a_read() );
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				break;
			}
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket );
			++recvReplies;
			if ( ( recvReplies & 0xFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, r_buff.size(), recvSize );
			recvSize += r_buff.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			socket->write(buf);
		}
		co_return;
	}

	awaitable<void> doWhateverWithIncomingData()
	{
		for (;;)
		{
			nodecpp::Buffer r_buff;
			try
			{
				r_buff = std::move( co_await clientSock.a_read() );
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(clientSock.getExtra()));
				break;
			}
//			size_t read_ret = co_await clientSock.read();
			++recvReplies;
			if ( ( recvReplies & 0xFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, clientSock.dataForCommandProcessing.recvBuffer.size(), recvSize );
			recvSize += r_buff.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			bool write_ok = co_await clientSock.a_write(buf);
			// TODO: address failure
		}
		co_return;
	}

	using ClientSockType = nodecpp::net::SocketN<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleTNode::onWhateverConnect>/*,
		nodecpp::net::OnData<&MySampleTNode::onWhateverData>,
		nodecpp::net::OnDataAwaitable<&MySampleTNode::incomingDataLoop>*/
	>;
	ClientSockType clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
};

#endif // NET_SOCKET_H
