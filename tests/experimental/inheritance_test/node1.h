#ifndef NODE1_H
#define NODE1_H

#include "common.h"
#include "socket.h"

class MyNodeOne : public NodeBase
{
	using SocketIdType = int;
	MySocket<MyNodeOne> sock2;

public:
	using noid = void;
	MyNodeOne() : sock1( this ), 
		sock2( this ),
		clientSocket(this),
		clientSocket2(this)
	{
		printf( "MyNodeOne::MyNodeOne()\n" );
	}

	virtual void main()
	{
		printf( "MyNodeOne::main()\n" );
		registerSocket( &sock1 );
		registerSocket( &sock2 );
		registerSocket( &clientSocket );
		registerSocket( &clientSocket2 );
	}

	void onConnect() { printf( "MyNodeOne::onConnect()\n" ); }
	void onClose() { printf( "MyNodeOne::onClose()\n" ); }
	void onWhateverConnect() { printf( "MyNodeOne::onWhateverConnect()\n" ); }
	void onWhateverClose() { printf( "MyNodeOne::onWhateverClose()\n" ); }
	void onWhateverConnect_2(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverConnect_2()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverClose_2(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverClose_2()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverConnect_3(const void* extra) { printf( "MyNodeOne::onWhateverConnect_3()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverClose_3(const void* extra) { printf( "MyNodeOne::onWhateverClose_3()%s\n", extra ? "+ extra data" : "" ); }
	struct SocketHandlers_
	{
		static constexpr void (MyNodeOne::*onConnect)() = &MyNodeOne::onWhateverConnect;
		static constexpr void (MyNodeOne::*onClose)() = &MyNodeOne::onWhateverClose;
	};
	MySocket<MyNodeOne,SocketHandlers_> sock1;
#if 0
	SocketN< MyNodeOne,
	  /*OnConnect<&MyNodeOne::onWhateverConnect_2>,*/
	  OnClose<MyNodeOne, &MyNodeOne::onWhateverClose_2>
//	  &MyNode::onWhateverClose_2
	> clientSocket;
#endif
	SocketN<MyNodeOne,SocketIdType,OnConnect<&MyNodeOne::onWhateverConnect_2>,OnClose<&MyNodeOne::onWhateverClose_2>> clientSocket;
//	SocketN<MyNodeOne,void/*,OnConnect<&MyNodeOne::onWhateverConnect_2>*/,OnClose<&MyNodeOne::onWhateverClose_2>> clientSocket2;
//	SocketN<MyNodeOne,SocketIdType/*,OnConnect<&MyNodeOne::onWhateverConnect_2>*/,OnClose<&MyNodeOne::onWhateverClose_2>> clientSocket2;
	SocketN<MyNodeOne,void,OnConnect<&MyNodeOne::onWhateverConnect_3>,OnClose<&MyNodeOne::onWhateverClose_3>> clientSocket2;
};


#endif // NODE1_H
