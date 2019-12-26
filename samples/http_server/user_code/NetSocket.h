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
	using ServerType = net::HttpServer<MySampleTNode>;
	safememory::owning_ptr<ServerType> srv; 

public:
	virtual handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		srv = net::createHttpServer<ServerType>();
		srv->listen(2000, "0.0.0.0", 5000);

		srv->on( event::HttpRequest::name, [](safememory::soft_ptr<net::IncomingHttpMessageAtServer> request, safememory::soft_ptr<net::HttpServerResponse> response){
			// unexpected method
			if ( !(request->getMethod() == "GET" || request->getMethod() == "HEAD" ) )
			{
				response->writeHeader( 405, "Method Not Allowed" );
				response->addHeader( "Connection", "close" );
				response->addHeader( "Content-Length", "0" );
				response->end(); // TODOX: non-awaitable version
				return;
			}

			const auto & url = request->getUrl();
			vector<std::pair<nodecpp::string, nodecpp::string>> queryValues;
			Url::parseUrlQueryString( url, queryValues );

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
			response->addHeader( "Content-Length", format( "{}", b.size() ) );
			response->end(b); // TODOX: non-awaitable version
		});

		CO_RETURN;
	}

//	using EmitterType = net::SocketTEmitter<>;
//	using EmitterTypeForServer = net::ServerTEmitter<>;

};

#endif // NET_SOCKET_H
