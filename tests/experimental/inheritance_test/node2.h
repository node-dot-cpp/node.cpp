#ifndef NODE2_H
#define NODE2_H

#include "common.h"
#include "socket.h"

class MyNodeTwo : public NodeBase
{
	MySocket<MyNodeTwo> sock2;

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

	void onConnect() { printf( "MyNodeTwo::onConnect()\n" ); }
	void onClose() { printf( "MyNodeTwo::onClose()\n" ); }
	void onWhateverConnect() { printf( "MyNodeTwo::onWhateverConnect()\n" ); }
	void onWhateverClose() { printf( "MyNodeTwo::onWhateverClose()\n" ); }
	struct SocketHandlers_
	{
		static constexpr void (MyNodeTwo::*onConnect)() = &MyNodeTwo::onWhateverConnect;
		static constexpr void (MyNodeTwo::*onClose)() = &MyNodeTwo::onWhateverClose;
	};
	MySocket<MyNodeTwo,SocketHandlers_> sock1;
};


#endif // NODE1_H
