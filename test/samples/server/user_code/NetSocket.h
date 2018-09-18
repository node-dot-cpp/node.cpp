// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
//#include "../../../../include/nodecpp/net.h"
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
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;
	bool letOnDrain = false;

	using SocketIdType = int;
	using ServerIdType = int;

public:
	MySampleTNode() : srv( this )
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

#ifndef NET_CLIENT_ONLY
		srv.listen(2000, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF
	}

	// server socket
	void onCloseSererSocket(const SocketIdType* extra, bool hadError) {print("server socket: onCloseSererSocket!\n");};
	void onConnectSererSocket(const SocketIdType* extra) {
		print("server socket: onConnect!\n");
	}
	void onDataSererSocket(const SocketIdType* extra, Buffer& buffer) {
		print("server socket: onData!\n");
		++serverSockCount;
		assert( serversSocks.size() );
		// todo: find a right sock using extra
		serversSocks[0]->write(buffer.begin(), buffer.size());
	}
	void onDrainSererSocket(const SocketIdType* extra) {
		print("server socket: onDrain!\n");
	}
	void onEndSererSocket(const SocketIdType* extra) {
		print("server socket: onEnd!\n");
		const char buff[] = "goodbye!";
		// todo: find a right sock using extra
		serversSocks[0]->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		serversSocks[0]->end();
	}
	void onErrorSererSocket(const SocketIdType* extra, nodecpp::Error&) {
		print("server socket: onError!\n");
	}
	void onAcceptedSererSocket(const SocketIdType* extra) {
		print("server socket: onAccepted!\n");
	}

	// server
private:
	std::vector<net::SocketTBase*> serversSocks;
	size_t serverSockCount = 0;

public:
	void onCloseServer(const ServerIdType* extra, bool hadError) {
		print("server: onCloseServer()!\n");
		for ( size_t i=0; i<serversSocks.size(); ++i )
		{
			NODECPP_ASSERT( serversSocks[i] != nullptr ); 
			delete serversSocks[i];
		}
		serversSocks.clear();
	}
//	void onConnection(SockTypeServerSocket* socket) { NODECPP_ASSERT( socket != nullptr ); *(socket->getExtra()) = 1;}
	void onConnection(const ServerIdType* extra, net::SocketTBase* socket) { 
		print("server: onConnection()!\n");
		srv.unref();
		NODECPP_ASSERT( socket != nullptr ); 
		serversSocks.push_back( socket );
		/**(socket->getExtra()) = 1;*/
	}
	void onListening(const ServerIdType* extra) {print("server: onListening()!\n");}
	void onErrorServer(const ServerIdType* extra, Error& err) {print("server: onErrorServer!\n");}

	using SockTypeServerSocket = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onConnectSererSocket>,
		nodecpp::net::OnCloseT<&MySampleTNode::onCloseSererSocket>,
		nodecpp::net::OnDataT<&MySampleTNode::onDataSererSocket>,
		nodecpp::net::OnDrainT<&MySampleTNode::onDrainSererSocket>,
		nodecpp::net::OnErrorT<&MySampleTNode::onErrorSererSocket>,
		nodecpp::net::OnEndT<&MySampleTNode::onEndSererSocket>,
		nodecpp::net::OnAcceptedT<&MySampleTNode::onAcceptedSererSocket>
	>;
	using ServerType = nodecpp::net::ServerT<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionST<&MySampleTNode::onConnection>,
		nodecpp::net::OnCloseST<&MySampleTNode::onCloseServer>,
		nodecpp::net::OnListeningST<&MySampleTNode::onListening>,
		nodecpp::net::OnErrorST<&MySampleTNode::onErrorServer>
	>;
	ServerType srv;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, SockTypeServerSocket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server, ServerType>;
};

#endif // NET_SOCKET_H
