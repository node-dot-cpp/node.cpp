// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>


using namespace std;
using namespace nodecpp;
using namespace fmt;

#ifndef NODECPP_NO_COROUTINES
//#define IMPL_VERSION 2 // main() is a single coro
//#define IMPL_VERSION 21 // main() is a single coro (using awaitable API with time restrictions)
//#define IMPL_VERSION 3 // onConnect is a coro
#define IMPL_VERSION 5 // adding handler per socket class before creating any socket instance
//#define IMPL_VERSION 6 // adding handler per socket class before creating any socket instance (template-based)
//#define IMPL_VERSION 7 // adding handler per socket class before creating any socket instance (template-based with use of DataParent concept)
//#define IMPL_VERSION 8 // adding handler per socket class before creating any socket instance (template-based) with no explicit awaitable staff
//#define IMPL_VERSION 9 // lambda-based
#else
#define IMPL_VERSION 8 // registering handlers (per class, template-based) with no explicit awaitable staff
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

	Buffer replyBuff;

public:
#if IMPL_VERSION == 2

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		srv = nodecpp::net::createServer<nodecpp::net::ServerBase>();
		srvCtrl = nodecpp::net::createServer<nodecpp::net::ServerBase, nodecpp::net::SocketBase>();

		co_await srv->a_listen(2000, "127.0.0.1", 5);
		co_await srvCtrl->a_listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		acceptServerLoop();
		acceptCtrlServerLoop();

		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptServerLoop()
	{
		for (;;)
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			co_await srv->a_connection<nodecpp::net::SocketBase>( socket );
			socketLoop(socket);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptCtrlServerLoop()
	{
		for (;;)
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			co_await srvCtrl->a_connection<nodecpp::net::SocketBase>( socket );
			socketCtrlLoop(socket);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketCtrlLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;

	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			replyBuff.clear();
			replyBuff.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(replyBuff);
		}
		CO_RETURN;
	}

	nodecpp::owning_ptr<nodecpp::net::ServerBase> srv; 
	nodecpp::owning_ptr<nodecpp::net::ServerBase>  srvCtrl;

#elif IMPL_VERSION == 21

#ifdef AUTOMATED_TESTING_ONLY
	static constexpr size_t maxAcceptanceTime = 3000;
	static constexpr size_t maxInteractionTime = 3000;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		srv = nodecpp::net::createServer<nodecpp::net::ServerBase>();
		srvCtrl = nodecpp::net::createServer<nodecpp::net::ServerBase, nodecpp::net::SocketBase>();

		co_await srv->a_listen(2000, "127.0.0.1", 5);
		co_await srvCtrl->a_listen(2001, "127.0.0.1", 5);

		acceptServerLoop();
		acceptCtrlServerLoop();

		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptServerLoop()
	{
#ifndef AUTOMATED_TESTING_ONLY
		for (;;)
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			co_await srv->a_connection<nodecpp::net::SocketBase>( socket );
			socketLoop(socket);
		}
#else
		size_t startTime = nodecpp::time::now();
		size_t acceptanceTime = 0;
		while ( acceptanceTime < maxAcceptanceTime )
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			try { co_await srv->a_connection<nodecpp::net::SocketBase>( socket, maxAcceptanceTime - acceptanceTime ); } catch ( ... ) { break; }
			socketLoop(socket);
			acceptanceTime = nodecpp::time::now() - startTime;
		}
		srv->close();
		srv->unref();
#endif
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer r_buff(0x200);
#ifndef AUTOMATED_TESTING_ONLY
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
#else
		size_t respondingStartTime = nodecpp::time::now();
		size_t respondingTime = 0;
		while ( respondingTime < maxInteractionTime )
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "iteration: waiting for {} at max", maxInteractionTime - respondingTime );
			try { co_await socket->a_read( maxInteractionTime - respondingTime, r_buff, 2 ); } catch ( ... ) { break; }
			co_await onDataServerSocket_(socket, r_buff);
			respondingTime = nodecpp::time::now() - respondingStartTime;
		}
		socket->end();
		socket->unref();
