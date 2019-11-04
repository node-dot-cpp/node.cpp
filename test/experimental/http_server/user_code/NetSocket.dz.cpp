#line 1 "<built-in>"
#include <dezombiefy.h>
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code\\NetSocket.cpp"
// NetSocket.cpp : sample of user-defined code


#include <infrastructure.h>
#if 0 /* expanded below */
#include "NetSocket.h"
#endif /* expanded below */
#line 1 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#include <nodecpp/http_server.h>
#ifdef NODECPP_ENABLE_CLUSTERING
#include <nodecpp/cluster.h>
#endif // NODECPP_ENABLE_CLUSTERING
#line 14 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

using namespace std;
using namespace nodecpp;
using namespace fmt;

//#define IMPL_VERSION 1 // main() is a single coro
#define IMPL_VERSION 12 // main() is a single coro (with clustering)
//#define IMPL_VERSION 2 // onRequest is a coro

class MySampleTNode : public NodeBase
{
public: // just temporarily
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	Buffer ctrlReplyBuff;

public:

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif
#line 44 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	class MyHttpServer : public nodecpp::net::HttpServer<MySampleTNode>
	{
	public:
		MyHttpServer() {}
		MyHttpServer(MySampleTNode* node) : HttpServer<MySampleTNode>(nodecpp::safememory::dezombiefy( node )) {}
		virtual ~MyHttpServer() {}
	};

	class CtrlServer : public nodecpp::net::ServerBase
	{
	public:
		CtrlServer() {}
		virtual ~CtrlServer() {}
	};

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyHttpServer;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = CtrlServer;
	nodecpp::safememory::owning_ptr<CtrlServerType>  srvCtrl;

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

#if IMPL_VERSION == 1
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

//		srv = nodecpp::net::createServer<ServerType, HttpSock>(this);
		srv = nodecpp::net::createHttpServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5000);
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
#line 98 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
		try { 
			for(;;) { 
				co_await srv->a_request(request, response); 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				++(stats.rqCnt);
//				request->dbgTrace();

//				simpleProcessing( request, response );
				yetSimpleProcessing( request, response );
			} 
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}

#elif IMPL_VERSION == 12
#line 120 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(nodecpp::safememory::dezombiefy( this ));

#ifdef NODECPP_ENABLE_CLUSTERING
		if ( getCluster().isMaster() )
		{
			size_t coreCnt = 1;
			auto argv = getArgv();
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 9 && argv[i].substr(0,9) == "numcores=" )
					coreCnt = atol(argv[i].c_str() + 9);
			}

			for ( size_t i=0; i<coreCnt; ++i )
				getCluster().fork();
		}
		else
#endif // NODECPP_ENABLE_CLUSTERING
#line 143 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
		{
			nodecpp::safememory::dezombiefy( this )->srv = nodecpp::net::createHttpServer<ServerType>();
			nodecpp::safememory::dezombiefy( this )->srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

			auto argv = getArgv();
			std::string ip = "0.0.0.0";
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 3 && argv[i].substr(0,9) == "ip=" )
					ip = argv[i].substr(3);
			}
			{ auto nodecpp_0 = &*(nodecpp::safememory::dezombiefy( this )->srv); auto nodecpp_1 = ip.c_str(); nodecpp::safememory::dezombiefy( nodecpp_0 )->listen(2000, nodecpp_1, 5000); };
			nodecpp::safememory::dezombiefy( this )->srvCtrl->listen(2001, "127.0.0.1", 5);

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
#line 167 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

			nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
			nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
			try { 
				for(;;) { 
					co_await nodecpp::safememory::dezombiefy( this )->srv->a_request(request, response); 
					Buffer b1(0x1000);
					co_await request->a_readBody( b1 );
					++(nodecpp::safememory::dezombiefy( this )->stats.rqCnt);
//					request->dbgTrace();

	//				simpleProcessing( request, response );
					yetSimpleProcessing( request, response );
				} 
			} 
			catch (...) { // TODO: what?
			}
		}

		CO_RETURN;
	}
#elif IMPL_VERSION == 2
#line 189 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::HttpServerBase::addHttpHandler<ServerType, nodecpp::net::HttpServerBase::Handler::IncomingRequest, &MySampleTNode::onRequest>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

