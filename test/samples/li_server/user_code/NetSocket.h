// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include <fmt/format.h>
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"

#include <functional>


using namespace std;
using namespace ::nodecpp;
using namespace fmt;

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
	bool letOnDrain = false;

	class MyServerSocketListener : public SocketListener
	{
		MySampleTNode* myNode;
		soft_ptr<net::Socket> mySocket;
		int id;
	public:
		MyServerSocketListener(MySampleTNode* node, soft_ptr<net::Socket> mySocket_, int id_) : myNode(node), mySocket( mySocket_ ), id(id_) {}
		void onClose(bool hadError) override
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
			myNode->srv->removeSocket(mySocket);
		}
		void onData(Buffer& buffer) override {
			if ( buffer.size() < 2 )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", id );
				mySocket->unref();
				return;
			}
	//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *extra );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", id, receivedSz, buffer.size() );
				mySocket->unref();
				return;
			}

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(requestedSz);
				//buffer.begin()[0] = (uint8_t)requestedSz;
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				mySocket->write(reply.begin(), requestedSz);
			}

			myNode->stats.recvSize += receivedSz;
			myNode->stats.sentSize += requestedSz;
			++(myNode->stats.rqCnt);
		}
		void onEnd() override {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
			const char buff[] = "goodbye!";
			mySocket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			mySocket->end();
		}
	};

	class MyServerCtrlSocketListener : public SocketListener
	{
		MySampleTNode* myNode;
		soft_ptr<net::Socket> mySocket;
		int id;
	public:
		MyServerCtrlSocketListener(MySampleTNode* node, soft_ptr<net::Socket> mySocket_, int id_) : myNode(node), mySocket( mySocket_ ), id(id_) {}
		void onClose(bool hadError) override
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
			myNode->srvCtrl->removeSocket(mySocket);
		}
		void onData(Buffer& buffer) override {
			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				myNode->stats.connCnt = myNode->srv->getSockCount();
				size_t replySz = sizeof(Stats);
				uint8_t* buff = myNode->ptr.get();
				memcpy( buff, &(myNode->stats), replySz); // naive marshalling will work for a limited number of cases
				mySocket->write(buff, replySz);
			}
		}
		void onEnd() override {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
			const char buff[] = "goodbye!";
			mySocket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			mySocket->end();
		}
	};

	class MyServerListener : public ServerListener
	{
		MySampleTNode* myNode;
		int id;
		int sockIDBase = 0;
	public:
		MyServerListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(/*const ServerIdType* extra,*/ bool hadError) override {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServer()!");
		}
		void onConnection(/*const ServerIdType* extra,*/ soft_ptr<net::SocketBase> socket) override { 
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
			//srv->unref();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket ); 
			//net::Socket* s = static_cast<net::Socket*>( socket );
			soft_ptr<net::Socket> s = soft_ptr_static_cast<net::Socket>( socket );
			nodecpp::safememory::owning_ptr<MyServerSocketListener> l = nodecpp::safememory::make_owning< MyServerSocketListener >( myNode, s, sockIDBase++ );
			s->on( std::move(l) );
		}
	};
	//MyServerListener myServerListener;

	class MyCtrlServerListener : public ServerListener
	{
		MySampleTNode* myNode;
		int id;
		int sockIDBase = 0;
	public:
		MyCtrlServerListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(/*const ServerIdType* extra,*/ bool hadError) override {
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServerCtrl()!");
		}
		void onConnection(/*const ServerIdType* extra,*/ soft_ptr<net::SocketBase> socket) override { 
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
			//srv->unref();
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket ); 
			//net::Socket* s = static_cast<net::Socket*>( socket );
			soft_ptr<net::Socket> s = soft_ptr_static_cast<net::Socket>( socket );
			nodecpp::safememory::owning_ptr<SocketListener> l = nodecpp::safememory::make_owning<MyServerCtrlSocketListener>( myNode, s, sockIDBase++ );
			s->on( std::move(l) );
//			myNode->serverCtrlSockets.add( socket );
		}
	};

public:
	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::awaitable<void> main()
	{
		srv = nodecpp::safememory::make_owning<net::Server>();
		srvCtrl = nodecpp::safememory::make_owning<net::Server>();

		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		nodecpp::safememory::owning_ptr<ServerListener> svrListener = nodecpp::safememory::make_owning<MyServerListener>( this, 0 );
		srv->on( svrListener );
		nodecpp::safememory::owning_ptr<ServerListener> ctrlSvrListener = nodecpp::safememory::make_owning<MyCtrlServerListener>( this, 1 );
		srvCtrl->on( ctrlSvrListener );

		srv->listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl->listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});
		
		co_return;
	}

	nodecpp::safememory::owning_ptr<net::Server> srv;
	nodecpp::safememory::owning_ptr<net::Server> srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
