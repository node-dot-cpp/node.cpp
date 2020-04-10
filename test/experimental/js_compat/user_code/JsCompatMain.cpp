// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>
#include "colors.h"
#include "../js-compat-tests/misc_tests.h"

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

//	JSVar styles = require<Styles>();
//	printf( "\n~~~~~~~~~~~~~~~%s\n~~~~~~~~~~~~~~~\n\n", styles.toString().c_str() );

	/*printf( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	JSVar colors = require<Colors>();
	printf( "%s\n", colors.toString().c_str() );*/

	JSVar miscTests = require<MiscTests>();
//	printf( "\n~~~~~~~~~~~~~~~%s\n~~~~~~~~~~~~~~~\n\n", miscTests.toString().c_str() );

	return 0;
}

