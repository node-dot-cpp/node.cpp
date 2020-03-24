// NetSocket.cpp : sample of user-defined code


#include <nodecpp/common.h>
#include <nodecpp/js_compat.h>
#include "styles.h"

void test_one()
{
	//nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id), "test_one()" );
	printf( "test_one()\n" );

	auto arr1 = nodecpp::js::JSArray::makeJSArray({
		nodecpp::js::JSVar::makeJSVar({1.}),
		nodecpp::js::JSVar::makeJSVar({2.}),
		nodecpp::js::JSVar::makeJSVar({3.})
		});
	auto arr2 = nodecpp::js::JSArray::makeJSArray({
		4,
		5,
		6
		});

	printf( "%s\n", arr1->toString().c_str() );
	printf( "%s\n", arr2->toString().c_str() );

	auto var1 = nodecpp::js::JSVar::makeJSVar( std::move(arr1) );
	auto var2 = nodecpp::js::JSVar::makeJSVar( std::move(arr2) );
	printf( "%s\n", var1->toString().c_str() );

	auto obj1 = nodecpp::js::JSObject::makeJSObject( { std::make_pair( nodecpp::string("one"), std::move(var1) ), std::make_pair( nodecpp::string("two"), std::move(var2) ) } );
	printf( "%s\n", obj1->toString().c_str() );

	auto obj2 = nodecpp::js::JSObject::makeJSObject( { 
		std::make_pair( nodecpp::string("one"), nodecpp::js::JSVar::makeJSVar( nodecpp::js::JSArray::makeJSArray({ 11, 12, 13 }) ) ), 
		std::make_pair( nodecpp::string("two"), nodecpp::js::JSVar::makeJSVar( nodecpp::js::JSArray::makeJSArray({ 21, 22, 23 }) ) )
		} );
	printf( "%s\n", obj2->toString().c_str() );

	auto obj3 = nodecpp::js::JSObject::makeJSObject( { 
		std::make_pair( "one", nodecpp::js::JSVar::makeJSVar( { 31, 32, 33 } ) ), 
		std::make_pair( "two", nodecpp::js::JSVar::makeJSVar( { 41, 42, 43 } ) )
		} );
	printf( "%s\n", obj3->toString().c_str() );

	auto obj4 = nodecpp::js::JSObject::makeJSObject( { 
		{ "one", nodecpp::js::JSVar::makeJSVar( { 131, 132, 133 } ) }, 
		{ "two", nodecpp::js::JSVar::makeJSVar( { 141, 142, 143 } ) }
		} );
	printf( "%s\n", obj4->toString().c_str() );

	printf( "%s\n", (*obj4)["two"]->toString().c_str() );

/* JS code:
var tmp = [];
tmp.close = "(close)";
tmp["999999"] = "(tmp[999999])";
tmp["1000000"] = "(tmp[1000000])";
tmp[1000001] = "(tmp[10000001])";
tmp[2] = "(tmp[2])";
tmp[0] = "(tmp[0])";
tmp.open = "(open)";

var cnt = 0;

Object.keys(tmp).forEach(function(key) {
  cnt += 1;
  var val = tmp[key];
  console.log( "" + cnt + ": " + key + ":" + val + " | " + tmp.indexOf( val ) );
});
*///////////////////////////////////////////////////////////////////////////////

	size_t ctr = 0;
	auto tmp = nodecpp::js::JSArray::makeJSArray({1,2,3,4,5});
	tmp->add( "six", nodecpp::js::JSVar::makeJSVar( "_six_" ) );
	tmp->add( "7", nodecpp::js::JSVar::makeJSVar( "_7_" ) );
//	nodecpp::js::JSArray::forEach( [&ctr](nodecpp::string key ){
	tmp->forEach( [&ctr](nodecpp::string key ){
		++ctr;
		printf( nodecpp::format( "{}: {}\n", ctr, key ).c_str() );
//		printf( key.c_str() );
		});
}

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

	test_one();

	return 0;
}

