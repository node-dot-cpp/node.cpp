// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include <nodecpp/common.h>


#include <fmt/format.h>
#include <nodecpp/socket_type_list.h>
#include <nodecpp/socket_t_base.h>
#include <nodecpp/server_t.h>
#include <nodecpp/server_type_list.h>

#include <functional>

//#define DO_INTEGRITY_CHECKING // TODO: consider making project-level definition


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

	/*std::unique_ptr<uint8_t[]> ptr;
	size_t size = 64 * 1024;*/
	bool letOnDrain = false;

#ifdef DO_INTEGRITY_CHECKING
	uint16_t Fletcher16( uint8_t *data, int count )
	{
	   uint16_t sum1 = 0;
	   uint16_t sum2 = 0;
	   int index;

	   for( index = 0; index < count; ++index )
	   {
		  sum1 = (sum1 + data[index]) % 255;
		  sum2 = (sum2 + sum1) % 255;
	   }

	   return (sum2 << 8) | sum1;
	}

	uint32_t clientIDBase = 0; // uint32 is quite sufficient for testing purposes
#endif

public:
	MySampleTNode()
	{
		printf( "MySampleTNode::MySampleTNode()\n" );
	}

	virtual void main()
	{
		printf( "MySampleLambdaOneNode::main()\n" );
//		ptr.reset(static_cast<uint8_t*>(malloc(size)));
//		ptr.reset(new uint8_t[size]);

		srv.on( event::close, [this](bool hadError) {
			print("server: onCloseServer()!\n");
		});
		srv.on( event::connection, [this](net::Socket* socket) {
			print("server: onConnection()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			socket->on( event::close, [this, socket](bool hadError) {
				print("server socket: onCloseServerSocket!\n");
				socket->unref();
				srv.removeSocket( socket );
			});
#ifdef DO_INTEGRITY_CHECKING
			uint32_t clientID = ++clientIDBase;
			socket->on( event::data, [this, socket, clientID](Buffer& buffer) {
				if ( buffer.size() < 10 )
				{
					NODECPP_TRACE( "Insufficient data on socket id = {}", clientID );
					socket->unref();
					return;
				}
		//		print("server socket: onData for idx %d !\n", *extra );
	
				size_t receivedSz = buffer.begin()[0];
				if ( receivedSz != buffer.size() )
				{
					NODECPP_TRACE( "Corrupted data on socket id = {}: received {}, expected: {} bytes", clientID, receivedSz, buffer.size() );
					socket->unref();
					return;
				}
	
				size_t requestedSz = buffer.begin()[1];
				uint32_t receivedID = *reinterpret_cast<uint32_t*>(buffer.begin()+2); // yes, we do not care about endiannes... it is just what we've sent them
				if ( !(receivedID == 0 || receivedID == clientID) )
				{
					NODECPP_TRACE( "Corrupted data on socket with client id = {}: received client id is: {}", clientID, receivedID );
					socket->unref();
					return;
				}
				uint32_t receivedPN = *reinterpret_cast<uint32_t*>(buffer.begin()+6); // yes, we do not care about endiannes... it is just what we've sent them
				size_t replySz = requestedSz + 11;
				Buffer reply(replySz);
				reply.begin()[2] = (uint8_t)requestedSz;
				*reinterpret_cast<uint32_t*>(reply.begin()+3) = clientID;
				*reinterpret_cast<uint32_t*>(reply.begin()+7) = receivedPN;
				if ( requestedSz )
					memset(reply.begin() + 11, (uint8_t)requestedSz, requestedSz);
				*reinterpret_cast<uint16_t*>(reply.begin()) = Fletcher16( reply.begin() + 2, requestedSz + 9 ); // TODO: endianness
				socket->write(reply.begin(), replySz);
	
				stats.recvSize += receivedSz;
				stats.sentSize += requestedSz + 9;
				++(stats.rqCnt);
			});
#else
			socket->on( event::data, [this, socket](Buffer& buffer) {
				if ( buffer.size() < 2 )
				{
					//printf( "Insufficient data on socket idx = %d\n", *extra );
					socket->unref();
					return;
				}
		//		print("server socket: onData for idx %d !\n", *extra );
	
				size_t receivedSz = buffer.readUInt8(0);
				if ( receivedSz != buffer.size() )
				{
//					printf( "Corrupted data on socket idx = %d: received %zd, expected: %zd bytes\n", *extra, receivedSz, buffer.size() );
					printf( "Corrupted data on socket idx = [??]: received %zd, expected: %zd bytes\n", receivedSz, buffer.size() );
					socket->unref();
					return;
				}
	
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(requestedSz);
					for ( size_t i=0; i<(uint8_t)requestedSz; ++i )
						reply.appendUint8( 0 );
					//buffer.begin()[0] = (uint8_t)requestedSz;
					//memset(reply.begin(), (uint8_t)requestedSz, requestedSz);
//					socket->write(reply.begin(), requestedSz);
					socket->write(reply);
				}
	
				stats.recvSize += receivedSz;
				stats.sentSize += requestedSz;
				++(stats.rqCnt);
			});
#endif
			socket->on( event::end, [this, socket]() {
				print("server socket: onEnd!\n");
//				const char buff[] = "goodbye!";
//				socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
				Buffer b;
				b.appendString( "goodbye!", sizeof( "goodbye!" ) );
				socket->write( b );
				socket->end();
			});

		});

		srvCtrl.on( event::close, [this](bool hadError) {
			print("server: onCloseServerCtrl()!\n");
			//serverCtrlSockets.clear();
		});
		srvCtrl.on( event::connection, [this](net::Socket* socket) {
			print("server: onConnectionCtrl()!\n");
			//srv.unref();
			NODECPP_ASSERT( socket != nullptr ); 
			socket->on( event::close, [this, socket](bool hadError) {
				print("server socket: onCloseServerSocket!\n");
				srvCtrl.removeSocket( socket );
			});
			socket->on( event::data, [this, socket](Buffer& buffer) {
				size_t requestedSz = buffer.readUInt8(1);
				if ( requestedSz )
				{
					Buffer reply(sizeof(stats));
					stats.connCnt = srv.getSockCount();
					size_t replySz = sizeof(Stats);
					//uint8_t* buff = ptr.get();
					//memcpy( buff, &stats, replySz ); // naive marshalling will work for a limited number of cases
					reply.append( &stats, replySz );
					socket->write(reply);
				}
			});
			socket->on( event::end, [this, socket]() {
				print("server socket: onEnd!\n");
//				const char buff[] = "goodbye!";
//				socket->write(reinterpret_cast<const uint8_t*>(buff), sizeof(buff));
				Buffer b;
				b.appendString( "goodbye!", sizeof( "goodbye!" ) );
				socket->write( b );
				socket->end();
			});
		});

		srv.listen(2000, "127.0.0.1", 5, [](size_t, net::Address){});
		srvCtrl.listen(2001, "127.0.0.1", 5, [](size_t, net::Address){});
	}

public:
	net::Server srv;
	net::Server srvCtrl;


	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
	using EmitterTypeForServer = nodecpp::net::ServerTEmitter<net::ServerO, net::Server>;
};

#endif // NET_SOCKET_H
