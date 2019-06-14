// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/socket_t_base.h>
//#include <nodecpp/server_t.h>
#include <nodecpp/server_type_list.h>


using namespace std;
using namespace nodecpp;
using namespace fmt;

//#define IMPL_VERSION 2 // main() is a single coro
//#define IMPL_VERSION 3 // onConnect is a coro
#define IMPL_VERSION 5 // adding handler per socket class before creating any socket instance

class MySampleTNode : public NodeBase
{
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;

	using SocketIdType = int;
	using ServerIdType = int;

public:
#if IMPL_VERSION == 2
	MySampleTNode() : srv( this ), srvCtrl( this )
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		co_await srv.a_listen(2000, "127.0.0.1", 5);
		co_await srvCtrl.a_listen(2001, "127.0.0.1", 5);

		acceptServerLoop();
		acceptCtrlServerLoop();

		co_return;
	}

	nodecpp::awaitable<void> acceptServerLoop()
	{
		for (;;)
		{
			nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket;
			co_await srv.a_connection<nodecpp::net::SocketO>( socket );
			socketLoop(socket);
		}
		co_return;
	}

	nodecpp::awaitable<void> socketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		co_return;
	}

	nodecpp::awaitable<void> acceptCtrlServerLoop()
	{
		for (;;)
		{
			nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket;
			co_await srvCtrl.a_connection<nodecpp::net::SocketO>( socket );
			socketCtrlLoop(socket);
		}
		co_return;
	}

	nodecpp::awaitable<void> socketCtrlLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		co_return;
	}

	using SockTypeServerSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;

	using ServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerSocket,ServerIdType>;
	ServerType srv; 

	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType>;
	CtrlServerType srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;

	nodecpp::awaitable<void> onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			uint32_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy(buff, &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
		co_return;
	}
	nodecpp::awaitable<void> onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		co_return;
	}

#elif IMPL_VERSION == 3
	MySampleTNode() : srv( this ), srvCtrl( this )
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}


#if 0
	nodecpp::awaitable<void> dummyL(size_t n, nodecpp::net::Address)
	{
		printf( "dummyL(%zd, ...)\n", n );
		co_return;
	}

	nodecpp::awaitable<void> dummyConn(nodecpp::safememory::soft_ptr<net::SocketBase> socket)
	{
		printf( "dummyConn(...)\n" );
		co_return;
	}

	nodecpp::awaitable<void> dummyCl(bool hadError)
	{
		printf( "dummyCl(%s, ...)\n", hadError ? "true" : "false" );
		co_return;
	}

	nodecpp::awaitable<void> dummyCl_1(bool hadError)
	{
		printf("dummyCl_1(%s, ...)\n", hadError ? "true" : "false");
		co_return;
	}

	nodecpp::awaitable<void> dummyE(Error& e)
	{
		printf( "dummyE(%s, ...)\n", e.what() );
		co_return;
	}
