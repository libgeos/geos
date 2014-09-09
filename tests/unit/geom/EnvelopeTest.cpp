// 
// Test Suite for geos::geom::Envelope class.

// tut
#include <tut.hpp>
// geos
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

namespace tut
{
    //
    // Test Group
    //

    // dummy data, not used
    struct test_envelope_data {};

    typedef test_group<test_envelope_data> group;
    typedef group::object object;

    group test_envelope_group("geos::geom::Envelope");

    //
    // Test Cases
    //

    // 1 - Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {         
        geos::geom::Envelope empty;

        ensure( empty.isNull() );

        ensure_equals( empty.getWidth(), 0 );
        ensure_equals( empty.getHeight(), 0 );
    }

    // 2 - Test of overriden constructor
    template<>
    template<>
    void object::test<2>()
    {
        geos::geom::Envelope box(0, 100, 0, 100);

        ensure( !box.isNull() );

        ensure_equals( box.getMinX(), 0 );
        ensure_equals( box.getMaxX(), 100 );
        ensure_equals( box.getMinY(), 0 );
        ensure_equals( box.getMaxY(), 100 );

        ensure_equals( box.getMinX(), box.getMinY() );
        ensure_equals( box.getMaxX(), box.getMaxY() );

        ensure_equals( box.getWidth(), box.getHeight() );
    }

    // 3 - Test of copy constructor
    template<>
    template<>
    void object::test<3>()
    {
        geos::geom::Envelope box(0, 100, 0, 100);

        ensure( !box.isNull() );
        ensure_equals( box.getWidth(), box.getHeight() );

        geos::geom::Envelope copied(box);

        ensure( !copied.isNull() );
        ensure( copied == box );
        ensure_equals( copied.getWidth(), copied.getHeight() );
    }

    // 4 - Test of setToNull()
    template<>
    template<>
    void object::test<4>()
    {
        geos::geom::Envelope e(100, 200, 100, 200);

        ensure( !e.isNull() );
        e.setToNull();
        ensure( e.isNull() );
    }

    // 5 - Test of equals()
    template<>
    template<>
    void object::test<5>()
    {
        geos::geom::Envelope empty;
        geos::geom::Envelope zero(0, 0, 0, 0);
        geos::geom::Envelope zero2(0, 0, 0, 0);
        geos::geom::Envelope box(0, 100, 0, 100);

        ensure( empty.isNull() );
        ensure( !zero.isNull() );
        ensure( !zero2.isNull() );
        ensure( !box.isNull() );

        /* See http://trac.osgeo.org/geos/ticket/703 */
        ensure( empty.equals( &empty ) );

        ensure( !empty.equals( &zero ) );
        ensure( !zero.equals( &empty ) );

        ensure( zero.equals( &zero2 ) );
        ensure( zero2.equals( &zero ) );

        ensure( !box.equals( &empty ) );
        ensure( !box.equals( &zero ) );
    }

    // 6 - Test of contains()
    template<>
    template<>
    void object::test<6>()
    {
        geos::geom::Envelope empty;
        geos::geom::Envelope small(-1, 1, -1, 1);
        geos::geom::Envelope big(-5, 5, -5, 5);

        ensure( empty.isNull() );
        ensure( !small.isNull() );
        ensure( !big.isNull() );

        // Test empty envelope by reference
        ensure( "empty envelope is not empty!", !empty.contains( small ) );
        ensure( "empty envelope is not empty!", !small.contains( empty ) );

        // Test empty envelope by pointer
        ensure( "empty envelope is not empty!", !empty.contains( &small ) );
        ensure( "empty envelope is not empty!", !small.contains( &empty ) );

        // Test non-empty envelope by reference
        ensure( !small.contains( big ) );
        ensure( big.contains( small ) );

        // Test raw point
        ensure( small.contains( 0, 0 ) );
        ensure( small.contains( -1, -1 ) );
        ensure( !small.contains( 5, 5 ) );

        // Test coordinate
        geos::geom::Coordinate origin(0, 0, 0);

        ensure_equals( origin.x, 0 );
        ensure_equals( origin.y, 0 );
        ensure_equals( origin.z, 0 );
        ensure( small.contains( origin ) );
    }

    // Test of intersects()
    template<>
    template<>
    void object::test<7>()
    {
        geos::geom::Envelope empty;
        geos::geom::Envelope with_origin(-100, 100, -100, 100);
        geos::geom::Envelope moved(50, 150, 50, 150);

        ensure( empty.isNull() );
        ensure( !with_origin.isNull() );
        ensure( !moved.isNull() );

        // Test empty envelope by reference
        ensure( "empty envelope seems not empty!", !empty.intersects( with_origin ) );
        ensure( "empty envelope seems not empty!", !with_origin.intersects( empty ) );

        // Test empty envelope by pointer
        ensure( "empty envelope seems not empty!", !empty.intersects( &with_origin ) );
        ensure( "empty envelope seems not empty!", !with_origin.intersects( &empty ) );

        // Test non-empty envelope by reference
        ensure( with_origin.intersects( moved ) );
        ensure( moved.intersects( with_origin ) );

        // Test intersection with raw point
        ensure( with_origin.intersects( 0, 0 ) );
        ensure( with_origin.intersects( -100, 100 ) );
        ensure( !with_origin.intersects( -200, 200 ) );

        // Test intersection with coordinate
        geos::geom::Coordinate origin(0, 0, 0);

        ensure_equals( origin.x, 0 );
        ensure_equals( origin.y, 0 );
        ensure_equals( origin.z, 0 );
        ensure( with_origin.intersects( origin ) );

    }

    // Test of expand()
    template<>
    template<>
    void object::test<8>()
    {
        geos::geom::Envelope empty;
        geos::geom::Envelope box(-5, 5, -5, 5); // Includes (0,0) point
        const geos::geom::Envelope exemplar(-5, 5, -5, 5);

        ensure( empty.isNull() );
        ensure( !box.isNull() );
        ensure( !exemplar.isNull() );

        // Expand box envelope to include null envelope
        box.expandToInclude(&empty);
        ensure( box == exemplar ); // no change expected

        // Expand null envelope to include box envelope
        empty.expandToInclude(&box);
        ensure( empty == exemplar );
    }

    // Second test of expand()
    template<>
    template<>
    void object::test<9>()
    {
        geos::geom::Envelope empty;
        geos::geom::Envelope box(100, 101, 100, 101); // Does not include (0,0) point
        const geos::geom::Envelope exemplar(100, 101, 100, 101);

        // Expand box envelope to include null envelope
        box.expandToInclude(&empty);
        ensure( box == exemplar ); // no change expected

        // Expand empty envelope to include bigger envelope
        empty.expandToInclude(&box);
        ensure( empty == exemplar );
    }

} // namespace tut

