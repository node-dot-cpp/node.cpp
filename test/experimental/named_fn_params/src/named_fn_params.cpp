// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
#include "test_2.h"


int main()
{
	/*{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b, m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3 );

		m::impl::Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::test2Call_C_parse( parser, m::firstParam = &firstParam, m::secondParam = &secondParam, m::thirdParam = &thirdParam );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
	}*/

	{
		nodecpp::Buffer b;
		m::test2Call_C_compose_to_json( b, m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3 );
		printf( "%s\n", b.begin() );

		/*m::impl::Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::test2Call_C_parse( parser, m::firstParam = &firstParam, m::secondParam = &secondParam, m::thirdParam = &thirdParam );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );*/
	}
/*
	{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b, m::secondParam = nodecpp::string("def"), m::thirdParam = 3 );
	}

	{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b, m::thirdParam = (uint64_t)3, m::firstParam = (uint64_t)(uint64_t)(-1) );
	}

	{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b, m::secondParam = nodecpp::string("def"), m::firstParam = 1 );
	}

	{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b, m::secondParam = nodecpp::string("def") );
	}

	{
		nodecpp::Buffer b;
		m::test2Call_C_compose( b );
	}*/
}
