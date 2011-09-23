// $Id: RobustLineIntersectorTest.cpp 2809 2009-12-06 01:05:24Z mloskot $
// 
// Ported from JTS junit/algorithm/RobustLineIntersectorTest.java rev. 1.1

#include <tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h> // required for use in auto_ptr
#include <geos/geom/LineString.h> 
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
// std
#include <sstream>
#include <string>
#include <memory>


using namespace geos::geom; // 
using geos::algorithm::LineIntersector;
using geos::algorithm::CGAlgorithms;


namespace tut
{
	//
	// Test Group
	//

	struct test_robustlineintersector_data {

	typedef std::auto_ptr<Geometry> GeomPtr;

	LineIntersector i;

	};

	typedef test_group<test_robustlineintersector_data> group;
	typedef group::object object;

	group test_robustlineintersector_group(
		"geos::algorithm::RobustLineIntersector");




	//
	// Test Cases
	//

	// 1 - test2Lines
	template<>
	template<>
	void object::test<1>()
	{         
    LineIntersector i;
    Coordinate p1(10, 10);
    Coordinate p2(20, 20);
    Coordinate q1(20, 10);
    Coordinate q2(10, 20);
    Coordinate x(15, 15);
    i.computeIntersection(p1, p2, q1, q2);

    ensure_equals(i.getIntersectionNum(), (int)LineIntersector::POINT_INTERSECTION);
    ensure_equals(i.getIntersectionNum(), 1);
    ensure_equals(i.getIntersection(0), x);
    ensure("isProper", i.isProper());
    ensure("hasIntersection", i.hasIntersection());
	}

	// 2 - testCollinear1
	template<>
	template<>
	void object::test<2>()
	{         
    LineIntersector i;
    Coordinate p1(10, 10);
    Coordinate p2(20, 10);
    Coordinate q1(22, 10);
    Coordinate q2(30, 10);
    i.computeIntersection(p1, p2, q1, q2);

    ensure_equals(i.getIntersectionNum(), (int)LineIntersector::NO_INTERSECTION);
    ensure_equals(i.getIntersectionNum(), 0);
    ensure("!isProper", !i.isProper());
    ensure("!hasIntersection", !i.hasIntersection());
	}

	// 3 - testCollinear2
	template<>
	template<>
	void object::test<3>()
	{         
    LineIntersector i;
    Coordinate p1(10, 10);
    Coordinate p2(20, 10);
    Coordinate q1(20, 10);
    Coordinate q2(30, 10);
    i.computeIntersection(p1, p2, q1, q2);

    ensure_equals(i.getIntersectionNum(), (int)LineIntersector::POINT_INTERSECTION);
    ensure_equals(i.getIntersectionNum(), 1);
    ensure("!isProper", !i.isProper());
    ensure("hasIntersection", i.hasIntersection());
	}

	// 4 - testCollinear3
	template<>
	template<>
	void object::test<4>()
	{         
    LineIntersector i;
    Coordinate p1(10, 10);
    Coordinate p2(20, 10);
    Coordinate q1(15, 10);
    Coordinate q2(30, 10);
    i.computeIntersection(p1, p2, q1, q2);

    ensure_equals(i.getIntersectionNum(), (int)LineIntersector::COLLINEAR_INTERSECTION);
    ensure_equals(i.getIntersectionNum(), 2);
    ensure("!isProper", !i.isProper());
    ensure("hasIntersection", i.hasIntersection());
	}

	// 5 - testCollinear4
	template<>
	template<>
	void object::test<5>()
	{         
    LineIntersector i;
    Coordinate p1(10, 10);
    Coordinate p2(20, 10);
    Coordinate q1(10, 10);
    Coordinate q2(30, 10);
    i.computeIntersection(p1, p2, q1, q2);

    ensure_equals(i.getIntersectionNum(), (int)LineIntersector::COLLINEAR_INTERSECTION);
    ensure_equals(i.getIntersectionNum(), 2);
    ensure("!isProper", !i.isProper());
    ensure("hasIntersection", i.hasIntersection());
	}

	// 6 - testEndpointIntersection
	template<>
	template<>
	void object::test<6>()
	{         
    i.computeIntersection(Coordinate(100, 100), Coordinate(10, 100),
        Coordinate(100, 10), Coordinate(100, 100));
    ensure("hasIntersection", i.hasIntersection());
    ensure_equals(i.getIntersectionNum(), 1);
	}

	// 7 - testEndpointIntersection2
	template<>
	template<>
	void object::test<7>()
	{         
    i.computeIntersection(Coordinate(190, 50), Coordinate(120, 100),
        Coordinate(120, 100), Coordinate(50, 150));
    ensure("hasIntersection", i.hasIntersection());
    ensure_equals(i.getIntersectionNum(), 1);
    ensure_equals(i.getIntersection(1), Coordinate(120, 100));
	}

	// 8 - testOverlap
	template<>
	template<>
	void object::test<8>()
	{         
    i.computeIntersection(Coordinate(180, 200), Coordinate(160, 180),
        Coordinate(220, 240), Coordinate(140, 160));
    ensure("hasIntersection", i.hasIntersection());
    ensure_equals(i.getIntersectionNum(), 2);
	}

	// 9 - testIsProper1
	template<>
	template<>
	void object::test<9>()
	{         
    i.computeIntersection(Coordinate(30, 10), Coordinate(30, 30),
        Coordinate(10, 10), Coordinate(90, 11));
    ensure("hasIntersection", i.hasIntersection());
    ensure_equals(i.getIntersectionNum(), 1);
    ensure("isProper", i.isProper());
	}

	// 10 - testIsProper2
	template<>
	template<>
	void object::test<10>()
	{         
    i.computeIntersection(Coordinate(10, 30), Coordinate(10, 0),
        Coordinate(11, 90), Coordinate(10, 10));
    ensure("hasIntersection", i.hasIntersection());
    ensure_equals(i.getIntersectionNum(), 1);
    ensure("!isProper", !i.isProper());
	}

	// 11 - testIsCCW
	template<>
	template<>
	void object::test<11>()
	{         
	    ensure_equals(CGAlgorithms::computeOrientation(
		Coordinate(-123456789, -40),
		Coordinate(0, 0),
		Coordinate(381039468754763.0, 123456789)), 1);
	}

	// 12 - testIsCCW2
	template<>
	template<>
	void object::test<12>()
	{         
	    ensure_equals(CGAlgorithms::computeOrientation(
		Coordinate(10, 10),
		Coordinate(20, 20),
		Coordinate(0, 0)), 0);
	}

	// 13 - testA
	template<>
	template<>
	void object::test<13>()
	{         
    Coordinate p1(-123456789, -40);
    Coordinate p2(381039468754763.0, 123456789);
    Coordinate q(0, 0);

    using geos::geom::CoordinateSequence;
    using geos::geom::GeometryFactory;
    using geos::geom::LineString;

    GeometryFactory factory;
    CoordinateSequence* cs = new CoordinateArraySequence();
    cs->add(p1);
    cs->add(p2);

    GeomPtr l ( factory.createLineString(cs) );
    GeomPtr p ( factory.createPoint(q) );
    ensure(!l->intersects(p.get()));

    ensure(!CGAlgorithms::isOnLine(q, cs));
    ensure_equals(CGAlgorithms::computeOrientation(p1, p2, q), -1);

	}

} // namespace tut

