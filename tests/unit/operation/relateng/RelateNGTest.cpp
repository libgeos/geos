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
struct test_relateng_data : test_relateng_support {};


typedef test_group<test_relateng_data> group;
typedef group::object object;

group test_relateng_group("geos::operation::relateng::RelateNG");


// testPointsDisjoint
template<>
template<>
void object::test<1> ()
{
    std::string a = "POINT (0 0)";
    std::string b = "POINT (1 1)";
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
    checkEquals(a, b, false);
    checkRelate(a, b, "FF0FFF0F2");
}

//======= P/P  =============

// testPointsContained
template<>
template<>
void object::test<2> ()
{
    std::string a = "MULTIPOINT (0 0, 1 1, 2 2)";
    std::string b = "MULTIPOINT (1 1, 2 2)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkEquals(a, b, false);
    checkRelate(a, b, "0F0FFFFF2");
}

// testPointsEqual
template<>
template<>
void object::test<3> ()
{
    std::string a = "MULTIPOINT (0 0, 1 1, 2 2)";
    std::string b = "MULTIPOINT (0 0, 1 1, 2 2)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkEquals(a, b, true);
}

// testValidateRelatePP_13
template<>
template<>
void object::test<4> ()
{
    std::string a = "MULTIPOINT ((80 70), (140 120), (20 20), (200 170))";
    std::string b = "MULTIPOINT ((80 70), (140 120), (80 170), (200 80))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, true);
    checkTouches(a, b, false);
}

//======= L/P  =============

