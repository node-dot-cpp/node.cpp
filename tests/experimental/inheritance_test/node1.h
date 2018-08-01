#ifndef NODE1_H
#define NODE1_H

#include "../../../include/nodecpp/common.h"
#include "socket_registration_common.h"

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
	void onClose(bool) { printf( "MyNodeOne::onClose()\n" ); }
	void onData(nodecpp::Buffer&) { printf( "MyNodeOne::onData()\n" ); }
	void onDrain() { printf( "MyNodeOne::onDrain()\n" ); }
	void onError(nodecpp::Error&) { printf( "MyNodeOne::onError()\n" ); }
	void onEnd() { printf( "MyNodeOne::onEnd()\n" ); }

	void onWhateverConnect() { printf( "MyNodeOne::onWhateverConnect()\n" ); }
	void onWhateverClose(bool) { printf( "MyNodeOne::onWhateverClose()\n" ); }
	void onWhateverData(nodecpp::Buffer&) { printf( "MyNodeOne::onWhateverData()\n" ); }
	void onWhateverDrain() { printf( "MyNodeOne::onWhateverDrain()\n" ); }
	void onWhateverError(nodecpp::Error&) { printf( "MyNodeOne::onWhateverError()\n" ); }
	void onWhateverEnd() { printf( "MyNodeOne::onWhateverEnd()\n" ); }

	void onWhateverConnect_2(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverConnect_2()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverClose_2(const SocketIdType* extra, bool) { printf( "MyNodeOne::onWhateverClose_2()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverData_2(const SocketIdType* extra, nodecpp::Buffer&) { printf( "MyNodeOne::onWhateverData_2()\n" ); }
	void onWhateverDrain_2(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverDrain_2()\n" ); }
	void onWhateverError_2(const SocketIdType* extra, nodecpp::Error&) { printf( "MyNodeOne::onWhateverError_2()\n" ); }
	void onWhateverEnd_2(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverEnd_2()\n" ); }

	void onWhateverConnect_3(const void* extra) { printf( "MyNodeOne::onWhateverConnect_3()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverClose_3(const void* extra,bool) { printf( "MyNodeOne::onWhateverClose_3()%s\n", extra ? "+ extra data" : "" ); }
	void onWhateverData_3(const SocketIdType* extra, nodecpp::Buffer&) { printf( "MyNodeOne::onWhateverData_3()\n" ); }
	void onWhateverDrain_3(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverDrain_3()\n" ); }
	void onWhateverError_3(const SocketIdType* extra, nodecpp::Error&) { printf( "MyNodeOne::onWhateverError_3()\n" ); }
	void onWhateverEnd_3(const SocketIdType* extra) { printf( "MyNodeOne::onWhateverEnd_3()\n" ); }

	struct SocketHandlers_
	{
		static constexpr void (MyNodeOne::*onConnect)() = &MyNodeOne::onWhateverConnect;
		static constexpr void (MyNodeOne::*onClose)(bool) = &MyNodeOne::onWhateverClose;
		static constexpr void (MyNodeOne::*onData)(nodecpp::Buffer&) = &MyNodeOne::onWhateverData;
		static constexpr void (MyNodeOne::*onDrain)() = &MyNodeOne::onWhateverDrain;
		static constexpr void (MyNodeOne::*onError)(nodecpp::Error&) = &MyNodeOne::onWhateverError;
		static constexpr void (MyNodeOne::*onEnd)() = &MyNodeOne::onWhateverEnd;
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