//		srv = nodecpp::net::createHttpServer<ServerType, HttpSock>(this);
//		srv = nodecpp::net::createHttpServer<ServerType, nodecpp::net::IncomingHttpMessageAtServer>(this);
		srv = nodecpp::net::createHttpServer<ServerType, nodecpp::net::IncomingHttpMessageAtServer>();
//		srv = nodecpp::net::createHttpServer<ServerType>();
		srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

		srv->listen(2000, "127.0.0.1", 5000);
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
#line 215 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

		CO_RETURN;
	}
	virtual nodecpp::handler_ret_type onRequest(nodecpp::safememory::soft_ptr<ServerType> server, nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response)
	{
//		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
//		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
		try { 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				++(stats.rqCnt);
//				request->dbgTrace();

//				simpleProcessing( request, response );
				yetSimpleProcessing( request, response );
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}
#else
#line 237 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
#error
#endif // IMPL_VERSION
#line 239 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"

	nodecpp::handler_ret_type simpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
//			response->dbgTrace();
			co_await response->flushHeaders();
			CO_RETURN;
		}

		bool dataAvailable = false;
		std::string replyHtml;
		if ( request->getUrl() == "/" )
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>HOME</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">another_page.html</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = true;
		}
		else if ( request->getUrl() == "/another_page.html" )
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>Another Page</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">this page</a> again, or back to <a href=\"/\">HOME</a><br/>"
				"( or try to access <a href=\"x.html\">inexistent</a> page to (hopefully) see 404 error"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = true;
		}
		else // not found (body for 404)
		{
			std::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>404 Not Found</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"/\">HOME</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = fmt::format( replyHtmlFormat.c_str(), nodecpp::safememory::dezombiefy( this )->stats.rqCnt );
			dataAvailable = false;
		}

		if ( dataAvailable )
		{
			response->setStatus( "HTTP/1.1 200 OK" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
			{ auto nodecpp_0 = &*(response); auto nodecpp_1 = replyHtml.size(); auto&& nodecpp_2 = "Content-Length"; auto&& nodecpp_3 = fmt::format( "{}", nodecpp_1); nodecpp::safememory::dezombiefy( nodecpp_0 )->addHeader( nodecpp::safememory::dezombiefy( nodecpp_2 ), nodecpp_3 ); };

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			{ auto nodecpp_4 = replyHtml.c_str(); auto nodecpp_5 = replyHtml.size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_4 ), nodecpp_5 ); };
			co_await response->writeBodyPart(b);
		}
		else
		{
			response->setStatus( "HTTP/1.1 404 Not Found" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
//					response->addHeader( "Content-Length", "0" );
			response->addHeader( "Content-Type", "text/html" );
			{ auto nodecpp_6 = &*(response); auto nodecpp_7 = replyHtml.size(); auto&& nodecpp_8 = "Content-Length"; auto&& nodecpp_9 = fmt::format( "{}", nodecpp_7); nodecpp::safememory::dezombiefy( nodecpp_6 )->addHeader( nodecpp::safememory::dezombiefy( nodecpp_8 ), nodecpp_9 ); };

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			{ auto nodecpp_10 = replyHtml.c_str(); auto nodecpp_11 = replyHtml.size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_10 ), nodecpp_11 ); };
			co_await response->writeBodyPart(b);
		}

		// TODO: co_await for msg body, if any
		// TODO: form and send response

		CO_RETURN;
	}
	void parseUrlQueryString(const std::string& query, std::vector<std::pair<std::string, std::string>>& queryValues )
	{
		size_t start = 0;

		for(;;)
		{
			size_t endEq = nodecpp::safememory::dezombiefy( query ).find_first_of( "=", start );
			if ( endEq == std::string::npos )
			{
				{ auto nodecpp_0 = endEq-start; auto&& nodecpp_1 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_0 ); auto&& nodecpp_2 = std::make_pair( nodecpp_1, "" ); auto&& nodecpp_3 = nodecpp_2; nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_3 ); };
				break;
			}
			size_t endAmp = nodecpp::safememory::dezombiefy( query ).find_first_of( "&", endEq+ 1  );
			if ( endAmp == std::string::npos )
			{
				{ auto nodecpp_4 = endEq-start; auto nodecpp_5 = endEq + 1; auto&& nodecpp_6 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_4 ); auto&& nodecpp_7 = nodecpp::safememory::dezombiefy( query ).substr( nodecpp_5 ); auto&& nodecpp_8 = std::make_pair( nodecpp_6, nodecpp_7 ); nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_8 ); };
				break;
			}
			else
			{
				{ auto nodecpp_9 = endEq-start; auto nodecpp_10 = endAmp - endEq; auto nodecpp_11 = endEq + 1; auto nodecpp_12 = nodecpp_10 - 1; auto&& nodecpp_13 = nodecpp::safememory::dezombiefy( query ).substr( start, nodecpp_9 ); auto&& nodecpp_14 = nodecpp::safememory::dezombiefy( query ).substr( nodecpp_11, nodecpp_12 ); auto&& nodecpp_15 = std::make_pair( nodecpp_13, nodecpp_14 ); nodecpp::safememory::dezombiefy( queryValues ).push_back( nodecpp_15 ); };
				start = endAmp + 1;
			}
		}
	}

	nodecpp::handler_ret_type yetSimpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
