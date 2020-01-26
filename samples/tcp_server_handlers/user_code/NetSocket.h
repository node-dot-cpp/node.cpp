// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_common.h>
#include <nodecpp/server_common.h>

using namespace nodecpp;

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

	class MyWorkingSocket : public nodecpp::net::SocketBase
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	private:
		MySampleTNode* myNode = nullptr;

	public:
		MyWorkingSocket() {}
		virtual ~MyWorkingSocket() {}
	};

	class MyStatSocket : public nodecpp::net::SocketBase
	{
	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	private:
		MySampleTNode* myNode = nullptr;

	public:
		MyStatSocket() {}
		virtual ~MyStatSocket() {}
	};

	class MyWorkingServer : public nodecpp::net::ServerBase
	{
	public:
		MyWorkingServer() {}
//		MyWorkingServer(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyWorkingServer() {}
	};

	class MyStatServer : public nodecpp::net::ServerBase
	{
	public:
		MyStatServer() {}
//		MyStatServer(acceptedSocketCreationRoutineType socketCreationCB) : MyServerSocketBase(socketCreationCB) {};
		virtual ~MyStatServer() {}
	};

public:
	MySampleTNode() {}

	virtual nodecpp::handler_ret_type main()
	{
		// registering handlers
		nodecpp::net::ServerBase::addHandler<MyWorkingServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionWorking>(this);
		nodecpp::net::ServerBase::addHandler<MyStatServer, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionStat>(this);

		nodecpp::net::SocketBase::addHandler<MyWorkingSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onWorkingData>(this);
		nodecpp::net::SocketBase::addHandler<MyStatSocket, nodecpp::net::SocketBase::DataForCommandProcessing::UserHandlers::Handler::Data, &MySampleTNode::onStatData>(this);

		srv = nodecpp::net::createServer<MyWorkingServer, MyWorkingSocket>();
		srvCtrl = nodecpp::net::createServer<MyStatServer, MyStatSocket>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

		CO_RETURN;
	}

public:
	// server
	nodecpp::handler_ret_type onConnectionWorking(nodecpp::safememory::soft_ptr<MyWorkingServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionWorking()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);

		CO_RETURN;
	}

	nodecpp::handler_ret_type onWorkingData(nodecpp::safememory::soft_ptr<MyWorkingSocket> socket, Buffer& buffer)
	{
		size_t receivedSz = buffer.begin()[0];
		if (receivedSz != buffer.size())
		{
			log::default_log::info( log::ModuleID(nodecpp_module_id), "Corrupted data on socket: received {}, expected: {} bytes", receivedSz, buffer.size());
			socket->unref();
			CO_RETURN;
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
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed. Exiting...");
				CO_RETURN;
			}
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);

		CO_RETURN;
	}

	// ctrl server
	nodecpp::handler_ret_type onConnectionStat(nodecpp::safememory::soft_ptr<MyStatServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) {
		log::default_log::info( log::ModuleID(nodecpp_module_id), "server: onConnectionStat()!");
		NODECPP_ASSERT(nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr);

		CO_RETURN;
	}

	nodecpp::handler_ret_type onStatData(nodecpp::safememory::soft_ptr<MyStatSocket> socket, Buffer& buffer)
	{
		Buffer reply(sizeof(stats));

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
				log::default_log::error( log::ModuleID(nodecpp_module_id), "Writing data failed). Exiting..." );
			}
		}
	}

	nodecpp::safememory::owning_ptr<MyWorkingServer> srv;
	nodecpp::safememory::owning_ptr<MyStatServer> srvCtrl;
};

#endif // NET_SOCKET_H
