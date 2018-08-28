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
