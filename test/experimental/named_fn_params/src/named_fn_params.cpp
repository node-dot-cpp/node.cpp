// named_fn_params.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "named_params_core.h"
#include "test_2.h"


int main()
{
    m::test2Call_A( m::firstParam = 1, m::secondParam = std::string("def"), m::thirdParam = 3 );
    m::test2Call_A( m::thirdParam = 3, m::firstParam = 1, m::secondParam = std::string("def") );
    m::test2Call_A( m::thirdParam = 3, m::secondParam = std::string("def"), m::firstParam = 1 );
    printf( "=====================\n" );
    m::test2Call_A( m::secondParam = std::string("def"), m::thirdParam = 3 );
    m::test2Call_A( m::thirdParam = 3, m::firstParam = 1 );
    m::test2Call_A( m::secondParam = std::string("def"), m::firstParam = 1 );
    printf( "=====================\n" );
    m::test2Call_B( m::firstParam = std::string("abc"), m::secondParam = std::string("def"), m::thirdParam = std::string("ghi") );
    m::test2Call_B( m::thirdParam = std::string("ghi"), m::firstParam = std::string("abc"), m::secondParam = std::string("def") );
    m::test2Call_B( m::thirdParam = std::string("ghi"), m::secondParam = std::string("def"), m::firstParam = std::string("abc") );
    printf( "=====================\n" );
    /*m::test2Call_B( m::secondParam = std::string("abc"), m::thirdParam = 3 );
    m::test2Call_B( m::thirdParam = 3, m::firstParam = 1 );
    m::test2Call_B( m::secondParam = std::string("abc"), m::firstParam = 1 );*/
    m::test2Call_B( m::secondParam = std::string("def"), m::thirdParam = std::string("ghi") );
    m::test2Call_B( m::thirdParam = std::string("ghi"), m::firstParam = std::string("abc") );
    m::test2Call_B( m::secondParam = std::string("def"), m::firstParam = std::string("abc") );
    printf( "=====================\n" );
//    m::test2Call_B( m::thirdParam = std::string("ghi"), m::secondParam = std::string("def"), m::thirdParam = std::string("abc") );
//    m::test2Call_B( m::thirdParam = std::string("ghi"), m::secondParam = std::string("def"), m::thirdParam = 3 );
//    m::test2Call_B( m::thirdParam = 3, m::secondParam = std::string("def"), m::thirdParam = std::string("ghi") );
}