#endif // 0

	virtual nodecpp::awaitable<void> main()
	{
#if 0
		Error e;
		srv.addHandler<nodecpp::net::ServerBase::Handler::Listen, &MySampleTNode::dummyL>( this );
		srv.addHandler<nodecpp::net::ServerBase::Handler::Connection, &MySampleTNode::dummyConn>( this );
		srv.addHandler<nodecpp::net::ServerBase::Handler::Close, &MySampleTNode::dummyCl>( this );
		srv.addHandler<nodecpp::net::ServerBase::Handler::Close, & MySampleTNode::dummyCl_1>(this);
		srv.addHandler<nodecpp::net::ServerBase::Handler::Error, &MySampleTNode::dummyE>( this );

		printf("checking presence of handlers after adding...\n");
		if ( srv.dataForCommandProcessing.isListenEventHandler())
			srv.dataForCommandProcessing.handleListenEvent(3, nodecpp::net::Address());
		else
			printf( "no L handler\n" );

		if (srv.dataForCommandProcessing.isConnectionEventHandler())
			srv.dataForCommandProcessing.handleConnectionEvent(nodecpp::safememory::soft_ptr<net::SocketBase>() );
		else
			printf( "no CONN handler\n" );

		if (srv.dataForCommandProcessing.isCloseEventHandler())
			srv.dataForCommandProcessing.handleCloseEvent( true );
		else
			printf( "no CL handler\n" );

		if (srv.dataForCommandProcessing.isErrorEventHandler())
			srv.dataForCommandProcessing.handleErrorEvent( e );
		else
			printf( "no E handler\n" );

		srv.removeHandler<nodecpp::net::ServerBase::Handler::Close, & MySampleTNode::dummyCl>(this);

		printf("checking presence of handlers after removing...\n");
		if (srv.dataForCommandProcessing.isListenEventHandler())
			srv.dataForCommandProcessing.handleListenEvent(3, nodecpp::net::Address());
		else
			printf("no L handler\n");

		if (srv.dataForCommandProcessing.isConnectionEventHandler())
			srv.dataForCommandProcessing.handleConnectionEvent(nodecpp::safememory::soft_ptr<net::SocketBase>());
		else
			printf("no CONN handler\n");

		if (srv.dataForCommandProcessing.isCloseEventHandler())
			srv.dataForCommandProcessing.handleCloseEvent(true);
		else
			printf("no CL handler\n");

		if (srv.dataForCommandProcessing.isErrorEventHandler())
			srv.dataForCommandProcessing.handleErrorEvent(e);
		else
			printf("no E handler\n");

		co_return;
#endif // 0
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		srv.listen(2000, "127.0.0.1", 5);
		srvCtrl.listen(2001, "127.0.0.1", 5);

		co_return;
	}

	nodecpp::awaitable<void> onConnectionx(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		co_return;
	}

	nodecpp::awaitable<void> onConnectionCtrl(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		co_return;
	}

	using SockTypeServerSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;

	using ServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionx>
	>;
	ServerType srv; 

	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionCtrl>
	>;
	CtrlServerType srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
	nodecpp::awaitable<void> onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			uint32_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy(buff, &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
		co_return;
	}
	nodecpp::awaitable<void> onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		co_return;
	}

#elif IMPL_VERSION == 5

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleLambdaOneNode::main()");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MyServerSocketOne::onListening>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MySampleTNode::onListening>(this);
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MyServerSocketOne::onConnection>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnection>(this);

		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MyServerSocketTwo::onListening>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MySampleTNode::onListeningCtrl>(this);
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MyServerSocketTwo::onConnection>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::safememory::make_owning<MyServerSocketOne>(
			[this](OpaqueSocketData& sdata) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: creating accepted socket\n");
			return nodecpp::net::createSocket<nodecpp::net::SocketBase>(nullptr, sdata);
				});
		srv_1 = nodecpp::net::createServer<MyServerSocketOne>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo>();
		srvCtrl_1 = nodecpp::safememory::make_owning<MyServerSocketTwo>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);
		srv_1->listen(2010, "127.0.0.1", 5);
		srvCtrl_1->listen(2011, "127.0.0.1", 5);

		co_return;
	}

	//	using SockTypeServerSocket = nodecpp::net::SocketBase;
	//	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	// server
public:
	nodecpp::awaitable<void> onListening(size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");
		co_return;
	}
	nodecpp::awaitable<void> onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataServerSocket_(socket, r_buff);
		}
		co_return;
	}

	// ctrl server
public:
	nodecpp::awaitable<void> onListeningCtrl(size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");
		co_return;
	}
	nodecpp::awaitable<void> onConnectionCtrl(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		co_return;
	}

//	using SockTypeServerSocket = nodecpp::net::SocketN<MySampleTNode, SocketIdType>;
//	using SockTypeServerCtrlSocket = nodecpp::net::SocketN<MySampleTNode, SocketIdType>;
	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

//	using ServerType = nodecpp::net::ServerN<MySampleTNode, SockTypeServerSocket, ServerIdType>;
	using ServerType = nodecpp::net::ServerBase;
