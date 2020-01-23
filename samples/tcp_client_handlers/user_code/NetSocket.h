// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>

using namespace nodecpp;

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using ClientSockType = net::SocketBase;

public:
	MySampleTNode() {}

	virtual handler_ret_type main()
	{
		net::SocketBase::addHandler<ClientSockType, net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onConnect>(this);
		net::SocketBase::addHandler<ClientSockType, net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onData>(this);

		clientSock = net::createSocket<ClientSockType>();
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}

	handler_ret_type onConnect(safememory::soft_ptr<net::SocketBase> socket) 
	{
			Buffer buf(2);
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			try
			{
				co_await socket->a_write(buf);
			}
			catch (...)
			{
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed). Exiting..." );
				socket->end();
				socket->unref();
			}
			CO_RETURN;
	}

	awaitable<void> onData(safememory::soft_ptr<net::SocketBase> socket, Buffer& r_buff )
	{
//		co_await socket->a_read(r_buff, (uint8_t)recvReplies | 1);
		++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
		if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
			socket->end();
			socket->unref();
			CO_RETURN;
		}
#endif
		if ( ( recvReplies & 0xFFF ) == 0 )
			log::default_log::info( log::ModuleID(nodecpp_module_id), "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, r_buff.size(), recvSize );
		recvSize += r_buff.size();
		buf.writeInt8( 2, 0 );
		buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
		try
		{
			co_await socket->a_write(buf);
		}
		catch (...)
		{
			log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed. Exiting..." );
			socket->end();
			socket->unref();
		}

		CO_RETURN;
	}

	safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // NET_SOCKET_H
