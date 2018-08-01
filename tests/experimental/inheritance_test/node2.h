#ifndef NODE2_H
#define NODE2_H

#include "../../../include/nodecpp/common.h"
#include "socket_registration_common.h"

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
	void onClose(bool) { printf( "MyNodeTwo::onClose()\n" ); }
	void onData(nodecpp::Buffer&) { printf( "MyNodeTwo::onData()\n" ); }
	void onDrain() { printf( "MyNodeTwo::onDrain()\n" ); }
	void onError(nodecpp::Error&) { printf( "MyNodeTwo::onError()\n" ); }
	void onEnd() { printf( "MyNodeTwo::onEnd()\n" ); }

	void onWhateverConnect() { printf( "MyNodeTwo::onWhateverConnect()\n" ); }
	void onWhateverClose(bool) { printf( "MyNodeTwo::onWhateverClose()\n" ); }
	void onWhateverData(nodecpp::Buffer&) { printf( "MyNodeTwo::onWhateverData()\n" ); }
	void onWhateverDrain() { printf( "MyNodeTwo::onWhateverDrain()\n" ); }
	void onWhateverError(nodecpp::Error&) { printf( "MyNodeTwo::onWhateverError()\n" ); }
	void onWhateverEnd() { printf( "MyNodeTwo::onWhateverEnd()\n" ); }

	struct SocketHandlers_
	{
		static constexpr void (MyNodeTwo::*onConnect)() = &MyNodeTwo::onWhateverConnect;
		static constexpr void (MyNodeTwo::*onClose)(bool) = &MyNodeTwo::onWhateverClose;
		static constexpr void (MyNodeTwo::*onData)(nodecpp::Buffer&) = &MyNodeTwo::onWhateverData;
		static constexpr void (MyNodeTwo::*onDrain)() = &MyNodeTwo::onWhateverDrain;
		static constexpr void (MyNodeTwo::*onError)(nodecpp::Error&) = &MyNodeTwo::onWhateverError;
		static constexpr void (MyNodeTwo::*onEnd)() = &MyNodeTwo::onWhateverEnd;
	};
	MySocket<MyNodeTwo,SocketHandlers_> sock1;
};


#endif // NODE1_H
