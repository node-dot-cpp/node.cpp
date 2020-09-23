// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
//#include "../generated/marshalling.h"
#include "../manual/test.h"

using m::impl::Parser;

struct Point
{
	int x;
	int y;
};

int main()
{
	{
		nodecpp::vector<Point> vec1 = { {0, 1}, {2, 3}, {4, 5} };
		nodecpp::vector<int> vec2 = { 0, 1, 2, 3, 4, 5 };
		nodecpp::vector<Point> vecout;
		nodecpp::vector<int> vecout2;
		nodecpp::Buffer b;
//		man::message_one_compose( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3, man::forthParam = [vec1]( Buffer&b, size_t idx){ if ( idx >= vec.size() ) return 0; man::point_compose( b, man::x = vec[idx].x, man::y = vec[idx].y ); return 1; } );
//		man::message_one_compose( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3, man::forthParam = [vec1]( Buffer&b, size_t idx){ if ( idx >= vec.size() ) return 0; int xx = vec[idx].x; int yy = vec[idx].y; man::point_compose( b, man::x = xx, man::y = yy ); return 1; } );
		man::message_one_compose( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3, man::forthParam = m::CollectionWrapper( vec2 ) );

		Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
//		man::message_one_parse( parser, man::firstParam = &firstParam, man::secondParam = &secondParam, man::thirdParam = &thirdParam, man::forthParam = [&vec1](size_t idx){ return 0; } );
		man::message_one_parse( parser, man::firstParam = &firstParam, man::secondParam = &secondParam, man::thirdParam = &thirdParam, man::forthParam = m::CollectionWrapper( vecout2 ) );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
		/*NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vec.size() == vecout.size(), "{} vs {}", vec.size(), vecout.size() );
		for ( size_t i=0; i<vec.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vec[i].x == vecout[i].x, "{} vs {}", vec[i].x, vecout[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vec[i].y == vecout[i].y, "{} vs {}", vec[i].y, vecout[i].y );
		}*/
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vec2.size() == vecout2.size(), "{} vs {}", vec2.size(), vecout2.size() );
		for ( size_t i=0; i<vec2.size(); ++i )
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vec2[i] == vecout2[i], "{} vs {}", vec2[i], vecout2[i] );
	}

	{
		nodecpp::Buffer b;
		man::message_one_composeJson( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3 );
		printf( "%s\n", b.begin() );

		Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		man::message_one_parseJson( parser, man::firstParam = &firstParam, man::secondParam = &secondParam, man::thirdParam = &thirdParam );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
	}
/*
	{
		nodecpp::Buffer b;
		man::test2Call_C_compose( b, man::secondParam = nodecpp::string("def"), man::thirdParam = 3 );
	}

	{
		nodecpp::Buffer b;
		man::test2Call_C_compose( b, man::thirdParam = (uint64_t)3, man::firstParam = (uint64_t)(uint64_t)(-1) );
	}

	{
		nodecpp::Buffer b;
		man::test2Call_C_compose( b, man::secondParam = nodecpp::string("def"), man::firstParam = 1 );
	}

	{
		nodecpp::Buffer b;
		man::test2Call_C_compose( b, man::secondParam = nodecpp::string("def") );
	}

	{
		nodecpp::Buffer b;
		man::test2Call_C_compose( b );
	}*/
}
