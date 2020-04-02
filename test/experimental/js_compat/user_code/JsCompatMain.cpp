// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>
#include "colors.h"

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

	soft_ptr<JSVar> styles = *require<Styles>();
	printf( "%s\n", styles->toString().c_str() );

	printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	soft_ptr<JSVar> colors = *require<Colors>();
	printf( "%s\n", colors->toString().c_str() );

	return 0;
}

