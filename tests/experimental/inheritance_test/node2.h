#ifndef NODE2_H
#define NODE2_H

#include "../../../include/nodecpp/common.h"
#include "socket_registration_common.h"

class MyNodeTwo : public NodeBase
{
	struct SockIData
	{
		uint64_t id;
		void* whatever;
	};
	using SocketIdType = SockIData;

public:
	MyNodeTwo() : sock1( this ), 
		sock2( this )
	{
		printf( "MyNodeTwo::MyNodeTwo()\n" );
	}

	virtual void main()
	{
		printf( "MyNodeTwo::main()\n" );
		registerSocket( &sock1 );
		registerSocket( &sock2 );
	}

	void onConnect(const void* extra) { printf( "MyNodeTwo::onConnect()\n" ); }
	void onClose(const void* extra,bool) { printf( "MyNodeTwo::onClose()\n" ); }
	void onData(const void* extra, nodecpp::Buffer&) { printf( "MyNodeTwo::onData()\n" ); }
	void onDrain(const void* extra) { printf( "MyNodeTwo::onDrain()\n" ); }
	void onError(const void* extra, nodecpp::Error&) { printf( "MyNodeTwo::onError()\n" ); }
	void onEnd(const void* extra) { printf( "MyNodeTwo::onEnd()\n" ); }

	void onWhateverConnect(const SocketIdType* extra) { printf( "MyNodeTwo::onWhateverConnect()\n" ); }
	void onWhateverClose(const SocketIdType* extra, bool) { printf( "MyNodeTwo::onWhateverClose()\n" ); }
	void onWhateverData(const SocketIdType* extra, nodecpp::Buffer&) { printf( "MyNodeTwo::onWhateverData()\n" ); }
	void onWhateverDrain(const SocketIdType* extra) { printf( "MyNodeTwo::onWhateverDrain()\n" ); }
	void onWhateverError(const SocketIdType* extra, nodecpp::Error&) { printf( "MyNodeTwo::onWhateverError()\n" ); }
	void onWhateverEnd(const SocketIdType* extra) { printf( "MyNodeTwo::onWhateverEnd()\n" ); }

	SocketN<MyNodeTwo,SocketIdType> sock1;
	SocketN<MyNodeTwo,SocketIdType,OnConnect<&MyNodeTwo::onWhateverConnect>,OnClose<&MyNodeTwo::onWhateverClose>> sock2;
};


#endif // NODE1_H
