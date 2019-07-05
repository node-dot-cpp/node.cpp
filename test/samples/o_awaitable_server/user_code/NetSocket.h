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

#ifndef NODECPP_NO_COROUTINES
//#define IMPL_VERSION 2 // main() is a single coro
#define IMPL_VERSION 3 // onConnect is a coro
//#define IMPL_VERSION 5 // adding handler per socket class before creating any socket instance
//#define IMPL_VERSION 6 // adding handler per socket class before creating any socket instance (template-based)
//#define IMPL_VERSION 7 // adding handler per socket class before creating any socket instance (template-based) with no explicit awaitable staff
#else
#define IMPL_VERSION 7 // registering handlers (per class, template-based) with no explicit awaitable staff
#endif // NODECPP_NO_COROUTINES

#ifdef AUTOMATED_TESTING_ONLY
#define AUTOMATED_TESTING_CYCLE_COUNT 30
#endif

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

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		co_await srv.a_listen(2000, "127.0.0.1", 5);
		co_await srvCtrl.a_listen(2001, "127.0.0.1", 5);

		acceptServerLoop();
		acceptCtrlServerLoop();

		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptServerLoop()
	{
		for (;;)
		{
			nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket;
			co_await srv.a_connection<nodecpp::net::SocketO>( socket );
			socketLoop(socket);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptCtrlServerLoop()
	{
		for (;;)
		{
			nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket;
			co_await srvCtrl.a_connection<nodecpp::net::SocketO>( socket );
			socketCtrlLoop(socket);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketCtrlLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType>;

	using ServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerSocket,ServerIdType>;
	ServerType srv; 

	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType>;
	CtrlServerType srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;

	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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
		CO_RETURN;
	}
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketO> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		CO_RETURN;
	}

#elif IMPL_VERSION == 3
	class MyServerSocketOne : public nodecpp::net::ServerBase
	{
	public:
		MyServerSocketOne() {}
		virtual ~MyServerSocketOne() {}
	};

	class MyServerSocketTwo : public nodecpp::net::ServerBase
	{
	public:
		MyServerSocketTwo() {}
		virtual ~MyServerSocketTwo() {}
	};

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}


	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		nodecpp::net::ServerBase::addHandler<ServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionx>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionx(nodecpp::safememory::soft_ptr<MyServerSocketOne>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyServerSocketOne;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = MyServerSocketTwo;
	nodecpp::safememory::owning_ptr<CtrlServerType>  srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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
		CO_RETURN;
	}

#elif IMPL_VERSION == 5

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleLambdaOneNode::main()");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MyServerSocketOne::onListening>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MySampleTNode::onListening>(this);
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MySampleTNode::onListening2>(this);
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MyServerSocketOne::onConnection>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketOne, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnection>(this);

		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MyServerSocketTwo::onListening>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Listen, &MySampleTNode::onListeningCtrl>(this);
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MyServerSocketTwo::onConnection>();
		nodecpp::net::ServerBase::addHandler<MyServerSocketTwo, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<MyServerSocketOne>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

		CO_RETURN;
	}

	// server
	class MyServerSocketOne; // just forward declaration
public:
	nodecpp::handler_ret_type onListening(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onListening2(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening2()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataServerSocket_(socket, r_buff);
#ifdef AUTOMATED_TESTING_ONLY
			if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
				exit( 0 );
#endif
		}
		CO_RETURN;
	}

	// ctrl server
	class MyServerSocketTwo; // just forward declaration
public:
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = nodecpp::net::ServerBase;
	using ServerType = nodecpp::net::ServerBase;


	class MyServerSocketBase : public ServerType
	{
	public:
		MyServerSocketBase() {}
//		MyServerSocketBase(acceptedSocketCreationRoutineType socketCreationCB) : ServerBase(socketCreationCB) {};
		virtual ~MyServerSocketBase() {}
	};

	class MyServerSocketOne : public MyServerSocketBase
	{
	public:
		MyServerSocketOne() {}
//		MyServerSocketOne(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyServerSocketOne() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	class MyServerSocketTwo : public MyServerSocketBase
	{
	public:
		MyServerSocketTwo() {}
//		MyServerSocketTwo(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyServerSocketTwo() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	nodecpp::safememory::owning_ptr<MyServerSocketOne> srv;
	nodecpp::safememory::owning_ptr<MyServerSocketTwo> srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter</*net::SocketO, net::Socket*/>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter</*net::ServerO, net::Server*/>;

	nodecpp::handler_ret_type serverSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
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

		CO_RETURN;
	}
	nodecpp::handler_ret_type serverCtrlSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
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
	nodecpp::handler_ret_type onDataCtrlServerSocket_1(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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
		CO_RETURN;
	}
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		CO_RETURN;
	}

