// NetSocket.cpp : sample of user-defined code


#include "../../../../src/infrastructure.h"
#include "NetSocket.h"

#if (defined USING_L_SOCKETS) || (defined USING_O_SOCKETS)
//static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
//static NodeRegistrator<Runnable<MySampleInheritanceOneNode>> noname( "MySampleInheritanceOneNode" );
#endif // USING_L_SOCKETS || USING_O_SOCKETS


#ifdef USING_T_SOCKETS

//static thread_local Infrastructure<MySampleTNode::EmitterType> infraPtr;
thread_local Infrastructure<MySampleTNode::EmitterType>* infraPtr;

size_t connectToInfra(net::SocketTBase* t, int typeId, const char* ip, uint16_t port)
{
	return infraPtr->getNetSocket().appConnect(t, typeId, ip, port);
}

void runLoop()
{
	MySampleTNode node;
	node.main();
	runInfraLoop2<Infrastructure<MySampleTNode::EmitterType>>();
}

NetSocketManagerBase& getNetSocket() { return infraPtr->getNetSocket(); }

#endif // USING_T_SOCKETS
