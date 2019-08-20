// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>


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

	Buffer replyBuff;

public:

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	class MyServerSocketOne : public nodecpp::net::ServerSocket<MySampleTNode>
	{
	public:
		MyServerSocketOne() {}
		MyServerSocketOne(MySampleTNode* node) : nodecpp::net::ServerSocket<MySampleTNode>(node) {}
		virtual ~MyServerSocketOne() {}
	};

	class MyServerSocketTwo : public nodecpp::net::ServerBase
	{
	public:
		MyServerSocketTwo() {}
		virtual ~MyServerSocketTwo() {}
	};

	class HttpSocket : public nodecpp::net::SocketBase, public ::nodecpp::DataParent<MySampleTNode>
	{
		class DummyBuffer
		{
			Buffer base;
			size_t currpos = 0;
		public:
			DummyBuffer() : base(0x10000) {}
			void pushFragment(const Buffer& b) { base.append( b ); }
			bool popLine(Buffer& b) 
			{ 
				for ( ; currpos<base.size(); ++currpos )
					if ( *(base.begin() + currpos) = '\n' )
					{
						b.clear();
						b.append( base, 0, currpos+1 );
						base.popFront( currpos+1 );
						currpos = 0;
						return true;
					}
				return false;
			}
		};
		DummyBuffer dbuf;

	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	public:
		HttpSocket() {}
		HttpSocket(MySampleTNode* node) : nodecpp::net::SocketBase(), ::nodecpp::DataParent<MySampleTNode>(node) {}
		virtual ~HttpSocket() {}

		nodecpp::handler_ret_type processRequests()
		{
			nodecpp::Buffer r_buff(0x200);
			for (;;)
			{
	#ifdef AUTOMATED_TESTING_ONLY
				if ( stopResponding )
				{
					nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by timer)" );
					socket->end();
					socket->unref();
					break;
				}
	#endif
				co_await a_read( r_buff, 2 );
				co_await onDataHttpServerSocket(r_buff);
			}
			CO_RETURN;
		}
		nodecpp::handler_ret_type onDataHttpServerSocket(Buffer& buffer) {

		printf( "Received data:\n\n%s", buffer.begin() );
				Buffer reply;

				std::string replyBegin = "HTTP/1.1 200 OK\r\n"
	"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
	"Server: Apache/2.2.14 (Win32)\r\n"
	"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
	"Content-Length: 88\r\n"
	"Content-Type: text/html\r\n"
	"Connection: keep-alive\r\n"
	"\r\n\r\n"
	"<html>\r\n"
	"<body>\r\n";
				std::string replyEnd = "</body>\r\n"
	"</html>\r\n"
	"\r\n\r\n";
				std::string replyBody = fmt::format( "<h1>Fuck you! (# {})</h1>\r\n", getDataParent()->stats.rqCnt + 1 );
				std::string r = replyBegin + replyBody + replyEnd;
				reply.append( r.c_str(), r.size() );
				write(reply);
				end();
			++(getDataParent()->stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
			/*if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by count)" );
				end();
				unref();
			}*/
#endif
			CO_RETURN;
		}

	};

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}


	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<ServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionx>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<ServerType, HttpSocket>(this);
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5);
		srvCtrl->listen(2001, "127.0.0.1", 5);

#ifdef AUTOMATED_TESTING_ONLY
		to = std::move( nodecpp::setTimeout(  [this]() { 
			srv->close();
			srv->unref();
			srvCtrl->close();
			srvCtrl->unref();
			stopAccepting = true;
			to = std::move( nodecpp::setTimeout(  [this]() {stopResponding = true;}, 3000 ) );
		}, 3000 ) );
#endif

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionx(nodecpp::safememory::soft_ptr<MyServerSocketOne> server, nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		soft_ptr<HttpSocket> socketPtr = nodecpp::safememory::soft_ptr_static_cast<HttpSocket>(socket);

		socketPtr->processRequests();

		CO_RETURN;
	}

	nodecpp::handler_ret_type readLine(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
	}

	nodecpp::handler_ret_type onDataHttpServerSocket(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
#if 0
		if ( buffer.size() < 2 )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data on socket idx = {}", *(socket->getExtra()) );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Insufficient data: received {} bytes", buffer.size() );
			socket->unref();
			CO_RETURN;
		}
//		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server socket: onData for idx {} !", *(socket->getExtra()) );

		size_t receivedSz = buffer.begin()[0];
		if ( receivedSz != buffer.size() )
		{
//			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data on socket idx = {}: received {}, expected: {} bytes", *(socket->getExtra()), receivedSz, buffer.size() );
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "Corrupted data: received {}, expected: {} bytes", receivedSz, buffer.size() );
			socket->unref();
			CO_RETURN;
		}

		uint32_t requestedSz = buffer.begin()[1];
		if ( requestedSz )
		{
			Buffer reply(requestedSz);
			//buffer.begin()[0] = (uint8_t)requestedSz;
			memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
			reply.set_size(requestedSz);
			socket->write(reply);
		}

		stats.recvSize += receivedSz;
		stats.sentSize += requestedSz;
#else
		printf( "Received data:\n\n%s", buffer.begin() );
			Buffer reply;

			std::string replyBegin = "HTTP/1.1 200 OK\r\n"
"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
"Server: Apache/2.2.14 (Win32)\r\n"
"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
"Content-Length: 88\r\n"
"Content-Type: text/html\r\n"
"Connection: keep-alive\r\n"
"\r\n\r\n"
"<html>\r\n"
"<body>\r\n";
			std::string replyEnd = "</body>\r\n"
"</html>\r\n"
"\r\n\r\n";
			std::string replyBody = fmt::format( "<h1>Fuck you! (# {})</h1>\r\n", stats.rqCnt + 1 );
			std::string r = replyBegin + replyBody + replyEnd;
			reply.append( r.c_str(), r.size() );
			socket->write(reply);
			socket->end();
#endif
		++(stats.rqCnt);
#ifdef AUTOMATED_TESTING_ONLY
		/*if ( stats.rqCnt > AUTOMATED_TESTING_CYCLE_COUNT )
		{
			nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by count)" );
			socket->end();
			socket->unref();
		}*/
#endif
		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<MyServerSocketTwo> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnectionCtrl()!");

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		nodecpp::Buffer r_buff(0x200);
		for (;;)
		{
#ifdef AUTOMATED_TESTING_ONLY
			if ( stopResponding )
			{
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "About to exit successfully in automated testing (by timer)" );
				socket->end();
				socket->unref();
				break;
			}
#endif
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyServerSocketOne;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = MyServerSocketTwo;
	nodecpp::safememory::owning_ptr<CtrlServerType>  srvCtrl;

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			replyBuff.clear();
			replyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(replyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H
