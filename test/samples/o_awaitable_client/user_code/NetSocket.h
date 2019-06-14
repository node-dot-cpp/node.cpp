// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/socket_t_base.h>


using namespace nodecpp;
using namespace fmt;

//#define IMPL_VERSION 2 // main() is a single coro
//#define IMPL_VERSION 3 // onConnect is a coro
#define IMPL_VERSION 4 // registering handlers (per class)

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	using SocketIdType = int;

public:
	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

#if IMPL_VERSION == 2

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		clientSock = nodecpp::safememory::make_owning<ClientSockType>(this);
		*( clientSock->getExtra() ) = 17;

		try
		{
			co_await clientSock->a_connect(2000, "127.0.0.1");
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			co_await clientSock->a_write(buf);
			// TODO: address failure
			co_await doWhateverWithIncomingData(clientSock);
		}
		catch (...)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("processing on socket with extra = {} failed. Exiting...", *(clientSock->getExtra()));
		}
		co_return;
	}

	using ClientSockType = nodecpp::net::SocketN<MySampleTNode,SocketIdType
	>;

	awaitable<void> doWhateverWithIncomingData(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		for (;;)
		{
			nodecpp::Buffer r_buff(0x200);
			try
			{
				co_await socket->a_read(r_buff);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
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
				co_await socket->a_write(buf);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
				break;
			}
			// TODO: address failure
		}
		co_return;
	}

#elif IMPL_VERSION == 3
	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		clientSock = nodecpp::safememory::make_owning<ClientSockType>(this);
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		co_return;
	}
	
	nodecpp::awaitable<void> onWhateverConnect(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) 
	{
		printf( "onWhateverConnect()\n" );
		Buffer buf(2);
		buf.writeInt8( 2, 0 );
		buf.writeInt8( 1, 1 );
		socket->a_write(buf);
		try
		{
			co_await socket->a_write(buf);
			co_await doWhateverWithIncomingData(socket);
		}
		catch (...)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
			// TODO: address failure
		}
		co_return;
	}

	using ClientSockType = nodecpp::net::SocketN<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleTNode::onWhateverConnect>
	>;

	awaitable<void> doWhateverWithIncomingData(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		for (;;)
		{
			nodecpp::Buffer r_buff(0x200);
			try
			{
				co_await socket->a_read(r_buff);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
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
				co_await socket->a_write(buf);
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
				break;
			}
			// TODO: address failure
		}
		co_return;
	}

#elif IMPL_VERSION == 4
	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onWhateverConnect>(this);
		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &ClientSockType::onWhateverConnect>();

		clientSock = nodecpp::safememory::make_owning<ClientSockType>(this);
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		co_return;
	}

	nodecpp::awaitable<void> onWhateverConnect() 
	{
		printf( "MySampleTNode::onWhateverConnect()\n" );
		co_return;
	}

//	using ClientSockBaseType = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;
	using ClientSockBaseType = nodecpp::net::SocketBase;

	class MySocketOne : public ClientSockBaseType
	{
		size_t recvSize = 0;
		size_t recvReplies = 0;
		Buffer buf;
		int extraData;

	public:
		MySocketOne(MySampleTNode* node) : ClientSockBaseType(node) {}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		nodecpp::awaitable<void> onWhateverConnect() 
		{
			printf( "onWhateverConnect()\n" );
			Buffer buf(2);
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			a_write(buf);
			try
			{
				co_await a_write(buf);
				co_await processIncomingData();
			}
			catch (...)
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(getExtra()));
				// TODO: address failure
			}
			co_return;
		}

		awaitable<void> processIncomingData()
		{
			for (;;)
			{
				nodecpp::Buffer r_buff(0x200);
				try
				{
					co_await a_read(r_buff);
				}
				catch (...)
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(getExtra()));
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
					co_await a_write(buf);
				}
				catch (...)
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(getExtra()));
					break;
				}
				// TODO: address failure
			}
			co_return;
		}
	};

	using ClientSockType = MySocketOne;


#else
#error
#endif

	nodecpp::safememory::owning_ptr<ClientSockType> clientSock;

	using EmitterType = nodecpp::net::SocketTEmitter</*net::SocketO,*/ net::Socket>;
};

#endif // NET_SOCKET_H
