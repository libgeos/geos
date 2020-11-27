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
        w.setTrim(true);
        w.setOutputDimension(3);
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
}

// testLineIntersectionPointZInterpolated
template<>
template<>
void object::test<3> ()
{
  checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 0 10 10)",
      "POINT(5 5 5)");
}

// testLineIntersectionPointZValue
template<>
template<>
void object::test<4> ()
{
  checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 5 5 999, 0 10 10)",
      "POINT(5 5 999)");
}

// testLineOverlapUnion
template<>
template<>
void object::test<5> ()
{
  checkUnion("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (5 5 990, 15 15 999)",
      "MULTILINESTRING Z((0 0 0, 5 5 990), (5 5 990, 10 10 10), (10 10 10, 15 15 999))");
}

// testLineLineXYDifferenceLineInterpolated
template<>
template<>
void object::test<6> ()
{
  checkDifference("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (5 5, 6 6)",
      "MULTILINESTRING ((0 0 0, 5 5 5), (6 6 6, 10 10 10))");
}

// testLinePolygonIntersection
template<>
template<>
void object::test<7> ()
{
  checkIntersection("LINESTRING Z (0 0 0, 5 5 5)", "POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "LINESTRING Z (1 1 1, 5 5 5)");
}

// testLinePolygonDifference
template<>
template<>
void object::test<8> ()
{
  checkDifference("LINESTRING Z (0 5 0, 10 5 10)", "POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
      "MULTILINESTRING Z((0 5 0, 1 5 2), (9 5 8, 10 5 10))");
}

// testPointXYPolygonIntersection
template<>
template<>
void object::test<9> ()
{
  checkIntersection("POINT (5 5)", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "POINT Z(5 5 50)");
}

// XY Polygon gets Z value from Point
// testPointPolygonXYUnionn
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
}

// testLineXYPolygonDifference
template<>
template<>
void object::test<12> ()
{
  checkDifference("LINESTRING (0 5, 10 5)", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "MULTILINESTRING Z((0 5 50, 1 5 30), (9 5 70, 10 5 50))");
}

// testPolygonXYPolygonIntersection
template<>
template<>
void object::test<13> ()
{
  checkIntersection("POLYGON ((4 12, 2 6, 7 6, 11 4, 15 15, 4 12))", "POLYGON Z ((1 9 50, 9 9 90, 9 1 50, 1 1 10, 1 9 50))",
      "POLYGON Z((2 6 50, 3 9 60, 9 9 90, 9 5 70, 7 6 90, 2 6 50))");
}

// testPolygonXYPolygonUnion
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

} // namespace tut
