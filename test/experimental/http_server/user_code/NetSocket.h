// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#include <nodecpp/http_server.h>

using namespace std;
using namespace nodecpp;
using namespace fmt;

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

	class HttpSock : public nodecpp::net::HttpSocket<MySampleTNode>
	{

	public:
		using NodeType = MySampleTNode;
		friend class MySampleTNode;

	public:
		HttpSock() {}
		HttpSock(MySampleTNode* node) : HttpSocket<MySampleTNode>(node) {}
		virtual ~HttpSock() {}
	};

	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}


	virtual nodecpp::handler_ret_type main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

//		nodecpp::net::ServerBase::addHandler<ServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionx>(this);
		nodecpp::net::ServerBase::addHandler<CtrlServerType, nodecpp::net::ServerBase::DataForCommandProcessing::UserHandlers::Handler::Connection, &MySampleTNode::onConnectionCtrl>(this);

		srv = nodecpp::net::createServer<ServerType, HttpSock>(this);
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

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::OutgoingHttpMessageAtServer> response;
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

	nodecpp::handler_ret_type simpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::OutgoingHttpMessageAtServer> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
			response->dbgTrace();
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
			replyHtml = fmt::format( replyHtmlFormat.c_str(), stats.rqCnt );
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
			replyHtml = fmt::format( replyHtmlFormat.c_str(), stats.rqCnt );
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
			replyHtml = fmt::format( replyHtmlFormat.c_str(), stats.rqCnt );
			dataAvailable = false;
		}

		if ( dataAvailable )
		{
			response->setStatus( "HTTP/1.1 200 OK" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", fmt::format( "{}", replyHtml.size()) );

			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			b.append( replyHtml.c_str(), replyHtml.size() );
			co_await response->writeBodyPart(b);
		}
		else
		{
			response->setStatus( "HTTP/1.1 404 Not Found" );
			response->addHeader( "Connection", "keep-alive" );
//				response->addHeader( "Connection", "close" );
//					response->addHeader( "Content-Length", "0" );
			response->addHeader( "Content-Type", "text/html" );
			response->addHeader( "Content-Length", fmt::format( "{}", replyHtml.size()) );

			response->dbgTrace();
			co_await response->flushHeaders();
			Buffer b;
			b.append( replyHtml.c_str(), replyHtml.size() );
			co_await response->writeBodyPart(b);
		}

		// TODO: co_await for msg body, if any
		// TODO: form and send response

		CO_RETURN;
	}

	nodecpp::handler_ret_type yetSimpleProcessing( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::OutgoingHttpMessageAtServer> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->setStatus( "HTTP/1.1 405 Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
			response->dbgTrace();
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
			b.append( "no value specified", sizeof( "no value specified" ) - 1 );
			response->addHeader( "Connection", "close" );
		}
		else
		{
			b.append( url.c_str() + start, url.size() - start );
			response->addHeader( "Connection", "keep-alive" );
		}
		response->addHeader( "Content-Length", fmt::format( "{}", b.size() ) );
//		response->dbgTrace();
		co_await response->flushHeaders();
		co_await response->writeBodyPart(b);

		CO_RETURN;
	}

	/*nodecpp::handler_ret_type onConnectionx(nodecpp::safememory::soft_ptr<MyHttpServer> server, nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket) { 
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>("server: onConnection()!");
		//srv.unref();
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, socket != nullptr ); 
		soft_ptr<HttpSock> socketPtr = nodecpp::safememory::soft_ptr_static_cast<HttpSock>(socket);

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::OutgoingHttpMessageAtServer> response;
		try { 
			for(;;) { 
				co_await socketPtr->a_(request, response); 
				// TODO: co_await for msg body, if any
				// TODO: form and send response
			} 
		} 
		catch (...) { socketPtr->end(); }

		CO_RETURN;
	}

	nodecpp::handler_ret_type readLine(nodecpp::safememory::soft_ptr<nodecpp::net::SocketBase> socket, Buffer& buffer) {
	}*/

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

	using SockTypeServerSocket = nodecpp::net::SocketBase;
	using SockTypeServerCtrlSocket = nodecpp::net::SocketBase;

	using ServerType = MyHttpServer;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	using CtrlServerType = CtrlServer;
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
			ctrlReplyBuff.clear();
			ctrlReplyBuff.append( &stats, replySz); // naive marshalling will work for a limited number of cases
			socket->write(ctrlReplyBuff);
		}
		CO_RETURN;
	}


};

#endif // NET_SOCKET_H
