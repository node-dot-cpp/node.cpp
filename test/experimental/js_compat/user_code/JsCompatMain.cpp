// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>
#include "styles.h"

int main( int argc, char *argv_[] )
{
#ifdef NODECPP_USE_IIBMALLOC
	g_AllocManager.initialize();
#endif
	// so far we will used old good printf()
/*	nodecpp::log::Log log;
//	log.level = nodecpp::log::LogLevel::info;
	log.add( stdout );
	nodecpp::logging_impl::currentLog = &log;
	nodecpp::logging_impl::instanceId = 0;*/

	auto styles_4 = require<Styles>();	
	printf( "%s\n", styles_4->toString().c_str() );

	printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	soft_ptr<JSVar> asVar = soft_ptr<JSVar>(*styles_4);
	printf( "%s\n", asVar->toString().c_str() );

	return 0;
}

