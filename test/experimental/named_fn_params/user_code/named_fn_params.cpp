// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
//#include "../generated/marshalling.h"
#include "../manual/test.h"

using m::Parser;

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
	bool compose_next( m::Composer& composer )
	{ 
		if ( it != coll.end() )
		{
			m::point_compose( composer, m::x = it->x, m::y = it->y );
			it++;
			return true;
		}
		else
			return false; // no more items
	}

	void size_hint( size_t count )
	{
		// Note: here we can do some preliminary steps based on a number of collection elements declared in the message (if such a number exists for a particular protocol) 
		if ( count != unknown_size )
			coll.reserve( count );
	}
	void parse_next( m::Parser& p )
	{
		Point pt;
		m::point_parse( p, m::x = &(pt.x), m::y = &(pt.y)  );
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
	bool compose_next( m::Composer& composer )
	{ 
		if ( it != coll.end() )
		{
			m::point3D_compose( composer, m::x = it->x, m::y = it->y, m::z = it->z );
			it++;
			return true;
		}
		else
			return false; // no more items
	}

	void size_hint( size_t count )
	{
		// Note: here we can do some preliminary steps based on a number of collection elements declared in the message (if such a number exists for a particular protocol) 
		if ( count != unknown_size )
			coll.reserve( count );
	}
	void parse_next( m::Parser& p )
	{
		Point3D pt;
		m::point3D_parse( p, m::x = &(pt.x), m::y = &(pt.y), m::z = &(pt.z) );
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
		m::Composer composer( m::Proto::GMQ, b );
		m::message_one_compose( composer, m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3, m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbers ), m::fifthParam = VectorOfPointsWrapper( vectorOfPoints ), m::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3D ) );

		Parser parser( m::Proto::GMQ, b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::message_one_parse( parser, m::firstParam = &firstParam, m::secondParam = &secondParam, m::thirdParam = &thirdParam, m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), m::fifthParam = VectorOfPointsWrapper( vectorOfPointsBack ), m::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3DBack ) );

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
		m::Composer composer( m::Proto::JSON, b );

		m::message_one_compose( composer, m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3, m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbers ), m::fifthParam = VectorOfPointsWrapper( vectorOfPoints ), m::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3D ) );
		b.appendUint8( 0 );
		printf( "%s\n", b.begin() );

		Parser parser( m::Proto::JSON, b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::message_one_parse( parser, m::firstParam = &firstParam, m::secondParam = &secondParam, m::thirdParam = &thirdParam, m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), m::fifthParam = VectorOfPointsWrapper( vectorOfPointsBack ), m::sixthParam = VectorOfPoints3DWrapper( vectorOfPoints3DBack ) );

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

