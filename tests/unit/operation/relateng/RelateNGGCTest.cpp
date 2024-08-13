//
// Test Suite for geos::operation::relateng::RelateNG class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include "RelateNGTest.h"

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relatenggc_data : test_relateng_support {

    const char* wktAdjacentPolys = "GEOMETRYCOLLECTION (POLYGON ((5 5, 2 9, 9 9, 9 5, 5 5)), POLYGON ((3 1, 5 5, 9 5, 9 1, 3 1)), POLYGON ((1 9, 2 9, 5 5, 3 1, 1 1, 1 9)))";

};


typedef test_group<test_relatenggc_data> group;
typedef group::object object;

group test_relatenggc_group("geos::operation::relateng::RelateNGGC");


// testDimensionWithEmpty
template<>
template<>
void object::test<1> ()
{
    std::string a = "LINESTRING(0 0, 1 1)";
    std::string b = "GEOMETRYCOLLECTION(POLYGON EMPTY,LINESTRING(0 0, 1 1))";
    checkCoversCoveredBy(a, b, true);
    checkEquals(a, b, true);
}
  
// see https://github.com/libgeos/geos/issues/1027
// testMP_GLP_GEOS1027
template<>
template<>
void object::test<2> ()
{
    std::string a = "MULTIPOLYGON (((0 0, 3 0, 3 3, 0 3, 0 0)))";
    std::string b = "GEOMETRYCOLLECTION ( LINESTRING (1 2, 1 1), POINT (0 0))";
    checkRelate(a, b, "1020F1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCrosses(a, b, false);
    checkEquals(a, b, false);
}
  
// see https://github.com/libgeos/geos/issues/1022
// testGPL_A
template<>
template<>
void object::test<3> ()
{
    std::string a = "GEOMETRYCOLLECTION (POINT (7 1), LINESTRING (6 5, 6 4))";
    std::string b = "POLYGON ((7 1, 1 3, 3 9, 7 1))";
    checkRelate(a, b, "F01FF0212");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCrosses(a, b, false);
    checkTouches(a, b, true);
    checkEquals(a, b, false);
}
  
// see https://github.com/libgeos/geos/issues/982
// testP_GPL
template<>
template<>
void object::test<4> ()
{
    std::string a = "POINT(0 0)";
    std::string b = "GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(0 0, 1 0))";
    checkRelate(a, b, "F0FFFF102");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCrosses(a, b, false);
    checkTouches(a, b, true);
    checkEquals(a, b, false);
}
  
// testLineInOverlappingPolygonsTouchingInteriorEdge
template<>
template<>
void object::test<5> ()
{
    std::string a = "LINESTRING (3 7, 7 3)";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 9, 7 9, 7 3, 1 3, 1 9)), POLYGON ((9 1, 3 1, 3 7, 9 7, 9 1)))";
    checkRelate(a, b, "1FF0FF212");
    checkContainsWithin(b, a, true);
}
  
// testLineInOverlappingPolygonsCrossingInteriorEdgeAtVertex
template<>
template<>
void object::test<6> ()
{
    std::string a = "LINESTRING (2 2, 8 8)";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 7, 7 7, 7 1, 1 1)), POLYGON ((9 9, 9 3, 3 3, 3 9, 9 9)))";
    checkRelate(a, b, "1FF0FF212");
    checkContainsWithin(b, a, true);
}
  
// testLineInOverlappingPolygonsCrossingInteriorEdgeProper
template<>
template<>
void object::test<7> ()
{
    std::string a = "LINESTRING (2 4, 6 8)";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 7, 7 7, 7 1, 1 1)), POLYGON ((9 9, 9 3, 3 3, 3 9, 9 9)))";
    checkRelate(a, b, "1FF0FF212");
    checkContainsWithin(b, a, true);
}
  
// testPolygonInOverlappingPolygonsTouchingBoundaries
template<>
template<>
void object::test<8> ()
{
    std::string a = "GEOMETRYCOLLECTION (POLYGON ((1 9, 6 9, 6 4, 1 4, 1 9)), POLYGON ((9 1, 4 1, 4 6, 9 6, 9 1)) )";
    std::string b = "POLYGON ((2 6, 6 2, 8 4, 4 8, 2 6))";
    checkRelate(a, b, "212F01FF2");
    checkContainsWithin(a, b, true);
}
  
// testLineInOverlappingPolygonsBoundaries
template<>
template<>
void object::test<9> ()
{
    std::string a = "LINESTRING (1 6, 9 6, 9 1, 1 1, 1 6)";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 6, 6 1, 1 1)), POLYGON ((9 1, 4 1, 4 6, 9 6, 9 1)))";
    checkRelate(a, b, "F1FFFF2F2");
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkCoversCoveredBy(b, a, true);
}
  
// testLineCoversOverlappingPolygonsBoundaries
template<>
template<>
void object::test<10> ()
{
    std::string a = "LINESTRING (1 6, 9 6, 9 1, 1 1, 1 6)";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 6, 6 1, 1 1)), POLYGON ((9 1, 4 1, 4 6, 9 6, 9 1)))";
    checkRelate(a, b, "F1FFFF2F2");
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(b, a, true);
}
  
