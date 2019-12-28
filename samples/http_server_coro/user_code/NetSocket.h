// NetSocket.h : sample of user-defined code for an http server

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

	virtual nodecpp::handler_ret_type main()
	{
		log.add( stdout );
		log.setLevel( nodecpp::LogLevel::debug );
		log.setGuaranteedLevel( nodecpp::LogLevel::warning );

		srv = nodecpp::net::createHttpServer<ServerType>();
		srv->listen(2000, "0.0.0.0", 5000);

		nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request;
		nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response;

		try { 
			for(;;) { 
				co_await srv->a_request(request, response); 
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				co_await processRequest( request, response );
			} 
		} 
		catch (...) {
			log.error( "failed to process request" );
		}

		CO_RETURN;
	}

	nodecpp::handler_ret_type processRequest( nodecpp::safememory::soft_ptr<nodecpp::net::IncomingHttpMessageAtServer> request, nodecpp::safememory::soft_ptr<nodecpp::net::HttpServerResponse> response )
	{
			if ( request->getMethod() == "GET" || request->getMethod() == "HEAD" ) {
				response->writeHead(200, {{"Content-Type", "text/xml"}});
				auto queryValues = Url::parseUrlQueryString( request->getUrl() );
				auto& value = queryValues["value"];
				if (value.toStr() == ""){
					co_await response->end("no value specified");
				} else {
					co_await response->end( nodecpp::format( "{}", value.toStr() ) );
				}
			} else {
				response->writeHead( 405, "Method Not Allowed", {{"Connection", "close" }} );
				co_await response->end();
			}

		CO_RETURN;
	}
};

#endif // NET_SOCKET_H
