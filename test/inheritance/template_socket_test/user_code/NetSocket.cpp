// NetSocket.cpp : sample of user-defined code


#include "../../../../src/infrastructure.h"
#include "NetSocket.h"

#ifdef USING_L_SOCKETS
static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
#endif // USING_L_SOCKETS
#ifdef USING_O_SOCKETS
static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
#endif // USING_O_SOCKETS


#ifdef USING_T_SOCKETS
static thread_local Infrastructure<MySampleTNode::EmitterType> infra;

size_t connectToInfra(net::SocketTBase* t, int typeId, const char* ip, uint16_t port)
{
	return infra.getNetSocket().appConnect(t, typeId, ip, port);
}
#endif // USING_T_SOCKETS

void runLoop()
{
	MySampleTNode node;
	node.main();
	void* nodePtr = nullptr;
	runInfraLoop2( infra );
}

NetSocketManagerBase& getNetSocket() { return infra.getNetSocket(); }