#elif IMPL_VERSION == 6

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleLambdaOneNode::main()");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		/*srv = nodecpp::safememory::make_owning<MyServerSocketOne>(
			[this](OpaqueSocketData& sdata) {
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: creating accepted socket as in node::main()\n");
				return nodecpp::net::createSocket<nodecpp::net::SocketBase>(nullptr, sdata);
			});
		srv_1 = nodecpp::net::createServer<MyServerSocketOne>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, nodecpp::net::SocketBase>();
		srvCtrl_1 = nodecpp::safememory::make_owning<MyServerSocketTwo>();*/

		srv = nodecpp::net::createServer<MyServerSocketOne, nodecpp::net::SocketBase>();
		srv_1 = nodecpp::net::createServer<MyServerSocketOne, nodecpp::net::SocketBase>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, nodecpp::net::SocketBase>();
		srvCtrl_1 = nodecpp::net::createServer<MyServerSocketTwo>(1);

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);
		srv_1->listen(2010, "127.0.0.1", 5);
		srvCtrl_1->listen(2011, "127.0.0.1", 5);

		CO_RETURN;
	}

// server
public:
	class MyServerSocketOne; // just forward declaration
	nodecpp::handler_ret_type onListening(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onListening2(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening2()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	// ctrl server
public:
	class MyServerSocketTwo; // just forward declaration
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address addr) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using ServerType = nodecpp::net::ServerBase;


	class MyServerSocketBase : public ServerType
	{
	public:
		MyServerSocketBase() {}
		virtual ~MyServerSocketBase() {}
	};

	class MyServerSocketOne : public MyServerSocketBase
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketOne() {
//			nodecpp::safememory::soft_ptr<MyServerSocketOne> p = myThis.getSoftPtr<MyServerSocketOne>(this);
//			registerServer<MySampleTNode, MyServerSocketOne>( p );
		}
		virtual ~MyServerSocketOne() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketOne::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	class MyServerSocketTwo : public MyServerSocketBase
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketTwo() {
//			nodecpp::safememory::soft_ptr<MyServerSocketTwo> p = myThis.getSoftPtr<MyServerSocketTwo>(this);
//			registerServer<MySampleTNode, MyServerSocketTwo>( p );
		}
		MyServerSocketTwo(int k) {
			printf( "k=%d\n", k );
//			nodecpp::safememory::soft_ptr<MyServerSocketTwo> p = myThis.getSoftPtr<MyServerSocketTwo>(this);
//			registerServer<MySampleTNode, MyServerSocketTwo>( p );
		}
		virtual ~MyServerSocketTwo() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MyServerSocketTwo::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	nodecpp::safememory::owning_ptr<MyServerSocketOne> srv, srv_1;
	nodecpp::safememory::owning_ptr<MyServerSocketTwo> srvCtrl, srvCtrl_1;

	nodecpp::handler_ret_type serverSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
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
#ifdef AUTOMATED_TESTING_ONLY
			if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
				exit( 0 );
#endif
		}

		CO_RETURN;
	}
	nodecpp::handler_ret_type serverCtrlSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
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
	nodecpp::handler_ret_type onDataCtrlServerSocket_1(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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
		CO_RETURN;
	}
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			size_t replySz = sizeof(Stats);
			reply.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			co_await socket->a_write(reply);
		}
		CO_RETURN;
	}

	// working server
	using workingServerListening_1 = nodecpp::net::HandlerData<MyServerSocketOne, &MyServerSocketOne::onListening>;
	using workingServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListening>;
	using workingServerListening_3 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListening2>;

	using workingServerConnection_1 = nodecpp::net::HandlerData<MyServerSocketOne, &MyServerSocketOne::onConnection>;
	using workingServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnection>;

	using workingServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerListening_1, workingServerListening_2, workingServerListening_3>;
	using workingServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerConnection_1, workingServerConnection_2>;

	using workingServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketOne, nodecpp::net::ServerHandlerDescriptorBase<nodecpp::net::OnConnectionST<workingServerConnection>, nodecpp::net::OnListeningST<workingServerListening> > >;

	// ctrl server
	using ctrlServerListening_1 = nodecpp::net::HandlerData<MyServerSocketTwo, &MyServerSocketTwo::onListening>;
	using ctrlServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningCtrl>;

	using ctrlServerConnection_1 = nodecpp::net::HandlerData<MyServerSocketTwo, &MyServerSocketTwo::onConnection>;
	using ctrlServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnectionCtrl>;

	using ctrlServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerListening_1, ctrlServerListening_2>;
	using ctrlServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerConnection_1, ctrlServerConnection_2>;

	using ctrlServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketTwo, nodecpp::net::ServerHandlerDescriptorBase< nodecpp::net::OnConnectionST<ctrlServerConnection>, nodecpp::net::OnListeningST<ctrlServerListening> > >;

	// all servers
	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<ctrlServerHD, workingServerHD>;


