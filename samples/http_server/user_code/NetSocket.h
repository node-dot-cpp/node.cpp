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
#include <nodecpp/logging.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	nodecpp::Log log;

public:

	class MyHttpServer : public nodecpp::net::HttpServer<MySampleTNode>
	{
	public:
		MyHttpServer() {}
		MyHttpServer(MySampleTNode* node) : HttpServer<MySampleTNode>(node) {}
		virtual ~MyHttpServer() {}
	};

	using SockTypeServerSocket = nodecpp::net::SocketBase;

	using ServerType = MyHttpServer;
	nodecpp::safememory::owning_ptr<ServerType> srv; 

	MySampleTNode()
	{
		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MySampleTNode::MySampleTNode()" );
	}

	virtual nodecpp::handler_ret_type main()
	{
		log.add( stdout );
		log.setLevel( nodecpp::LogLevel::debug );
		log.setGuaranteedLevel( nodecpp::LogLevel::warning );

		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MySampleLambdaOneNode::main()" );

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
			srv = nodecpp::net::createHttpServer<ServerType>();

			auto argv = getArgv();
			nodecpp::string ip = "0.0.0.0";
			for ( size_t i=1; i<argv.size(); ++i )
			{
				if ( argv[i].size() > 3 && argv[i].substr(0,9) == "ip=" )
					ip = argv[i].substr(3);
			}
			srv->listen(2000, ip, 5000);

			nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
			nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;
			try { 
				for(;;) { 
					co_await srv->a_request(request, response); 
					Buffer b1(0x1000);
					co_await request->a_readBody( b1 );
//					request->dbgTrace();

					processRequest( request, response );
				} 
			} 
			catch (...) { // TODO: what?
			}
		}

		CO_RETURN;
	}

	nodecpp::handler_ret_type processRequest( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
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
		nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>> queryValues;
		nodecpp::Url::parseUrlQueryString( url, queryValues );

		response->setStatus( "HTTP/1.1 200 OK" );
		response->addHeader( "Content-Type", "text/xml" );
		Buffer b;
		if ( queryValues.empty() )
		{
			b.append( "no value specified", sizeof( "undefined" ) - 1 );
			response->addHeader( "Connection", "close" );
		}
		else
		{
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

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;

};

#endif // NET_SOCKET_H
