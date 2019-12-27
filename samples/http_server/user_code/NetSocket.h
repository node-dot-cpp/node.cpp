// NetSocket.h : sample of user-defined code for an http server

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#include <nodecpp/http_server.h>
#include <nodecpp/misc.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	using ServerType = net::HttpServer<MySampleTNode>;
	safememory::owning_ptr<ServerType> srv; 

public:
	void main()
	{
		srv = net::createHttpServer<ServerType>( [](safememory::soft_ptr<net::IncomingHttpMessageAtServer> request, safememory::soft_ptr<net::HttpServerResponse> response){
			if ( request->getMethod() == "GET" || request->getMethod() == "HEAD" ) {
				response->writeHead(200, {{"Content-Type", "text/xml"}});
				auto queryValues2 = Url::parseUrlQueryString( request->getUrl() );
				auto value = queryValues2["value"];
				if (value == ""){
					response->end("no value specified");
				} else {
					auto chksm = nodecpp::Fletcher16( value.c_str(), value.size() );
					response->end( nodecpp::format( "{} ({})", value, chksm ) );
				}
			} else {
				response->writeHead( 405, "Method Not Allowed", {{"Connection", "close" }} );
				response->end();
			}
		});
		srv->listen(2000, "0.0.0.0", 5000);
	}
};

#endif // NET_SOCKET_H