#endif
		CO_RETURN;
	}

	nodecpp::handler_ret_type acceptCtrlServerLoop()
	{
#ifndef AUTOMATED_TESTING_ONLY
		for (;;)
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			co_await srvCtrl->a_connection<nodecpp::net::SocketBase>( socket );
			socketCtrlLoop(socket);
		}
#else
		size_t startTime = nodecpp::time::now();
		size_t acceptanceTime = 0;
		while ( acceptanceTime < maxAcceptanceTime )
		{
			nodecpp::soft_ptr<nodecpp::net::SocketBase> socket;
			try { co_await srvCtrl->a_connection<nodecpp::net::SocketBase>( socket, maxAcceptanceTime - acceptanceTime ); } catch ( ... ) { break; }
			socketLoop(socket);
			acceptanceTime = nodecpp::time::now() - startTime;
		}
		srvCtrl->close();
		srvCtrl->unref();
#endif
		CO_RETURN;
	}

	nodecpp::handler_ret_type socketCtrlLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer r_buff(0x200);
#ifndef AUTOMATED_TESTING_ONLY
		for (;;)
		{
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
#else
		size_t respondingStartTime = nodecpp::time::now();
		size_t respondingTime = 0;
		while ( respondingTime < maxInteractionTime )
		{
			try { co_await socket->a_read( maxInteractionTime - respondingTime, r_buff, 2 ); } catch ( ... ) { break; }
			co_await onDataCtrlServerSocket_(socket, r_buff);
			respondingTime = nodecpp::time::now() - respondingStartTime;
		}
		socket->end();
		socket->unref();
#endif
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;

	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			replyBuff.clear();
			replyBuff.append(&stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(replyBuff);
		}
		CO_RETURN;
	}

	nodecpp::owning_ptr<nodecpp::net::ServerBase> srv; 
	nodecpp::owning_ptr<nodecpp::net::ServerBase>  srvCtrl;

#elif IMPL_VERSION == 3

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

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
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()" );
	}


	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<ServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionx>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionx(nodecpp::soft_ptr<MyServerSocketOne> server, nodecpp::soft_ptr<net::SocketBase> socket) { 
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 

		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, nodecpp::soft_ptr<net::SocketBase> socket) { 
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyServerSocketOne;
	nodecpp::owning_ptr<ServerType> srv; 

	using CtrlServerType = MyServerSocketTwo;
	nodecpp::owning_ptr<CtrlServerType>  srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			replyBuff.clear();
			replyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(replyBuff);
		}
		CO_RETURN;
	}

#elif IMPL_VERSION == 5

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()");

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

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	// server
	class MyServerSocketOne; // just forward declaration
public:
	nodecpp::handler_ret_type onListening(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onListening2(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening2()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<MyServerSocketOne> server, nodecpp::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);

		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
			co_await socket->a_read(r_buff, 2);
			co_await onDataServerSocket_(socket, r_buff);
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
		}
		CO_RETURN;
	}

	// ctrl server
	class MyServerSocketTwo; // just forward declaration
