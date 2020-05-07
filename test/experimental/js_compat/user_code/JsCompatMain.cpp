// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>

#include "./styles.h"
#include "./setMap.h"



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


	JSVar styles = require<Styles>();
	printf(styles(styles("COLOR TEST\n", "whiteBG"),"cyan").toString().c_str());

	JSVar setMap = require<SetMap>();
	printf(setMap("MAPPED TEST", "america").toString().c_str());

	

	//auto& colors = import<Colors>();
	//printf( "\n~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~\n\n", colors.toString().c_str() );

	//auto& miscTests = import<MiscTests>();
//	printf( "\n~~~~~~~~~~~~~~~%s\n~~~~~~~~~~~~~~~\n\n", miscTests.toString().c_str() );


	//JS TEST
	/*require('./extendStringPrototype')();
	console.log('drop the bass'.trap);*/
	return 0;
}

