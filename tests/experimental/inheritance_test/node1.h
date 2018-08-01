#ifndef NODE1_H
#define NODE1_H

#include "../../../include/nodecpp/common.h"
#include "socket_registration_common.h"

class MyNodeOne : public NodeBase
{
	using SocketIdType = int;

public:
	using noid = void;
	MyNodeOne() : sock1( this ), 
		clientSocket( this ),
		clientSocket2(this),
		clientSocket3(this)
	{
		printf( "MyNodeOne::MyNodeOne()\n" );
	}

	virtual void main()
	{
		printf( "MyNodeOne::main()\n" );
		registerSocket( &sock1 );
		registerSocket( &clientSocket );
		registerSocket( &clientSocket2 );
		registerSocket( &clientSocket3 );
	}

	void onConnect(const void* extra) { printf( "MyNodeOne::onConnect()\n" ); }
	void onClose(const void* extra,bool) { printf( "MyNodeOne::onClose()\n" ); }
	void onData(const void* extra, nodecpp::Buffer&) { printf( "MyNodeOne::onData()\n" ); }
	void onDrain(const void* extra) { printf( "MyNodeOne::onDrain()\n" ); }
	void onError(const void* extra, nodecpp::Error&) { printf( "MyNodeOne::onError()\n" ); }
	void onEnd(const void* extra) { printf( "MyNodeOne::onEnd()\n" ); }

	void onWhateverConnect(const void* extra) { printf( "MyNodeOne::onWhateverConnect()\n" ); }
	void onWhateverClose(const void* extra, bool) { printf( "MyNodeOne::onWhateverClose()\n" ); }
	void onWhateverData(const void* extra, nodecpp::Buffer&) { printf( "MyNodeOne::onWhateverData()\n" ); }
	void onWhateverDrain(const void* extra) { printf( "MyNodeOne::onWhateverDrain()\n" ); }
	void onWhateverError(const void* extra, nodecpp::Error&) { printf( "MyNodeOne::onWhateverError()\n" ); }
	void onWhateverEnd(const void* extra) { printf( "MyNodeOne::onWhateverEnd()\n" ); }

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

	nodecpp::SocketN<MyNodeOne,SocketIdType> sock1;
	nodecpp::SocketN<MyNodeOne,SocketIdType,nodecpp::OnConnect<&MyNodeOne::onWhateverConnect_2>,nodecpp::OnClose<&MyNodeOne::onWhateverClose_2>> clientSocket;
	nodecpp::SocketN<MyNodeOne,void,nodecpp::OnEnd<&MyNodeOne::onWhateverEnd>> clientSocket2;
	nodecpp::SocketN<MyNodeOne,void,nodecpp::OnConnect<&MyNodeOne::onWhateverConnect_3>,nodecpp::OnClose<&MyNodeOne::onWhateverClose_3>> clientSocket3;
};


#endif // NODE1_H