public:
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListeninCtrlg()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, nodecpp::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);

		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Closing socket (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketOne::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketOne::onConnection()!");
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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketTwo::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketTwo::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	nodecpp::owning_ptr<MyServerSocketOne> srv;
	nodecpp::owning_ptr<MyServerSocketTwo> srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter</*net::SocketO, net::Socket*/>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter</*net::ServerO, net::Server*/>;

	nodecpp::handler_ret_type serverSocketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer buffer(0x20);

		for (;; )
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Closing socket (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			buffer.clear();
			try
			{
				co_await socket->a_read(buffer, 2);
			}
			catch (...)
			{
//				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed). Exiting...");
				break;
			}

			size_t receivedSz = buffer.begin()[0];
			if (receivedSz != buffer.size())
			{
//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size());
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size());
				socket->unref();
				break;
			}

			size_t requestedSz = buffer.begin()[1];
			if (requestedSz)
			{
				Buffer reply(requestedSz);
				reply.set_size(requestedSz);
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				try
				{
					co_await socket->a_write(reply);
				}
				catch (...)
				{
//					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed. Exiting...");
					break;
				}
			}

			stats.recvSize += receivedSz;
			stats.sentSize += requestedSz;
			++(stats.rqCnt);
		}

		CO_RETURN;
	}
	nodecpp::handler_ret_type serverCtrlSocketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
	{
		nodecpp::Buffer buffer(0x20);
		Buffer reply(sizeof(stats));

		for (;; )
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Closing socket (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			buffer.clear();
			try
			{
				co_await socket->a_read(buffer, 2);
			}
			catch (...)
			{
//				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed). Exiting..." );
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
//					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed). Exiting..." );
					break;
				}
			}
		}
	}
	/*nodecpp::handler_ret_type onDataCtrlServerSocket_1(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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
	}*/
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()");

		srv = nodecpp::net::createServer<MyServerSocketOne, nodecpp::net::SocketBase>();
		srv_1 = nodecpp::net::createServer<MyServerSocketOne, nodecpp::net::SocketBase>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, nodecpp::net::SocketBase>();
		srvCtrl_1 = nodecpp::net::createServer<MyServerSocketTwo>(1);

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

