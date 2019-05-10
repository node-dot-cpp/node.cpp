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
		co_await clientSock.a_connect(2000, "127.0.0.1");
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		co_await clientSock.a_write(buf);
		// TODO: address failure
		co_await doWhateverWithIncomingData();
		co_return;
	}

	awaitable<void> doWhateverWithIncomingData()
	{
		for (;;)
		{
			nodecpp::Buffer r_buff(0x200);
			try
			{
				co_await clientSock.a_read(r_buff);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(clientSock.getExtra()));
				break;
			}
			++recvReplies;
			if ( ( recvReplies & 0xFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, r_buff.size(), recvSize );
			recvSize += r_buff.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			try
			{
				co_await clientSock.a_write(buf);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(clientSock.getExtra()));
				break;
			}
			// TODO: address failure
		}
		co_return;
	}

	using ClientSockType = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;
	ClientSockType clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
};

#endif // NET_SOCKET_H
