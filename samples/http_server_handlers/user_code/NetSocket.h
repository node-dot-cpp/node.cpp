// NetSocket.h : sample of user-defined code for an http server

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/http_server.h>
#include <nodecpp/logging.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	Log log;

public:
	using ServerType = net::HttpServer<MySampleTNode>;

	safememory::owning_ptr<ServerType> srv; 

	handler_ret_type main()
	{
		log.add( stdout );
		log.setLevel( LogLevel::debug );
		log.setGuaranteedLevel( LogLevel::warning );

		net::HttpServerBase::addHttpHandler<ServerType, net::HttpServerBase::Handler::IncomingRequest, &MySampleTNode::processRequest>(this);

		srv = net::createHttpServer<ServerType>();
		srv->listen(2000, "0.0.0.0", 5000);

		CO_RETURN;
	}

	handler_ret_type processRequest( safememory::soft_ptr<ServerType> srv, safememory::soft_ptr<net::IncomingHttpMessageAtServer> request, safememory::soft_ptr<net::HttpServerResponse> response )
	{
			if ( request->getMethod() == "GET" || request->getMethod() == "HEAD" ) {
				response->writeHead(200, {{"Content-Type", "text/xml"}});
				auto queryValues = Url::parseUrlQueryString( request->getUrl() );
				auto& value = queryValues["value"];
				if (value.toStr() == ""){
					co_await response->end("no value specified");
				} else if (value.toStr() == "close") {
					srv->close();
					co_await response->end( value.toStr() );
				} else {
					co_await response->end( value.toStr() );
				}
			} else {
				response->writeHead( 405, "Method Not Allowed" );
				co_await response->end();
			}

		CO_RETURN;
	}
};

#endif // NET_SOCKET_H
