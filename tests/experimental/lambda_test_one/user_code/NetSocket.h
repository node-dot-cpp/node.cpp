// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/net.h"

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

inline
void MyServerSocketMember::didClose(bool hadError) {
	print("onClose!\n");
	if (server)
		server->closeMe(this);
}

class MySampleLambdaOneNode : public NodeBase
{
	MyServerMember srv;
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

		srv.listen(2000, "127.0.0.1", 5);

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

class MySampleInheritanceOneNode : public NodeBase
{
	size_t recvSize = 0;
	size_t sentSize = 0;
	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;
	bool letOnDrain = false;

	using SocketIdType = int;

	MyServerMember srv;
//	unique_ptr<MySocketLambda> cli;
//	MySocketLambda* cli;
public:
	MySampleInheritanceOneNode() : sockNN1_3( this )/*, sockNN1_4( this )*/
	{
		printf( "MySampleLambdaOneNode::MySampleLambdaOneNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );

		srv.listen(2000, "127.0.0.1", 5);

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

#endif // NET_SOCKET_H
