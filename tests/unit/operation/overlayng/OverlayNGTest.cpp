//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/overlayng/OverlayNG.h>

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
struct test_overlayng_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

  // public static Geometry intersectionNoOpt(Geometry a, Geometry b, double scaleFactor) {
  //   PrecisionModel pm = new PrecisionModel(scaleFactor);
  //   OverlayNG ov = new OverlayNG(a, b, pm, INTERSECTION);
  //   ov.setOptimized(false);
  //   return ov.getResult();
  // }

};

typedef test_group<test_overlayng_data> group;
typedef group::object object;

group test_overlayng_group("geos::overation::overlayng::OverlayNG");

//
// Test Cases
//

//  Square overlapping square
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON((1000 1000, 2000 1000, 2000 2000, 1000 2000, 1000 1000))";
    std::string b = "POLYGON((1500 1500, 2500 1500, 2500 2500, 1500 2500, 1500 1500))";
    std::string exp = "POLYGON((1500 2000,2000 2000,2000 1500,1500 1500,1500 2000))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyGCBothIntersection
template<>
template<>
void object::test<2> ()
{
    std::string a = "GEOMETRYCOLLECTION EMPTY";
    std::string b = "GEOMETRYCOLLECTION EMPTY";
    std::string exp = "GEOMETRYCOLLECTION EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyAPolygonIntersection
template<>
template<>
void object::test<3> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyBIntersection
template<>
template<>
void object::test<4> ()
{
    std::string a = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string b = "POLYGON EMPTY";
    std::string exp = "POLYGON EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyABIntersection
template<>
template<>
void object::test<5> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "POLYGON EMPTY";
    std::string exp = "POLYGON EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyADifference
template<>
template<>
void object::test<6> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON EMPTY";
    testOverlay(a, b, exp, OverlayNG::DIFFERENCE, 1);
}

//  testEmptyAUnion
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

//  testEmptyASymDifference
template<>
template<>
void object::test<8> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    testOverlay(a, b, exp, OverlayNG::SYMDIFFERENCE, 1);
}

//  testEmptyLinePolygonIntersection
template<>
template<>
void object::test<9> ()
{
    std::string a = "LINESTRING EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "LINESTRING EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testEmptyLinePolygonDifference
template<>
template<>
void object::test<10> ()
{
    std::string a = "LINESTRING EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "LINESTRING EMPTY";
    testOverlay(a, b, exp, OverlayNG::DIFFERENCE, 1);
}

//  testEmptyPointPolygonIntersection
template<>
template<>
void object::test<11> ()
{
    std::string a = "POINT EMPTY";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testDisjointIntersection
template<>
template<>
void object::test<12> ()
{
    std::string a = "POLYGON ((60 90, 90 90, 90 60, 60 60, 60 90))";
    std::string b = "POLYGON ((200 300, 300 300, 300 200, 200 200, 200 300))";
    std::string exp = "POLYGON EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testPolygoPolygonWithLineTouchIntersection
template<>
template<>
void object::test<13> ()
{
    std::string a = "POLYGON ((360 200, 220 200, 220 180, 300 180, 300 160, 300 140, 360 200))";
    std::string b = "MULTIPOLYGON (((280 180, 280 160, 300 160, 300 180, 280 180)), ((220 230, 240 230, 240 180, 220 180, 220 230)))";
    std::string exp = "POLYGON ((220 200, 240 200, 240 180, 220 180, 220 200))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testBoxTriIntersection
template<>
template<>
void object::test<14> ()
{
    std::string a = "POLYGON ((0 6, 4 6, 4 2, 0 2, 0 6))";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((3 2, 1 2, 2 5, 3 2))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testBoxTriUnion
template<>
template<>
void object::test<15> ()
{
    std::string a = "POLYGON ((0 6, 4 6, 4 2, 0 2, 0 6))";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((0 6, 4 6, 4 2, 3 2, 3 0, 1 0, 1 2, 0 2, 0 6))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

// //  test2spikesIntersection
// template<>
// template<>
// void object::test<16> ()
// {
//     std::string a = "POLYGON ((0 100, 40 100, 40 0, 0 0, 0 100))";
//     std::string b = "POLYGON ((70 80, 10 80, 60 50, 11 20, 69 11, 70 80))";
//     std::string exp = "MULTIPOLYGON (((40 80, 40 62, 10 80, 40 80)), ((40 38, 40 16, 11 20, 40 38)))";
//     testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
// }

// //  test2spikesUnion
// template<>
// template<>
// void object::test<17> ()
// {
//     std::string a = "POLYGON ((0 100, 40 100, 40 0, 0 0, 0 100))";
//     std::string b = "POLYGON ((70 80, 10 80, 60 50, 11 20, 69 11, 70 80))";
//     std::string exp = "POLYGON ((0 100, 40 100, 40 80, 70 80, 69 11, 40 16, 40 0, 0 0, 0 100), (40 62, 40 38, 60 50, 40 62))";
//     testOverlay(a, b, exp, OverlayNG::UNION, 1);
// }

//  testTriBoxIntersection
template<>
template<>
void object::test<18> ()
{
    std::string a = "POLYGON ((68 35, 35 42, 40 9, 68 35))";
    std::string b = "POLYGON ((20 60, 50 60, 50 30, 20 30, 20 60))";
    std::string exp = "POLYGON ((37 30, 35 42, 50 39, 50 30, 37 30))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testNestedShellsIntersection
template<>
template<>
void object::test<19> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    std::string exp = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

//  testNestedShellsUnion
template<>
template<>
void object::test<20> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    std::string exp = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}








 // public void testDisjointIntersectionNoOpt() {
 //    Geometry a = read("POLYGON ((60 90, 90 90, 90 60, 60 60, 60 90))");
 //    Geometry b = read("POLYGON ((200 300, 300 300, 300 200, 200 200, 200 300))");
 //    Geometry expected = read("POLYGON EMPTY");
 //    Geometry actual = intersectionNoOpt(a, b, 1);
 //    checkEqual(expected, actual);
 //  }


} // namespace tut
