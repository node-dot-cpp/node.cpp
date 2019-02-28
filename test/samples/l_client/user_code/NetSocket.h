// NetSocket.h : sample of user-defined code

#ifndef NET_SOCKET_H
#define NET_SOCKET_H
#include "../../../../include/nodecpp/common.h"


#include <fmt/format.h>
#include "../../../../include/nodecpp/socket_type_list.h"
#include "../../../../include/nodecpp/socket_t_base.h"

//#define DO_INTEGRITY_CHECKING // TODO: consider making project-level definition


using namespace nodecpp;
using namespace fmt;

class MySampleTNode : public NodeBase
{
	size_t recvSize = 0;
	size_t recvReplies = 0;
	Buffer buf;

	net::Socket clientSock;

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

	uint32_t myID = 0; // uint32 is quite sufficient for testing purposes
	uint32_t lastPacketSN = 0;
	uint8_t lastRequestedSize = 0;
#endif

public:
	MySampleTNode()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleTNode::MySampleTNode()" );
	}

	virtual void main()
	{
		nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "MySampleLambdaOneNode::main()" );

#ifdef DO_INTEGRITY_CHECKING
		clientSock.on(event::connect, [this]() { 
			buf.writeInt8( 10, 0 );
			buf.writeInt8( ++lastRequestedSize, 1 );
			buf.append( reinterpret_cast<uint8_t*>(&myID), sizeof(myID) );
			buf.append( reinterpret_cast<uint8_t*>(&(++lastPacketSN)), sizeof(myID) );
			clientSock.write(buf);
			buf.clear();
		});

		clientSock.on(event::data, [this](Buffer& buffer) { 
			++recvReplies;
			if ( ( recvReplies & 0xFFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );
			recvSize += buffer.size();

			if ( buffer.size() < 11 )
			{
				NODECPP_TRACE( "Insufficient data on socket id = {}: received just {} bytes", myID, buffer.size() );
				clientSock.unref();
				return;
			}

			uint16_t chS = Fletcher16( buffer.begin() + 2, buffer.size() - 2 );
			if ( *reinterpret_cast<uint16_t*>(buffer.begin()) != chS )
			{
				NODECPP_TRACE0( "Checksum failed" );
				clientSock.unref();
				return;
			}

			uint8_t requestedSz = buffer.begin()[2];
			uint32_t recClientID = *reinterpret_cast<uint32_t*>(buffer.begin()+3);
			uint32_t receivedPN = *reinterpret_cast<uint32_t*>(buffer.begin()+7);

			if ( myID == 0 )
			{
				myID = recClientID;
				NODECPP_TRACE( "Server-assigned ID: {}", myID );
			}
			else if ( myID != recClientID )
			{
				NODECPP_TRACE( "Packet for wrong ID received: myID = {}, in-packet for-id = {}", myID, recClientID );
				clientSock.unref();
				return;
			}

			if ( receivedPN != lastPacketSN )
			{
				NODECPP_TRACE( "Reply to packet with unexpected sequential num = {} received (expected {})", receivedPN, lastPacketSN);
				clientSock.unref();
				return;
			}

			if ( requestedSz != lastRequestedSize )
			{
				NODECPP_TRACE( "Reply size as indicated in header ({}) is not that has been requested (expected {})", requestedSz, lastRequestedSize);
				clientSock.unref();
				return;
			}

			if ( requestedSz + 11 != buffer.size() )
			{
				NODECPP_TRACE( "Reply size ({}) is not that has been requested (expected {})", requestedSz, lastRequestedSize);
				clientSock.unref();
				return;
			}

			buf.writeInt8( 10, 0 );
			buf.writeInt8( ++lastRequestedSize, 1 );
			buf.append( reinterpret_cast<uint8_t*>(&myID), sizeof(myID) );
			buf.append( reinterpret_cast<uint8_t*>(&(++lastPacketSN)), sizeof(myID) );
			clientSock.write(buf);
			buf.clear();
		});
#else
		clientSock.on(event::connect, [this]() { 
			buf.writeInt8( 2, 0 );
			buf.writeInt8( 1, 1 );
			clientSock.write(buf);
		});

		clientSock.on(event::data, [this](Buffer& buffer) { 
			++recvReplies;
			if ( ( recvReplies & 0xFFFF ) == 0 )
				nodecpp::log::log<nodecpp::module_id, nodecpp::log::LogLevel::info>( "[{}] MySampleTNode::onData(), size = {}", recvReplies, buffer.size() );
			recvSize += buffer.size();
			buf.writeInt8( 2, 0 );
			buf.writeInt8( (uint8_t)recvReplies | 1, 1 );
			clientSock.write(buf);
		});
#endif

		clientSock.connect(2000, "127.0.0.1");
	}

	using EmitterType = nodecpp::net::SocketTEmitter<net::SocketO, net::Socket>;
};

#endif // NET_SOCKET_H
