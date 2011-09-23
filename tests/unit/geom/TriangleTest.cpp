// $Id: TriangleTest.cpp 2873 2010-01-18 18:15:40Z mloskot $
// 
// Test Suite for geos::geom::Triangle class.

// tut
#include <tut.hpp>
// geos
#include <geos/platform.h>
#include <geos/geom/Triangle.h>
#include <geos/geom/Coordinate.h>
// std
#include <cmath>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_triangle_data
    {
		// Coordiantes of simple triangle: ABC
		geos::geom::Coordinate a;
		geos::geom::Coordinate b;
		geos::geom::Coordinate c;

		geos::geom::Coordinate d;
		geos::geom::Coordinate e;
		geos::geom::Coordinate f;

        test_triangle_data()
			: a(3, 3), b(9, 3), c(6, 6), d(-4, -2), e(-8, -2), f(-4, -4)
		{}
    };

    typedef test_group<test_triangle_data> group;
    typedef group::object object;

    group test_triangle_group("geos::geom::Triangle");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
        // TODO - mloskot - discuss about adding default constructor
        ensure("NOTE: Triangle has no default constructor.", true);
    }

    // Test of overriden ctor
    template<>
    template<>
    void object::test<2>()
    {
		geos::geom::Triangle abc(a, b, c);
		
		ensure_equals( abc.p0, a );
		ensure_equals( abc.p1, b );
		ensure_equals( abc.p2, c );
    }

	// Test of copy ctor
    template<>
    template<>
    void object::test<3>()
    {
		geos::geom::Triangle abc(a, b, c);
		geos::geom::Triangle copy(abc);

		ensure_equals( copy.p0, a );
		ensure_equals( copy.p1, b );
		ensure_equals( copy.p2, c );
	}

	// Test of assignment operator
    template<>
    template<>
    void object::test<4>()
    {
		geos::geom::Triangle abc(a, b, c);
		geos::geom::Triangle copy(d, e, f);

		ensure_equals( abc.p0, a );
		ensure_equals( abc.p1, b );
		ensure_equals( abc.p2, c );
		ensure_equals( copy.p0, d );
		ensure_equals( copy.p1, e );
		ensure_equals( copy.p2, f );

		copy = abc;

		ensure_equals( copy.p0, a );
		ensure_equals( copy.p1, b );
		ensure_equals( copy.p2, c );
		ensure( copy.p0 != d );
		ensure( copy.p1 != e );
		ensure( copy.p2 != f );
	}

    // Test of inCenter()
    template<>
    template<>
    void object::test<5>()
    {
		geos::geom::Coordinate center;
		geos::geom::Triangle abc(a, b, c);
		
		// Expected: ~4.2426406871192857
		abc.inCentre(center);
		// 1e-16 fails sometimes
		ensure( std::fabs(center.x - 6.0) < 1e-15 );
		ensure( center.y > 4.2 );
		ensure( center.y < 4.3 );
		ensure( 0 != ISNAN( center.z ) );
    }
} // namespace tut
