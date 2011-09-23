// $Id: LineSegmentTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::geom::LineSegment class.

#include <tut.hpp>
// geos
#include <geos/geom/LineSegment.h>
#include <geos/geom/Coordinate.h>
// std
#include <iostream>

namespace tut
{
	//
	// Test Group
	//

	struct test_lineseg_data
    {
        geos::geom::Coordinate ph1;
        geos::geom::Coordinate ph2;
        geos::geom::Coordinate pv1;
        geos::geom::Coordinate pv2;
        geos::geom::LineSegment h1;
        geos::geom::LineSegment v1;

        test_lineseg_data()
            : ph1(0, 2), ph2(10, 2), pv1(0, 0), pv2(0, 10), h1(ph1, ph2), v1(pv1, pv2)
        {}
    };

	typedef test_group<test_lineseg_data> group;
	typedef group::object object;

	group test_lineseg_group("geos::geom::LineSegment");

	//
	// Test Cases
	//

	// 1 - Test reverse()
	template<>
	template<>
	void object::test<1>()
	{
		ensure(v1[0] == pv1);
		ensure(v1[1] == pv2);
		v1.reverse();
		ensure(v1[1] == pv1);
		ensure(v1[0] == pv2);

		ensure(h1[0] == ph1);
		ensure(h1[1] == ph2);
		h1.reverse();
		ensure(h1[1] == ph1);
		ensure(h1[0] == ph2);
	}


	// 2 - Horizontal LineSegment test
	template<>
	template<>
	void object::test<2>()
	{         
		ensure( h1.isHorizontal() );
		v1.reverse();
		ensure( h1.isHorizontal() );
	}

	// 3 - Vertical LineSegment test
	template<>
	template<>
	void object::test<3>()
	{
		ensure( v1.isVertical() );
		v1.reverse();
		ensure( v1.isVertical() );
	}

	// 4 - Test distance()
	template<>
	template<>
	void object::test<4>()
	{
		ensure_equals( h1.distance(v1), 0 );
		v1.reverse();
		ensure_equals( h1.distance(v1), 0 );
		h1.reverse();
		ensure_equals( h1.distance(v1), 0 );
	}

	// 5 - Test getLength()
	template<>
	template<>
	void object::test<5>()
	{
		ensure_equals( v1.getLength(), 10 );
		v1.reverse();
		ensure_equals( v1.getLength(), 10 );

		ensure_equals( h1.getLength(), 10 );
		h1.reverse();
		ensure_equals( h1.getLength(), 10 );
	}

	// 6 - Test distance again()
	template<>
	template<>
	void object::test<6>()
	{
		geos::geom::Coordinate p(1, 1);
		ensure_equals( v1.distance(p), 1 );
		v1.reverse();
		ensure_equals( v1.distance(p), 1 );
	}

} // namespace tut

