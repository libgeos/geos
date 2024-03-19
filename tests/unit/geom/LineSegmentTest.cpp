//
// Test Suite for geos::geom::LineSegment class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/geom/LineSegment.h>
#include <geos/geom/Coordinate.h>

// std
#include <iostream>

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::LineSegment;

namespace tut {
//
// Test Group
//

struct test_lineseg_data {

    geos::geom::Coordinate ph1;
    geos::geom::Coordinate ph2;
    geos::geom::Coordinate pv1;
    geos::geom::Coordinate pv2;
    geos::geom::LineSegment h1;
    geos::geom::LineSegment v1;
    double MAX_ABS_ERROR_INTERSECTION = 1e-5;

    void checkLineIntersection(
        double p1x, double p1y, double p2x, double p2y,
        double q1x, double q1y, double q2x, double q2y,
        double expectedx, double expectedy)
    {
        LineSegment seg1(p1x, p1y, p2x, p2y);
        LineSegment seg2(q1x, q1y, q2x, q2y);

        Coordinate actual = seg1.lineIntersection(seg2);
        Coordinate expected(expectedx, expectedy);
        double dist = actual.distance(expected);
        // std::cout << "Expected: " << expected << "  Actual: " << actual << "  Dist = " << dist << std::endl;
        ensure("checkLineIntersection", dist <= MAX_ABS_ERROR_INTERSECTION);
    }

    void checkOffsetPoint(
        double x0, double y0,
        double x1, double y1,
        double segFrac, double offset,
        double expectedX, double expectedY)
    {
        LineSegment seg(x0, y0, x1, y1);
        Coordinate actual;
        seg.pointAlongOffset(segFrac, offset, actual);
        Coordinate expected(expectedX, expectedY);
        ensure_equals(actual.x, expected.x);
        ensure_equals(actual.y, expected.y);
    }

    void checkOffsetLine(
        double x0, double y0,
        double x1, double y1,
        double offset,
        double expectedX0, double expectedY0,
        double expectedX1, double expectedY1)
    {
        LineSegment seg(x0, y0, x1, y1);
        LineSegment actual = seg.offset(offset);

        Coordinate expected0(expectedX0, expectedY0);
        Coordinate expected1(expectedX1, expectedY1);
        ensure_equals_xyz(actual.p0, expected0);
        ensure_equals_xyz(actual.p1, expected1);
    }

    void checkOrientationIndex(
        LineSegment seg,
        double x, double y,
        int expectedOrient)
    {
        Coordinate p(x, y);
  	    int orient = seg.orientationIndex(p);
        ensure_equals(expectedOrient, orient);
    }

    void checkOrientationIndex(
        LineSegment seg,
        double x0, double y0,
        double x1, double y1,
        int expectedOrient)
    {
        LineSegment seg1(x0, y0, x1, y1);
  	    int orient = seg.orientationIndex(seg1);
        ensure_equals(expectedOrient, orient);
    }

    void checkDistancePerpendicular(
        double x0, double y0,
        double x1, double y1,
        double px, double py,
        double expected)
    {
        LineSegment seg(x0, y0, x1, y1);
        Coordinate c(px, py);
        double dist = seg.distancePerpendicular(c);
        ensure_equals("checkDistancePerpendicular", expected, dist, 0.000001);
    }

    void checkDistancePerpendicularOriented(
        double x0, double y0,
        double x1, double y1,
        double px, double py,
        double expected)
    {
        LineSegment seg(x0, y0, x1, y1);
        Coordinate c(px, py);
        double dist = seg.distancePerpendicularOriented(c);
        ensure_equals("checkDistancePerpendicularOriented", expected, dist, 0.000001);
    }

