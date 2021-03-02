// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>

#include "./styles.h"
#include "./setMap.h"
#include "../js-compat-tests/misc_tests.h"



nodecpp::stdvector<nodecpp::stdstring> argv;

int main( int argc, char *argv_[] )
{
	for ( int i=0; i<argc; ++i )
		argv.push_back( argv_[i] );

#ifdef NODECPP_USE_IIBMALLOC
	nodecpp::iibmalloc::g_AllocManager.initialize();
#endif
	// so far we will used old good printf()
	nodecpp::log::Log log;
//	log.level = nodecpp::log::LogLevel::info;
	log.add( stdout );
	nodecpp::logging_impl::currentLog = &log;
	nodecpp::logging_impl::instanceId = 0;
	log.setCriticalLevel( nodecpp::log::LogLevel::err );

	try {
		JSVar styles = require<Styles>();
		printf( "%s", styles(styles("COLOR TEST\n", "whiteBG"),"cyan").toString().c_str() );

		JSVar setMap = require<SetMap>();
		printf( "%s", setMap("MAPPED TEST\n", "america").toString().c_str() );
		printf( "%s", setMap("MAPPED TEST\n", "rainbow").toString().c_str() );
		printf( "%s", setMap("MAPPED TEST\n", "random").toString().c_str() );
		printf( "%s", setMap("MAPPED TEST\n", "zebra").toString().c_str() );
		printf( "%s", setMap("DROP THE BASS\n", "trap").toString().c_str() );


	

		//auto& colors = import<Colors>();
		//printf( "\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", colors.toString().c_str() );

		auto& miscTests = import<MiscTests>();
	//	printf( "\n~~~~~~~~~~~~~~~%s\n~~~~~~~~~~~~~~~\n\n", miscTests.toString().c_str() );

		//JS TEST
		/*require('./extendStringPrototype')();
		console.log('drop the bass'.trap);*/
	}
	catch (nodecpp::error::error e)
	{
		e.log(log, nodecpp::log::LogLevel::fatal);
	}
	catch (...)
	{
		nodecpp::log::default_log::fatal("Unknown error happened. About to exit...");
		return 0;
	}
	nodecpp::log::default_log::fatal( "About to exit..." );
	return 0;
}