//			response->dbgTrace();
			co_await response->flushHeaders();
			CO_RETURN;
		}

		const auto & url = request->getUrl();
		size_t start = url.find_first_of( "?" );
		response->setStatus( "HTTP/1.1 200 OK" );
		response->addHeader( "Content-Type", "text/xml" );
		Buffer b;
		if ( start == std::string::npos )
		{
			b.append( "no value specified", sizeof( "undefined" ) - 1 );
			response->addHeader( "Connection", "close" );
		}
		else
		{
			std::vector<std::pair<std::string, std::string>> queryValues;
			nodecpp::safememory::dezombiefy( this )->parseUrlQueryString(nodecpp::safememory::dezombiefy( url ).substr(start+1), queryValues );
			for ( auto entry: queryValues )
				if ( entry.first == "value" )
				{
					{ auto& nodecpp_0 = entry.second; auto& nodecpp_1 = entry.second; auto nodecpp_2 = nodecpp_0.c_str(); auto nodecpp_3 = nodecpp::safememory::dezombiefy( nodecpp_1 ).size(); b.append( nodecpp::safememory::dezombiefy( nodecpp_2 ), nodecpp_3 ); };
					b.appendUint8( ',' );
				}
			if ( b.size() )
			{
				b.trim( 1 );
				response->addHeader( "Connection", "keep-alive" );
			}
			else
			{
				b.append( "no value specified", sizeof( "no value specified" ) - 1 );
				response->addHeader( "Connection", "close" );
			}
		}
		{ auto nodecpp_4 = &*(response); auto nodecpp_5 = b.size(); auto&& nodecpp_6 = "Content-Length"; auto&& nodecpp_7 = fmt::format( "{}", nodecpp_5 ); nodecpp::safememory::dezombiefy( nodecpp_4 )->addHeader( nodecpp::safememory::dezombiefy( nodecpp_6 ), nodecpp_7 ); };
//		response->dbgTrace();
		co_await response->flushHeaders();
		co_await response->writeBodyPart(b);

		CO_RETURN;
	}

	nodecpp::handler_ret_type onConnectionCtrl(nodecpp::safememory::soft_ptr<CtrlServer> server, nodecpp::safememory::soft_ptr<net::SocketBase> socket) { 
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
#line 422 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
			co_await socket->a_read( r_buff, 2 );
			co_await nodecpp::safememory::dezombiefy( this )->onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = nodecpp::safememory::dezombiefy( buffer ).begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(nodecpp::safememory::dezombiefy( this )->stats));
			nodecpp::safememory::dezombiefy( this )->stats.connCnt = nodecpp::safememory::dezombiefy( this )->srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			ctrlReplyBuff.clear();
			nodecpp::safememory::dezombiefy( this )->ctrlReplyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(nodecpp::safememory::dezombiefy( this )->ctrlReplyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H
#line 449 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code/NetSocket.h"
#line 6 "C:\\project\\node-dot-cpp\\node.cpp\\test\\experimental\\http_server\\build\\..\\user_code\\NetSocket.cpp"

static NodeRegistrator<Runnable<MySampleTNode>> noname( "MySampleTemplateNode" );
