// HttpServerWithReplay.h : sample of user-defined code for an http server with record and replay

#ifndef HTTP_SERVER_WITH_REPLAY_H
#define HTTP_SERVER_WITH_REPLAY_H

#include <nodecpp/common.h>
#include <nodecpp/http_server.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	using ServerType = net::HttpServer<MySampleTNode>;	
	safememory::owning_ptr<ServerType> srv; 

public:
	void main()
	{
		srv = net::createHttpServer<ServerType>( [this](net::IncomingHttpMessageAtServer& request, net::HttpServerResponse& response){
			if ( request.getMethod() == "GET" || request.getMethod() == "HEAD" ) {
				response.writeHead(200, {{"Content-Type", "text/xml"}});
				auto queryValues = Url::parseUrlQueryString( request.getUrl() );
				auto& value = queryValues["value"];
				if (value.toStr() == ""){
					response.end("no value specified");
				} else if (value.toStr() == "close"){
					srv->close();
					response.end("closing server...");
				} else {
					response.end( value.toStr() );
				}
			} else {
				response.writeHead( 405, "Method Not Allowed" );
				response.end();
			}
		});
		srv->listen(2000, "0.0.0.0", 5000);
	}
};

#endif // HTTP_SERVER_WITH_REPLAY_H
