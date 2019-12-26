// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H


#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#include <nodecpp/http_server.h>
#include <nodecpp/logging.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	nodecpp::Log log;

public:
	using ServerType = nodecpp::net::HttpServer<MySampleTNode>;

	nodecpp::safememory::owning_ptr<ServerType> srv; 

	MySampleTNode() { nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MySampleTNode::MySampleTNode()" ); }

	virtual nodecpp::handler_ret_type main()
	{
		log.add( stdout );
		log.setLevel( nodecpp::LogLevel::debug );
		log.setGuaranteedLevel( nodecpp::LogLevel::warning );

		nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		srv = nodecpp::net::createHttpServer<ServerType>();
		srv->listen(2000, "0.0.0.0", 5000);

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;

		try { 
			for(;;) { 
				co_await srv->a_request(request, response); 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );

				processRequest( request, response );
			} 
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}

	nodecpp::handler_ret_type processRequest( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
		// unexpected method
		if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
		{
			response->writeHeader( 405, "Method Not Allowed" );
			response->addHeader( "Connection", "close" );
			response->addHeader( "Content-Length", "0" );
//			response->dbgTrace();
			co_await response->end();
			CO_RETURN;
		}

		const auto & url = request->getUrl();
		nodecpp::vector<std::pair<nodecpp::string, nodecpp::string>> queryValues;
		nodecpp::Url::parseUrlQueryString( url, queryValues );

		response->writeHeader( 200, "OK" );
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
		co_await response->end(b);

		CO_RETURN;
	}

	using EmitterType = nodecpp::net::SocketTEmitter<>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<>;

};

#endif // NET_SOCKET_H
