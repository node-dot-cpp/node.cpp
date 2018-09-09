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

class MyServer;

#if 0
class MySocket :public net::Socket {
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;

public:
	MySocket() {}

	void onClose(bool hadError) override {
		print("onClose!\n");
	}

	void onConnect() override {
		print("onConnect!\n");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
			ok = write(ptr.get(), size);
			sentSize += size;
		}
	}

	void onData(Buffer& buffer) override {
		print("onData!\n");
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			end();
	}

	void onDrain() override {
		print("onDrain!\n");
	}

	void onEnd() override {
		print("onEnd!\n");
	}

	void onError(Error&) override {
		print("onError!\n");
	}
};
#endif

#if 1//def USING_L_SOCKETS
class MySocketLambda :public net::Socket {
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;

public:
	MySocketLambda() {
		//preset handlers
//		on(event::close, std::bind(&MySocketLambda::didClose, this, std::placeholders::_1));
//		on(event::close, std::bind(&MySocketLambda::didClose, this, std::placeholders::_1));
		on(event::close, [this](bool b) { this->didClose(b); });

//		on<event::Connect>(std::bind(&MySocketLambda::didConnect, this));
//		on<event::Data>(std::bind(&MySocket2::didData, this, std::placeholders::_1));
//		on<event::Drain>(std::bind(&MySocketLambda::didDrain, this));
//		on(event::end, std::bind(&MySocketLambda::didEnd, this));
		on(event::end, [this]() { this->didEnd(); });
//		on(event::error, std::bind(&MySocketLambda::didError, this));
		on(event::error, [this](Error&) { this->didError(); });
	}

	void didClose(bool hadError) {
		print("onClose!\n");
	}

	void didConnect() {
		print("onConnect!\n");
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			sentSize += size;
		}
	}

	void didData(Buffer& buffer) {
		print("onData!\n");
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			end();
	}

	//void didDrain() {
	//	print("onDrain!\n");
	//}

	void didEnd() {
		print("onEnd!\n");
	}

	void didError() {
		print("onError!\n");
	}
};
#endif // USING_L_SOCKETS

/*
class MyServerSocket :public net::Socket {
	size_t count = 0;
	MyServer* server = nullptr;

public:
	MyServerSocket(MyServer* server) :server(server) {}

	void onClose(bool hadError) override;

	void onConnect() override {
		print("onConnect!\n");
	}

	void onData(Buffer& buffer) override {
		print("onData!\n");
		++count;
		write(buffer.begin(), buffer.size());
	}

	void onDrain() override {
		print("onDrain!\n");
	}

	void onEnd() override {
		print("onEnd!\n");
		const char buff[] = "goodbye!";
		write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		end();
	}

	void onError() override {
		print("onError!\n");
	}
};



class MyServer :public net::Server {
	list<unique_ptr<net::Socket>> socks;
public:


	void onClose(bool hadError) override {
		print("onClose!\n");
	}
	void onConnection(net::Socket* socket) override {
		print("onConnection!\n");
		unref();
		socks.emplace_back(socket);
	}
	void onListening() override {
		print("onListening!\n");
	}

	void onError() override {
		print("onError!\n");
	}
	MyServerSocket* makeSocket() override {
		return new MyServerSocket(this);
	}

	void closeMe(MyServerSocket* ptr) {
		for (auto it = socks.begin(); it != socks.end(); ++it) {
			if (it->get() == ptr) {
				socks.erase(it);
				return;
			}
		}
	}

};

inline
void MyServerSocket::onClose(bool hadError) {
	print("onClose!\n");
	if (server)
		server->closeMe(this);
}
*/

#ifndef NET_CLIENT_ONLY
#ifdef USING_L_SOCKETS
class MyServerMember;

class MyServerSocketMember {
	unique_ptr<net::Socket> socket;
	size_t count = 0;
	MyServerMember* server = nullptr;

public:
	MyServerSocketMember(net::Socket* socket, MyServerMember* server) 
		:socket(socket), server(server)
	{
		socket->on(event::close, [this](bool b) { this->didClose(b); });
		socket->on(event::data, [this](Buffer& buffer) { this->didData(buffer); });
		socket->on(event::end, [this]() { this->didEnd(); });
		socket->on(event::error, [this](Error&) { this->didError(); });
	}