// server
public:
	class MyServerSocketOne; // just forward declaration
	nodecpp::handler_ret_type onListening(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onListening2(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening2()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<MyServerSocketOne> server, nodecpp::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Closing socket (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read(r_buff, 2);
			co_await onDataServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	// ctrl server
public:
	class MyServerSocketTwo; // just forward declaration
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListeninCtrlg()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, nodecpp::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Closing socket (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
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
//			nodecpp::soft_ptr<MyServerSocketOne> p = myThis.getSoftPtr<MyServerSocketOne>(this);
//			registerServer<MySampleTNode, MyServerSocketOne>( p );
		}
		virtual ~MyServerSocketOne() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketOne::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketOne::onConnection()!");
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
//			nodecpp::soft_ptr<MyServerSocketTwo> p = myThis.getSoftPtr<MyServerSocketTwo>(this);
//			registerServer<MySampleTNode, MyServerSocketTwo>( p );
		}
		MyServerSocketTwo(int k) {
			printf( "k=%d\n", k );
//			nodecpp::soft_ptr<MyServerSocketTwo> p = myThis.getSoftPtr<MyServerSocketTwo>(this);
//			registerServer<MySampleTNode, MyServerSocketTwo>( p );
		}
		virtual ~MyServerSocketTwo() {}

		nodecpp::handler_ret_type onListening(size_t id, nodecpp::net::Address addr) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketTwo::onListening()!");
			CO_RETURN;
		}
		nodecpp::handler_ret_type onConnection(nodecpp::soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketTwo::onConnection()!");
			NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);
			CO_RETURN;
		}
	};

	nodecpp::owning_ptr<MyServerSocketOne> srv, srv_1;
	nodecpp::owning_ptr<MyServerSocketTwo> srvCtrl, srvCtrl_1;

	nodecpp::handler_ret_type serverSocketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
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
				//				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed). Exiting...");
				break;
			}

			size_t receivedSz = buffer.begin()[0];
			if (receivedSz != buffer.size())
			{
				//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size());
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size());
				socket->unref();
				break;
			}

			size_t requestedSz = buffer.begin()[1];
			if (requestedSz)
			{
				Buffer reply(requestedSz);
				reply.set_size(requestedSz);
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				try
				{
					co_await socket->a_write(reply);
				}
				catch (...)
				{
					//					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed. Exiting...");
					break;
				}
			}

			stats.recvSize += receivedSz;
			stats.sentSize += requestedSz;
			++(stats.rqCnt);
		}

		CO_RETURN;
	}
	nodecpp::handler_ret_type serverCtrlSocketLoop(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket)
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
				//				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed (extra = {}). Exiting...", *(socket->getExtra()));
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Reading data failed). Exiting..." );
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
					//					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed (extra = {}). Exiting...", *(socket->getExtra()));
					log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed). Exiting..." );
					break;
				}
			}
		}
	}
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()");

		srv = nodecpp::net::createServer<MyServerSocketOne, MySocketSocketOne>(this);
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, MySocketSocketTwo>(this);

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	// handler implementations at node itself

	class MyServerSocketOne; // just forward declaration
	nodecpp::handler_ret_type onListeningServer(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address a) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening()!");
		CO_RETURN;
	}

	class MyServerSocketTwo; // just forward declaration
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address a) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListeninCtrlg()!");
		CO_RETURN;
	}


	// servers

	using ServerType = nodecpp::net::ServerSocket<MySampleTNode>;

	class MyServerSocketOne : public ServerType
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketOne() {}
		MyServerSocketOne(MySampleTNode* node) : ServerType(node) {}
		virtual ~MyServerSocketOne() {}
		nodecpp::handler_ret_type onConnectionServer(nodecpp::soft_ptr<net::SocketBase> socket) { 
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
			soft_ptr<MySocketSocketOne> socketPtr = nodecpp::soft_ptr_static_cast<MySocketSocketOne>(socket);
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
			CO_RETURN;
		}
	};

	class MyServerSocketTwo : public ServerType
	{
	public:
		using NodeType = MySampleTNode;

	public:
		MyServerSocketTwo() {}
		MyServerSocketTwo(MySampleTNode* node) : ServerType(node) {}
		virtual ~MyServerSocketTwo() {}
		nodecpp::handler_ret_type onConnectionCtrl(nodecpp::soft_ptr<net::SocketBase> socket) { 
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
			soft_ptr<MySocketSocketTwo> socketPtr = nodecpp::soft_ptr_static_cast<MySocketSocketTwo>(socket);
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
			CO_RETURN;
		}
	};

	nodecpp::owning_ptr<MyServerSocketOne> srv;
	nodecpp::owning_ptr<MyServerSocketTwo> srvCtrl;
	
	// sockets
	
	using SocketType = nodecpp::net::SocketBase;

	class MySocketSocketOne : public SocketType, public ::nodecpp::DataParent<MySampleTNode>
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	public:
		MySocketSocketOne() {}
		MySocketSocketOne(MySampleTNode* node) : SocketType(), ::nodecpp::DataParent<MySampleTNode>(node) {}
		virtual ~MySocketSocketOne() {}

		void onCloseServerSocket(bool hadError)
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
			//srv.removeSocket( socket );
		}
		void onDataServerSocket(Buffer& buffer) {
			if ( buffer.size() < 2 )
			{
//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = {}", *(socket->getExtra()) );
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket" );
				//socket->unref();
				return;
			}
			//log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onData for idx {} !", *(socket->getExtra()) );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size() );
				//socket->unref();
				return;
			}

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(requestedSz);
				reply.set_size(requestedSz);
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				write(reply);
			}

			getDataParent()->stats.recvSize += receivedSz;
			getDataParent()->stats.sentSize += requestedSz;
			++(getDataParent()->stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
			if ( getDataParent()->stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
				end();
				unref();
			}
#endif
		}
		void onEndServerSocket() {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
			Buffer b;
			b.appendString( "goodbye!", sizeof( "goodbye!" ) );
			write(b);
//			end(); // so far (yet to be changable) default is allowHalfOpen == false, so this call is not mandatory
		}
	};

	class MySocketSocketTwo : public SocketType, public ::nodecpp::DataParent<MySampleTNode>
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	public:
		MySocketSocketTwo() {}
		MySocketSocketTwo(MySampleTNode* node) : SocketType(), ::nodecpp::DataParent<MySampleTNode>(node) {}
		virtual ~MySocketSocketTwo() {}
		void onCloseCtrlServerSocket(bool hadError)
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
			//srvCtrl.removeSocket( socket );
		}
		void onDataCtrlServerSocket(Buffer& buffer) {

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				getDataParent()->stats.connCnt = getDataParent()->srv->getSockCount();
				size_t replySz = sizeof(Stats);
				reply.append( &(getDataParent()->stats), replySz ); // naive marshalling will work for a limited number of cases
				write(reply);
			}
		}
	};

	// declarative part

	// working server
	using workingServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningServer>;
	using workingServerConnection_2 = nodecpp::net::HandlerData<MyServerSocketOne, &MyServerSocketOne::onConnectionServer>;

	using workingServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerListening_2>;
	using workingServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerConnection_2>;

	using workingServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketOne, nodecpp::net::ServerHandlerDescriptorBase<nodecpp::net::OnConnectionST<workingServerConnection>, nodecpp::net::OnListeningST<workingServerListening> > >;

	// ctrl server
	using ctrlServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningCtrl>;
	using ctrlServerConnection_2 = nodecpp::net::HandlerData<MyServerSocketTwo, &MyServerSocketTwo::onConnectionCtrl>;

	using ctrlServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerListening_2>;
	using ctrlServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerConnection_2>;

	using ctrlServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketTwo, nodecpp::net::ServerHandlerDescriptorBase< nodecpp::net::OnConnectionST<ctrlServerConnection>, nodecpp::net::OnListeningST<ctrlServerListening> > >;

	// all servers
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<ctrlServerHD, workingServerHD>;


	// working socket
	using workingSocketData_1 = nodecpp::net::HandlerData<MySocketSocketOne, &MySocketSocketOne::onDataServerSocket>;
	using workingSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketOne, workingSocketData_1>;
	using workingSocketEnd_1 = nodecpp::net::HandlerData<MySocketSocketOne, &MySocketSocketOne::onEndServerSocket>;
	using workingSocketEnd = nodecpp::net::SocketHandlerDataList<MySocketSocketOne, workingSocketEnd_1>;

	using workingSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketOne, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<workingSocketData>, nodecpp::net::OnEndT<workingSocketEnd> > >;

	using ctrlSocketData_1 = nodecpp::net::HandlerData<MySocketSocketTwo, &MySocketSocketTwo::onDataCtrlServerSocket>;
	using ctrlSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketTwo, ctrlSocketData_1>;

	using ctrlSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketTwo, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<ctrlSocketData> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<workingSocketHD, ctrlSocketHD>;


