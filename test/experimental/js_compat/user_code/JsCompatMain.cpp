// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>
#include "colors.h"
#include "../js-compat-tests/misc_tests.h"
#include "./maps/america.h"
#include "./maps/rainbow.h"
#include "./maps/random.h"
#include "./maps/zebra.h"
#include "./custom/trap.h"

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

	JSVar america = require<America>();
	printf("\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", america.toString().c_str());

	JSVar rainbow = require<Rainbow>();
	printf("\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", rainbow.toString().c_str());

	JSVar random = require<Random>();
	printf(random("\n RANDOM TEST \n").toString().c_str());

	JSVar zebra = require<Zebra>();
	printf(zebra("\n ZEBRA TEST \n").toString().c_str());

	//JSVar trap = require<Trap>();
	//printf(trap("TRAP TEST\n").toString().c_str());


	JSVar styles = require<Styles>();
	printf( "\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", styles.toString().c_str() );

	auto& colors = import<Colors>();
	printf( "\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", colors.toString().c_str() );

	auto& miscTests = import<MiscTests>();
//	printf( "\n~~~~~~~~~~~~~~~%s\n~~~~~~~~~~~~~~~\n\n", miscTests.toString().c_str() );


	//JS TEST
	/*require('./extendStringPrototype')();
	console.log('drop the bass'.trap);*/
	return 0;
}