	void didClose(bool hadError);

	void didData(Buffer& buffer) {
		print("onData!\n");
		++count;
		socket->write(buffer.begin(), buffer.size());
	}

	void didEnd() {
		print("onEnd!\n");
		const char buff[] = "goodbye!";
		socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
		socket->end();
	}

	void didError() {
		print("onError!\n");
	}
};

class MyServerMember 
{
	unique_ptr<net::Server> server;
	list<unique_ptr<MyServerSocketMember>> socks;
public:

	MyServerMember() :server(new net::Server()) {}

	void listen(uint16_t port, const char* ip, int backlog) {
		server->on(event::error, [](Error&) { print("onError!\n"); });
		server->on(event::close, [](bool b) { print("onClose!\n"); });
		server->on(event::connection, [this](net::Socket* socket) {
			print("onConnection!\n");
			this->server->unref();
			socks.emplace_back(new MyServerSocketMember(socket, this));
		});
		server->listen(port, ip, backlog, []() { print("onListening!\n"); });
	}
	//MyServerSocket* makeSocket() override {
	//	return new MyServerSocket(server.get());
	//}

	void closeMe(MyServerSocketMember* ptr) {
		for (auto it = socks.begin(); it != socks.end(); ++it) {
			if (it->get() == ptr) {
				socks.erase(it);
				return;
			}
		}
	}
};
#endif // NO_SERVER_STAFF
/*
inline
void MyServerSocketMember::didClose(bool hadError) {
	print("onClose!\n");
	if (server)
		server->closeMe(this);
}*/
#endif // USING_L_SOCKETS