#elif IMPL_VERSION == 8

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()");
	}

	virtual nodecpp::handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()");

		srv = nodecpp::net::createServer<MyServerSocketOne, MySocketSocketOne>();
		srvCtrl = nodecpp::net::createServer<MyServerSocketTwo, MySocketSocketTwo>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to request closing server");
			srv->close();
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to request closing ctrl server");
			srvCtrl->close();
			stopAccepting = true;
			log::default_log::info( log::ModuleID(nodecpp_module_id), "resetting timer");
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	// handler implementations

	// server socket



	class MyServerSocketOne; // just forward declaration
	void onConnectionServer(nodecpp::soft_ptr<MyServerSocketOne> server, nodecpp::soft_ptr<net::SocketBase> socket) { 
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
		soft_ptr<MySocketSocketOne> socketPtr = nodecpp::soft_ptr_static_cast<MySocketSocketOne>(socket);
		socketPtr->myNode = this;
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningServer(nodecpp::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address a) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening()!");
	}
	void onCloseServer(nodecpp::soft_ptr<MyServerSocketOne> server, bool hasError) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onClose()!");
		srv->unref();
	}


	class MyServerSocketTwo; // just forward declaration
	void onConnectionCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, nodecpp::soft_ptr<net::SocketBase> socket) { 
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
		soft_ptr<MySocketSocketTwo> socketPtr = nodecpp::soft_ptr_static_cast<MySocketSocketTwo>(socket);
		socketPtr->myNode = this;
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address a) {log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListeninCtrlg()!");}
	void onCloseServerCtrl(nodecpp::soft_ptr<MyServerSocketTwo> server, bool hasError) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onCloseCtrl()!");
		srvCtrl->unref();
	}



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

	nodecpp::owning_ptr<MyServerSocketOne> srv;
	nodecpp::owning_ptr<MyServerSocketTwo> srvCtrl;
	
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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
		}
		void onDataServerSocket(const Buffer& buffer) {
			if ( buffer.size() < 2 )
			{
//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = {}", *(socket->getExtra()) );
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket" );
				//socket->unref();
				return;
			}
			//log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onData for idx {} !", *(socket->getExtra()) );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
//				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
				log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size() );
				//socket->unref();
				return;
			}

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(requestedSz);
				reply.set_size(requestedSz);
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				write(reply);
			}

			myNode->stats.recvSize += receivedSz;
			myNode->stats.sentSize += requestedSz;
			++(myNode->stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
			if ( myNode->stopResponding )
			{
				log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing" );
				end();
				unref();
			}
#endif
		}
		void onEndServerSocket() {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
			Buffer b;
			b.appendString( "goodbye!", sizeof( "goodbye!" ) );
			write(b);
//			end();
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
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
		}
		void onDataCtrlServerSocket(const Buffer& buffer) {

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				myNode->stats.connCnt = myNode->srv->getSockCount();
				size_t replySz = sizeof(Stats);
				reply.append( &(myNode->stats), replySz ); // naive marshalling will work for a limited number of cases
				write(reply);
			}
		}
		void onEndCtrlServerSocket() {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "ctrl server socket: onEnd!");
			Buffer b;
			b.appendString( "goodbye!", sizeof( "goodbye!" ) );
			write(b);
