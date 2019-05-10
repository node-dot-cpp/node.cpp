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
	MySampleTNode() : srv( this ), srvCtrl( this )
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::awaitable<void> main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		srv.listen(2000, "127.0.0.1", 5);
		srvCtrl.listen(2001, "127.0.0.1", 5);

		co_return;
	}

	// server socket
	void onCloseServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, bool hadError)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
		srv.removeSocket( socket );
	}
	void onConnectServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onConnect!");
	}
	void onDataServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			socket->unref();
			return;
		}
//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
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
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onDrain!");
	}
	void onEndServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
	}
	void onErrorServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Error&) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onError!");
	}
	void onAcceptedServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onAccepted!");
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
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
		srvCtrl.removeSocket( socket );
	}
	void onDataCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv.getSockCount();
			size_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
	}
	void onEndCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketOUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
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
	void onCloseServer(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, bool hadError) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServer()!");
	}
	void onConnectionx(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
	}
	void onListeningx(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, size_t id, nodecpp::net::Address addr) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");}
	void onErrorServer(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, Error& err) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onErrorServer!");}

	using ServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionx>,
		nodecpp::net::OnCloseSO<&MySampleTNode::onCloseServer>,
		nodecpp::net::OnListeningSO<&MySampleTNode::onListeningx>,
		nodecpp::net::OnErrorSO<&MySampleTNode::onErrorServer>
	>;
	ServerType srv;

	// ctrl server
public:
	void onCloseServerCtrl(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, bool hadError) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServerCtrl()!");
	}
	void onConnectionCtrl(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
	}
	void onListeningCtrl(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, size_t id, nodecpp::net::Address addr) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");}
	void onErrorServerCtrl(nodecpp::safememory::soft_ptr<nodecpp::net::ServerOUserBase<MySampleTNode,ServerIdType>>, Error& err) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onErrorServerCtrl!");}

	using CtrlServerType = nodecpp::net::ServerN<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType,
		nodecpp::net::OnConnectionSO<&MySampleTNode::onConnectionCtrl>,
		nodecpp::net::OnCloseSO<&MySampleTNode::onCloseServerCtrl>,
		nodecpp::net::OnListeningSO<&MySampleTNode::onListeningCtrl>,
		nodecpp::net::OnErrorSO<&MySampleTNode::onErrorServerCtrl>
	>;
	CtrlServerType srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
