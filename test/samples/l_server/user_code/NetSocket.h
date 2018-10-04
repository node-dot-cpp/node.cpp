// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include "../../../../3rdparty/fmt/include/fmt/format.h"
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"
#include "../../../../include/nodecpp/server_t.h"
#include "../../../../include/nodecpp/server_type_list.h"

#include <functional>


using namespace std;
using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	struct Stats
	{
		uint64_t recvSize = 0;
		uint64_t sentSize = 0;
		uint64_t rqCnt;
		uint64_t connCnt = 0;
	};
	Stats stats;

	std::unique_ptr<uint8_t> ptr;
	size_t size = 64 * 1024;
	bool letOnDrain = false;

	using SocketIdType = int;
	using ServerIdType = int;

public:
	MySampleTNode() : srv( [this](OpaqueSocketData& sdata) { return makeSocket(sdata);} ), srvCtrl( [this](OpaqueSocketData& sdata) { return makeCtrlSocket(sdata);} )
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );
		ptr.reset(static_cast<uint8_t*>(malloc(size)));

		srv.on( event::close, [this](bool hadError) {
			print("server: onCloseServer()!\n");
			//serverSockets.clear();
		});
		srv.on( event::connection, [this](net::SocketTBase* socket) {
			print("server: onConnection()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
//			serverSockets.add( socket );
			net::Socket* s = static_cast<net::Socket*>(socket);
			s->on( event::close, [this, s](bool hadError) {
				print("server socket: onCloseServerSocket!\n");
				srv.removeSocket( s );
			});
			s->on( event::data, [this, s](Buffer& buffer) {
				if ( buffer.size() < 2 )
				{
					//printf( "Insufficient data on socket idx = %d\n", *extra );
					s->unref();
					return;
				}
		//		print("server socket: onData for idx %d !\n", *extra );
	
				size_t receivedSz = buffer.begin()[0];
				if ( receivedSz != buffer.size() )
				{
//					printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", *extra, receivedSz, buffer.size() );
					printf( "Corrupted data on socket idx = [??]: received %zd, expected: %zd bytes\n", receivedSz, buffer.size() );
					s->unref();
					return;
				}
	
				size_t requestedSz = buffer.begin()[1];
				if ( requestedSz )
				{
					Buffer reply(requestedSz);
					//buffer.begin()[0] = (uint8_t)requestedSz;
					memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
					s->write(reply.begin(), requestedSz);
				}
	
				stats.recvSize += receivedSz;
				stats.sentSize += requestedSz;
				++(stats.rqCnt);
			});
			s->on( event::end, [this, s]() {
				print("server socket: onEnd!\n");
				const char buff[] = "goodbye!";
				s->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
				s->end();
			});

		});

		srvCtrl.on( event::close, [this](bool hadError) {
			print("server: onCloseServerCtrl()!\n");
			//serverCtrlSockets.clear();
		});
		srvCtrl.on( event::connection, [this](net::SocketTBase* socket) {
			print("server: onConnectionCtrl()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			net::Socket* s = static_cast<net::Socket*>(socket);
			s->on( event::close, [this, s](bool hadError) {
				print("server socket: onCloseServerSocket!\n");
				srvCtrl.removeSocket( s );
			});
			s->on( event::data, [this, s](Buffer& buffer) {
				size_t requestedSz = buffer.begin()[1];
				if ( requestedSz )
				{
					Buffer reply(sizeof(stats));
					stats.connCnt = srv.getSockCount();
					size_t replySz = sizeof(Stats);
					uint8_t* buff = ptr.get();
					memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
					s->write(buff, replySz);
				}
			});
			s->on( event::end, [this, s]() {
				print("server socket: onEnd!\n");
				const char buff[] = "goodbye!";
				s->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
				s->end();
			});
		});

		srv.listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl.listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});
	}

private:
	net::SocketTBase* makeSocket(OpaqueSocketData& sdata) { return new net::Socket( sdata ); }
	net::SocketTBase* makeCtrlSocket(OpaqueSocketData& sdata) { return new net::Socket( sdata ); }

public:
	net::Server srv;
	net::Server srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