// testLinePointContains
template<>
template<>
void object::test<5> ()
{
    std::string a = "LINESTRING (0 0, 1 1, 2 2)";
    std::string b = "MULTIPOINT (0 0, 1 1, 2 2)";
    checkRelate(a, b, "0F10FFFF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(a, b, true);
    checkCoversCoveredBy(b, a, false);
}

// testLinePointOverlaps
template<>
template<>
void object::test<6> ()
{
    std::string a = "LINESTRING (0 0, 1 1)";
    std::string b = "MULTIPOINT (0 0, 1 1, 2 2)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(a, b, false);
    checkCoversCoveredBy(b, a, false);
}

// testZeroLengthLinePoint
template<>
template<>
void object::test<7> ()
{
    std::string a = "LINESTRING (0 0, 0 0)";
    std::string b = "POINT (0 0)";
    checkRelate(a, b, "0FFFFFFF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkContainsWithin(b, a, true);
    checkCoversCoveredBy(a, b, true);
    checkCoversCoveredBy(b, a, true);
    checkEquals(a, b, true);
}

// testZeroLengthLineLine
template<>
template<>
void object::test<8> ()
{
    std::string a = "LINESTRING (10 10, 10 10, 10 10)";
    std::string b = "LINESTRING (10 10, 10 10)";
    checkRelate(a, b, "0FFFFFFF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkContainsWithin(b, a, true);
    checkCoversCoveredBy(a, b, true);
    checkCoversCoveredBy(b, a, true);
    checkEquals(a, b, true);
}

// tests bug involving checking for non-zero-length lines
// testNonZeroLengthLinePoint
template<>
template<>
void object::test<9> ()
{
    std::string a = "LINESTRING (0 0, 0 0, 9 9)";
    std::string b = "POINT (1 1)";
    checkRelate(a, b, "0F1FF0FF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(a, b, true);
    checkCoversCoveredBy(b, a, false);
    checkEquals(a, b, false);
}

// testLinePointIntAndExt
template<>
template<>
void object::test<10> ()
{
    std::string a = "MULTIPOINT((60 60), (100 100))";
    std::string b = "LINESTRING(40 40, 80 80)";
    checkRelate(a, b, "0F0FFF102");
}

//======= L/L  =============

// testLinesCrossProper
template<>
template<>
void object::test<11> ()
{
    std::string a = "LINESTRING (0 0, 9 9)";
    std::string b = "LINESTRING(0 9, 9 0)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
}

// testLinesOverlap
template<>
template<>
void object::test<12> ()
{
    std::string a = "LINESTRING (0 0, 5 5)";
    std::string b = "LINESTRING(3 3, 9 9)";
    checkIntersectsDisjoint(a, b, true);
    checkTouches(a, b, false);
    checkOverlaps(a, b, true);
}

// testLinesCrossVertex
template<>
template<>
void object::test<13> ()
{
    std::string a = "LINESTRING (0 0, 8 8)";
    std::string b = "LINESTRING(0 8, 4 4, 8 0)";
    checkIntersectsDisjoint(a, b, true);
}

// testLinesTouchVertex
template<>
template<>
void object::test<14> ()
{
    std::string a = "LINESTRING (0 0, 8 0)";
    std::string b = "LINESTRING(0 8, 4 0, 8 8)";
    checkIntersectsDisjoint(a, b, true);
}

// testLinesDisjointByEnvelope
template<>
template<>
void object::test<15> ()
{
    std::string a = "LINESTRING (0 0, 9 9)";
    std::string b = "LINESTRING(10 19, 19 10)";
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
}

// testLinesDisjoint
template<>
template<>
void object::test<16> ()
{
    std::string a = "LINESTRING (0 0, 9 9)";
    std::string b = "LINESTRING (4 2, 8 6)";
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
}

// testLinesClosedEmpty
template<>
template<>
void object::test<17> ()
{
    std::string a = "MULTILINESTRING ((0 0, 0 1), (0 1, 1 1, 1 0, 0 0))";
    std::string b = "LINESTRING EMPTY";
    checkRelate(a, b, "FF1FFFFF2");
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
}

// testLinesRingTouchAtNode
template<>
template<>
void object::test<18> ()
{
    std::string a = "LINESTRING (5 5, 1 8, 1 1, 5 5)";
    std::string b = "LINESTRING (5 5, 9 5)";
    checkRelate(a, b, "F01FFF102");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkTouches(a, b, true);
}

// testLinesTouchAtBdy
template<>
template<>
void object::test<19> ()
{
    std::string a = "LINESTRING (5 5, 1 8)";
    std::string b = "LINESTRING (5 5, 9 5)";
    checkRelate(a, b, "FF1F00102");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkTouches(a, b, true);
}

// testLinesOverlapWithDisjointLine
template<>
template<>
void object::test<20> ()
{
    std::string a = "LINESTRING (1 1, 9 9)";
    std::string b = "MULTILINESTRING ((2 2, 8 8), (6 2, 8 4))";
    checkRelate(a, b, "101FF0102");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkOverlaps(a, b, true);
}

// testLinesDisjointOverlappingEnvelopes
template<>
template<>
void object::test<21> ()
{
    std::string a = "LINESTRING (60 0, 20 80, 100 80, 80 120, 40 140)";
    std::string b = "LINESTRING (60 40, 140 40, 140 160, 0 160)";
    checkRelate(a, b, "FF1FF0102");
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
    checkTouches(a, b, false);
}

/**
* Case from https://github.com/locationtech/jts/issues/270
* Strictly, the lines cross, since their interiors intersect
* according to the Orientation predicate.
* However, the computation of the intersection point is
* non-robust, and reports it as being equal to the endpoint
* POINT (-10 0.0000000000000012)
* For consistency the relate algorithm uses the intersection node topology.
*/
// testLinesCross_JTS270
template<>
template<>
void object::test<22> ()
{
    std::string a = "LINESTRING (0 0, -10 0.0000000000000012)";
    std::string b = "LINESTRING (-9.999143275740073 -0.1308959557133398, -10 0.0000000000001054)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkCrosses(a, b, false);
    checkOverlaps(a, b, false);
    checkTouches(a, b, true);
}

// testLinesContained_JTS396
template<>
template<>
void object::test<23> ()
{
    std::string a = "LINESTRING (1 0, 0 2, 0 0, 2 2)";
    std::string b = "LINESTRING (0 0, 2 2)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkCrosses(a, b, false);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}


/**
* This case shows that lines must be self-noded,
* so that node topology is constructed correctly
* (at least for some predicates).
*/
// testLinesContainedWithSelfIntersection
template<>
template<>
void object::test<24> ()
{
    std::string a = "LINESTRING (2 0, 0 2, 0 0, 2 2)";
    std::string b = "LINESTRING (0 0, 2 2)";
  //checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkCrosses(a, b, false);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}

// testLineContainedInRing
template<>
template<>
void object::test<25> ()
{
    std::string a = "LINESTRING(60 60, 100 100, 140 60)";
    std::string b = "LINESTRING(100 100, 180 20, 20 20, 100 100)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(b, a, true);
    checkCoversCoveredBy(b, a, true);
    checkCrosses(a, b, false);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}

// see https://github.com/libgeos/geos/issues/933
// testLineLineProperIntersection
template<>
template<>
void object::test<26> ()
{
    std::string a = "MULTILINESTRING ((0 0, 1 1), (0.5 0.5, 1 0.1, -1 0.1))";
    std::string b = "LINESTRING (0 0, 1 1)";
  //checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkCrosses(a, b, false);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}

// testLineSelfIntersectionCollinear
template<>
template<>
void object::test<27> ()
{
    std::string a = "LINESTRING (9 6, 1 6, 1 0, 5 6, 9 6)";
    std::string b = "LINESTRING (9 9, 3 1)";
    checkRelate(a, b, "0F1FFF102");
}

//======= A/P  =============

// testPolygonPointInside
template<>
template<>
void object::test<28> ()
{
    std::string a = "POLYGON ((0 10, 10 10, 10 0, 0 0, 0 10))";
    std::string b = "POINT (1 1)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
}

// testPolygonPointOutside
template<>
template<>
void object::test<29> ()
{
    std::string a = "POLYGON ((10 0, 0 0, 0 10, 10 0))";
    std::string b = "POINT (8 8)";
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
}

// testPolygonPointInBoundary
template<>
template<>
void object::test<30> ()
{
    std::string a = "POLYGON ((10 0, 0 0, 0 10, 10 0))";
    std::string b = "POINT (1 0)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, true);
}

// testAreaPointInExterior
template<>
template<>
void object::test<31> ()
{
    std::string a = "POLYGON ((1 5, 5 5, 5 1, 1 1, 1 5))";
    std::string b = "POINT (7 7)";
    checkRelate(a, b, "FF2FF10F2");
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkTouches(a, b, false);
    checkOverlaps(a, b, false);
}

//======= A/L  =============


// testAreaLineContainedAtLineVertex
template<>
template<>
void object::test<32> ()
{
    std::string a = "POLYGON ((1 5, 5 5, 5 1, 1 1, 1 5))";
    std::string b = "LINESTRING (2 3, 3 5, 4 3)";
    checkIntersectsDisjoint(a, b, true);
    checkTouches(a, b, false);
    checkOverlaps(a, b, false);
}

// testAreaLineTouchAtLineVertex
template<>
template<>
void object::test<33> ()
{
    std::string a = "POLYGON ((1 5, 5 5, 5 1, 1 1, 1 5))";
    std::string b = "LINESTRING (1 8, 3 5, 5 8)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkTouches(a, b, true);
    checkOverlaps(a, b, false);
}

// testPolygonLineInside
template<>
template<>
void object::test<34> ()
{
    std::string a = "POLYGON ((0 10, 10 10, 10 0, 0 0, 0 10))";
    std::string b = "LINESTRING (1 8, 3 5, 5 8)";
    checkRelate(a, b, "102FF1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
}

// testPolygonLineOutside
template<>
template<>
void object::test<35> ()
{
    std::string a = "POLYGON ((10 0, 0 0, 0 10, 10 0))";
    std::string b = "LINESTRING (4 8, 9 3)";
    checkIntersectsDisjoint(a, b, false);
    checkContainsWithin(a, b, false);
}

// testPolygonLineInBoundary
template<>
template<>
void object::test<36> ()
{
    std::string a = "POLYGON ((10 0, 0 0, 0 10, 10 0))";
    std::string b = "LINESTRING (1 0, 9 0)";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, true);
    checkTouches(a, b, true);
    checkOverlaps(a, b, false);
}

// testPolygonLineCrossingContained
template<>
template<>
void object::test<37> ()
{
    std::string a = "MULTIPOLYGON (((20 80, 180 80, 100 0, 20 80)), ((20 160, 180 160, 100 80, 20 160)))";
    std::string b = "LINESTRING (100 140, 100 40)";
    checkRelate(a, b, "1020F1FF2");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkTouches(a, b, false);
    checkOverlaps(a, b, false);
}

// testValidateRelateLA_220
template<>
template<>
void object::test<38> ()
{
    std::string a = "LINESTRING (90 210, 210 90)";
    std::string b = "POLYGON ((150 150, 410 150, 280 20, 20 20, 150 150))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkTouches(a, b, false);
    checkOverlaps(a, b, false);
}

  /**
   * See RelateLA.xml (line 585)
   */
// testLineCrossingPolygonAtShellHolePoint
template<>
template<>
void object::test<39> ()
{
    std::string a = "LINESTRING (60 160, 150 70)";
    std::string b = "POLYGON ((190 190, 360 20, 20 20, 190 190), (110 110, 250 100, 140 30, 110 110))";
    checkRelate(a, b, "F01FF0212");
    checkTouches(a, b, true);
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkTouches(a, b, true);
    checkOverlaps(a, b, false);
}

// testLineCrossingPolygonAtNonVertex
template<>
template<>
void object::test<40> ()
{
    std::string a = "LINESTRING (20 60, 150 60)";
    std::string b = "POLYGON ((150 150, 410 150, 280 20, 20 20, 150 150))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkTouches(a, b, false);
    checkOverlaps(a, b, false);
}

// testPolygonLinesContainedCollinearEdge
template<>
template<>
void object::test<41> ()
{
    std::string a = "POLYGON ((110 110, 200 20, 20 20, 110 110))";
    std::string b = "MULTILINESTRING ((110 110, 60 40, 70 20, 150 20, 170 40), (180 30, 40 30, 110 80))";
    checkRelate(a, b, "102101FF2");
}

//======= A/A  =============


// testPolygonsEdgeAdjacent
template<>
template<>
void object::test<42> ()
{
    std::string a = "POLYGON ((1 3, 3 3, 3 1, 1 1, 1 3))";
    std::string b = "POLYGON ((5 3, 5 1, 3 1, 3 3, 5 3))";
  //checkIntersectsDisjoint(a, b, true);
    checkOverlaps(a, b, false);
    checkTouches(a, b, true);
}

// testPolygonsEdgeAdjacent2
template<>
template<>
void object::test<43> ()
{
    std::string a = "POLYGON ((1 3, 4 3, 3 0, 1 1, 1 3))";
    std::string b = "POLYGON ((5 3, 5 1, 3 0, 4 3, 5 3))";
  //checkIntersectsDisjoint(a, b, true);
    checkOverlaps(a, b, false);
    checkTouches(a, b, true);
}

// testPolygonsNested
template<>
template<>
void object::test<44> ()
{
    std::string a = "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))";
    std::string b = "POLYGON ((2 8, 8 8, 8 2, 2 2, 2 8))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}

// testPolygonsOverlapProper
template<>
template<>
void object::test<45> ()
{
    std::string a = "POLYGON ((1 1, 1 7, 7 7, 7 1, 1 1))";
    std::string b = "POLYGON ((2 8, 8 8, 8 2, 2 2, 2 8))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, true);
    checkTouches(a, b, false);
}

// testPolygonsOverlapAtNodes
template<>
template<>
void object::test<46> ()
{
    std::string a = "POLYGON ((1 5, 5 5, 5 1, 1 1, 1 5))";
    std::string b = "POLYGON ((7 3, 5 1, 3 3, 5 5, 7 3))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, true);
    checkTouches(a, b, false);
}

// testPolygonsContainedAtNodes
template<>
template<>
void object::test<47> ()
{
    std::string a = "POLYGON ((1 5, 5 5, 6 2, 1 1, 1 5))";
    std::string b = "POLYGON ((1 1, 5 5, 6 2, 1 1))";
  //checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, true);
    checkCoversCoveredBy(a, b, true);
    checkOverlaps(a, b, false);
    checkTouches(a, b, false);
}

// testPolygonsNestedWithHole
template<>
template<>
void object::test<48> ()
{
    std::string a = "POLYGON ((40 60, 420 60, 420 320, 40 320, 40 60), (200 140, 160 220, 260 200, 200 140))";
    std::string b = "POLYGON ((80 100, 360 100, 360 280, 80 280, 80 100))";
  //checkIntersectsDisjoint(true, a, b);
    checkContainsWithin(a, b, false);
    checkContainsWithin(b, a, false);
  //checkCoversCoveredBy(false, a, b);
  //checkOverlaps(true, a, b);
    checkPredicate(*RelatePredicate::contains(), a, b, false);
  //checkTouches(false, a, b);
}

// testPolygonsOverlappingWithBoundaryInside
template<>
template<>
void object::test<49> ()
{
    std::string a = "POLYGON ((100 60, 140 100, 100 140, 60 100, 100 60))";
    std::string b = "MULTIPOLYGON (((80 40, 120 40, 120 80, 80 80, 80 40)), ((120 80, 160 80, 160 120, 120 120, 120 80)), ((80 120, 120 120, 120 160, 80 160, 80 120)), ((40 80, 80 80, 80 120, 40 120, 40 80)))";
    checkRelate(a, b, "21210F212");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkContainsWithin(b, a, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, true);
    checkTouches(a, b, false);
}

// testPolygonsOverlapVeryNarrow
template<>
template<>
void object::test<50> ()
{
    std::string a = "POLYGON ((120 100, 120 200, 200 200, 200 100, 120 100))";
    std::string b = "POLYGON ((100 100, 100000 110, 100000 100, 100 100))";
    checkRelate(a, b, "212111212");
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkContainsWithin(b, a, false);
  //checkCoversCoveredBy(false, a, b);
  //checkOverlaps(true, a, b);
  //checkTouches(false, a, b);
}

// testValidateRelateAA_86
template<>
template<>
void object::test<51> ()
{
    std::string a = "POLYGON ((170 120, 300 120, 250 70, 120 70, 170 120))";
    std::string b = "POLYGON ((150 150, 410 150, 280 20, 20 20, 150 150), (170 120, 330 120, 260 50, 100 50, 170 120))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, false);
    checkPredicate(*RelatePredicate::within(), a, b, false);
    checkTouches(a, b, true);
}

// testValidateRelateAA_97
template<>
template<>
void object::test<52> ()
{
    std::string a = "POLYGON ((330 150, 200 110, 150 150, 280 190, 330 150))";
    std::string b = "MULTIPOLYGON (((140 110, 260 110, 170 20, 50 20, 140 110)), ((300 270, 420 270, 340 190, 220 190, 300 270)))";
    checkIntersectsDisjoint(a, b, true);
    checkContainsWithin(a, b, false);
    checkCoversCoveredBy(a, b, false);
    checkOverlaps(a, b, false);
    checkPredicate(*RelatePredicate::within(), a, b, false);
    checkTouches(a, b, true);
}

// testAdjacentPolygons
template<>
template<>
void object::test<53> ()
{
    std::string a = "POLYGON ((1 9, 6 9, 6 1, 1 1, 1 9))";
    std::string b = "POLYGON ((9 9, 9 4, 6 4, 6 9, 9 9))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::ADJACENT, true);
}

// testAdjacentPolygonsTouchingAtPoint
template<>
template<>
void object::test<54> ()
{
    std::string a = "POLYGON ((1 9, 6 9, 6 1, 1 1, 1 9))";
    std::string b = "POLYGON ((9 9, 9 4, 6 4, 7 9, 9 9))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::ADJACENT, false);
}

// testAdjacentPolygonsOverlappping
template<>
template<>
void object::test<55> ()
{
    std::string a = "POLYGON ((1 9, 6 9, 6 1, 1 1, 1 9))";
    std::string b = "POLYGON ((9 9, 9 4, 6 4, 5 9, 9 9))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::ADJACENT, false);
}

// testContainsProperlyPolygonContained
template<>
template<>
void object::test<56> ()
{
    std::string a = "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))";
    std::string b = "POLYGON ((2 8, 5 8, 5 5, 2 5, 2 8))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::CONTAINS_PROPERLY, true);
}

// testContainsProperlyPolygonTouching
template<>
template<>
void object::test<57> ()
{
    std::string a = "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))";
    std::string b = "POLYGON ((9 1, 5 1, 5 5, 9 5, 9 1))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::CONTAINS_PROPERLY, false);
}

// testContainsProperlyPolygonsOverlapping
template<>
template<>
void object::test<58> ()
{
    std::string a = "GEOMETRYCOLLECTION (POLYGON ((1 9, 6 9, 6 4, 1 4, 1 9)), POLYGON ((2 4, 6 7, 9 1, 2 4)))";
    std::string b = "POLYGON ((5 5, 6 5, 6 4, 5 4, 5 5))";
    checkRelateMatches(a, b, IntersectionMatrixPattern::CONTAINS_PROPERLY, true);
}

//================  Repeated Points  ==============

// testRepeatedPointLL
template<>
template<>
void object::test<59> ()
{
    std::string a = "LINESTRING(0 0, 5 5, 5 5, 5 5, 9 9)";
    std::string b = "LINESTRING(0 9, 5 5, 5 5, 5 5, 9 0)";
    checkRelate(a, b, "0F1FF0102");
    checkIntersectsDisjoint(a, b, true);
}

// testRepeatedPointAA
template<>
template<>
void object::test<60> ()
{
    std::string a = "POLYGON ((1 9, 9 7, 9 1, 1 3, 1 9))";
    std::string b = "POLYGON ((1 3, 1 3, 1 3, 3 7, 9 7, 9 7, 1 3))";
    checkRelate(a, b, "212F01FF2");
}

//================  Empty Geometries  ==============

std::string empties[] = {
    "POINT EMPTY",
    "LINESTRING EMPTY",
    "POLYGON EMPTY",
    "MULTIPOINT EMPTY",
    "MULTILINESTRING EMPTY",
    "MULTIPOLYGON EMPTY",
    "GEOMETRYCOLLECTION EMPTY"
};

//-- test equals against all combinations of empty geometries
template<>
template<>
void object::test<61> ()
{
    int nempty = 7;
    for (int i = 0; i < nempty; i++) {
        for (int j = 0; j < nempty; j++) {
            std::string a = empties[i];
            std::string b = empties[j];
            checkRelate(a, b, "FFFFFFFF2");
            checkEquals(a, b, true);
        }
    }
}

// testEmptyNonEmpty
template<>
template<>
void object::test<62> ()
{
    std::string nonEmptyPoint = "POINT (1 1)";
    std::string nonEmptyLine = "LINESTRING (1 1, 2 2)";
    std::string nonEmptyPolygon = "POLYGON ((1 1, 1 2, 2 1, 1 1))";

    for (int i = 0; i < 7; i++) {
        std::string empty = empties[i];
        
        checkRelate(empty, nonEmptyPoint, "FFFFFF0F2");
        checkRelate(nonEmptyPoint, empty, "FF0FFFFF2");
        
        checkRelate(empty, nonEmptyLine, "FFFFFF102");
        checkRelate(nonEmptyLine, empty, "FF1FF0FF2");
        
        checkRelate(empty, nonEmptyPolygon, "FFFFFF212");
        checkRelate(nonEmptyPolygon, empty, "FF2FF1FF2");
        
        checkEquals(empty, nonEmptyPoint, false);
        checkEquals(empty, nonEmptyLine, false);
        checkEquals(empty, nonEmptyPolygon, false);
        
        checkIntersectsDisjoint(empty, nonEmptyPoint, false);
        checkIntersectsDisjoint(empty, nonEmptyLine, false);
        checkIntersectsDisjoint(empty, nonEmptyPolygon, false);
        
        checkContainsWithin(empty, nonEmptyPoint, false);
        checkContainsWithin(empty, nonEmptyLine, false);
        checkContainsWithin(empty, nonEmptyPolygon, false);
        
        checkContainsWithin(nonEmptyPoint, empty, false);
        checkContainsWithin(nonEmptyLine, empty, false);
        checkContainsWithin(nonEmptyPolygon, empty, false);
    }  
}

// Prepared test
template<>
template<>
void object::test<63> ()
{
    std::string a = "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))";
    std::string b = "POLYGON((0.5 0.5, 1.5 0.5, 1.5 1.5, 0.5 1.5, 0.5 0.5))";
    checkRelate(a, b, "212101212");
}


} // namespace tut
