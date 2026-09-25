//
// Test Suite for geos::algorithm::CGAlgorithmsDD

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/math/DD.h>
// std
#include <random>

using geos::algorithm::CGAlgorithmsDD;
using geos::algorithm::LineIntersector;
using geos::geom::CoordinateXY;
using geos::math::DD;

namespace tut {
//
// Test Group
//

struct test_cgalgorithmsdd_data {};

typedef test_group<test_cgalgorithmsdd_data> group;
typedef group::object object;

group test_cgalgorithmsdd_group("geos::algorithm::CGAlgorithmsDD");

//
// Test Cases
//

// Parallel lines have no single intersection point.
template<>
template<>
void object::test<1>
()
{
    set_test_name("intersectionDD of parallel lines");
    DD x(7.0);
    DD y(8.0);
    ensure("parallel", !CGAlgorithmsDD::intersectionDD(CoordinateXY(0, 0), CoordinateXY(1, 1),
                                                         CoordinateXY(0, 1), CoordinateXY(1, 2), x, y));
    ensure("x unchanged", x == DD(7.0));
    ensure("y unchanged", y == DD(8.0));
    ensure("null point", CGAlgorithmsDD::intersection(CoordinateXY(0, 0), CoordinateXY(1, 1),
                                                      CoordinateXY(0, 1), CoordinateXY(1, 2)).isNull());
}

// The exact intersection lies about 2.8e-17 left of -0.5, and its nearest
// double is -0.5. The double-double value keeps the side of -0.5 it lies on,
// and intersection() returns its nearest double.
template<>
template<>
void object::test<2>
()
{
    set_test_name("intersectionDD keeps what the nearest double loses");
    CoordinateXY p1(-1, -1);
    CoordinateXY p2(0, 1);
    CoordinateXY q1(-1, 1);
    CoordinateXY q2(0, -1.0000000000000002);
    DD x;
    DD y;
    ensure("intersects", CGAlgorithmsDD::intersectionDD(p1, p2, q1, q2, x, y));
    ensure("left of -0.5", x < DD(-0.5));
    ensure_equals("nearest double of x", x.ToDouble(), -0.5, 0.0);
    CoordinateXY pt = CGAlgorithmsDD::intersection(p1, p2, q1, q2);
    ensure_equals("x", pt.x, x.ToDouble(), 0.0);
    ensure_equals("y", pt.y, y.ToDouble(), 0.0);
}

// LineIntersector computes a proper intersection as the nearest doubles of
// intersectionDD. SnapRoundingIntersectionAdder relies on this to place the
// intersection in the grid cell of the double-double point with one step.
template<>
template<>
void object::test<3>
()
{
    set_test_name("LineIntersector returns the nearest doubles of intersectionDD");
    std::mt19937_64 gen(1);
    std::uniform_real_distribution<double> ordinate(-1000.0, 1000.0);
    int properCount = 0;
    for (int i = 0; i < 1000; i++) {
        CoordinateXY p1(ordinate(gen), ordinate(gen));
        CoordinateXY p2(ordinate(gen), ordinate(gen));
        CoordinateXY q1(ordinate(gen), ordinate(gen));
        CoordinateXY q2(ordinate(gen), ordinate(gen));
        LineIntersector li;
        li.computeIntersection(p1, p2, q1, q2);
        if (!li.isProper()) {
            continue;
        }
        properCount++;
        DD x;
        DD y;
        ensure("intersects", CGAlgorithmsDD::intersectionDD(p1, p2, q1, q2, x, y));
        ensure_equals("x", li.getIntersection(0).x, x.ToDouble(), 0.0);
        ensure_equals("y", li.getIntersection(0).y, y.ToDouble(), 0.0);
    }
    ensure("proper intersections tested", properCount > 100);
}

} // namespace tut
