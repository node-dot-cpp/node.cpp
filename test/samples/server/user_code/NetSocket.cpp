// NetSocket.cpp : sample of user-defined code


#include "../../../../src/infrastructure.h"
#include "NetSocket.h"

#if (defined USING_L_SOCKETS) || (defined USING_O_SOCKETS)
//static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
//static NodeRegistrator<Runnable<MySampleInheritanceOneNode>> noname( "MySampleInheritanceOneNode" );
#endif // USING_L_SOCKETS || USING_O_SOCKETS


#ifdef USING_T_SOCKETS

//static NodeRegistrator<Runnable<MySampleTNode>, Infrastructure<MySampleTNode::EmitterType>> noname( "MySampleTemplateNode" );
static NodeRegistrator<Runnable<MySampleTNode>> noname( "MySampleTemplateNode" );

#endif // USING_T_SOCKETS

/*void test()
{
	owning_ptr<nodecpp::net::Socket> p = make_owning<nodecpp::net::Socket>();
	soft_ptr<nodecpp::net::SocketBase> p1 = p;
	soft_ptr<nodecpp::net::Socket> p2 = soft_ptr_static_cast<nodecpp::net::Socket,nodecpp::net::SocketBase>(p1);
}*/
