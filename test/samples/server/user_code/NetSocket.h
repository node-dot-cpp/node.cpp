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

public:
	MySampleTNode() : srv( this ), srvCtrl( this )
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

#ifndef NET_CLIENT_ONLY
		srv.listen(2000, "127.0.0.1", 5);
		srvCtrl.listen(2001, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF
	}

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
				*(reinterpret_cast<Socket*>(sock)->getExtra()) = toUse.idx;
			}
			else
			{
				size_t idx = serverSocks.size();
				serverSocks.emplace_back( sock, idx );
				*(reinterpret_cast<Socket*>(sock)->getExtra()) = idx;
			}
			++serverSockCount;
		}
		void remove( size_t idx )
		{
			NODECPP_ASSERT( idx < serverSocks.size() );
			NODECPP_ASSERT( *(at(idx)->getExtra()) == idx );
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
	ServerSockets<SockTypeServerSocket> serverSockets;

public:
	void onCloseServer(const ServerIdType* extra, bool hadError) {
		print("server: onCloseServer()!\n");
		serverSockets.clear();
	}
	void onConnection(const ServerIdType* extra, net::SocketTBase* socket) { 
		print("server: onConnection()!\n");
		//srv.unref();
		NODECPP_ASSERT( socket != nullptr ); 
		serverSockets.add( socket );
	}
	void onListening(const ServerIdType* extra) {print("server: onListening()!\n");}
	void onErrorServer(const ServerIdType* extra, Error& err) {print("server: onErrorServer!\n");}

	using ServerType = nodecpp::net::ServerT<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionST<&MySampleTNode::onConnection>,
		nodecpp::net::OnCloseST<&MySampleTNode::onCloseServer>,
		nodecpp::net::OnListeningST<&MySampleTNode::onListening>,
		nodecpp::net::OnErrorST<&MySampleTNode::onErrorServer>
	>;
	ServerType srv;

	// ctrl server
private:
	ServerSockets<SockTypeServerCtrlSocket> serverCtrlSockets;

public:
	void onCloseServerCtrl(const ServerIdType* extra, bool hadError) {
		print("server: onCloseServerCtrl()!\n");
		serverCtrlSockets.clear();
	}
	void onConnectionCtrl(const ServerIdType* extra, net::SocketTBase* socket) { 
		print("server: onConnectionCtrl()!\n");
		//srv.unref();
		NODECPP_ASSERT( socket != nullptr ); 
		serverCtrlSockets.add( socket );
	}
	void onListeningCtrl(const ServerIdType* extra) {print("server: onListeninCtrlg()!\n");}
	void onErrorServerCtrl(const ServerIdType* extra, Error& err) {print("server: onErrorServerCtrl!\n");}

	using CtrlServerType = nodecpp::net::ServerT<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType,
		nodecpp::net::OnConnectionST<&MySampleTNode::onConnectionCtrl>,
		nodecpp::net::OnCloseST<&MySampleTNode::onCloseServerCtrl>,
		nodecpp::net::OnListeningST<&MySampleTNode::onListeningCtrl>,
		nodecpp::net::OnErrorST<&MySampleTNode::onErrorServerCtrl>
	>;
	CtrlServerType srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, SockTypeServerSocket, SockTypeServerCtrlSocket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server, ServerType, CtrlServerType>;
};

#endif // NET_SOCKET_H
