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
#include <utility>


using namespace geos::geom;

typedef geos::algorithm::LineIntersector RobustLineIntersector;

namespace tut {
//
// Test Group
//

struct test_robustlineintersectorz_data {
    template<typename C>
    using Segment = std::pair<C, C>;

    using XY = CoordinateXY;
    using XYZ = Coordinate;
    using XYM = CoordinateXYM;
    using XYZM = CoordinateXYZM;

    template<typename C1, typename C2>
    void checkIntersection(const Segment<C1>& line1,
                           const Segment<C2>& line2,
                           const CoordinateXYZM& p1,
                           const CoordinateXYZM& p2 = CoordinateXYZM::getNull())
    {
        checkIntersectionDir(line1, line2, p1, p2);
        checkIntersectionDir(line2, line1, p1, p2);
        Segment<C1> line1Rev(line1.second, line1.first);
        Segment<C2> line2Rev(line2.second, line2.first);
        checkIntersectionDir(line1Rev, line2Rev, p1, p2);
        checkIntersectionDir(line2Rev, line1Rev, p1, p2);
    }

    template<typename C1, typename C2>
    void checkIntersectionDir(
                const Segment<C1>& line1,
                const Segment<C2>& line2,
                const CoordinateXYZM& p1,
                const CoordinateXYZM& p2)
    {
        RobustLineIntersector li;
        li.computeIntersection(
            line1.first, line1.second,
            line2.first, line2.second);

        auto actual1 = li.getIntersection(0);
        auto actual2 = li.getIntersection(1);

        if (p2.isNull()) {
            ensure_equals(li.getIntersectionNum(), 1u);
        } else {
            ensure_equals(li.getIntersectionNum(), 2u);

            // normalize actual results
            if (actual1.compareTo(actual2) > 0) {
                actual1 = li.getIntersection(1);
                actual2 = li.getIntersection(0);
            }
        }

        ensure_equals_xyzm( actual1, p1 );
        if (!p2.isNull())
            ensure_equals_xyzm( actual2, p2 );
  }

    static CoordinateXYZM xyz(double x, double y, double z) {
        return CoordinateXYZM(Coordinate(x, y, z));
    }

    static CoordinateXYZM xy(double x, double y) {
        return CoordinateXYZM(CoordinateXY(x, y));
    }

    static CoordinateXYZM xym(double x, double y, double m) {
        return CoordinateXYZM(CoordinateXYM(x, y, m));
    }

    static CoordinateXYZM xyzm(double x, double y, double z, double m) {
        return CoordinateXYZM(x, y, z, m);
    }

    template<typename C=Coordinate>
    static Segment<C> line(const C& p0, const C& p1) {
        return Segment<C>(p0, p1);
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
    // XYZ intersects XYZ at interior point.
    // Z value at the intersection point is the average of the interpolated values from each line.
    set_test_name("testInterior");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({1, 3, 10}, {3, 1, 30}),
                xyz(2, 2, 11));
}


// 2 - testInterior2D
template<>
template<>
void object::test<2>
()
{
    set_test_name("testInterior2D");

    checkIntersection(
                line<XY>({1, 1}, {3, 3}),
                line<XY>({1, 3}, {3, 1}),
                xy(2, 2));
}

// testInterior3D2D
template<>
template<>
void object::test<3>
()
{
    // XYZ intersects XY at interior point.
    // Z value at the intersection point is the interpolated value from the XYZ line.
    set_test_name("testInterior3D2D");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XY>({1, 3}, {3, 1}),
                xyz(2, 2, 2));
}

// testInterior2D3D
template<>
template<>
void object::test<4>
()
{
    // XY intersects XYZ at interior point.
    // Z value at the intersection point is the interpolated value from the XYZ line.
    set_test_name("testInterior2D3D");

    checkIntersection(
                line<XY>({1, 1}, {3, 3}),
                line<XYZ>({1, 3, 10}, {3, 1, 30}),
                xyz(2, 2, 20));
}

// testInterior2D3DPart
template<>
template<>
void object::test<5>
()
{
    // XYZ intersects XYZ at interior point.
    // Second line has a Z value of NaN at one point.
    // result is average of line1 interpolated and line2 p0 Z
    set_test_name("testInterior2D3DPart");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({1, 3, 10}, {3, 1, geos::DoubleNotANumber}),
                xyz(2, 2, 6));
}

// testEndpoint
template<>
template<>
void object::test<6>
()
{
    // XYZ intersects XYZ at endpoint.
    // Result Z value at intersection point is taken from the first line.
    set_test_name("testEndpoint");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({3, 3, 3}, {3, 1, 30}),
                xyz(3, 3, 3));
}

// testEndpoint2D
template<>
template<>
void object::test<7>
()
{
    // XY intersects XY at endpoint.
    // Result Z value at intersection point is NaN.
    set_test_name("testEndpoint2D");

    checkIntersection(
                line<XY>({1, 1}, {3, 3}),
                line<XY>({3, 3}, {3, 1}),
                xyz(3, 3, geos::DoubleNotANumber));
}

