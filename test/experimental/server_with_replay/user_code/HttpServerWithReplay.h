// HttpServerWithReplay.h : sample of user-defined code for an http server with record and replay

#ifndef HTTP_SERVER_WITH_REPLAY_H
#define HTTP_SERVER_WITH_REPLAY_H

#include <nodecpp/common.h>
#include <nodecpp/http_server.h>

using namespace nodecpp;

#if 0
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
#endif

class MySampleTNode : public NodeBase
{
	size_t callCtr = 0;
public:

	class MyHttpServer : public net::HttpServer<MySampleTNode>
	{
		nodecpp::log::Log log;
	public:
		MyHttpServer() {
			log.add( stdout );
			nodecpp::logging_impl::currentLog = &log;
			nodecpp::logging_impl::instanceId = 0;
			log.setCriticalLevel( nodecpp::log::LogLevel::err );
		}
		MyHttpServer(MySampleTNode* node) : HttpServer<MySampleTNode>(node) {}
		virtual ~MyHttpServer() {}
	};

	using SockTypeServerSocket = net::SocketBase;

	using ServerType = MyHttpServer;
	safememory::owning_ptr<ServerType> srv; 

	MySampleTNode()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleTNode::MySampleTNode()" );
	}

	virtual handler_ret_type main()
	{
		log::default_log::info( log::ModuleID(nodecpp_module_id), "MySampleLambdaOneNode::main()" );

		srv = net::createHttpServer<ServerType>();

		srv->listen(2000, "127.0.0.1", 5000);

		safememory::soft_ptr<net::IncomingHttpMessageAtServer> request;
		safememory::soft_ptr<net::HttpServerResponse> response;
		try { 
			for(;;) { 
				co_await srv->a_request(request, response); 
				log::default_log::info( log::ModuleID(nodecpp_module_id), "receiving request # {}", callCtr++ );
				Buffer b1(0x1000);
				co_await request->a_readBody( b1 );
				yetSimpleProcessing( request, response );
			} 
		} 
		catch (...) { // TODO: what?
		}

		CO_RETURN;
	}

	handler_ret_type yetSimpleProcessing( safememory::soft_ptr<net::IncomingHttpMessageAtServer> request, safememory::soft_ptr<net::HttpServerResponse> response )
	{
		if ( request->getMethod() == "GET" || request->getMethod() == "HEAD" ) {
			response->writeHead(200, {{"Content-Type", "text/xml"}});
			auto queryValues = Url::parseUrlQueryString( request->getUrl() );
			auto& value = queryValues["value"];
			if (value.toStr() == ""){
				log::default_log::info( log::ModuleID(nodecpp_module_id), "   replying \"no value specified\"" );
				co_await response->end("no value specified");
			} else if (value.toStr() == "close"){
				log::default_log::info( log::ModuleID(nodecpp_module_id), "   proceeding to closing" );
				srv->close();
				co_await response->end("closing server...");
			} else {
				log::default_log::info( log::ModuleID(nodecpp_module_id), "   replying \"{}\"", value.toStr() );
				co_await response->end( value.toStr() );
			}
		} else {
			response->writeHead( 405, "Method Not Allowed" );
			co_await response->end();
		}

		CO_RETURN;
	}

	using EmitterType = net::SocketTEmitter<>;
	using EmitterTypeForServer = net::ServerTEmitter<>;


};


#endif // HTTP_SERVER_WITH_REPLAY_H
