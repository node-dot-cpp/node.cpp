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

	using SocketIdType = int;
	using ServerIdType = int;

	class MyServerSocketListener : public SocketListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyServerSocketListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(bool hadError) override
		{
			print("server socket: onCloseServerSocket!\n");
			myNode->serverSockets.remove(id);
		}
		void onData(Buffer& buffer) override {
			if ( buffer.size() < 2 )
			{
				printf( "Insufficient data on socket idx = %d\n", id );
				myNode->serverSockets.at(id)->unref();
				return;
			}
	//		print("server socket: onData for idx %d !\n", *extra );

			size_t receivedSz = buffer.begin()[0];
			if ( receivedSz != buffer.size() )
			{
				printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", id, receivedSz, buffer.size() );
				myNode->serverSockets.at(id)->unref();
				return;
			}

			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(requestedSz);
				//buffer.begin()[0] = (uint8_t)requestedSz;
				memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
				myNode->serverSockets.at(id)->write(reply.begin(), requestedSz);
			}

			myNode->stats.recvSize += receivedSz;
			myNode->stats.sentSize += requestedSz;
			++(myNode->stats.rqCnt);
		}
		void onEnd() override {
			print("server socket: onEnd!\n");
			const char buff[] = "goodbye!";
			myNode->serverSockets.at(id)->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			myNode->serverSockets.at(id)->end();
		}
	};

	class MyServerCtrlSocketListener : public SocketListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyServerCtrlSocketListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(bool hadError) override
		{
			print("server socket: onCloseServerSocket!\n");
			myNode->serverCtrlSockets.remove(id);
		}
		void onData(Buffer& buffer) override {
			size_t requestedSz = buffer.begin()[1];
			if ( requestedSz )
			{
				Buffer reply(sizeof(stats));
				myNode->stats.connCnt = myNode->serverSockets.getServerSockCount();
				size_t replySz = sizeof(Stats);
				uint8_t* buff = myNode->ptr.get();
				memcpy( buff, &(myNode->stats), replySz); // naive marshalling will work for a limited number of cases
				myNode->serverCtrlSockets.at(id)->write(buff, replySz);
			}
		}
		void onEnd() override {
			print("server socket: onEnd!\n");
			const char buff[] = "goodbye!";
			myNode->serverCtrlSockets.at(id)->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
			myNode->serverCtrlSockets.at(id)->end();
		}
	};

	class MyServerListener : public ServerListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyServerListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(/*const ServerIdType* extra,*/ bool hadError) override {
			print("server: onCloseServer()!\n");
			myNode->serverSockets.clear();
		}
		void onConnection(/*const ServerIdType* extra,*/ net::SocketTBase* socket) override { 
			print("server: onConnection()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			myNode->serverSockets.add( socket );
		}
	};
	MyServerListener myServerListener;

	class MyCtrlServerListener : public ServerListener
	{
		MySampleTNode* myNode;
		int id;
	public:
		MyCtrlServerListener(MySampleTNode* node, int id_) : myNode(node), id(id_) {}
		void onClose(/*const ServerIdType* extra,*/ bool hadError) override {
			print("server: onCloseServerCtrl()!\n");
			myNode->serverCtrlSockets.clear();
		}
		void onConnection(/*const ServerIdType* extra,*/ net::SocketTBase* socket) override { 
			print("server: onConnectionCtrl()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			myNode->serverCtrlSockets.add( socket );
		}
	};
	MyCtrlServerListener myCtrlServerListener;

public:
	MySampleTNode() : myServerListener(this, 0), myCtrlServerListener(this, 1), srv( [this](OpaqueSocketData& sdata) { return makeSocket(sdata);} ), srvCtrl( [this](OpaqueSocketData& sdata) { return makeCtrlSocket(sdata);} )
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

#if 0
	// server socket
	void onCloseServerSocket(const SocketIdType* extra, bool hadError)
	{
		print("server socket: onCloseServerSocket!\n");
		serverSockets.remove(*extra);
	}
	void onConnectServerSocket(const SocketIdType* extra) {
		print("server socket: onConnect!\n");
	}
	void onDataServerSocket(const SocketIdType* extra, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
			printf( "Insufficient data on socket idx = %d\n", *extra );
			serverSockets.at(*extra)->unref();
			return;
		}
//		print("server socket: onData for idx %d !\n", *extra );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
			printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", *extra, receivedSz, buffer.size() );
			serverSockets.at(*extra)->unref();
			return;
		}

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			serverSockets.at(*extra)->write(reply.begin(), requestedSz);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
		++(stats.rqCnt);
	}
	void onDrainServerSocket(const SocketIdType* extra) {
		print("server socket: onDrain!\n");
	}
	void onEndServerSocket(const SocketIdType* extra) {
		print("server socket: onEnd!\n");
		const char buff[] = "goodbye!";
		serverSockets.at(*extra)->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		serverSockets.at(*extra)->end();
	}
	void onErrorServerSocket(const SocketIdType* extra, nodecpp::Error&) {
		print("server socket: onError!\n");
	}
	void onAcceptedServerSocket(const SocketIdType* extra) {
		print("server socket: onAccepted!\n");
	}

	using SockTypeServerSocket = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onConnectServerSocket>,
		nodecpp::net::OnCloseT<&MySampleTNode::onCloseServerSocket>,
		nodecpp::net::OnDataT<&MySampleTNode::onDataServerSocket>,
		nodecpp::net::OnDrainT<&MySampleTNode::onDrainServerSocket>,
		nodecpp::net::OnErrorT<&MySampleTNode::onErrorServerSocket>,
		nodecpp::net::OnEndT<&MySampleTNode::onEndServerSocket>,
		nodecpp::net::OnAcceptedT<&MySampleTNode::onAcceptedServerSocket>
	>;

	void onCloseCtrlServerSocket(const SocketIdType* extra, bool hadError)
	{
		print("server socket: onCloseServerSocket!\n");
		serverCtrlSockets.remove(*extra);
	}
	void onDataCtrlServerSocket(const SocketIdType* extra, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = serverSockets.getServerSockCount();
			size_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
			serverCtrlSockets.at(*extra)->write(buff, replySz);
		}
	}
	void onEndCtrlServerSocket(const SocketIdType* extra) {
		print("server socket: onEnd!\n");
		const char buff[] = "goodbye!";
		serverCtrlSockets.at(*extra)->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		serverCtrlSockets.at(*extra)->end();
	}
	using SockTypeServerCtrlSocket = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onConnectServerSocket>,
		nodecpp::net::OnCloseT<&MySampleTNode::onCloseCtrlServerSocket>,
		nodecpp::net::OnDataT<&MySampleTNode::onDataCtrlServerSocket>,
		nodecpp::net::OnDrainT<&MySampleTNode::onDrainServerSocket>,
		nodecpp::net::OnErrorT<&MySampleTNode::onErrorServerSocket>,
		nodecpp::net::OnEndT<&MySampleTNode::onEndCtrlServerSocket>,
		nodecpp::net::OnAcceptedT<&MySampleTNode::onAcceptedServerSocket>
	>;
