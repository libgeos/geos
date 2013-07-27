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
    // Test circumcentre()
        template<>
	template<>
	void object::test<6>()
	{   
		using geos::geom::Triangle;
		using geos::geom::Coordinate;

		Coordinate x1(5,7);
		Coordinate x2(6,6);
		Coordinate x3(2,-2);

		Coordinate y1(3,3);
		Coordinate y2(9,10);
		Coordinate y3(6,7);

		Coordinate a1(5,10);
		Coordinate a2(11,23);
		Coordinate a3(22,19);

		Triangle t1(x1,x2,x3);

		Triangle t2(y1,y2,y3);

		Triangle t3(a1,a2,a3);

		// For t1:
		Coordinate c1(0,0);
		t1.circumcentre(c1);
		ensure_equals(c1.x ,2 );
		ensure_equals(c1.y ,3 );
		ensure( 0 != ISNAN( c1.z ) );

		//For t2:
		Coordinate c2(0,0);
		t2.circumcentre(c2);
		ensure_equals(c2.x ,30.5 );
		ensure_equals(c2.y ,- 14.5 );
		ensure( 0 != ISNAN( c2.z ) );


		//For t3:
		Coordinate c3(0,0);
		t3.circumcentre(c3);
		ensure( std::fabs(c3.x - 13.0) < 1 );
		ensure( c3.y > 13.7 );
		ensure( c3.y < 13.8 );
		ensure( 0 != ISNAN( c3.z ) );
		// cout << "CicumCenter of triangle ABC:: " << c1.x << " " << c1.y << endl;

		//  std::cout << "CicumCenter of triangle DEF:: " << c2.x << " " << c2.y << std::endl;
	}

} // namespace tut