//			end();
		}
	};

	// declarative part

	// working server
	using workingServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningServer>;
	using workingServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnectionServer>;
	using workingServerClose_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onCloseServer>;

	using workingServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerListening_2>;
	using workingServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerConnection_2>;
	using workingServerClose = nodecpp::net::ServerHandlerDataList<MyServerSocketOne, workingServerClose_2>;

	using workingServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketOne, nodecpp::net::ServerHandlerDescriptorBase<nodecpp::net::OnConnectionST<workingServerConnection>, nodecpp::net::OnListeningST<workingServerListening>, nodecpp::net::OnCloseST<workingServerClose> > >;

	// ctrl server
	using ctrlServerListening_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onListeningCtrl>;
	using ctrlServerConnection_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onConnectionCtrl>;
	using ctrlServerClose_2 = nodecpp::net::HandlerData<MySampleTNode, &MySampleTNode::onCloseServerCtrl>;

	using ctrlServerListening = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerListening_2>;
	using ctrlServerConnection = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerConnection_2>;
	using ctrlServerClose = nodecpp::net::ServerHandlerDataList<MyServerSocketTwo, ctrlServerClose_2>;

	using ctrlServerHD = nodecpp::net::ServerHandlerDescriptor< MyServerSocketTwo, nodecpp::net::ServerHandlerDescriptorBase< nodecpp::net::OnConnectionST<ctrlServerConnection>, nodecpp::net::OnListeningST<ctrlServerListening>, nodecpp::net::OnCloseST<ctrlServerClose> > >;

	// all servers
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<ctrlServerHD, workingServerHD>;


	// working socket
	using workingSocketData_1 = nodecpp::net::HandlerData<MySocketSocketOne, &MySocketSocketOne::onDataServerSocket>;
	using workingSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketOne, workingSocketData_1>;
	using workingSocketEnd_1 = nodecpp::net::HandlerData<MySocketSocketOne, &MySocketSocketOne::onEndServerSocket>;
	using workingSocketEnd = nodecpp::net::SocketHandlerDataList<MySocketSocketOne, workingSocketEnd_1>;

	using workingSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketOne, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<workingSocketData>, nodecpp::net::OnEndT<workingSocketEnd> > >;

	using ctrlSocketData_1 = nodecpp::net::HandlerData<MySocketSocketTwo, &MySocketSocketTwo::onDataCtrlServerSocket>;
	using ctrlSocketData = nodecpp::net::SocketHandlerDataList<MySocketSocketTwo, ctrlSocketData_1>;
	using ctrlSocketEnd_1 = nodecpp::net::HandlerData<MySocketSocketTwo, &MySocketSocketTwo::onEndCtrlServerSocket>;
	using ctrlSocketEnd = nodecpp::net::SocketHandlerDataList<MySocketSocketTwo, ctrlSocketEnd_1>;

	using ctrlSocketHD = nodecpp::net::SocketHandlerDescriptor< MySocketSocketTwo, nodecpp::net::SocketHandlerDescriptorBase<nodecpp::net::OnDataT<ctrlSocketData>, nodecpp::net::OnEndT<ctrlSocketEnd> > >;

	using EmitterType = nodecpp::net::SocketTEmitter<workingSocketHD, ctrlSocketHD>;