#endif // 0

	// server
private:
	template<class Socket>
	class ServerSockets
	{
		struct ServerSock
		{
			size_t idx;
			size_t nextFree;
			std::unique_ptr<net::SocketTBase> socket;
			ServerSock( net::SocketTBase* sock, size_t idx_ ) : idx( idx_ ), nextFree( size_t(-1) ), socket( sock )  {}
		};
		size_t firstFree = size_t(-1);
		std::vector<ServerSock> serverSocks;
		size_t serverSockCount = 0;
	public:
		ServerSockets() {}
		void add( net::SocketTBase* sock )
		{
			if ( firstFree != size_t(-1) )
			{
				NODECPP_ASSERT( firstFree < serverSocks.size() );
				ServerSock& toUse = serverSocks[firstFree];
				NODECPP_ASSERT( firstFree == toUse.idx );
				firstFree = toUse.nextFree;
				toUse.socket.reset( sock );
//				*(reinterpret_cast<Socket*>(sock)->getExtra()) = toUse.idx;
			}
			else
			{
				size_t idx = serverSocks.size();
				serverSocks.emplace_back( sock, idx );
//				*(reinterpret_cast<Socket*>(sock)->getExtra()) = idx;
			}
			++serverSockCount;
		}
		void remove( size_t idx )
		{
			NODECPP_ASSERT( idx < serverSocks.size() );
//			NODECPP_ASSERT( *(at(idx)->getExtra()) == idx );
			ServerSock& toUse = serverSocks[idx];
			toUse.nextFree = firstFree;
			toUse.socket.reset();
			firstFree = idx;
			--serverSockCount;
		}
		Socket* at(size_t idx)
		{
			NODECPP_ASSERT( idx < serverSocks.size() );
			return reinterpret_cast<Socket*>(serverSocks[idx].socket.get());
		}
		void clear()
		{
			for ( size_t i=0; i<serverSocks.size(); ++i )
				serverSocks[i].socket.reset();
			serverSocks.clear();
		}
		size_t getServerSockCount() { return serverSockCount; }
	};
	ServerSockets<net::Socket> serverSockets;
	net::SocketTBase* makeSocket(OpaqueSocketData& sdata) { return new net::Socket( sdata ); }

public:

	net::Server srv;

	// ctrl server
private:
	ServerSockets<net::Socket> serverCtrlSockets;
	net::SocketTBase* makeCtrlSocket(OpaqueSocketData& sdata) { return new net::Socket( sdata ); }

public:
	net::Server srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
