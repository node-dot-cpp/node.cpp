// NetSocket.cpp : sample of user-defined code


#include "NetSocket.h"

//static NodeRegistrator<MySampleLambdaOneNode> noname( "MySampleLambdaOneNode" );
#ifdef USING_O_SOCKETS
static NodeRegistrator<MySampleInheritanceOneNode> noname( "MySampleInheritanceOneNode" );
#endif // USING_O_SOCKETS
