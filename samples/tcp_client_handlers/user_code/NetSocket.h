// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>

using namespace nodecpp;
using namespace fmt;

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

class MySampleTNode : public NodeBase
{
public:
	MySampleTNode() {}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onConnect>(this);
		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &ClientSockType::onConnect>();

		clientSock = nodecpp::net::createSocket<ClientSockType>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}

	class MySocketOne; // just forward declaration
	nodecpp::handler_ret_type onConnect(nodecpp::safememory::soft_ptr<MySocketOne> socket) 
	{
		printf( "MySampleTNode::onConnect() with extra = %d\n", *(socket->getExtra()) );
		CO_RETURN;
	}

	using ClientSockBaseType = nodecpp::net::SocketBase;

	class MySocketOne : public ClientSockBaseType
	{
		size_t recvSize = 0;
		size_t recvReplies = 0;
		Buffer buf;
		int extraData;

	public:
		MySocketOne() {}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		nodecpp::handler_ret_type onConnect() 
		{
			printf( "onConnect()\n" );
			Buffer buf(2);
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			try
			{
				co_await a_write(buf);
				co_await processIncomingData();
			}
			catch (...)
			{
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(getExtra()));
				// TODO: address failure
			}
			CO_RETURN;
		}

		awaitable<void> processIncomingData()
		{
			for (;;)
			{
				nodecpp::Buffer r_buff(0x200);
				try
				{
					co_await a_read(r_buff, (uint8_t)recvReplies | 1);
				}
				catch (...)
				{
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed (extra = {}). Exiting...", *(getExtra()));
					break;
				}
				++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
				if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
				{
					log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
					end();
					unref();
					break;
				}
#endif
				if ( ( recvReplies & 0xFFF ) == 0 )
					log::default_log::info( log::ModuleID(nodecpp_module_id), "[{}] MySampleTNode::onWhateverData(), size = {}, total received size = {}", recvReplies, r_buff.size(), recvSize );
				recvSize += r_buff.size();
				buf.writeInt8( 2, 0 );
				buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
				try
				{
					co_await a_write(buf);
				}
				catch (...)
				{
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(getExtra()));
					break;
				}
				// TODO: address failure
			}
			CO_RETURN;
		}
	};

	using ClientSockType = MySocketOne;

	nodecpp::safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // NET_SOCKET_H
