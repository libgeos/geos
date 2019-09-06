//
// Test Suite for geos::geom::LineSegment class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/LineSegment.h>
#include <geos/geom/Coordinate.h>
// std
#include <iostream>

namespace tut {
//
// Test Group
//

struct test_lineseg_data {

    typedef geos::geom::Coordinate Coordinate;
    typedef geos::geom::LineSegment LineSegment;

    geos::geom::Coordinate ph1;
    geos::geom::Coordinate ph2;
    geos::geom::Coordinate pv1;
    geos::geom::Coordinate pv2;
    geos::geom::LineSegment h1;
    geos::geom::LineSegment v1;
    double MAX_ABS_ERROR_INTERSECTION = 1e-5;

    void checkLineIntersection(double p1x, double p1y, double p2x, double p2y,
                               double q1x, double q1y, double q2x, double q2y,
                               double expectedx, double expectedy) {
        LineSegment seg1(p1x, p1y, p2x, p2y);
        LineSegment seg2(q1x, q1y, q2x, q2y);

        Coordinate actual = seg1.lineIntersection(seg2);
        Coordinate expected(expectedx, expectedy);
        double dist = actual.distance(expected);
        // std::cout << "Expected: " << expected << "  Actual: " << actual << "  Dist = " << dist << std::endl;
        ensure("checkLineIntersection", dist <= MAX_ABS_ERROR_INTERSECTION);
    }

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
void object::test<1>
()
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
void object::test<2>
()
{
    ensure(h1.isHorizontal());
    v1.reverse();
    ensure(h1.isHorizontal());
}

// 3 - Vertical LineSegment test
template<>
template<>
void object::test<3>
()
{
    ensure(v1.isVertical());
    v1.reverse();
    ensure(v1.isVertical());
}

// 4 - Test distance()
template<>
template<>
void object::test<4>
()
{
    ensure_equals(h1.distance(v1), 0);
    v1.reverse();
    ensure_equals(h1.distance(v1), 0);
    h1.reverse();
    ensure_equals(h1.distance(v1), 0);
}

// 5 - Test getLength()
template<>
template<>
void object::test<5>
()
{
    ensure_equals(v1.getLength(), 10);
    v1.reverse();
    ensure_equals(v1.getLength(), 10);

    ensure_equals(h1.getLength(), 10);
    h1.reverse();
    ensure_equals(h1.getLength(), 10);
}

// 6 - Test distance again()
template<>
template<>
void object::test<6>
()
{
    geos::geom::Coordinate p(1, 1);
    ensure_equals(v1.distance(p), 1);
    v1.reverse();
    ensure_equals(v1.distance(p), 1);
}

template<>
template<>
void object::test<7>
()
{
    // simple case
    checkLineIntersection(
        0,0,  10,10,
        0,10, 10,0,
        5,5);

    //Almost collinear - See JTS GitHub issue #464
    checkLineIntersection(
        35613471.6165017, 4257145.306132293, 35613477.7705378, 4257160.528222711,
        35613477.77505724, 4257160.539653536, 35613479.85607389, 4257165.92369170,
        35613477.772841461, 4257160.5339209242 );
}

} // namespace tut

