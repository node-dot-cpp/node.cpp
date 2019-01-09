// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"

#include <functional>


using namespace std;
using namespace nodecpp;
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
			print("server socket: onCloseServerSocket!\n");
			myNode->srv.removeSocket(mySocket);
		}
		void onData(Buffer& buffer) override {
			if ( buffer.size() < 2 )
			{
				printf( "Insufficient data on socket idx = %d\n", id );
				mySocket->unref();
				return;
			}
	//		print("server socket: onData for idx %d !\n", *extra );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
				printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", id, receivedSz, buffer.size() );
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
			print("server socket: onEnd!\n");
			const char buff[] = "goodbye!";
			mySocket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			mySocket->end();
		}
	};

	class MyServerCtrlSocketListener : public SocketListener
	{
		MySampleTNode* myNode;
		net::Socket* mySocket;
		int id;
	public:
		MyServerCtrlSocketListener(MySampleTNode* node, net::Socket* mySocket_, int id_) : myNode(node), mySocket( mySocket_ ), id(id_) {}
		void onClose(bool hadError) override
		{
			print("server socket: onCloseServerSocket!\n");
			myNode->srvCtrl.removeSocket(mySocket);
		}
		void onData(Buffer& buffer) override {
			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				myNode->stats.connCnt = myNode->srv.getSockCount();
				size_t replySz = sizeof(Stats);
				uint8_t* buff = myNode->ptr.get();
				memcpy( buff, &(myNode->stats), replySz); // naive marshalling will work for a limited number of cases
				mySocket->write(buff, replySz);
			}
		}
		void onEnd() override {
			print("server socket: onEnd!\n");
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
			print("server: onCloseServer()!\n");
		}
		void onConnection(/*const ServerIdType* extra,*/ net::SocketBase* socket) override { 
			print("server: onConnection()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			net::Socket* s = static_cast<net::Socket*>( socket );
			std::unique_ptr<SocketListener> l( new MyServerCtrlSocketListener( myNode, s, sockIDBase++ ) );
			s->on( l );
		}
	};
	MyServerListener myServerListener;

	class MyCtrlServerListener : public ServerListener
	{
		MySampleTNode* myNode;
		int id;
		int sockIDBase = 0;
	public:
		MyCtrlServerListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(/*const ServerIdType* extra,*/ bool hadError) override {
			print("server: onCloseServerCtrl()!\n");
		}
		void onConnection(/*const ServerIdType* extra,*/ net::SocketBase* socket) override { 
			print("server: onConnectionCtrl()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			net::Socket* s = static_cast<net::Socket*>( socket );
			std::unique_ptr<SocketListener> l( new MyServerSocketListener( myNode, s, sockIDBase++ ) );
			s->on( l );
//			myNode->serverCtrlSockets.add( socket );
		}
	};
	MyCtrlServerListener myCtrlServerListener;

public:
	MySampleTNode() : myServerListener(this, 0), myCtrlServerListener(this, 1)
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		srv.on( &myCtrlServerListener );
		srvCtrl.on( &myCtrlServerListener );

		srv.listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl.listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});
	}

	net::Server srv;
	net::Server srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
