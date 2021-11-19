// test_co_await.cpp : Defines the entry point for the console application.
//

#include <nodecpp/common.h>
#include "tests.h"

int main()
{
	test_coro_presence();
	fmt::print( "\n-----------------\n" );
	processing_loop();

	return 0;
}
