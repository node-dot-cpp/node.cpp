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
#include <log.h>

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

	nodecpp::Log log;

public:

#ifdef AUTOMATED_TESTING_ONLY
	bool stopAccepting = false;
	bool stopResponding = false;
	nodecpp::Timeout to;
#endif

	class MyHttpServer : public nodecpp::net::HttpServer<MySampleTNode>
	{
	public:
		MyHttpServer() {}
		MyHttpServer(MySampleTNode* node) : HttpServer<MySampleTNode>(node) {}
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

	virtual nodecpp::handler_ret_type main()
	{
		log.add( stdout );
		log.level = nodecpp::Log::Level::silly;

		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

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
		{
			for ( size_t i=0; i<3000; ++i )
				log.debug( "[{}] some silly msg with data {}", getCluster().worker().id(), i );

			srv = nodecpp::net::createHttpServer<ServerType>();
			srvCtrl = nodecpp::net::createServer<CtrlServerType, nodecpp::net::SocketBase>();

			auto argv = getArgv();
			nodecpp::string ip = "0.0.0.0";
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 3 && argv[i].substr(0,9) == "ip=" )
					ip = argv[i].substr(3);
			}
			srv->listen(2000, ip, 5000);
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

			nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
			nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
			try { 
				for(;;) { 
					co_await srv->a_request(request, response); 
					Buffer b1(0x1000);
					co_await request->a_readBody( b1 );
					++(stats.rqCnt);
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
#error
#endif // IMPL_VERSION

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
		nodecpp::string replyHtml;
		if ( request->getUrl() == "/" )
		{
			nodecpp::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>HOME</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">another_page.html</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = nodecpp::format( replyHtmlFormat, stats.rqCnt );
			dataAvailable = true;
		}
		else if ( request->getUrl() == "/another_page.html" )
		{
			nodecpp::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>Another Page</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"another_page.html\">this page</a> again, or back to <a href=\"/\">HOME</a><br/>"
				"( or try to access <a href=\"x.html\">inexistent</a> page to (hopefully) see 404 error"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = nodecpp::format( replyHtmlFormat, stats.rqCnt );
			dataAvailable = true;
		}
		else // not found (body for 404)
		{
			nodecpp::string replyHtmlFormat = "<html>\r\n"
				"<body>\r\n"
				"<h1>404 Not Found</h1>\r\n"
				"<p>ordinal at server: {}\r\n"
				"<p>Please proceed to <a href=\"/\">HOME</a>"
				"</body>\r\n"
				"</html>\r\n";
			replyHtml = nodecpp::format( replyHtmlFormat, stats.rqCnt );
			dataAvailable = false;
		}

		if ( dataAvailable )
		{
			response->setStatus( "HTTP/1.1 200 OK" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", nodecpp::format( "{}", replyHtml.size()) );

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			b.appendString( replyHtml );
			co_await response->writeBodyPart(b);
		}
		else
		{
			response->setStatus( "HTTP/1.1 404 Not Found" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
//					response->addHeader( "Content-Length", "0" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Content-Length", nodecpp::format( "{}", replyHtml.size()) );

//			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			b.appendString( replyHtml );
			co_await response->writeBodyPart(b);
		}
		co_await response->end();

		// TODO: co_await for msg body, if any
		// TODO: form and send response

		CO_RETURN;
	}
	void parseUrlQueryString(const nodecpp::string& query, nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>>& queryValues )
	{
		size_t start = 0;

		for(;;)
		{
			size_t endEq = query.find_first_of( "=", start );
			if ( endEq == nodecpp::string::npos )
			{
				queryValues.push_back( std::make_pair( query.substr( start, endEq-start ), "" ) );
				break;
			}
			size_t endAmp = query.find_first_of( "&", endEq+ 1  );
			if ( endAmp == nodecpp::string::npos )
			{
				queryValues.push_back( std::make_pair( query.substr( start, endEq-start ), query.substr( endEq + 1 ) ) );
				break;
			}
			else
			{
				queryValues.push_back( std::make_pair( query.substr( start, endEq-start ), query.substr( endEq + 1, endAmp - endEq - 1 ) ) );
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
			co_await response->end();
			CO_RETURN;
		}

		const auto & url = request->getUrl();
		size_t start = url.find_first_of( "?" );
		response->setStatus( "HTTP/1.1 200 OK" );
		response->addHeader( "Content-Type", "text/xml" );
		Buffer b;
		if ( start == nodecpp::string::npos )
		{
			b.append( "no value specified", sizeof( "undefined" ) - 1 );
			response->addHeader( "Connection", "close" );
		}
		else
		{
			nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>> queryValues;
			parseUrlQueryString(url.substr(start+1), queryValues );
			for ( auto entry: queryValues )
				if ( entry.first == "value" )
				{
					b.appendString( entry.second );
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
		response->addHeader( "Content-Length", nodecpp::format( "{}", b.size() ) );
//		response->dbgTrace();
		co_await response->flushHeaders();
		co_await response->writeBodyPart(b);
		co_await response->end();

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
			co_await socket->a_read( r_buff, 2 );
			co_await onDataCtrlServerSocket_(socket, r_buff);
		}
		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;
	nodecpp::handler_ret_type onDataCtrlServerSocket_(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {

		size_t requestedSz = buffer.begin()[1];
		if (requestedSz)
		{
			Buffer reply(sizeof(stats));
			stats.connCnt = srv->getSockCount();
			uint32_t replySz = sizeof(Stats);
			ctrlReplyBuff.clear();
			ctrlReplyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(ctrlReplyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H
