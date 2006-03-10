///////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////
 
// BOOST
#define BOOST_TEST_MODULE envelope_class_test
#include <boost/test/unit_test.hpp>

// GEOS
#include <geos/geom.h>

// Test Suite
BOOST_AUTO_TEST_SUITE( envelope_class_test );

BOOST_AUTO_TEST_CASE( envelope_constructors_test )
{
    // 1 - default constructor
    geos::Envelope e1;
    
    BOOST_CHECK( e1.isNull() );
    BOOST_CHECK_EQUAL( e1.getWidth(), 0 );
    BOOST_CHECK_EQUAL( e1.getHeight(), 0 );

    // 2
    geos::Envelope e2(100, 200, 100, 200);
    
    BOOST_CHECK( !e2.isNull() );
    BOOST_CHECK_EQUAL( e2.getMinX(), 100 );
    BOOST_CHECK_EQUAL( e2.getMaxX(), 200 );
    BOOST_CHECK_EQUAL( e2.getMinY(), 100 );
    BOOST_CHECK_EQUAL( e2.getMaxY(), 200 );
    BOOST_CHECK_EQUAL( e2.getWidth(), 100 );
    BOOST_CHECK_EQUAL( e2.getHeight(), 100 );

    // 3 - copy constructor
    geos::Envelope e3(e2);
    
    BOOST_CHECK( !e3.isNull() );
    BOOST_CHECK_EQUAL( e3.getMinX(), 100 );
    BOOST_CHECK_EQUAL( e3.getMaxX(), 200 );
    BOOST_CHECK_EQUAL( e3.getMinY(), 100 );
    BOOST_CHECK_EQUAL( e3.getMaxY(), 200 );
    BOOST_CHECK_EQUAL( e3.getWidth(), 100 );
    BOOST_CHECK_EQUAL( e3.getHeight(), 100 );
    BOOST_CHECK( e3 == e2 );
    BOOST_CHECK( e3.equals( &e2 ) );

} // envelope_constructors_test

BOOST_AUTO_TEST_CASE( envelope_null_test )
{
    // 1 - Test isNull() and setToNull() functions
    geos::Envelope e1(100, 200, 100, 200);

    BOOST_CHECK( !e1.isNull() );
    e1.setToNull();
    BOOST_CHECK( e1.isNull() );

} // envelope_null_test

BOOST_AUTO_TEST_CASE( envelope_equals_test )
{
    // 1 - Test equals() function
    geos::Envelope e1(100, 200, 100, 200);
    geos::Envelope e2(100, 200, 100, 200);
    geos::Envelope e3(1, 2, 1, 2);

    BOOST_CHECK( !e1.isNull() );
    BOOST_CHECK( !e2.isNull() );
    BOOST_CHECK( !e3.isNull() );

    BOOST_CHECK( e1.equals( &e2 ) );
    BOOST_CHECK( !e1.equals( &e3 ) );
    BOOST_CHECK( !e2.equals( &e3 ) );

} // envelope_equals_test

BOOST_AUTO_TEST_CASE( envelope_contains_test )
{
    // Test contains() predicates
    geos::Envelope e1;
    geos::Envelope e2(-1, 1, -1, 1);
    geos::Envelope e3(-5, 5, -5, 5);
    
    BOOST_CHECK( !e2.isNull() );
    BOOST_CHECK( !e3.isNull() );

    // 1 - Test empty envelope by reference
    BOOST_CHECK_MESSAGE( !e1.contains( e2 ), "e1 envelope is not empty!" );
    BOOST_CHECK_MESSAGE( !e2.contains( e1 ), "e1 envelope is not empty!");

    // 2 - Test empty envelope by pointer
    BOOST_CHECK_MESSAGE( !e2.contains( &e1 ), "e1 envelope is not empty!");
    BOOST_CHECK_MESSAGE( !e1.contains( &e2 ), "e1 envelope is not empty!" );

    // 3 - Test non-empty envelope by reference
    BOOST_CHECK( !e2.contains( e3 ) );
    BOOST_CHECK( e3.contains( e2 ) );

    // 3 - Test raw point
    BOOST_CHECK( e2.contains( 0, 0 ) );
    BOOST_CHECK( e2.contains( -1, -1 ) );
    BOOST_CHECK( !e2.contains( 5, 5 ) );
    
    // 4 - Test coordinate
    geos::Coordinate c1(0, 0, 0);
    BOOST_REQUIRE_EQUAL( c1.x, 0 );
    BOOST_REQUIRE_EQUAL( c1.y, 0 );
    BOOST_REQUIRE_EQUAL( c1.z, 0 );
    BOOST_CHECK( e2.contains( c1 ) );

} // envelope_contains_test

BOOST_AUTO_TEST_CASE( envelope_intersects_test )
{
    // Test intersects() predicates
    geos::Envelope e1;
    geos::Envelope e2(-100, 100, -100, 100);
    geos::Envelope e3(50, 150, 50, 150);

    BOOST_CHECK( !e2.isNull() );
    BOOST_CHECK( !e3.isNull() );

    // 1 - Test empty envelope by pointer
    BOOST_CHECK_MESSAGE( !e1.intersects( &e2 ), "e1 envelope is not empty!" );
    BOOST_CHECK_MESSAGE( !e2.intersects( &e1 ), "e1 envelope is not empty!");

    // 2 - Test empty envelope by reference
    BOOST_CHECK_MESSAGE( !e1.intersects( e2 ), "e1 envelope is not empty!" );
    BOOST_CHECK_MESSAGE( !e2.intersects( e1 ), "e1 envelope is not empty!");

    // 3 - Test empty envelope by reference
    BOOST_CHECK( e2.intersects( e3 ) );
    BOOST_CHECK( e3.intersects( e2 ) );

    // 3 - Test point
    BOOST_CHECK( e2.intersects( 0, 0 ) );
    BOOST_CHECK( e2.intersects( -100, 100 ) );
    BOOST_CHECK( !e2.intersects( -200, 200 ) );

    // 4 - Test coordinate
    geos::Coordinate c1(0, 0, 0);
    BOOST_REQUIRE_EQUAL( c1.x, 0 );
    BOOST_REQUIRE_EQUAL( c1.y, 0 );
    BOOST_REQUIRE_EQUAL( c1.z, 0 );
    BOOST_CHECK( e2.intersects( c1 ) );

} // envelope_intersects_test

BOOST_AUTO_TEST_CASE( envelope_expand_test )
{
    // TODO

} //envelope_expand_test

BOOST_AUTO_TEST_SUITE_END(); // envelope_class_test

// EOF
