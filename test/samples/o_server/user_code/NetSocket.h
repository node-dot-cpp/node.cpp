// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


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
	MySampleTNode()/* : srv( this ), srvCtrl( this )*/
	{
		srv = nodecpp::safememory::make_owning<ServerType>(this);
		srvCtrl = nodecpp::safememory::make_owning<CtrlServerType>(this);
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

#ifndef NET_CLIENT_ONLY
		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF
	}

	// server socket
	void onCloseServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, bool hadError)
	{
		print("server socket: onCloseServerSocket!\n");
	}
	void onConnectServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		print("server socket: onConnect!\n");
	}
	void onDataServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
			printf( "Insufficient data on socket idx = %d\n", *(socket->getExtra()) );
			socket->unref();
			return;
		}
//		print("server socket: onData for idx %d !\n", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
			printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", *(socket->getExtra()), receivedSz, buffer.size() );
			socket->unref();
			return;
		}

		size_t requestedSz = buffer.begin()[1];
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
	}
	void onDrainServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		print("server socket: onDrain!\n");
	}
	void onEndServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		print("server socket: onEnd!\n");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
		srv->removeSocket( socket );
	}
	void onErrorServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Error&) {
		print("server socket: onError!\n");
	}
	void onAcceptedServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		print("server socket: onAccepted!\n");
	}

	using SockTypeServerSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleTNode::onConnectServerSocket>,
		nodecpp::net::OnClose<&MySampleTNode::onCloseServerSocket>,
		nodecpp::net::OnData<&MySampleTNode::onDataServerSocket>,
		nodecpp::net::OnDrain<&MySampleTNode::onDrainServerSocket>,
		nodecpp::net::OnError<&MySampleTNode::onErrorServerSocket>,
		nodecpp::net::OnEnd<&MySampleTNode::onEndServerSocket>,
		nodecpp::net::OnAccepted<&MySampleTNode::onAcceptedServerSocket>
	>;

	void onCloseCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, bool hadError)
	{
		print("server socket: onCloseServerSocket!\n");
	}
	void onDataCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			size_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
	}
	void onEndCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		print("server socket: onEnd!\n");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
		srvCtrl->removeSocket( socket );
	}
	using SockTypeServerCtrlSocket = nodecpp::net::SocketN<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleTNode::onConnectServerSocket>,
		nodecpp::net::OnClose<&MySampleTNode::onCloseCtrlServerSocket>,
		nodecpp::net::OnData<&MySampleTNode::onDataCtrlServerSocket>,
		nodecpp::net::OnDrain<&MySampleTNode::onDrainServerSocket>,
		nodecpp::net::OnError<&MySampleTNode::onErrorServerSocket>,
		nodecpp::net::OnEnd<&MySampleTNode::onEndCtrlServerSocket>,
		nodecpp::net::OnAccepted<&MySampleTNode::onAcceptedServerSocket>
	>;

	// server
public:
	void onCloseServer(const ServerIdType* extra, bool hadError) {
		print("server: onCloseServer()!\n");
	}
	void onConnectionx(const ServerIdType* extra, net::SocketBase* socket) { 
		print("server: onConnection()!\n");
		//srv->unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
	}
	void onListeningx(const ServerIdType* extra, size_t id, nodecpp::net::Address addr) {print("server: onListening()!\n");}
	void onErrorServer(const ServerIdType* extra, Error& err) {print("server: onErrorServer!\n");}

	using ServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionx>,
		nodecpp::net::OnCloseSO<&MySampleTNode::onCloseServer>,
		nodecpp::net::OnListeningSO<&MySampleTNode::onListeningx>,
		nodecpp::net::OnErrorSO<&MySampleTNode::onErrorServer>
	>;
	nodecpp::safememory::owning_ptr<ServerType> srv;

	// ctrl server
public:
	void onCloseServerCtrl(const ServerIdType* extra, bool hadError) {
		print("server: onCloseServerCtrl()!\n");
	}
	void onConnectionCtrl(const ServerIdType* extra, net::SocketBase* socket) { 
		print("server: onConnectionCtrl()!\n");
		//srv->unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
	}
	void onListeningCtrl(const ServerIdType* extra, size_t id, nodecpp::net::Address addr) {print("server: onListeninCtrlg()!\n");}
	void onErrorServerCtrl(const ServerIdType* extra, Error& err) {print("server: onErrorServerCtrl!\n");}

	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionCtrl>,
		nodecpp::net::OnCloseSO<&MySampleTNode::onCloseServerCtrl>,
		nodecpp::net::OnListeningSO<&MySampleTNode::onListeningCtrl>,
		nodecpp::net::OnErrorSO<&MySampleTNode::onErrorServerCtrl>
	>;
	nodecpp::safememory::owning_ptr<CtrlServerType> srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
