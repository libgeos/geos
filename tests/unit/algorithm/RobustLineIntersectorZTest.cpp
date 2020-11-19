//
// Ported from JTS
// junit/algorithm/RobustLineIntersectorZTest.java
// 1ecc94caa9e188beb63bfb95089e7dd6869bab20

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
// std
#include <sstream>
#include <string>
#include <memory>


using namespace geos::geom;

typedef geos::algorithm::LineIntersector RobustLineIntersector;

#define DoubleNaN  std::numeric_limits<double>::quiet_NaN()

namespace tut {
//
// Test Group
//

struct test_robustlineintersectorz_data {

    void checkIntersection(const LineSegment& line1, const LineSegment& line2,
                           const Coordinate& p1, const Coordinate& p2)
    {
        checkIntersectionDir(line1, line2, p1, p2);
        checkIntersectionDir(line2, line1, p1, p2);
        LineSegment line1Rev(line1.p1, line1.p0);
        LineSegment line2Rev(line2.p1, line2.p0);
        checkIntersectionDir(line1Rev, line2Rev, p1, p2);
        checkIntersectionDir(line2Rev, line1Rev, p1, p2);
    }


    void checkIntersectionDir(
                const LineSegment& line1,
                const LineSegment& line2,
                const Coordinate& p1,
                const Coordinate& p2)
    {
        RobustLineIntersector li;
        li.computeIntersection(
            line1.p0, line1.p1,
            line2.p0, line2.p1);

        ensure_equals(li.getIntersectionNum(), 2u);

        Coordinate actual1 = li.getIntersection(0);
        Coordinate actual2 = li.getIntersection(1);
        // normalize actual results
        if (actual1.compareTo(actual2) > 0) {
            actual1 = li.getIntersection(1);
            actual2 = li.getIntersection(0);
        }

        ensure_equals_xyz( actual1, p1 );
        ensure_equals_xyz( actual2, p2 );
  }

    void checkIntersection(
            const LineSegment& line1,
            const LineSegment& line2,
            const Coordinate& pt)
    {
        checkIntersectionDir(line1, line2, pt);
        checkIntersectionDir(line2, line1, pt);
        LineSegment line1Rev(line1.p1, line1.p0);
        LineSegment line2Rev(line2.p1, line2.p0);
        checkIntersectionDir(line1Rev, line2Rev, pt);
        checkIntersectionDir(line2Rev, line1Rev, pt);
    }

    void checkIntersectionDir(
            const LineSegment& line1,
            const LineSegment& line2,
            const Coordinate& pt)
    {
        RobustLineIntersector li;
        li.computeIntersection(
            line1.p0, line1.p1,
            line2.p0, line2.p1);
        ensure_equals(li.getIntersectionNum(), 1u);
        Coordinate actual = li.getIntersection(0);
        ensure_equals_xyz( actual, pt );
    }

    static Coordinate pt(double x, double y, double z) {
        return Coordinate(x, y, z);
    }

    static Coordinate pt(double x, double y) {
        return Coordinate(x, y);
    }

    static LineSegment line(double x1, double y1, double z1,
                            double x2, double y2, double z2)
    {
        return LineSegment(Coordinate(x1, y1, z1),
                           Coordinate(x2, y2, z2));
    }

    static LineSegment line(double x1, double y1,
                            double x2, double y2)
    {
        return LineSegment(Coordinate(x1, y1), Coordinate(x2, y2));
    }

};

typedef test_group<test_robustlineintersectorz_data> group;
typedef group::object object;

group test_robustlineintersectorz_group(
    "geos::algorithm::RobustLineIntersectorZ");




//
// Test Cases
//


// 1 - testInterior
template<>
template<>
void object::test<1>
()
{
    checkIntersection(
        line(1, 1, 1, 3, 3, 3),
        line(1, 3, 10, 3, 1, 30),
        pt(2, 2, 11)
    );
}


// 2 - testInterior2D
template<>
template<>
void object::test<2>
()
{
    checkIntersection(
        line(1, 1, 3, 3),
        line(1, 3, 3, 1),
        pt(2, 2)
    );
}

// testInterior3D2D
template<>
template<>
void object::test<3>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(1, 3, 3, 1),
        pt(2, 2, 2));
}

// testInterior2D3D
template<>
template<>
void object::test<4>
()
{
    checkIntersection( line(1, 1, 3, 3), line(1, 3, 10, 3, 1, 30),
        pt(2, 2, 20));
}

// testInterior2D3DPart
    // result is average of line1 interpolated and line2 p0 Z
template<>
template<>
void object::test<5>
()
{
    checkIntersection(
        line(1, 1, 1, 3, 3, 3),
        line(1, 3, 10, 3, 1, DoubleNaN),
        pt(2, 2, 6));
}

// testEndpoint
template<>
template<>
void object::test<6>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(3, 3, 3, 3, 1, 30),
        pt(3, 3, 3));
}

// testEndpoint2D
template<>
template<>
void object::test<7>
()
{
    checkIntersection( line(1, 1, 3, 3), line(3, 3, 3, 1),
        pt(3, 3, DoubleNaN));
}

// testEndpoint2D3D
template<>
template<>
void object::test<8>
()
{
    // result Z is from 3D point
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(3, 3, 3, 1),
        pt(3, 3, 3));
}

// testInteriorEndpoint
template<>
template<>
void object::test<9>
()
{
    // result Z is from 3D point
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(2, 2, 10, 3, 1, 30),
        pt(2, 2, 10));
}

// testInteriorEndpoint3D2D
template<>
template<>
void object::test<10>
()
{
    // result Z is interpolated
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(2, 2, 3, 1),
        pt(2, 2, 2));
}

// testInteriorEndpoint2D3D
    // result Z is from 3D point
template<>
template<>
void object::test<11>
()
{
    checkIntersection( line(1, 1, 3, 3), line(2, 2, 10, 3, 1, 20),
        pt(2, 2, 10));
}

// testCollinearEqual
template<>
template<>
void object::test<12>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(1, 1, 1, 3, 3, 3),
        pt(1, 1, 1), pt( 3, 3, 3));
}

// testCollinearEqual3D2D
template<>
template<>
void object::test<13>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(1, 1, 3, 3),
        pt(1, 1, 1), pt( 3, 3, 3));
}

// testCollinearEndpoint
template<>
template<>
void object::test<14>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(3, 3, 3, 5, 5, 5),
        pt(3, 3, 3));
}

// testCollinearEndpoint3D2D
    // result Z is from 3D point
template<>
template<>
void object::test<15>
()
{
    checkIntersection( line(1, 1, 1, 3, 3, 3), line(3, 3, 5, 5),
        pt(3, 3, 3));
}

// testCollinearContained
template<>
template<>
void object::test<16>
()
{
    checkIntersection( line(1, 1, 1, 5, 5, 5), line(3, 3, 3, 4, 4, 4),
        pt(3, 3, 3), pt(4, 4, 4));
}

// testCollinearContained3D2D
template<>
template<>
void object::test<17>
()
{
    // result Z is interpolated
    checkIntersection( line(1, 1, 1, 5, 5, 5), line(3, 3, 4, 4),
        pt(3, 3, 3), pt(4, 4, 4));
}

} // namespace tut

