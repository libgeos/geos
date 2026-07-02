//
// Test Suite for handling of Z in geos::operation::overlayng::OverlayNG class.
//
//
// Ported from JTS
// modules/core/src/test/java/org/locationtech/jts/operation/overlayng/OverlayNGZTest.java
// 4c88fea526567b752ddb9a42aa16cfad2ee74ef1
//

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayngz_data {

    WKTReader r;
    WKTWriter w;

    test_overlayngz_data()
    {
    }

    void checkOverlay(int opCode, const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> a = r.read(wktA);
        std::unique_ptr<Geometry> b = r.read(wktB);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = OverlayNG::overlay(a.get(), b.get(), opCode);
        expected->normalize();
        result->normalize();
        std::string obtainedWKT = w.write(result.get());
        std::string expectedWKT = w.write(expected.get());
        ensure_equals(obtainedWKT, expectedWKT);
    }

    void checkIntersection(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::INTERSECTION, wktA, wktB, wktExpected);
    }

    void checkDifference(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::DIFFERENCE, wktA, wktB, wktExpected);
    }

    void checkUnion(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::UNION, wktA, wktB, wktExpected);
    }

};

typedef test_group<test_overlayngz_data> group;
typedef group::object object;

group test_overlayngz_group("geos::operation::overlayng::OverlayNGZ");

//
// Test Cases
//

// testPointXYPointDifference
// uses ElevationModel
template<>
template<>
void object::test<1> ()
{
  checkDifference("MULTIPOINT ((1 1), (5 5))", "POINT Z (5 5 99)",
      "POINT Z(1 1 99)");
}

// checks that Point Z is preserved
// testPointPolygonIntersection
template<>
template<>
void object::test<2> ()
{
  checkIntersection("POINT Z (5 5 99)", "POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "POINT Z(5 5 99)");

  checkIntersection("POINT M (5 5 99)", "POLYGON M ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "POINT M(5 5 99)");
}

// testLineIntersectionPointZInterpolated
template<>
template<>
void object::test<3> ()
{
  checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 0 10 10)",
      "POINT(5 5 5)");

  checkIntersection("LINESTRING M (0 0 0, 10 10 10)", "LINESTRING M (10 0 0, 0 10 10)",
      "POINT M(5 5 5)");
}

// testLineIntersectionPointZValue
template<>
template<>
void object::test<4> ()
{
  checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 5 5 999, 0 10 10)",
      "POINT(5 5 999)");

  checkIntersection("LINESTRING M (0 0 0, 10 10 10)", "LINESTRING M (10 0 0, 5 5 999, 0 10 10)",
      "POINT M(5 5 999)");
}

// testLineOverlapUnion
template<>
template<>
void object::test<5> ()
{
  set_test_name("testLineOverlapUnion");

  checkUnion("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (5 5 990, 15 15 999)",
      "LINESTRING Z(0 0 0, 5 5 990, 10 10 10, 15 15 999)");

  checkUnion("LINESTRING M (0 0 0, 10 10 10)", "LINESTRING M (5 5 990, 15 15 999)",
      "LINESTRING M(0 0 0, 5 5 990, 10 10 10, 15 15 999)");
}

// testLineLineXYDifferenceLineInterpolated
template<>
template<>
void object::test<6> ()
{
  checkDifference("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (5 5, 6 6)",
      "MULTILINESTRING ((0 0 0, 5 5 5), (6 6 6, 10 10 10))");

  checkDifference("LINESTRING M (0 0 0, 10 10 10)", "LINESTRING (5 5, 6 6)",
      "MULTILINESTRING M ((0 0 0, 5 5 5), (6 6 6, 10 10 10))");
}

