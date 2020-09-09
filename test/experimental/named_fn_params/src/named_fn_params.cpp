// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
#include "test_2.h"


int main()
{
	nodecpp::Buffer b;
	m::test2Call_C_compose( b, m::firstParam = 1, m::secondParam = std::string("def"), m::thirdParam = 3 );
	m::test2Call_C_compose( b, m::secondParam = std::string("def"), m::thirdParam = 3 );
	m::test2Call_C_compose( b, m::thirdParam = (uint64_t)3, m::firstParam = (uint64_t)(uint64_t)(-1) );
	m::test2Call_C_compose( b, m::secondParam = std::string("def"), m::firstParam = 1 );
}
