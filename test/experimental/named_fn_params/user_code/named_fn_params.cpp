// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
#include "../generated/marshalling.h"
//#include "../manual/test.h"

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

int main()
{
	{
		nodecpp::vector<int> vectorOfNumbers = { 0, 1, 2, 3, 4, 5 };
		nodecpp::vector<Point> vectorOfPoints;
		nodecpp::vector<Point3D> vectorOfPoints3D = { {0, 1, 2}, {3, 4, 5} };
		nodecpp::vector<int> vectorOfNumbersBack;
		nodecpp::vector<Point> vectorOfPointsBack;
		nodecpp::vector<Point3D> vectorOfPoints3DBack;
		nodecpp::Buffer b;
		m::Composer composer( m::Proto::GMQ, b );
		m::message_one_compose( composer, 
			m::sixthParam = m::CollectionWrapperForComposing( [&vectorOfPoints3D]() { return vectorOfPoints3D.size(); }, [&vectorOfPoints3D](m::Composer& c, size_t ordinal){ m::point3D_compose( c, m::x = vectorOfPoints3D[ordinal].x, m::y = vectorOfPoints3D[ordinal].y, m::z = vectorOfPoints3D[ordinal].z );} ), 
			m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3, 
			m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbers ) );

		m::Parser parser( m::Proto::GMQ, b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::message_one_parse( parser, 
			m::firstParam = &firstParam, m::thirdParam = &thirdParam, 
			m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), 
			m::sixthParam = m::CollactionWrapperForParsing( nullptr, [&vectorOfPoints3DBack](m::Parser& p, size_t ordinal){ Point3D pt; m::point3D_parse( p, m::x = &(pt.x), m::y = &(pt.y), m::z = &(pt.z) ); vectorOfPoints3DBack.push_back( pt );} ), 
			m::fifthParam = m::CollactionWrapperForParsing( [&vectorOfPointsBack](size_t sz){vectorOfPointsBack.reserve( sz );}, [&vectorOfPointsBack](m::Parser& p, size_t ordinal){ Point pt; m::point_parse( p,  m::x = &(pt.x), m::y = &(pt.y) ); vectorOfPointsBack.push_back( pt );} ), 
			m::secondParam = &secondParam );

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers.size() == vectorOfNumbersBack.size(), "{} vs {}", vectorOfNumbers.size(), vectorOfNumbersBack.size() );
		for ( size_t i=0; i<vectorOfNumbers.size(); ++i )
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers[i] == vectorOfNumbersBack[i], "{} vs {}", vectorOfNumbers[i], vectorOfNumbersBack[i] );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPointsBack.size() == 0, "indeed: {}", vectorOfPointsBack.size() );
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
		nodecpp::vector<Point3D> vectorOfPoints3D = { {0, 1, 2}, {3, 4, 5} };
		nodecpp::vector<int> vectorOfNumbersBack;
		nodecpp::vector<Point> vectorOfPointsBack;
		nodecpp::vector<Point3D> vectorOfPoints3DBack;
		nodecpp::Buffer b;
		m::Composer composer( m::Proto::JSON, b );

		m::message_one_compose( composer, 
			m::sixthParam = m::CollectionWrapperForComposing( [&vectorOfPoints3D]() { return vectorOfPoints3D.size(); }, [&vectorOfPoints3D](m::Composer& c, size_t ordinal){ m::point3D_compose( c, m::x = vectorOfPoints3D[ordinal].x, m::y = vectorOfPoints3D[ordinal].y, m::z = vectorOfPoints3D[ordinal].z );} ), 
			m::firstParam = 1, m::secondParam = nodecpp::string("def"), m::thirdParam = 3, 
			m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbers ) );
		b.appendUint8( 0 );
		printf( "%s\n", b.begin() );

		m::Parser parser( m::Proto::JSON, b.begin(), b.size() );
		int firstParam = -1;
		nodecpp::string secondParam = "";
		int thirdParam = -1;
		m::message_one_parse( parser, 
			m::firstParam = &firstParam, m::secondParam = &secondParam, m::thirdParam = &thirdParam, 
			m::forthParam = m::SimpleTypeCollectionWrapper( vectorOfNumbersBack ), 
			m::sixthParam = m::CollactionWrapperForParsing( nullptr, [&vectorOfPoints3DBack](m::Parser& p, size_t ordinal){ Point3D pt; m::point3D_parse( p, m::x = &(pt.x), m::y = &(pt.y), m::z = &(pt.z) ); vectorOfPoints3DBack.push_back( pt );} ), 
			m::fifthParam = m::CollactionWrapperForParsing( [&vectorOfPointsBack](size_t sz){vectorOfPointsBack.reserve( sz );}, [&vectorOfPointsBack](m::Parser& p, size_t ordinal){ Point pt; m::point_parse( p,  m::x = &(pt.x), m::y = &(pt.y) ); vectorOfPointsBack.push_back( pt );} )
		);

		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, firstParam == 1, "Indeed: {}", firstParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, secondParam == "def", "Indeed: {}", secondParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, thirdParam == 3, "Indeed: {}", thirdParam );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers.size() == vectorOfNumbersBack.size(), "{} vs {}", vectorOfNumbers.size(), vectorOfNumbersBack.size() );
		for ( size_t i=0; i<vectorOfNumbers.size(); ++i )
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfNumbers[i] == vectorOfNumbersBack[i], "{} vs {}", vectorOfNumbers[i], vectorOfNumbersBack[i] );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPointsBack.size() == 0, "indeed: {}", vectorOfPointsBack.size() );
		NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3DBack.size() == vectorOfPoints3D.size(), "{} vs {}", vectorOfPoints3DBack.size(), vectorOfPoints3D.size() );
		for ( size_t i=0; i<vectorOfPoints3D.size(); ++i )
		{
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].x == vectorOfPoints3DBack[i].x, "{} vs {}", vectorOfPoints3D[i].x, vectorOfPoints3DBack[i].x );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].y == vectorOfPoints3DBack[i].y, "{} vs {}", vectorOfPoints3D[i].y, vectorOfPoints3DBack[i].y );
			NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, vectorOfPoints3D[i].z == vectorOfPoints3DBack[i].z, "{} vs {}", vectorOfPoints3D[i].z, vectorOfPoints3DBack[i].z );
		}
	}
}
