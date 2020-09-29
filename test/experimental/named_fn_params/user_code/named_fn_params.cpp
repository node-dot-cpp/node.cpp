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

struct Point3D
{
	int x;
	int y;
	int z;
};

class VectorOfPointsWrapper : public m::CollectionWrapperBase
{
	nodecpp::vector<Point>& coll;
	typename nodecpp::vector<Point>::iterator it;

public:
	VectorOfPointsWrapper( nodecpp::vector<Point>& coll_ ) : coll( coll_ ), it( coll.begin() ) {};

	size_t size() const { return coll.size(); }
	bool compose_next_to_gmq( Buffer& b )
	{ 
		if ( it != coll.end() )
		{
			man::point_compose( b, man::x = it->x, man::y = it->y );
			it++;
			return true;
		}
		else
			return false; // no more items
	}
	bool compose_next_to_json( Buffer& b )
	{ 
		if ( it != coll.end() )
		{
			man::point_composeJson( b, man::x = it->x, man::y = it->y );
			it++;
			return true;
		}
		else
			return false; // no more items
	}
	void parse_next_from_gmq( man::impl::Parser& p )
	{
		Point pt;
		man::point_parse( p, man::x = &(pt.x), man::y = &(pt.y)  );
		coll.push_back( pt );
	}
	void parse_next_from_json( man::impl::Parser& p )
	{
		Point pt;
		man::point_parseJson( p, man::x = &(pt.x), man::y = &(pt.y)  );
		coll.push_back( pt );
	}
};

class VectorOfPoints3DWrapper : public m::CollectionWrapperBase
{
	nodecpp::vector<Point3D>& coll;
	typename nodecpp::vector<Point3D>::iterator it;

public:
	VectorOfPoints3DWrapper( nodecpp::vector<Point3D>& coll_ ) : coll( coll_ ), it( coll.begin() ) {};

	size_t size() const { return coll.size(); }
	bool compose_next_to_gmq( Buffer& b )
	{ 
		if ( it != coll.end() )
		{
			man::point3D_compose( b, man::x = it->x, man::y = it->y, man::z = it->z );
			it++;
			return true;
		}
		else
			return false; // no more items
	}
	bool compose_next_to_json( Buffer& b )
	{ 
		if ( it != coll.end() )
		{
			man::point3D_composeJson( b, man::x = it->x, man::y = it->y, man::z = it->z );
			it++;
			return true;
		}
		else
			return false; // no more items
	}
	void parse_next_from_gmq( man::impl::Parser& p )
	{
		Point3D pt;
		man::point3D_parse( p, man::x = &(pt.x), man::y = &(pt.y), man::z = &(pt.z) );
		coll.push_back( pt );
	}
	void parse_next_from_json( man::impl::Parser& p )
	{
		Point3D pt;
		man::point3D_parseJson( p, man::x = &(pt.x), man::y = &(pt.y), man::z = &(pt.z) );
		coll.push_back( pt );
	}
};