    void checkMidPoint(
        double x0, double y0,
        double x1, double y1,
        double px, double py)
    {
        LineSegment seg(x0, y0, x1, y1);
        Coordinate expected(px, py);
        Coordinate actual = Coordinate(seg.midPoint());
        ensure_equals_xy(actual, expected);
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

// testOffsetLine
template<>
template<>
void object::test<8>()
{
    const double ROOT2 = std::sqrt(2.0);
    checkOffsetLine(0, 0, 10, 10, 0, 0, 0, 10, 10 );
    checkOffsetLine(0, 0, 10, 10, ROOT2, -1, 1,  9, 11 );
    checkOffsetLine(0, 0, 10, 10, -ROOT2, 1, -1, 11, 9);
}

// testOffsetPoint
template<>
template<>
void object::test<9>()
{
    double ROOT2 = std::sqrt(2.0);
    checkOffsetPoint(0, 0, 10, 10, 0.0, ROOT2, -1, 1);
    checkOffsetPoint(0, 0, 10, 10, 0.0, -ROOT2, 1, -1);

    checkOffsetPoint(0, 0, 10, 10, 1.0, ROOT2, 9, 11);
    checkOffsetPoint(0, 0, 10, 10, 0.5, ROOT2, 4, 6);

    checkOffsetPoint(0, 0, 10, 10, 0.5, -ROOT2, 6, 4);
    checkOffsetPoint(0, 0, 10, 10, 0.5, -ROOT2, 6, 4);

    checkOffsetPoint(0, 0, 10, 10, 2.0, ROOT2, 19, 21);
    checkOffsetPoint(0, 0, 10, 10, 2.0, -ROOT2, 21, 19);

    checkOffsetPoint(0, 0, 10, 10, 2.0, 5 * ROOT2, 15, 25);
    checkOffsetPoint(0, 0, 10, 10, -2.0, 5 * ROOT2, -25, -15);
}

// testOrientationIndexCoordinate
template<>
template<>
void object::test<10>()
{
    LineSegment seg(0, 0, 10, 10);
    checkOrientationIndex(seg, 10, 11, 1);
  	checkOrientationIndex(seg, 10, 9, -1);

  	checkOrientationIndex(seg, 11, 11, 0);

  	checkOrientationIndex(seg, 11, 11.0000001, 1);
  	checkOrientationIndex(seg, 11, 10.9999999, -1);

  	checkOrientationIndex(seg, -2, -1.9999999, 1);
  	checkOrientationIndex(seg, -2, -2.0000001, -1);
}

// testOrientationIndexSeg
template<>
template<>
void object::test<11>()
{
    LineSegment seg(100, 100, 110, 110);
    checkOrientationIndex(seg, 10, 11, 1);
  	checkOrientationIndex(seg, 10, 9, -1);

  	checkOrientationIndex(seg, 100, 101, 105, 106, 1);
  	checkOrientationIndex(seg, 100, 99, 105, 96, -1);

  	checkOrientationIndex(seg, 200, 200, 210, 210, 0);

  	checkOrientationIndex(seg, 105, 105, 110, 100, -1);
}


// testDistancePerpendicular
template<>
template<>
void object::test<12>()
{
    checkDistancePerpendicular(1,1,  1,3,  2,4, 1);
    checkDistancePerpendicular(1,1,  1,3,  0,4, 1);
    checkDistancePerpendicular(1,1,  1,3,  1,4, 0);
    checkDistancePerpendicular(1,1,  2,2,  4,4, 0);
    //-- zero-length line segment
    checkDistancePerpendicular(1,1,  1,1,  1,2, 1);
}

// testDistancePerpendicularOriented
template<>
template<>
void object::test<13>()
{
    //-- right of line
    checkDistancePerpendicularOriented(1,1,  1,3,  2,4, -1);
    //-- left of line
    checkDistancePerpendicularOriented(1,1,  1,3,  0,4, 1);
    //-- on line
    checkDistancePerpendicularOriented(1,1,  1,3,  1,4, 0);
    checkDistancePerpendicularOriented(1,1,  2,2,  4,4, 0);
    //-- zero-length segment
    checkDistancePerpendicularOriented(1,1,  1,1,  1,2, 1);
}

// test midpoint
template<>
template<>
void object::test<14>()
{
    //-- right of line
    checkMidPoint(1,1,  1,3,  1,2);
    checkMidPoint(1,1,  1,1,  1,1);
    checkMidPoint(1,1,  5,5,  3,3);
}


} // namespace tut
