// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>


using namespace nodecpp;
using namespace fmt;

#ifndef NODECPP_NO_COROUTINES
#define IMPL_VERSION 2 // main() is a single coro
//#define IMPL_VERSION 21 // main() is a single coro with non-default socket class
//#define IMPL_VERSION 3 // onConnect is a coro (onConnect is added via addHandler<...>(...))
//#define IMPL_VERSION 4 // registering handlers (per class)
//#define IMPL_VERSION 5 // registering handlers (per class, template-based)
//#define IMPL_VERSION 6 // registering handlers (per class, template-based) with no explicit awaitable staff
//#define IMPL_VERSION 7 // lambda-based
#else
#define IMPL_VERSION 6 // registering handlers (per class, template-based) with no explicit awaitable staff
#endif // NODECPP_NO_COROUTINES

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

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

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		clientSock = nodecpp::net::createSocket();

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
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("processing on socket failed. Exiting...");
		}
		CO_RETURN;
	}

	using ClientSockType = nodecpp::net::SocketBase;

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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed). Exiting...");
				break;
			}
			++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				socket->end();
				socket->unref();
				break;
			}
#endif
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
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed). Exiting...");
				break;
			}
			// TODO: address failure
		}
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;

#elif IMPL_VERSION == 21

	class MySocketOne : public nodecpp::net::SocketBase
	{
		int extraData;

	public:
		MySocketOne() {}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }
	};

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		nodecpp::a_timeout(1000);
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "   ...after timeout" );

		clientSock = nodecpp::net::createSocket<ClientSockType>();
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
		CO_RETURN;
	}

	using ClientSockType = MySocketOne;

	awaitable<void> doWhateverWithIncomingData(nodecpp::safememory::soft_ptr<ClientSockType> socket)
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
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				socket->end();
				socket->unref();
				break;
			}
#endif
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
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;

#elif IMPL_VERSION == 3

	class MySocketOne : public nodecpp::net::SocketBase
	{
		int extraData;

	public:
		MySocketOne() {}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }
	};

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::SocketBase::addHandler<MySocketOne, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onWhateverConnect>(this);

		clientSock = nodecpp::net::createSocket<MySocketOne>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}
	
	nodecpp::handler_ret_type onWhateverConnect(nodecpp::safememory::soft_ptr<MySocketOne> socket) 
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
		CO_RETURN;
	}

	using ClientSockType = MySocketOne;

	awaitable<void> doWhateverWithIncomingData(nodecpp::safememory::soft_ptr<MySocketOne> socket)
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
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				socket->end();
				socket->unref();
				break;
			}
#endif
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
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;

#elif IMPL_VERSION == 4
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &MySampleTNode::onWhateverConnect>(this);
		nodecpp::net::SocketBase::addHandler<ClientSockType, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Connect, &ClientSockType::onWhateverConnect>();

		clientSock = nodecpp::net::createSocket<ClientSockType>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}

	class MySocketOne; // just forward declaration
	nodecpp::handler_ret_type onWhateverConnect(nodecpp::safememory::soft_ptr<MySocketOne> socket) 
	{
		printf( "MySampleTNode::onWhateverConnect() with extra = %d\n", *(socket->getExtra()) );
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

		nodecpp::handler_ret_type onWhateverConnect() 
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
			CO_RETURN;
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
#ifdef AUTOMATED_TESTING_ONLY
				if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
					end();
					unref();
					break;
				}
#endif
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
			CO_RETURN;
		}
	};

	using ClientSockType = MySocketOne;


#elif IMPL_VERSION == 5
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

