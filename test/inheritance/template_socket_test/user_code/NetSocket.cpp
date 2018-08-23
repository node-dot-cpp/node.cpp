// NetSocket.cpp : sample of user-defined code


#include "../../../../src/infrastructure.h"
#include "NetSocket.h"

#if (defined USING_L_SOCKETS) || (defined USING_O_SOCKETS)
//static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
//static NodeRegistrator<Runnable<MySampleInheritanceOneNode>> noname( "MySampleInheritanceOneNode" );
#endif // USING_L_SOCKETS || USING_O_SOCKETS


#ifdef USING_T_SOCKETS

//static NodeRegistrator<Runnable<MySampleTNode>> noname( "MySampleTemplateNode" );
static NodeRegistrator<Runnable<MySampleTNode>, Infrastructure<MySampleTNode::EmitterType>> noname( "MySampleTemplateNode" );

/*size_t connectToInfra(net::SocketTBase* t, int typeId, const char* ip, uint16_t port)
{
	return NodeRegistrator<Runnable<MySampleTNode>, Infrastructure<MySampleTNode::EmitterType>>::infraPtr->getNetSocket().appConnect(t, typeId, ip, port);
}*/

NetSocketManagerBase& getNetSocket() { return NodeRegistrator<Runnable<MySampleTNode>, Infrastructure<MySampleTNode::EmitterType>>::infraPtr->getNetSocket(); }

#endif // USING_T_SOCKETS
