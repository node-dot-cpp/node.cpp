// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "../../../../include/nodecpp/common.h"

#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"


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
	MySampleTNode()
	{
		_srv = nodecpp::safememory::make_owning<ServerType>(this);
		_srvCtrl = nodecpp::safememory::make_owning<CtrlServerType>(this);
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual void main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

#ifndef NET_CLIENT_ONLY
		_srv->listen(2000, "127.0.0.1", 5);
		_srvCtrl->listen(2001, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF
	}

	// server socket
	void onCloseServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, bool hadError)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
		_srv->removeSocket( socket );
	}
	void onConnectServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onConnect!");
	}
	void onDataServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {
		if ( buffer.size() < 2 )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			socket->unref();
			return;
		}
		//nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

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
	void onDrainServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onDrain!");
	}
	void onEndServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
	}
	void onErrorServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Error&) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onError!");
	}
	void onAcceptedServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onAccepted!");
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

	void onCloseCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, bool hadError)
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onCloseServerSocket!");
		_srvCtrl->removeSocket( socket );
	}
	void onDataCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = _srv->getSockCount();
			size_t replySz = sizeof(Stats);
			uint8_t* buff = ptr.get();
			memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
			socket->write(buff, replySz);
		}
	}
	void onEndCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onEnd!");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
	}
	void onConnectCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("ctrl server socket: onConnect!");
	}
	void onDrainCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("ctrl server socket: onDrain!");
	}
	void onErrorCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket, nodecpp::Error&) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("ctrl server socket: onError!");
	}
	void onAcceptedCtrlServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketTUserBase<MySampleTNode,SocketIdType>> socket) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("ctrl server socket: onAccepted!");
	}
	using SockTypeServerCtrlSocket = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onConnectCtrlServerSocket>,
		nodecpp::net::OnCloseT<&MySampleTNode::onCloseCtrlServerSocket>,
		nodecpp::net::OnDataT<&MySampleTNode::onDataCtrlServerSocket>,
		nodecpp::net::OnDrainT<&MySampleTNode::onDrainCtrlServerSocket>,
		nodecpp::net::OnErrorT<&MySampleTNode::onErrorCtrlServerSocket>,
		nodecpp::net::OnEndT<&MySampleTNode::onEndCtrlServerSocket>,
		nodecpp::net::OnAcceptedT<&MySampleTNode::onAcceptedCtrlServerSocket>
	>;

	// server
public:
	void onCloseServer(const ServerIdType* extra, bool hadError) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServer()!");
	}
	void onConnectionServer(const ServerIdType* extra, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningServer(const ServerIdType* extra) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListening()!");}
	void onErrorServer(const ServerIdType* extra, Error& err) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onErrorServer!");}

	using ServerType = nodecpp::net::ServerT<MySampleTNode,SockTypeServerSocket,ServerIdType,
		nodecpp::net::OnConnectionST<&MySampleTNode::onConnectionServer>,
		nodecpp::net::OnCloseST<&MySampleTNode::onCloseServer>,
		nodecpp::net::OnListeningST<&MySampleTNode::onListeningServer>,
		nodecpp::net::OnErrorST<&MySampleTNode::onErrorServer>
	>;
	nodecpp::safememory::owning_ptr<ServerType> _srv;

public:
	void onCloseServerCtrl(const ServerIdType* extra, bool hadError) {
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onCloseServerCtrl()!");
		//serverCtrlSockets.clear();
	}
	void onConnectionCtrl(const ServerIdType* extra, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, nodecpp::assert::AssertLevel::critical, socket ); 
	}
	void onListeningCtrl(const ServerIdType* extra) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onListeninCtrlg()!");}
	void onErrorServerCtrl(const ServerIdType* extra, Error& err) {nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onErrorServerCtrl!");}

	using CtrlServerType = nodecpp::net::ServerT<MySampleTNode,SockTypeServerCtrlSocket,ServerIdType,
		nodecpp::net::OnConnectionST<&MySampleTNode::onConnectionCtrl>,
		nodecpp::net::OnCloseST<&MySampleTNode::onCloseServerCtrl>,
		nodecpp::net::OnListeningST<&MySampleTNode::onListeningCtrl>,
		nodecpp::net::OnErrorST<&MySampleTNode::onErrorServerCtrl>
	>;
	nodecpp::safememory::owning_ptr<CtrlServerType> _srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, SockTypeServerSocket, SockTypeServerCtrlSocket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server, ServerType, CtrlServerType>;
};

#endif // NET_SOCKET_H