//		clientSock = nodecpp::net::createSocket<MySampleTNode, ClientSockType>();
		clientSock = nodecpp::net::createSocket<ClientSockType>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}

	class MySocketOne; // just forward declaration
	nodecpp::handler_ret_type onWhateverConnect(nodecpp::safememory::soft_ptr<MySocketOne> socket) 
	{
		printf( "MySampleTNode::onWhateverConnect() with extra = %d\n", *(socket->getExtra()) );
		CO_RETURN;
	}

	using ClientSockBaseType = nodecpp::net::SocketBase;

	class MySocketOne : public ClientSockBaseType
	{
	public:
		using NodeType = MySampleTNode;

	private:
		size_t recvSize = 0;
		size_t recvReplies = 0;
		Buffer buf;
		int extraData;

	public:
		MySocketOne() {
//			nodecpp::safememory::soft_ptr<MySocketOne> p = myThis.getSoftPtr<MySocketOne>(this);
///			registerMeAndAcquireSocket<MySampleTNode, MySocketOne>( p );
		}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		nodecpp::handler_ret_type onWhateverConnect() 
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
			CO_RETURN;
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
#ifdef AUTOMATED_TESTING_ONLY
				if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
					end();
					unref();
					break;
				}
#endif
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
			CO_RETURN;
		}
	};

	using ClientSockType = MySocketOne;

	using clientConnect_1 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onWhateverConnect>;
	using clientConnect_2 = nodecpp::net::HandlerData<ClientSockType, &ClientSockType::onWhateverConnect>;
	using clientConnect = nodecpp::net::SocketHandlerDataList<ClientSockType, clientConnect_1, clientConnect_2>;
	using clientSocketHD = nodecpp::net::SocketHandlerDescriptor< ClientSockType, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnConnectT<clientConnect> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<clientSocketHD>;


#elif IMPL_VERSION == 6

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

//		clientSock = nodecpp::net::createSocket<MySampleTNode, ClientSockType>();
		clientSock = nodecpp::net::createSocket<ClientSockType>();
		*( clientSock->getExtra() ) = 17;
		clientSock->connect(2000, "127.0.0.1");
		CO_RETURN;
	}

	using ClientSockBaseType = nodecpp::net::SocketBase;

	class MySocketOne : public ClientSockBaseType
	{
	public:
		using NodeType = MySampleTNode;

	private:
		size_t recvSize = 0;
		size_t recvReplies = 0;
		Buffer buf;
		int extraData;

	public:
		MySocketOne() {}
		virtual ~MySocketOne() {}

		int* getExtra() { return &extraData; }

		void onWhateverConnect() 
		{
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			write(buf);
		}
		void onWhateverData(nodecpp::Buffer& buffer)
		{
			++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				end();
				unref();
			}
#endif
			if ( ( recvReplies & 0xFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onWhateverData(), extra = {}, size = {}, total received size = {}", recvReplies, *(getExtra()), buffer.size(), recvSize );
			recvSize += buffer.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			write(buf);
		}
	};

	using ClientSockType = MySocketOne;

	using clientConnect_1 = nodecpp::net::HandlerData<MySocketOne, &MySocketOne::onWhateverConnect>;
	using clientConnect = nodecpp::net::SocketHandlerDataList<MySocketOne, clientConnect_1>;
	using clientData_1 = nodecpp::net::HandlerData<MySocketOne, &MySocketOne::onWhateverData>;
	using clientData = nodecpp::net::SocketHandlerDataList<MySocketOne, clientData_1>;
	using clientSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketOne, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnConnectT<clientConnect>, nodecpp::net::OnDataT<clientData> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<clientSocketHD>;

#elif IMPL_VERSION == 7

	using ClientSockType = nodecpp::net::SocketBase;
	nodecpp::Timeout to;

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		clientSock = nodecpp::net::createSocket();

		to = std::move( nodecpp::setTimeout( [this]() { 
			printf( "   !!!TIMER!!!\n" );
			nodecpp::refreshTimeout(to);
		}, 1000) );

		clientSock->on(event::connect, [this]() { 
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			clientSock->write(buf);
		});

		clientSock->on(event::data, [this](const Buffer& buffer) { 
			++recvReplies;
#ifdef AUTOMATED_TESTING_ONLY
			if ( recvReplies > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing" );
				clientSock->end();
				clientSock->unref();
			}
#endif
			if ( ( recvReplies & 0xFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );
			recvSize += buffer.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			clientSock->write(buf);
		});

		clientSock->connect(2000, "127.0.0.1");
		
		co_return;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;

#else
#error
#endif

	nodecpp::safememory::owning_ptr<ClientSockType> clientSock;
};

#endif // NET_SOCKET_H