// testEndpoint2D3D
template<>
template<>
void object::test<8>
()
{
    // XYZ intersects XY at endpoint.
    // result Z is from 3D point
    set_test_name("testEndpoint2D3D");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XY>({3, 3}, {3, 1}),
                xyz(3, 3, 3));
}

// testInteriorEndpoint
template<>
template<>
void object::test<9>
()
{
    // Intersection at interior of 3D line, endpoint of 3D line
    // result Z is from 3D endpoint
    set_test_name("testInteriorEndpoint");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({2, 2, 10}, {3, 1, 30}),
                xyz(2, 2, 10));
}

// testInteriorEndpoint3D2D
template<>
template<>
void object::test<10>
()
{
    // Intersection at interior of 3D line, endpoint of 2D line
    // result Z is interpolated
    set_test_name("testInteriorEndpoint3D2D");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XY>({2, 2}, {3, 1}),
                xyz(2, 2, 2));
}

// testInteriorEndpoint2D3D
template<>
template<>
void object::test<11>
()
{
    // Intersection at interior of 2D line, endpoint of 3D line
    // result Z is from 3D point
    set_test_name("testInteriorEndpoint2D3D");

    checkIntersection(
                line<XY>({1, 1}, {3, 3}),
                line<XYZ>({2, 2, 10}, {3, 1, 20}),
                xyz(2, 2, 10));
}

// testCollinearEqual
template<>
template<>
void object::test<12>
()
{
    // Collinear intersection of two XYZ lines
    // Z values are equivalent in both inputs
    set_test_name("testCollinearEqual");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                xyz(1, 1, 1), xyz( 3, 3, 3));
}

// testCollinearEqual3D2D
template<>
template<>
void object::test<13>
()
{
    // Collinear intersection of XY and XYZ
    // Z values taken from XYZ line
    set_test_name("testCollinearEqual3D2D");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XY>({1, 1}, {3, 3}),
                xyz(1, 1, 1), xyz( 3, 3, 3));
}

// testCollinearEndpoint
template<>
template<>
void object::test<14>
()
{
    // Endpoint intersection of two collinear XYZ lines
    // Z values of inputs are the same and are copied to output
    set_test_name("testCollinearEndpoint");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XYZ>({3, 3, 3}, {5, 5, 5}),
                xyz(3, 3, 3));
}

// testCollinearEndpoint3D2D
// result Z is from 3D point
template<>
template<>
void object::test<15>
()
{
    // Endpoint intersection of collinear XY and XYZ lines
    // Z values of result is taken from the XYZ input
    set_test_name("testCollinearEndpoint3D2D");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {3, 3, 3}),
                line<XY>({3, 3}, {5, 5}),
                xyz(3, 3, 3));
}

// testCollinearContained
template<>
template<>
void object::test<16>
()
{
    // Collinear intersection of XYZ lines
    // Z values in the second line match interpolated values in the first
    set_test_name("testCollinearContained");


    checkIntersection(
                line<XYZ>({1, 1, 1}, {5, 5, 5}),
                line<XYZ>({3, 3, 3}, {4, 4, 4}),
                xyz(3, 3, 3),
                xyz(4, 4, 4));
}

// testCollinearContained3D2D
template<>
template<>
void object::test<17>
()
{
    // Collinear intersection of XYZ line with XY line
    // result Z is interpolated
    set_test_name("testCollinearContained3D2D");


    checkIntersection(
                line<XYZ>({1, 1, 1}, {5, 5, 5}),
                line<XY>({3, 3}, {4, 4}),
                xyz(3, 3, 3),
                xyz(4, 4, 4));
}

// Interior XYM-XYM
template<>
template<>
void object::test<18>
()
{
    // Interior intersection of two XYM lines.
    // Result M is the average of the interpolated coordinate values.
    set_test_name("testInteriorXYM-XYM");


    checkIntersection(
                line<XYM>({1, 1, 1}, {3, 3, 3}),
                line<XYM>({1, 3, 10}, {3, 1, 30}),
                xym(2, 2, 11));
}

// Interior XYZM-XYZM
template<>
template<>
void object::test<19>
()
{
    // Interior intersection of two XYZM lines.
    // Result Z and M are the average of the interpolated coordinate values.
    set_test_name("testInteriorXYZM-XYZM");

    checkIntersection(
                line<XYZM>({1, 1, 1, -1}, {3, 3, 3, -3}),
                line<XYZM>({1, 3, 10, -10}, {3, 1, 30, -30}),
                xyzm(2, 2, 11, -11));
}

template<>
template<>
void object::test<20>
()
{
    // Collinear intersection of XYZ lines
    // Z values in the second line do not match interpolated values in the first
    // Result Z uses endpoint values of the second line
    set_test_name("testCollinearContainedDifferentZ");

    checkIntersection(
                line<XYZ>({1, 1, 1}, {5, 5, 5}),
                line<XYZ>({3, 3, 7}, {4, 4, 13}),
                xyz(3, 3, 7),
                xyz(4, 4, 13));
}

} // namespace tut