#elif IMPL_VERSION == 7

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("MySampleLambdaOneNode::main()");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		srv = nodecpp::net::createServer<MyServerSocketOne, MySocketSocketOne>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, MySocketSocketTwo>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

		CO_RETURN;
	}

	// handler implementations

	// server socket



	class MyServerSocketOne; // just forward declaration
	void onConnectionServer(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		soft_ptr<MySocketSocketOne> socketPtr = nodecpp::safememory::soft_ptr_static_cast<MySocketSocketOne>(socket);
		socketPtr->myNode = this;
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningServer(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address a) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");}


	class MyServerSocketTwo; // just forward declaration
	void onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		soft_ptr<MySocketSocketTwo> socketPtr = nodecpp::safememory::soft_ptr_static_cast<MySocketSocketTwo>(socket);
		socketPtr->myNode = this;
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address a) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");}



	// servers

	using ServerType = nodecpp::net::ServerBase;

	class MyServerSocketBase : public ServerType
	{
	public:
		MyServerSocketBase() {}
		virtual ~MyServerSocketBase() {}
	};

	class MyServerSocketOne : public MyServerSocketBase
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketOne() {
		}
		virtual ~MyServerSocketOne() {}
	};

	class MyServerSocketTwo : public MyServerSocketBase
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketTwo() {}
		virtual ~MyServerSocketTwo() {}
	};

	nodecpp::safememory::owning_ptr<MyServerSocketOne> srv;
	nodecpp::safememory::owning_ptr<MyServerSocketTwo> srvCtrl;
	
	// sockets
	
	using SocketType = nodecpp::net::SocketBase;

	class MySocketSocketOne : public SocketType
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	private:
		MySampleTNode* myNode = nullptr;

	public:
		MySocketSocketOne() {}
		virtual ~MySocketSocketOne() {}

		void onCloseServerSocket(bool hadError)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
			//srv.removeSocket( socket );
		}
		void onDataServerSocket(Buffer& buffer) {
			if ( buffer.size() < 2 )
			{
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket" );
				//socket->unref();
				return;
			}
			//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
//				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size() );
				//socket->unref();
				return;
			}

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(requestedSz);
				//buffer.begin()[0] = (uint8_t)requestedSz;
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				write(reply.begin(), requestedSz);
			}

			myNode->stats.recvSize += receivedSz;
			myNode->stats.sentSize += requestedSz;
			++(myNode->stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
			if ( myNode->stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
				exit( 0 );
#endif
		}
		void onEndServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
			const char buff[] = "goodbye!";
			socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			socket->end();
		}
	};

	class MySocketSocketTwo : public SocketType
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	private:
		MySampleTNode* myNode = nullptr;

	public:
		MySocketSocketTwo() {}
		virtual ~MySocketSocketTwo() {}
		void onCloseCtrlServerSocket(bool hadError)
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
			//srvCtrl.removeSocket( socket );
		}
		void onDataCtrlServerSocket(Buffer& buffer) {

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				myNode->stats.connCnt = myNode->srv->getSockCount();
				size_t replySz = sizeof(Stats);
				uint8_t* buff = myNode->ptr.get();
				memcpy( buff, &(myNode->stats), replySz ); // naive marshalling will work for a limited number of cases
				write(buff, replySz);
			}
		}
	};

	// declarative part

	// working server
	using workingServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningServer>;
	using workingServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnectionServer>;

	using workingServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerListening_2>;
	using workingServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerConnection_2>;

	using workingServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketOne, nodecpp::net::ServerHandlerDescriptorBase<nodecpp::net::OnConnectionST<workingServerConnection>, nodecpp::net::OnListeningST<workingServerListening> > >;

	// ctrl server
	using ctrlServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningCtrl>;
	using ctrlServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnectionCtrl>;

	using ctrlServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerListening_2>;
	using ctrlServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerConnection_2>;

	using ctrlServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketTwo, nodecpp::net::ServerHandlerDescriptorBase< nodecpp::net::OnConnectionST<ctrlServerConnection>, nodecpp::net::OnListeningST<ctrlServerListening> > >;

	// all servers
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<ctrlServerHD, workingServerHD>;


	// working socket
	using workingSocketData_1 = nodecpp::net::HandlerData<MySocketSocketOne, &MySocketSocketOne::onDataServerSocket>;
	using workingSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketOne, workingSocketData_1>;

	using workingSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketOne, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<workingSocketData> > >;

	using ctrlSocketData_1 = nodecpp::net::HandlerData<MySocketSocketTwo, &MySocketSocketTwo::onDataCtrlServerSocket>;
	using ctrlSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketTwo, ctrlSocketData_1>;

	using ctrlSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketTwo, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<ctrlSocketData> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<workingSocketHD, ctrlSocketHD>;

#else
#error
#endif // IMPL_VERSION

	nodecpp::handler_ret_type onDataServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data: received {} bytes", buffer.size() );
			socket->unref();
			CO_RETURN;
		}
//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			CO_RETURN;
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
#ifdef AUTOMATED_TESTING_ONLY
			if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
				exit( 0 );
#endif
		CO_RETURN;
	}
};

#endif // NET_SOCKET_H
