// NetSocket.cpp : sample of user-defined code


#include "../../../../src/infrastructure.h"
#include "NetSocket.h"

//static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
#ifdef USING_O_SOCKETS
//static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
#endif // USING_O_SOCKETS


static thread_local Infrastructure<MySampleTNode::EmitterType> infra;

#ifdef USE_TEMPLATE_SOCKETS
size_t connectToInfra(net::SocketTBase* t, int typeId, const char* ip, uint16_t port)
{
	return infra.getNetSocket().appConnect(t, typeId, ip, port);
}
#endif // USE_TEMPLATE_SOCKETS

void runLoop()
{
	MySampleTNode node;
	node.main();
	void* nodePtr = nullptr;
//	void* ptr = nullptr;
//	nodecpp::Error e;
//	reinterpret_cast<typename T1::userNodeType*>(nodePtr)->*(T1::Handlers::onError)(reinterpret_cast<T1*>(ptr->getPtr())->getExtra(), e);
//	(reinterpret_cast<MySampleTNode*>(nodePtr)->*MySampleTNode::SockType_1::Handlers::onError)(reinterpret_cast<MySampleTNode::SockType_1*>(ptr)->getExtra(), e);
	runInfraLoop2( infra );
}

NetSocketManagerBase& getNetSocket() { return infra.getNetSocket(); }