#elif IMPL_VERSION == 9

	virtual nodecpp::handler_ret_type main()
	{
		srv = nodecpp::net::createServer<net::ServerBase>();
		srvCtrl = nodecpp::net::createServer<net::ServerBase>();

		srv->on( event::close, [this](bool hadError) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onCloseServer()!");
		});
		srv->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {
#ifdef AUTOMATED_TESTING_ONLY
			nodecpp::setTimeout(  [this, socket]() { 
				socket->end();
				socket->unref();
				}, 3000 );
#endif
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnection()!");
			//srv->unref();
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
			socket->on( event::close, [this, socket](bool hadError) {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
				socket->unref();
			});

			socket->on( event::data, [this, socket](const Buffer& buffer) {
				if ( buffer.size() < 2 )
				{
					//log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = %d", *extra );
					socket->end();
					return;
				}
	
				size_t receivedSz = buffer.readUInt8(0);
				if ( receivedSz != buffer.size() )
				{
//					log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes", *extra, receivedSz, buffer.size() );
					log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = [??]: received %zd, expected: %zd bytes", receivedSz, buffer.size() );
					socket->unref();
					return;
				}
	
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(requestedSz);
					for ( size_t i=0; i<(uint8_t)requestedSz; ++i )
						reply.appendUint8( 0 );
					socket->write(reply);
				}
	
				stats.recvSize += receivedSz;
				stats.sentSize += requestedSz;
				++(stats.rqCnt);
			});
			socket->on( event::end, [this, socket]() {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
				Buffer b;
				b.appendString( "goodbye!", sizeof( "goodbye!" ) );
				socket->write( b );
				socket->end();
			});

		});

		srvCtrl->on( event::close, [this](bool hadError) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onCloseServerCtrl()!");
		});
		srvCtrl->on( event::connection, [this](soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionCtrl()!");
			NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
#ifdef AUTOMATED_TESTING_ONLY
			nodecpp::setTimeout(  [this, socket]() { 
				socket->end();
				socket->unref();
				}, 3000 );
#endif
			socket->on( event::close, [this, socket](bool hadError) {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onCloseServerSocket!");
			});
			socket->on( event::data, [this, socket](const Buffer& buffer) {
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(sizeof(stats));
					stats.connCnt = srv->getSockCount();
					size_t replySz = sizeof(Stats);
					reply.append( &stats, replySz );
					socket->write(reply);
				}
			});
			socket->on( event::end, [this, socket]() {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onEnd!");
				Buffer b;
				b.appendString( "goodbye!", sizeof( "goodbye!" ) );
				socket->write( b );
			});
		});

		srv->listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl->listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});

#ifdef AUTOMATED_TESTING_ONLY
		nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
		}, 3000 );
#endif

		CO_RETURN;
	}

	nodecpp::owning_ptr<nodecpp::net::ServerBase> srv;
	nodecpp::owning_ptr<nodecpp::net::ServerBase> srvCtrl;

	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	using EmitterType = nodecpp::net::SocketTEmitter<>;

#else
#error
#endif // IMPL_VERSION

	nodecpp::handler_ret_type onDataServerSocket_(nodecpp::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
//			log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Insufficient data: received {} bytes", buffer.size() );
			socket->unref();
			CO_RETURN;
		}
//		log::default_log::info( log::ModuleID(nodecpp_module_id), "server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
//			log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			CO_RETURN;
		}

		uint32_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			reply.set_size(requestedSz);
			socket->write(reply);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
		/*if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "About to exit successfully in automated testing (by count)" );
			socket->end();
			socket->unref();
		}*/
#endif
		CO_RETURN;
	}
};

#endif // NET_SOCKET_H