int main()
{
	{
		nodecpp::vector<int> vectorOfNumbers = { 0, 1, 2, 3, 4, 5 };
		nodecpp::vector<Point> vectorOfPoints = { {0, 1}, {2, 3}, {4, 5} };
		nodecpp::vector<Point3D> vectorOfPoints3D = { {0, 1, 2}, {3, 4, 5} };
		nodecpp::vector<int> vectorOfNumbersBack;
		nodecpp::vector<Point> vectorOfPointsBack;
		nodecpp::vector<Point3D> vectorOfPoints3DBack;
		nodecpp::Buffer b;
		man::message_one_compose( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3, man::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbers ), man::fifthParam = VectorOfPointsWrapper( vectorOfPoints ), man::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3D ) );

		Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		man::message_one_parse( parser, man::firstParam = &firstParam, man::secondParam = &secondParam, man::thirdParam = &thirdParam, man::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), man::fifthParam = VectorOfPointsWrapper( vectorOfPointsBack ), man::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3DBack ) );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers.size() == vectorOfNumbersBack.size(), "{} vs {}", vectorOfNumbers.size(), vectorOfNumbersBack.size() );
		for ( size_t i=0; i<vectorOfNumbers.size(); ++i )
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers[i] == vectorOfNumbersBack[i], "{} vs {}", vectorOfNumbers[i], vectorOfNumbersBack[i] );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPointsBack.size() == vectorOfPoints.size(), "{} vs {}", vectorOfPointsBack.size(), vectorOfPoints.size() );
		for ( size_t i=0; i<vectorOfPoints.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints[i].x == vectorOfPointsBack[i].x, "{} vs {}", vectorOfPoints[i].x, vectorOfPointsBack[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints[i].y == vectorOfPointsBack[i].y, "{} vs {}", vectorOfPoints[i].y, vectorOfPointsBack[i].y );
		}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3DBack.size() == vectorOfPoints3D.size(), "{} vs {}", vectorOfPoints3DBack.size(), vectorOfPoints3D.size() );
		for ( size_t i=0; i<vectorOfPoints3D.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].x == vectorOfPoints3DBack[i].x, "{} vs {}", vectorOfPoints3D[i].x, vectorOfPoints3DBack[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].y == vectorOfPoints3DBack[i].y, "{} vs {}", vectorOfPoints3D[i].y, vectorOfPoints3DBack[i].y );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].z == vectorOfPoints3DBack[i].z, "{} vs {}", vectorOfPoints3D[i].z, vectorOfPoints3DBack[i].z );
		}
	}

	{
		nodecpp::vector<int> vectorOfNumbers = { 0, 1, 2, 3, 4, 5 };
		nodecpp::vector<Point> vectorOfPoints = { {0, 1}, {2, 3}, {4, 5} };
		nodecpp::vector<Point3D> vectorOfPoints3D = { {0, 1, 2}, {3, 4, 5} };
		nodecpp::vector<int> vectorOfNumbersBack;
		nodecpp::vector<Point> vectorOfPointsBack;
		nodecpp::vector<Point3D> vectorOfPoints3DBack;
		nodecpp::Buffer b;

		man::message_one_composeJson( b, man::firstParam = 1, man::secondParam = nodecpp::string("def"), man::thirdParam = 3, man::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), man::fifthParam = VectorOfPointsWrapper( vectorOfPointsBack ), man::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3DBack ) );
		printf( "%s\n", b.begin() );

		Parser parser( b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		man::message_one_parseJson( parser, man::firstParam = &firstParam, man::secondParam = &secondParam, man::thirdParam = &thirdParam, man::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), man::fifthParam = VectorOfPointsWrapper( vectorOfPointsBack ), man::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3DBack ) );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers.size() == vectorOfNumbersBack.size(), "{} vs {}", vectorOfNumbers.size(), vectorOfNumbersBack.size() );
		for ( size_t i=0; i<vectorOfNumbers.size(); ++i )
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers[i] == vectorOfNumbersBack[i], "{} vs {}", vectorOfNumbers[i], vectorOfNumbersBack[i] );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPointsBack.size() == vectorOfPoints.size(), "{} vs {}", vectorOfPointsBack.size(), vectorOfPoints.size() );
		for ( size_t i=0; i<vectorOfPoints.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints[i].x == vectorOfPointsBack[i].x, "{} vs {}", vectorOfPoints[i].x, vectorOfPointsBack[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints[i].y == vectorOfPointsBack[i].y, "{} vs {}", vectorOfPoints[i].y, vectorOfPointsBack[i].y );
		}
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3DBack.size() == vectorOfPoints3D.size(), "{} vs {}", vectorOfPoints3DBack.size(), vectorOfPoints3D.size() );
		for ( size_t i=0; i<vectorOfPoints3D.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].x == vectorOfPoints3DBack[i].x, "{} vs {}", vectorOfPoints3D[i].x, vectorOfPoints3DBack[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].y == vectorOfPoints3DBack[i].y, "{} vs {}", vectorOfPoints3D[i].y, vectorOfPoints3DBack[i].y );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].z == vectorOfPoints3DBack[i].z, "{} vs {}", vectorOfPoints3D[i].z, vectorOfPoints3DBack[i].z );
		}
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
