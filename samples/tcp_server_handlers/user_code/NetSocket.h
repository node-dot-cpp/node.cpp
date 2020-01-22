// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

using namespace std;
using namespace nodecpp;
using namespace fmt;

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
	nodecpp::handler_ret_type onListening(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onListening2(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListening2()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
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
	nodecpp::handler_ret_type onListeningCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, size_t id, nodecpp::net::Address addr) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onListeninCtrlg()!");
		CO_RETURN;
	}
	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
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
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
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
		nodecpp::handler_ret_type onConnection(nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
			log::default_log::info( log::ModuleID(nodecpp_module_id), "MyServerSocketTwo::onConnection()!");
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
	nodecpp::handler_ret_type serverCtrlSocketLoop(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket)
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
	/*nodecpp::handler_ret_type onDataCtrlServerSocket_1(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

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

	nodecpp::handler_ret_type onDataServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
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