//	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode, SockTypeServerCtrlSocket, ServerIdType>;
	using ServerType = nodecpp::net::ServerBase;


	class MyServerSocketBase : public ServerType
	{
	public:
		MyServerSocketBase() {}
		MyServerSocketBase(acceptedSocketCreationRoutineType socketCreationCB) : ServerBase(socketCreationCB) {};
		virtual ~MyServerSocketBase() {}
	};

	class MyServerSocketOne : public MyServerSocketBase
	{
	public:
		MyServerSocketOne() {}
		MyServerSocketOne(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyServerSocketOne() {}

		nodecpp::awaitable<void> onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onListening()!");
			co_return;
		}
		nodecpp::awaitable<void> onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			co_return;
		}
	};

	class MyServerSocketTwo : public MyServerSocketBase
	{
	public:
		MyServerSocketTwo() {}
		MyServerSocketTwo(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyServerSocketTwo() {}

		nodecpp::awaitable<void> onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onListening()!");
			co_return;
		}
		nodecpp::awaitable<void> onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			co_return;
		}
	};

	nodecpp::safememory::owning_ptr<MyServerSocketOne> srv, srv_1;
	nodecpp::safememory::owning_ptr<MyServerSocketTwo> srvCtrl, srvCtrl_1;

	using EmitterType = nodecpp::net::SocketTEmitter</*net::SocketO, net::Socket*/>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter</*net::ServerO, net::Server*/>;

	nodecpp::awaitable<void> serverSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer buffer(0x20);

		for (;; )
		{
			buffer.clear();
			try
			{
				co_await socket->a_read(buffer, 2);
			}
			catch (...)
			{
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed). Exiting...");
				break;
			}

			size_t receivedSz = buffer.begin()[0];
			if (receivedSz != buffer.size())
			{
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size());
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size());
				socket->unref();
				break;
			}

			size_t requestedSz = buffer.begin()[1];
			if (requestedSz)
			{
				Buffer reply(requestedSz);
				//buffer.begin()[0] = (uint8_t)requestedSz;
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				socket->write(reply.begin(), requestedSz);
				try
				{
					co_await socket->a_write(reply);
				}
				catch (...)
				{
//					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed. Exiting...");
					break;
				}
			}

			stats.recvSize += receivedSz;
			stats.sentSize += requestedSz;
			++(stats.rqCnt);
		}

		co_return;
	}
	nodecpp::awaitable<void> serverCtrlSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer buffer(0x20);
		Buffer reply(sizeof(stats));

		for (;; )
		{
			buffer.clear();
			try
			{
				co_await socket->a_read(buffer, 2);
			}
			catch (...)
			{
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Reading data failed). Exiting..." );
				break;
			}
			size_t requestedSz = buffer.begin()[1];
			if (requestedSz)
			{
				stats.connCnt = srv->getSockCount();
				size_t replySz = sizeof(Stats);
				reply.clear();
				reply.append(&stats, replySz);
				try
				{
					co_await socket->a_write(reply);
				}
				catch (...)
				{
//					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::error>("Writing data failed). Exiting..." );
					break;
				}
			}
		}
	}
	nodecpp::awaitable<void> onDataCtrlServerSocket_1(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy(buff, &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
		co_return;
	}
	nodecpp::awaitable<void> onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		co_return;
	}

#else
#error
#endif // IMPL_VERSION

	nodecpp::awaitable<void> onDataServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data: received {} bytes", buffer.size() );
			socket->unref();
			co_return;
		}
//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			co_return;
		}

		uint32_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			socket->write(reply.begin(), requestedSz);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);
		co_return;
	}

	nodecpp::awaitable<void> onDataServerSocket_1(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket" );
			socket->unref();
			co_return;
		}
//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			co_return;
		}

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			reply.set_size( requestedSz );
			co_await socket->a_write(reply);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);
		co_return;
	}
};

#endif // NET_SOCKET_H
