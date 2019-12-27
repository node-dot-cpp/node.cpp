// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <nodecpp/common.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/server_type_list.h>
#include <nodecpp/http_server.h>

using namespace nodecpp;

class MySampleTNode : public NodeBase
{
	using ServerType = net::HttpServer<MySampleTNode>;
	safememory::owning_ptr<ServerType> srv; 

public:
	void main()
	{
		srv = net::createHttpServer<ServerType>( [](safememory::soft_ptr<net::IncomingHttpMessageAtServer> request, safememory::soft_ptr<net::HttpServerResponse> response){
			// unexpected method
			if ( request->getMethod() == "GET" || request->getMethod() == "HEAD" ) {
				auto queryValues = std::move( Url::parseUrlQueryString( request->getUrl() ) );

				response->writeHead( 200, "OK", {{"Content-Type", "text/xml"}} );
				Buffer b;
				if ( queryValues.empty() ) {
					b.append( "no value specified", sizeof( "undefined" ) - 1 );
					response->addHeader( "Connection", "close" );
				}
				else {
					for ( auto entry: queryValues )
						if ( entry.first == "value" ) {
							b.appendString( entry.second );
							b.appendUint8( ',' );
						}
					if ( b.size() ) {
						b.trim( 1 );
						response->addHeader( "Connection", "keep-alive" );
					}
					else {
						b.append( "no value specified", sizeof( "no value specified" ) - 1 );
						response->addHeader( "Connection", "close" );
					}
				}
				response->addHeader( "Content-Length", format( "{}", b.size() ) );
				response->dbgTrace();
				response->end(b); // TODOX: non-awaitable version
			} else {
				response->writeHead( 405, "Method Not Allowed", {{"Connection", "close" }} );
				response->end(); // TODOX: non-awaitable version
			}
		});
		srv->listen(2000, "0.0.0.0", 5000);
	}
};

#endif // NET_SOCKET_H