#ifdef USING_L_SOCKETS
class MySampleLambdaOneNode : public NodeBase
{
#ifndef NET_CLIENT_ONLY
	MyServerMember srv;
#endif // NO_SERVER_STAFF
//	unique_ptr<MySocketLambda> cli;
	MySocketLambda* cli;
public:
	MySampleLambdaOneNode() : cli( new MySocketLambda() )
	{
		printf( "MySampleLambdaOneNode::MySampleLambdaOneNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

#ifndef NET_CLIENT_ONLY
		srv.listen(2000, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF

		cli->on(event::data, [this](Buffer& buffer) { this->cli->didData(buffer); });

	//	cli->once<event::Connect>([this]() { fmt::print( "welcome lambda-based solution [1]!\n" ); });
		cli->once(event::connect, [this]() { fmt::print( "welcome lambda-based solution [2]!\n" ); });
		cli->once(event::Connect::name, [this]() { fmt::print( "welcome lambda-based solution [3]!\n" ); });
		cli->once("connect", [this]() { fmt::print( "welcome lambda-based solution [3.1]!\n" ); });

	//	cli->once<event::Close>([this](bool) { fmt::print( "close lambda-based solution [1]!\n" ); return true; });
		cli->once(event::close, [this](bool) { fmt::print( "close lambda-based solution [2]!\n" ); return true; });
		cli->once(event::Close::name, [this](bool) { fmt::print( "close lambda-based solution [3]!\n" ); return true; });
		cli->once("close", [this](bool) { fmt::print( "close lambda-based solution [3.1]!\n" ); return true; });

		cli->connect(2000, "127.0.0.1", [this] {cli->didConnect(); });
	}
};
#endif // USING_L_SOCKETS

#ifdef USING_O_SOCKETS
class MySampleInheritanceOneNode : public NodeBase
{
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;
	bool letOnDrain = false;

	using SocketIdType = int;

#ifndef NET_CLIENT_ONLY
	MyServerMember srv;
#endif // NO_SERVER_STAFF
//	unique_ptr<MySocketLambda> cli;
//	MySocketLambda* cli;
public:
	MySampleInheritanceOneNode() : sockNN1_3( this )/*, sockNN1_4( this )*/
	{
		printf( "MySampleInheritanceOneNode::MySampleInheritanceOneNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

#ifndef NET_CLIENT_ONLY
		srv.listen(2000, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF

		*( sockNN1_3.getExtra() ) = 17;
//		*( sockNN1_4.getExtra() ) = 71;
		sockNN1_3.connect(2000, "127.0.0.1");
//		sockNN1_4.connect(2008, "127.0.0.1");
	}

	void onConnect(const void* extra) 
	{ 
		printf( "MySampleInheritanceOneNode::onConnect()\n" ); 

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockNN1_3.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onClose(const void* extra,bool) { printf( "MySampleInheritanceOneNode::onClose()\n" ); }
	void onData(const void* extra, nodecpp::Buffer& buffer) 
	{ 
		printf( "MySampleInheritanceOneNode::onData()\n" );  
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockNN1_3.end();
	}
	void onDrain(const void* extra) 
	{
		if ( letOnDrain )
			printf( "MySampleInheritanceOneNode::onDrain()\n" ); 
	}
	void onError(const void* extra, nodecpp::Error&) { printf( "MySampleInheritanceOneNode::onError()\n" ); }
	void onEnd(const void* extra) { printf( "MySampleInheritanceOneNode::onEnd()\n" ); }

	void onWhateverConnect_2(const SocketIdType* extra) 
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleInheritanceOneNode::onWhateverConnect_2(), extra = %d\n", *extra );

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockNN1_3.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onWhateverClose_2(const SocketIdType* extra, bool)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleInheritanceOneNode::onWhateverClose_2(), extra = %d\n", *extra );
	}
	void onWhateverData_2(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleInheritanceOneNode::onWhateverData_2(), extra = %d\n", *extra );
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockNN1_3.end();
	}
	void onWhateverDrain_2(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		if ( letOnDrain )
			printf( "MySampleInheritanceOneNode::onWhateverDrain_2(), extra = %d\n", *extra );
	}
	void onWhateverError_2(const SocketIdType* extra, nodecpp::Error&)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleInheritanceOneNode::onWhateverError_2(), extra = %d\n", *extra );
	}
	void onWhateverEnd_2(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleInheritanceOneNode::onWhateverEnd_2(), extra = %d\n", *extra );
	}


	nodecpp::net::SocketN<MySampleInheritanceOneNode,SocketIdType,
		nodecpp::net::OnConnect<&MySampleInheritanceOneNode::onWhateverConnect_2>,
		nodecpp::net::OnClose<&MySampleInheritanceOneNode::onWhateverClose_2>,
		nodecpp::net::OnData<&MySampleInheritanceOneNode::onWhateverData_2>,
		nodecpp::net::OnDrain<&MySampleInheritanceOneNode::onWhateverDrain_2>,
		nodecpp::net::OnError<&MySampleInheritanceOneNode::onWhateverError_2>,
		nodecpp::net::OnEnd<&MySampleInheritanceOneNode::onWhateverEnd_2>
	> sockNN1_3/*, sockNN1_4*/;
};
#endif // USING_O_SOCKETS

class MySampleTNode; // forward declaration; needed in SocketO-derived classes

template<class Extra>
class MySocketO : public net::SocketO
{
	MySampleTNode* myNode = nullptr;
	Extra myId;

public:
	MySocketO( MySampleTNode* myNode_ ) : myNode( myNode_ ) {}
	void onClose(bool hadError) override;
	void onConnect() override;
	void onData(Buffer& buffer)override;
	void onDrain() override;
	void onEnd() override;
	void onError(Error& err) override;
};

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;
	bool letOnDrain = false;

	using SocketIdType = int;
	using ServerIdType = int;

	MySocketO<SocketIdType> sockO_1;
	MySocketLambda* cli;

#ifndef NET_CLIENT_ONLY
//	MyServerMember srv;
#endif // NO_SERVER_STAFF
//	unique_ptr<MySocketLambda> cli;
//	MySocketLambda* cli;
public:
	MySampleTNode() : sockO_1( this ), cli( new MySocketLambda() ), sockNN_1(this), sockNN_2(this), srv( this )
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

#ifndef NET_CLIENT_ONLY
		srv.listen(2000, "127.0.0.1", 5);
#endif // NO_SERVER_STAFF

		*( sockNN_1.getExtra() ) = 17;
//		sockO_1.connect(2000, "127.0.0.1");
//		*( sockNN1_4.getExtra() ) = 71;
		sockNN_1.connect(2000, "127.0.0.1");
//		sockNN1_4.connect(2008, "127.0.0.1");

		// lambda staff
		cli->on(event::data, [this](Buffer& buffer) { this->cli->didData(buffer); });

	//	cli->once<event::Connect>([this]() { fmt::print( "welcome lambda-based solution [1]!\n" ); });
		cli->once(event::connect, [this]() { fmt::print( "welcome lambda-based solution [2]!\n" ); });
		cli->once(event::Connect::name, [this]() { fmt::print( "welcome lambda-based solution [3]!\n" ); });
		cli->once("connect", [this]() { fmt::print( "welcome lambda-based solution [3.1]!\n" ); });

	//	cli->once<event::Close>([this](bool) { fmt::print( "close lambda-based solution [1]!\n" ); return true; });
		cli->once(event::close, [this](bool) { fmt::print( "close lambda-based solution [2]!\n" ); return true; });
		cli->once(event::Close::name, [this](bool) { fmt::print( "close lambda-based solution [3]!\n" ); return true; });
		cli->once("close", [this](bool) { fmt::print( "close lambda-based solution [3.1]!\n" ); return true; });

		cli->connect(2000, "127.0.0.1", [this] {cli->didConnect(); });
	}

	void onConnect(const void* extra) 
	{ 
		printf( "MySampleTNode::onConnect()\n" ); 

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockNN_1.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onClose(const void* extra,bool) { printf( "MySampleTNode::onClose()\n" ); }
	void onData(const void* extra, nodecpp::Buffer& buffer) 
	{ 
		printf( "MySampleTNode::onData()\n" );  
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockNN_1.end();
	}
	void onDrain(const void* extra) 
	{
		if ( letOnDrain )
			printf( "MySampleTNode::onDrain()\n" ); 
	}
	void onError(const void* extra, nodecpp::Error&) { printf( "MySampleTNode::onError()\n" ); }
	void onEnd(const void* extra) { printf( "MySampleTNode::onEnd()\n" ); }

	void onConnectO(const void* extra) 
	{ 
		printf( "MySampleTNode::onConnect()\n" ); 

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockO_1.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onDataO(const void* extra, nodecpp::Buffer& buffer) 
	{ 
		printf( "MySampleTNode::onData()\n" );  
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockO_1.end();
	}

	
	void onWhateverConnect(const SocketIdType* extra) 
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverConnect(), extra = %d\n", *extra );

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockNN_1.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onWhateverClose(const SocketIdType* extra, bool)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverClose(), extra = %d\n", *extra );
	}
	void onWhateverData(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverData(), extra = %d\n", *extra );
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockNN_1.end();
	}
	void onWhateverDrain(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		if ( letOnDrain )
			printf( "MySampleTNode::onWhateverDrain(), extra = %d\n", *extra );
	}
	void onWhateverError(const SocketIdType* extra, nodecpp::Error&)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverError(), extra = %d\n", *extra );
	}
	void onWhateverEnd(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverEnd(), extra = %d\n", *extra );
	}


	void onWhateverConnect_2(const SocketIdType* extra) 
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverConnect_2(), extra = %d\n", *extra );

		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		bool ok = true;
		while (ok) {
//			ok = write(ptr.get(), size, [] { print("onDrain!\n"); });
			ok = sockNN_1.write(ptr.get(), size);
			letOnDrain = !ok;
			sentSize += size;
		}
	}
	void onWhateverClose_2(const SocketIdType* extra, bool)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverClose_2(), extra = %d\n", *extra );
	}
	void onWhateverData_2(const SocketIdType* extra, nodecpp::Buffer& buffer)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverData_2(), extra = %d\n", *extra );
		recvSize += buffer.size();

		if (recvSize >= sentSize)
			sockNN_1.end();
	}
	void onWhateverDrain_2(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		if ( letOnDrain )
			printf( "MySampleTNode::onWhateverDrain_2(), extra = %d\n", *extra );
	}
	void onWhateverError_2(const SocketIdType* extra, nodecpp::Error&)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverError_2(), extra = %d\n", *extra );
	}
	void onWhateverEnd_2(const SocketIdType* extra)
	{
		NODECPP_ASSERT( extra != nullptr );
		printf( "MySampleTNode::onWhateverEnd_2(), extra = %d\n", *extra );
	}

	// server socket
	void onCloseSererSocket(const SocketIdType* extra, bool hadError) {print("server socket: onCloseSererSocket!\n");};
	void onConnectSererSocket(const SocketIdType* extra) {
		print("server socket: onConnect!\n");
	}
	void onDataSererSocket(const SocketIdType* extra, Buffer& buffer) {
		print("server socket: onData!\n");
//		++count;
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
public:
	void onCloseServer(const ServerIdType* extra, bool hadError) {print("server: onCloseServer()!\n");}
//	void onConnection(SockTypeServerSocket* socket) { NODECPP_ASSERT( socket != nullptr ); *(socket->getExtra()) = 1;}
	void onConnection(const ServerIdType* extra, net::SocketTBase* socket) { 
		print("server: onConnection()!\n");
		NODECPP_ASSERT( socket != nullptr ); 
		serversSocks.push_back( socket );
		/**(socket->getExtra()) = 1;*/
	}
	void onListening(const ServerIdType* extra) {print("server: onListening()!\n");}
	void onErrorServer(const ServerIdType* extra, Error& err) {print("server: onErrorServer!\n");}

	/*	using Initializer = SocketTInitializer<
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnCloseT<&MySampleTNode::onWhateverClose>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData>,
		nodecpp::net::OnDrainT<&MySampleTNode::onWhateverDrain>,
		nodecpp::net::OnErrorT<&MySampleTNode::onWhateverError>,
		nodecpp::net::OnEndT<&MySampleTNode::onWhateverEnd>
	>;*/

	using SockType_1 = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect>,
		nodecpp::net::OnCloseT<&MySampleTNode::onWhateverClose>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData>,
		nodecpp::net::OnDrainT<&MySampleTNode::onWhateverDrain>,
		nodecpp::net::OnErrorT<&MySampleTNode::onWhateverError>,
		nodecpp::net::OnEndT<&MySampleTNode::onWhateverEnd>
	>;
	SockType_1 sockNN_1;

	using SockType_2 = nodecpp::net::SocketT<MySampleTNode,SocketIdType,
		nodecpp::net::OnConnectT<&MySampleTNode::onWhateverConnect_2>,
		nodecpp::net::OnCloseT<&MySampleTNode::onWhateverClose_2>,
		nodecpp::net::OnDataT<&MySampleTNode::onWhateverData_2>,
		nodecpp::net::OnDrainT<&MySampleTNode::onWhateverDrain_2>,
		nodecpp::net::OnErrorT<&MySampleTNode::onWhateverError_2>,
		nodecpp::net::OnEndT<&MySampleTNode::onWhateverEnd_2>
	>;
	SockType_2 sockNN_2;

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


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket, SockType_1, SockType_2, SockTypeServerSocket>;
//	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<ServerType>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server, ServerType>;
};

template<class Extra> void MySocketO<Extra>::onClose(bool hadError) { myNode->onClose( &myId, hadError ); }
template<class Extra> void MySocketO<Extra>::onConnect() { myNode->onConnectO( &myId ); }
template<class Extra> void MySocketO<Extra>::onData(Buffer& buffer) { myNode->onDataO( &myId, buffer ); }
template<class Extra> void MySocketO<Extra>::onDrain() { myNode->onDrain( &myId ); }
template<class Extra> void MySocketO<Extra>::onEnd() { myNode->onEnd( &myId ); }
template<class Extra> void MySocketO<Extra>::onError(Error& err) { myNode->onError( &myId, err ); }

#endif // NET_SOCKET_H