// testAdjacentPolygonsContainedInAdjacentPolygons
template<>
template<>
void object::test<11> ()
{
    std::string a = "GEOMETRYCOLLECTION (POLYGON ((2 2, 2 5, 4 5, 4 2, 2 2)), POLYGON ((8 2, 4 3, 4 4, 8 5, 8 2)))";
    std::string b = "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 4 6, 4 1, 1 1)), POLYGON ((9 1, 4 1, 4 6, 9 6, 9 1)))";
    checkRelate(a, b, "2FF1FF212");
    checkContainsWithin(b, a, true);
    checkCoversCoveredBy(b, a, true);
}
  
// testGCMultiPolygonIntersectsPolygon
template<>
template<>
void object::test<12> ()
{
    std::string a = "POLYGON ((2 5, 3 5, 3 3, 2 3, 2 5))";
    std::string b = "GEOMETRYCOLLECTION (MULTIPOLYGON (((1 4, 4 4, 4 1, 1 1, 1 4)), ((5 4, 8 4, 8 1, 5 1, 5 4))))";
    checkRelate(a, b, "212101212");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(b, a, false);
}
  

// testPolygonContainsGCMultiPolygonElement
template<>
template<>
void object::test<13> ()
{
    std::string a = "POLYGON ((0 5, 4 5, 4 1, 0 1, 0 5))";
    std::string b = "GEOMETRYCOLLECTION (MULTIPOLYGON (((1 4, 3 4, 3 2, 1 2, 1 4)), ((6 4, 8 4, 8 2, 6 2, 6 4))))";
    checkRelate(a, b, "212FF1212");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(b, a, false);
}

  /**
   * Demonstrates the need for assigning computed nodes to their rings,
   * so that subsequent PIP testing can report node as being on ring boundary.
   */
// testPolygonOverlappingGCPolygon
template<>
template<>
void object::test<14> ()
{
    std::string a = "GEOMETRYCOLLECTION (POLYGON ((18.6 40.8, 16.8825 39.618567, 16.9319 39.5461, 17.10985 39.485133, 16.6143 38.4302, 16.43145 38.313267, 16.2 37.5, 14.8 37.8, 14.96475 40.474933, 18.6 40.8)))";
    std::string b = "POLYGON ((16.3649953125 38.37219358064516, 16.3649953125 39.545924774193544, 17.949465625000002 39.545924774193544, 17.949465625000002 38.37219358064516, 16.3649953125 38.37219358064516))";
    checkRelate(b, a, "212101212");
    checkRelate(a, b, "212101212");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, false);
}
  

// testAdjPolygonsCoverPolygonWithEndpointInside
template<>
template<>
void object::test<15> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "POLYGON ((3 7, 7 7, 7 3, 3 3, 3 7))";
    checkRelate(b, a, "2FF1FF212");
    checkRelate(a, b, "212FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// testAdjPolygonsCoverPointAtNode
template<>
template<>
void object::test<16> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "POINT (5 5)";
    checkRelate(b, a, "0FFFFF212");
    checkRelate(a, b, "0F2FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// testAdjPolygonsCoverPointOnEdge
template<>
template<>
void object::test<17> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "POINT (7 5)";
    checkRelate(b, a, "0FFFFF212");
    checkRelate(a, b, "0F2FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// testAdjPolygonsContainingPolygonTouchingInteriorEndpoint
template<>
template<>
void object::test<18> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "POLYGON ((5 5, 7 5, 7 3, 5 3, 5 5))";
    checkRelate(a, b, "212FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// testAdjPolygonsOverlappedByPolygonWithHole
template<>
template<>
void object::test<19> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "POLYGON ((0 10, 10 10, 10 0, 0 0, 0 10), (2 8, 8 8, 8 2, 2 2, 2 8))";
    checkRelate(a, b, "2121FF212");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, false);
}
  
// testAdjPolygonsContainingLine
template<>
template<>
void object::test<20> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "LINESTRING (5 5, 7 7)";
    checkRelate(a, b, "102FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// testAdjPolygonsContainingLineAndPoint
template<>
template<>
void object::test<21> ()
{
    std::string a = wktAdjacentPolys;
    std::string b = "GEOMETRYCOLLECTION (POINT (5 5), LINESTRING (5 7, 7 7))";
    checkRelate(a, b, "102FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkCoversCoveredBy(a, b, true);
}
  
// https://trac.osgeo.org/geos/ticket/1110
template<>
template<>
void object::test<22> ()
{
    std::string a = "POLYGON ((3 7, 7 7, 7 3, 3 3, 3 7))";
    std::string b = "GEOMETRYCOLLECTION (MULTIPOINT (EMPTY, (5 5)), LINESTRING (1 9, 4 9))";
    checkIntersectsDisjoint(a, b, true);
}



} // namespace tut