// testLinePolygonIntersection
template<>
template<>
void object::test<7> ()
{
  checkIntersection("LINESTRING Z (0 0 0, 5 5 5)", "POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "LINESTRING Z (1 1 1, 5 5 5)");

  checkIntersection("LINESTRING M (0 0 0, 5 5 5)", "POLYGON M ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "LINESTRING M (1 1 1, 5 5 5)");
}

// testLinePolygonDifference
template<>
template<>
void object::test<8> ()
{
  checkDifference("LINESTRING Z (0 5 0, 10 5 10)", "POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "MULTILINESTRING Z((0 5 0, 1 5 2), (9 5 8, 10 5 10))");

  checkDifference("LINESTRING M (0 5 0, 10 5 10)", "POLYGON M ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "MULTILINESTRING M((0 5 0, 1 5 2), (9 5 8, 10 5 10))");
}

// testPointXYPolygonIntersection
// 2D point intersects interior of 3D polygon
// uses ElevationModel
template<>
template<>
void object::test<9> ()
{
  checkIntersection("POINT (5 5)", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "POINT Z(5 5 50)");
}

// XY Polygon gets Z value from Point
// testPointPolygonXYUnion
// uses ElevationModel
template<>
template<>
void object::test<10> ()
{
  checkUnion("POINT Z (5 5 77)", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
      "POLYGON Z((1 1 77, 1 9 77, 9 9 77, 9 1 77, 1 1 77))");
}

// testLinePolygonXYDifference
template<>
template<>
void object::test<11> ()
{
  checkDifference("LINESTRING Z (0 5 0, 10 5 10)", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
      "MULTILINESTRING Z((0 5 0, 1 5 1), (9 5 9, 10 5 10))");

  checkDifference("LINESTRING M (0 5 0, 10 5 10)", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
      "MULTILINESTRING M((0 5 0, 1 5 1), (9 5 9, 10 5 10))");
}

// testLineXYPolygonDifference
// Z values where line intersects polygon are populated by interpolating along polygon edges
// Z values from line points outside the polygon are populated by ElevationModel
template<>
template<>
void object::test<12> ()
{
  checkDifference("LINESTRING (0 5, 10 5)", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "MULTILINESTRING Z((0 5 50, 1 5 30), (9 5 70, 10 5 50))");
}

// testPolygonXYPolygonIntersection
// uses ElevationModel
template<>
template<>
void object::test<13> ()
{
  checkIntersection("POLYGON ((4 12, 2 6, 7 6, 11 4, 15 15, 4 12))", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "POLYGON Z((2 6 50, 3 9 60, 9 9 90, 9 5 70, 7 6 90, 2 6 50))");
}

// testPolygonXYPolygonUnion
// uses ElevationModel
template<>
template<>
void object::test<14> ()
{
  checkUnion("POLYGON ((0 3, 3 3, 3 0, 0 0, 0 3))", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "POLYGON Z((0 0 10, 0 3 50, 1 3 20, 1 9 50, 9 9 90, 9 1 50, 3 1 20, 3 0 50, 0 0 10))");
}

// Test that operation on XY geoms produces XY (Z = NaN)
// testPolygonXYPolygonXYIntersection
template<>
template<>
void object::test<15> ()
{
  checkIntersection("POLYGON ((4 12, 2 6, 7 6, 11 4, 15 15, 4 12))", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
      "POLYGON ((2 6, 3 9, 9 9, 9 5, 7 6, 2 6))");
}

// from https://trac.osgeo.org/geos/ticket/435
// testLineXYLineIntersection
// uses ElevationModel
template<>
template<>
void object::test<16> ()
{
    checkIntersection(
        "LINESTRING(0 0,0 10,10 10,10 0)",
        "LINESTRING(10 10 4,10 0 5,0 0 5)",
        "GEOMETRYCOLLECTION Z(POINT Z(0 0 5), LINESTRING Z(10 0 5, 10 10 4))"
    );
}

// Point Intersection: XYM - XYM
template<>
template<>
void object::test<17>()
{
  checkIntersection(
              "LINESTRING M (0 0 0, 10 10 10)",
              "LINESTRING M (10 0 0, 0 10 10)",
              "POINT M (5 5 5)");
}

// Point Intersection: XYZ - XYM
template<>
template<>
void object::test<18>()
{
  // XYZ - XYM
  checkIntersection(
              "LINESTRING Z (0 0 10, 10 10 20)",
              "LINESTRING M (10 0 0, 0 10 10)",
              "POINT ZM (5 5 15 5)");
}

// Point M is preserved
template<>
template<>
void object::test<19>()
{
    checkIntersection("POINT M (5 5 99)",
                      "POLYGON ((0 0, 5 0, 5 5, 0 0))",
                      "POINT M (5 5 99)");
}

template<>
template<>
void object::test<20>()
{
    set_test_name("LINESTRING / LINESTRING ZM intersection");

    checkIntersection("LINESTRING (0 0, 1 0)",
        "LINESTRING ZM (0 0 2 9, 1 0 5 7)",
        "LINESTRING ZM (0 0 2 9, 1 0 5 7)");
}

template<>
template<>
void object::test<21>()
{
    set_test_name("LINESTRING M / LINESTRING Z intersection");

    checkIntersection("LINESTRING M (0 0 9, 1 0 7)",
        "LINESTRING Z (1 0 5, 0 0 2)",
        "LINESTRING ZM (0 0 2 9, 1 0 5 7)");
}

template<>
template<>
void object::test<22> ()
{
    set_test_name("POLYGON XYM / POLYGON XYM intersection");

    checkIntersection("POLYGON M ((4 12 6, 2 6 7, 7 6 8, 11 4 9, 15 15 10, 4 12 6))", "POLYGON M ((1 9 1, 9 9 2, 9 1 3, 1 1 4, 1 9 1))",
        "POLYGON M ((2 6 7, 3 9 3.875, 9 9 2, 9 5 5.5, 7 6 8, 2 6 7))");
}

template<>
template<>
void object::test<23> ()
{
    set_test_name("LINESTRING XYM / LINESTRING XYM intersection");

    // inputs need > 20 vertices to activate LineLimiter
    checkIntersection("LINESTRING M (4 12 6,3.5 10.5 6.2,3 9 6.5,2.5 7.5 6.8,2 6 7,3.7 6 7.3,5.3 6 7.7,7 6 8,8.3 5.3 8.3,9.7 4.7 8.7,11 4 9,11.7 5.8 9.2,12.3 7.7 9.3,13 9.5 9.5,13.7 11.3 9.7,14.3 13.2 9.8,15 15 10,13.2 14.5 9.3,11.3 14 8.7,9.5 13.5 8,7.7 13 7.3,5.8 12.5 6.7,4 12 6)",
        "LINESTRING M (1 9 1,2.3 9 1.2,3.7 9 1.3,5 9 1.5,6.3 9 1.7,7.7 9 1.8,9 9 2,9 7.7 2.2,9 6.3 2.3,9 5 2.5,9 3.7 2.7,9 2.3 2.8,9 1 3,7.7 1 3.2,6.3 1 3.3,5 1 3.5,3.7 1 3.7,2.3 1 3.8,1 1 4,1 2.3 3.5,1 3.7 3,1 5 2.5,1 6.3 2,1 7.7 1.5,1 9 1)",
        "MULTIPOINT M ((3 9 6.5), (9 5 2.5))");
}

} // namespace tut
