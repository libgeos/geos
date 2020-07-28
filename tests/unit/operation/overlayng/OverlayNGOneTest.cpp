//
// Test Suite for geos::operation::overlayng::OverlayNG class with SnappingNoder.

#include <tut/tut.hpp>
#include <utility.h>

// geos
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
struct test_overlayngone_data {

    WKTReader r;
    WKTWriter w;

    void
    geomTest(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        geos::geom::PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngone_data> group;
typedef group::object object;

group test_overlayngone_group("geos::operation::overlayng::OverlayNGOne");

//
// Test Cases
//

// testRoundedBoxesIntersection
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON ((0.6 0.1, 0.6 1.9, 2.9 1.9, 2.9 0.1, 0.6 0.1))";
    std::string b = "POLYGON ((1.1 3.9, 2.9 3.9, 2.9 2.1, 1.1 2.1, 1.1 3.9))";
    std::string exp = "LINESTRING (1 2, 3 2)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestRoundedLinesIntersection
template<>
template<>
void object::test<2> ()
{
    std::string a = "LINESTRING (3 2, 3 4)";
    std::string b = "LINESTRING (1.1 1.6, 3.8 1.9)";
    std::string exp = "POINT (3 2)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestRoundedPointsIntersection
template<>
template<>
void object::test<3> ()
{
    std::string a = "POINT (10.1 10)";
    std::string b = "POINT (10 10.1)";
    std::string exp = "POINT (10 10)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestLineLineIntersectionFloat
template<>
template<>
void object::test<4> ()
{
    std::string a = "LINESTRING (10 10, 20 20)";
    std::string b = "LINESTRING (13 13, 10 10, 10 20, 20 20, 17 17)";
    std::string exp = "LINESTRING (10 10, 10 20, 20 20, 17 17, 13 13, 10 10)";
    geomTest(a, b, exp, OverlayNG::UNION, 10);
}

// xtestPolygonPointIntersection
template<>
template<>
void object::test<5> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "MULTIPOINT ((150 150), (250 150))";
    std::string exp = "POINT (150 150)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestPolygonPointUnion
template<>
template<>
void object::test<6> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "MULTIPOINT ((150 150), (250 150))";
    std::string exp = "GEOMETRYCOLLECTION (POINT (250 150), POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestPolygoPolygonWithLineTouchIntersection
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON ((360 200, 220 200, 220 180, 300 180, 300 160, 300 140, 360 200))";
    std::string b = "MULTIPOLYGON (((280 180, 280 160, 300 160, 300 180, 280 180)), ((220 230, 240 230, 240 180, 220 180, 220 230)))";
    std::string exp = "POLYGON ((220 200, 240 200, 240 180, 220 180, 220 200))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestLinePolygonIntersectionAlongCollapse
template<>
template<>
void object::test<8> ()
{
    std::string a = "POLYGON ((100 300, 300 300, 300 200, 130 200, 300 199.9, 300 100, 100 100, 100 300))";
    std::string b = "LINESTRING (130 200, 200 200)";
    std::string exp = "LINESTRING (130 200, 200 200)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestLinePolygonIntersectionAlongPolyBoundary
template<>
template<>
void object::test<9> ()
{
    std::string a = "LINESTRING (150 300, 250 300)";
    std::string b = "POLYGON ((100 400, 200 400, 200 300, 100 300, 100 400))";
    std::string exp = "LINESTRING (200 300, 150 300)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestPolygonMultiLineUnion
template<>
template<>
void object::test<10> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "MULTILINESTRING ((150 250, 150 50), (250 250, 250 50))";
    std::string exp = "GEOMETRYCOLLECTION (LINESTRING (150 50, 150 100), LINESTRING (150 200, 150 250), LINESTRING (250 50, 250 250), POLYGON ((100 100, 100 200, 150 200, 200 200, 200 100, 150 100, 100 100)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLinePolygonUnion
template<>
template<>
void object::test<11> ()
{
    std::string a = "LINESTRING (50 150, 150 150)";
    std::string b = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string exp = "GEOMETRYCOLLECTION (LINESTRING (50 150, 100 150), POLYGON ((100 200, 200 200, 200 100, 100 100, 100 150, 100 200)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestBoxGoreIntersection
template<>
template<>
void object::test<12> ()
{
    std::string a = "MULTIPOLYGON (((1 1, 5 1, 5 0, 1 0, 1 1)), ((1 1, 5 2, 5 4, 1 4, 1 1)))";
    std::string b = "POLYGON ((1 0, 1 2, 2 2, 2 0, 1 0))";
    std::string exp = "POLYGON ((2 0, 1 0, 1 1, 1 2, 2 2, 2 1, 2 0))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestBoxGoreUnion
template<>
template<>
void object::test<13> ()
{
    std::string a = "MULTIPOLYGON (((1 1, 5 1, 5 0, 1 0, 1 1)), ((1 1, 5 2, 5 4, 1 4, 1 1)))";
    std::string b = "POLYGON ((1 0, 1 2, 2 2, 2 0, 1 0))";
    std::string exp = "POLYGON ((2 0, 1 0, 1 1, 1 2, 1 4, 5 4, 5 2, 2 1, 5 1, 5 0, 2 0))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestCollapseBoxGoreIntersection
template<>
template<>
void object::test<14> ()
{
    std::string a = "MULTIPOLYGON (((1 1, 5 1, 5 0, 1 0, 1 1)), ((1 1, 5 2, 5 4, 1 4, 1 1)))";
    std::string b = "POLYGON ((1 0, 1 2, 2 2, 2 0, 1 0))";
    std::string exp = "POLYGON ((2 0, 1 0, 1 1, 1 2, 2 2, 2 1, 2 0))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestCollapseTriBoxIntersection
template<>
template<>
void object::test<15> ()
{
    std::string a = "POLYGON ((1 2, 1 1, 9 1, 1 2))";
    std::string b = "POLYGON ((9 2, 9 1, 8 1, 8 2, 9 2))";
    std::string exp = "POINT (8 1)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// XtestCollapseTriBoxUnion
template<>
template<>
void object::test<16> ()
{
    std::string a = "POLYGON ((1 2, 1 1, 9 1, 1 2))";
    std::string b = "POLYGON ((9 2, 9 1, 8 1, 8 2, 9 2))";
    std::string exp = "MULTIPOLYGON (((1 1, 1 2, 8 1, 1 1)), ((8 1, 8 2, 9 2, 9 1, 8 1)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestAdjacentBoxesUnion
template<>
template<>
void object::test<17> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((300 200, 300 100, 200 100, 200 200, 300 200))";
    std::string exp = "POLYGON ((100 100, 100 200, 200 200, 300 200, 300 100, 200 100, 100 100))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestBoxTriIntersection
template<>
template<>
void object::test<18> ()
{
    std::string a = "POLYGON ((0 6, 4 6, 4 2, 0 2, 0 6))";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((3 2, 1 2, 2 5, 3 2))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}


// xtestBoxTriUnion
template<>
template<>
void object::test<19> ()
{
    std::string a = "POLYGON ((0 6, 4 6, 4 2, 0 2, 0 6))";
    std::string b = "POLYGON ((1 0, 2 5, 3 0, 1 0))";
    std::string exp = "POLYGON ((0 6, 4 6, 4 2, 3 2, 3 0, 1 0, 1 2, 0 2, 0 6))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestMultiHoleBoxUnion
template<>
template<>
void object::test<20> ()
{
    std::string a = "MULTIPOLYGON (((0 200, 200 200, 200 0, 0 0, 0 200), (50 50, 190 50, 50 200, 50 50), (20 20, 20 50, 50 50, 50 20, 20 20)), ((60 100, 50 50, 100 60, 60 100)))";
    std::string b = "POLYGON ((60 110, 100 110, 100 60, 60 60, 60 110))";
    std::string exp = "MULTIPOLYGON (((0 200, 50 200, 200 200, 200 0, 0 0, 0 200), (50 50, 190 50, 50 200, 50 50), (20 50, 20 20, 50 20, 50 50, 20 50)), ((60 100, 60 110, 100 110, 100 60, 50 50, 60 100)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}



// xtestNestedPolysUnion
template<>
template<>
void object::test<21> ()
{
    std::string a = "MULTIPOLYGON (((0 200, 200 200, 200 0, 0 0, 0 200), (50 50, 190 50, 50 200, 50 50)), ((60 100, 100 60, 50 50, 60 100)))";
    std::string b = "POLYGON ((135 176, 180 176, 180 130, 135 130, 135 176))";
    std::string exp = "MULTIPOLYGON (((0 0, 0 200, 50 200, 200 200, 200 0, 0 0), (50 50, 190 50, 50 200, 50 50)), ((50 50, 60 100, 100 60, 50 50)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// TODO: check this when it has be implemented...
// xtestMultiHoleSideTouchingBoxUnion
template<>
template<>
void object::test<22> ()
{
    std::string a = "MULTIPOLYGON (((0 200, 200 200, 200 0, 0 0, 0 200), (50 50, 190 50, 50 200, 50 50), (20 20, 20 50, 50 50, 50 20, 20 20)))";
    std::string b = "POLYGON ((100 100, 100 50, 50 50, 50 100, 100 100))";
    std::string exp = "LINESTRING (50 100.0000000000000000, 50 50, 100 50)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestNestedShellsIntersection
template<>
template<>
void object::test<23> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    std::string exp = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestNestedShellsUnion
template<>
template<>
void object::test<24> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((120 180, 180 180, 180 120, 120 120, 120 180))";
    std::string exp = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestBoxLineIntersection
template<>
template<>
void object::test<25> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "LINESTRING (50 150, 150 150)";
    std::string exp = "LINESTRING (100 150, 150 150)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}
// xtestBoxLineUnion
template<>
template<>
void object::test<26> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "LINESTRING (50 150, 150 150)";
    std::string exp = "GEOMETRYCOLLECTION (LINESTRING (50 150, 100 150), POLYGON ((100 200, 200 200, 200 100, 100 100, 100 150, 100 200)))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestAdjacentBoxesIntersection
template<>
template<>
void object::test<27> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))";
    std::string b = "POLYGON ((300 200, 300 100, 200 100, 200 200, 300 200))";
    std::string exp = "LINESTRING (200 100, 200 200)";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestBoxContainingPolygonCollapseIntersection
template<>
template<>
void object::test<28> ()
{
    std::string a = "POLYGON ((100 200, 300 200, 300 0, 100 0, 100 200))";
    std::string b = "POLYGON ((250 100, 150 100, 150 100.4, 250 100))";
    std::string exp = "POLYGON EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestBoxContainingPolygonCollapseManyPtsIntersection
template<>
template<>
void object::test<29> ()
{
    std::string a = "POLYGON ((100 200, 300 200, 300 0, 100 0, 100 200))";
    std::string b = "POLYGON ((250 100, 150 100, 150 100.4, 160 100.2, 170 100.1, 250 100))";
    std::string exp = "POLYGON EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestPolygonsSpikeCollapseIntersection
template<>
template<>
void object::test<30> ()
{
    std::string a = "POLYGON ((2.33906 48.78994, 2.33768 48.78857, 2.33768 48.78788, 2.33974 48.78719, 2.34009 48.78616, 2.33974 48.78513, 2.33871 48.78479, 2.33734 48.78479, 2.33631 48.78445, 2.33597 48.78342, 2.33631 48.78239, 2.337 48.7817, 2.33734 48.78067, 2.33734 48.7793, 2.337 48.77827, 2.3178 48.7849, 2.32099 48.79376, 2.33906 48.78994))";
    std::string b = "POLYGON ((2.33768 48.78857, 2.33768 48.78788, 2.33974 48.78719, 2.34009 48.78616, 2.33974 48.78513, 2.33871 48.78479, 2.33734 48.78479, 2.33631 48.78445, 2.3362 48.7841, 2.33562 48.78582, 2.33425 48.78719, 2.33768 48.78857))";
    std::string exp = "POLYGON ((2.33425 48.78719, 2.33768 48.78857, 2.33768 48.78788, 2.33974 48.78719, 2.34009 48.78616, 2.33974 48.78513, 2.33871 48.78479, 2.33734 48.78479, 2.33631 48.78445, 2.3362 48.78411, 2.33562 48.78582, 2.33425 48.78719))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 100000);
}

/**
* Fails because polygon A collapses totally, but one
* L edge is still labelled with location A:iL due to being located
* inside original A polygon by PiP test for incomplete edges.
* That edge is then marked as in-result-area, but result ring can't
* be formed because ring is incomplete
*/
// xtestCollapseAIncompleteRingUnion
template<>
template<>
void object::test<31> ()
{
    std::string a = "POLYGON ((0.9 1.7, 1.3 1.4, 2.1 1.4, 2.1 0.9, 1.3 0.9, 0.9 0, 0.9 1.7))";
    std::string b = "POLYGON ((1 3, 3 3, 3 1, 1.3 0.9, 1 0.4, 1 3))";
    std::string exp = "POLYGON ((1 2, 1 3, 3 3, 3 1, 2 1, 1 1, 1 2))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestCollapseHoleAlongEdgeOfBIntersection
template<>
template<>
void object::test<32> ()
{
    std::string a = "POLYGON ((0 3, 3 3, 3 0, 0 0, 0 3), (1 1.2, 1 1.1, 2.3 1.1, 1 1.2))";
    std::string b = "POLYGON ((1 1, 2 1, 2 0, 1 0, 1 1))";
    std::string exp = "POLYGON ((1 1, 2 1, 2 0, 1 0, 1 1))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// xtestCollapseResultShouldHavePolygonUnion
template<>
template<>
void object::test<33> ()
{
    std::string a = "POLYGON ((1 3.3, 1.3 1.4, 3.1 1.4, 3.1 0.9, 1.3 0.9, 1 -0.2, 0.8 1.3, 1 3.3))";
    std::string b = "POLYGON ((1 2.9, 2.9 2.9, 2.9 1.3, 1.7 1, 1.3 0.9, 1 0.4, 1 2.9))";
    std::string exp = "POLYGON ((1 1, 1 3, 3 3, 3 1, 2 1, 1 1))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestVerySmallBIntersection
template<>
template<>
void object::test<34> ()
{
    std::string a = "POLYGON ((2.526855443750341 48.82324221874807, 2.5258255 48.8235855, 2.5251389 48.8242722, 2.5241089 48.8246155, 2.5254822 48.8246155, 2.5265121 48.8242722, 2.526855443750341 48.82324221874807))";
    std::string b = "POLYGON ((2.526512100000002 48.824272199999996, 2.5265120999999953 48.8242722, 2.5265121 48.8242722, 2.526512100000002 48.824272199999996))";
    std::string exp = "POLYGON EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 100000000);
}

/**
* Currently noding is incorrect, producing one 2pt edge which is coincident
* with a 3-pt edge.  The EdgeMerger doesn't check that merged edges are identical,
* so merges the 3pt edge into the 2-pt edge
*/
// xtestEdgeDisappears
template<>
template<>
void object::test<35> ()
{
    std::string a = "LINESTRING (2.1279144 48.8445282, 2.126884443750796 48.84555818124935, 2.1268845 48.8455582, 2.1268845 48.8462448)";
    std::string b = "LINESTRING EMPTY";
    std::string exp = "LINESTRING EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1000000);
}

/**
* Probably due to B collapsing completely and disconnected edges being located incorrectly in B interior.
* Have seen other cases of this as well.
* Also - a B edge is marked as a Hole, which is incorrect
*/
// xtestBcollapseLocateIssue
template<>
template<>
void object::test<36> ()
{
    std::string a = "POLYGON ((2.3442078 48.9331054, 2.3435211 48.9337921, 2.3428345 48.9358521, 2.3428345 48.9372253, 2.3433495 48.9370537, 2.3440361 48.936367, 2.3442078 48.9358521, 2.3442078 48.9331054))";
    std::string b = "POLYGON ((2.3442078 48.9331054, 2.3435211 48.9337921, 2.3433494499999985 48.934307100000005, 2.3438644 48.9341354, 2.3442078 48.9331055, 2.3442078 48.9331054))";
    std::string exp = "POLYGON EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 1000);
}

/**
* A component of B collapses completely.
* Labelling marks a single collapsed edge as B:i.
* Edge is only connected to two other edges both marked B:e.
* B:i edge is included in area result edges, and faild because it does not form a ring.
*
* Perhaps a fix is to ignore connected single Bi edges which do not form a ring?
* This may be dangerous since it may hide other labelling problems?
*
* FIXED by requiring both endpoints of edge to lie in Interior to be located as i
*/
// xtestBcollapseEdgeLabeledInterior
template<>
template<>
void object::test<37> ()
{
    std::string a = "POLYGON ((2.384376506250038 48.91765596875102, 2.3840332 48.916626, 2.3840332 48.9138794, 2.3833466 48.9118195, 2.3812866 48.9111328, 2.37854 48.9111328, 2.3764801 48.9118195, 2.3723602 48.9159393, 2.3703003 48.916626, 2.3723602 48.9173126, 2.3737335 48.9186859, 2.3757935 48.9193726, 2.3812866 48.9193726, 2.3833466 48.9186859, 2.384376506250038 48.91765596875102))";
    std::string b = "MULTIPOLYGON (((2.3751067666731345 48.919143677778855, 2.3757935 48.9193726, 2.3812866 48.9193726, 2.3812866 48.9179993, 2.3809433 48.9169693, 2.3799133 48.916626, 2.3771667 48.916626, 2.3761368 48.9169693, 2.3754501 48.9190292, 2.3751067666731345 48.919143677778855)), ((2.3826108673454116 48.91893115612326, 2.3833466 48.9186859, 2.3840331750033394 48.91799930833141, 2.3830032 48.9183426, 2.3826108673454116 48.91893115612326)))";
    std::string exp = "POLYGON ((2.375 48.91833333333334, 2.375 48.92, 2.381666666666667 48.92, 2.381666666666667 48.91833333333334, 2.381666666666667 48.916666666666664, 2.38 48.916666666666664, 2.3766666666666665 48.916666666666664, 2.375 48.91833333333334))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 600);
}

// xtestBcollapseNullEdgeInRingIssue
template<>
template<>
void object::test<38> ()
{
    std::string a = "POLYGON ((2.2494507 48.8864136, 2.2484207 48.8867569, 2.2477341 48.8874435, 2.2470474 48.8874435, 2.2463608 48.8853836, 2.2453308 48.8850403, 2.2439575 48.8850403, 2.2429276 48.8853836, 2.2422409 48.8860703, 2.2360611 48.8970566, 2.2504807 48.8956833, 2.2494507 48.8864136))";
    std::string b = "POLYGON ((2.247734099999997 48.8874435, 2.2467041 48.8877869, 2.2453308 48.8877869, 2.2443008 48.8881302, 2.243957512499544 48.888473487500455, 2.2443008 48.8888168, 2.2453308 48.8891602, 2.2463608 48.8888168, 2.247734099999997 48.8874435))";
    std::string exp = "POLYGON EMPTY";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 200);
}

// xtestLineUnion
template<>
template<>
void object::test<39> ()
{
    std::string a = "LINESTRING (0 0, 1 1)";
    std::string b = "LINESTRING (1 1, 2 2)";
    std::string exp = "LINESTRING (0 0, 1 1, 2 2)";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLine2Union
template<>
template<>
void object::test<40> ()
{
    std::string a = "LINESTRING (0 0, 1 1, 0 1)";
    std::string b = "LINESTRING (1 1, 2 2, 3 3)";
    std::string exp = "MULTILINESTRING ((0 0, 1 1), (0 1, 1 1), (1 1, 2 2, 3 3))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLine3Union
template<>
template<>
void object::test<41> ()
{
    std::string a = "MULTILINESTRING ((0 1, 1 1), (2 2, 2 0))";
    std::string b = "LINESTRING (0 0, 1 1, 2 2, 3 3)";
    std::string exp = "MULTILINESTRING ((0 0, 1 1), (0 1, 1 1), (1 1, 2 2), (2 0, 2 2), (2 2, 3 3))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLine4Union
template<>
template<>
void object::test<42> ()
{
    std::string a = "LINESTRING (100 300, 200 300, 200 100, 100 100)";
    std::string b = "LINESTRING (300 300, 200 300, 200 300, 200 100, 300 100)";
    std::string exp = "MULTILINESTRING ((200 100, 100 100), (300 300, 200 300), (200 300, 200 100), (200 100, 300 100), (100 300, 200 300))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLineFigure8Union
template<>
template<>
void object::test<43> ()
{
    std::string a = "LINESTRING (5 1, 2 2, 5 3, 2 4, 5 5)";
    std::string b = "LINESTRING (5 1, 8 2, 5 3, 8 4, 5 5)";
    std::string exp = "MULTILINESTRING ((5 3, 2 2, 5 1, 8 2, 5 3), (5 3, 2 4, 5 5, 8 4, 5 3))";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

// xtestLineRingUnion
template<>
template<>
void object::test<44> ()
{
    std::string a = "LINESTRING (1 1, 5 5, 9 1)";
    std::string b = "LINESTRING (1 1, 9 1)";
    std::string exp = "LINESTRING (1 1, 5 5, 9 1, 1 1)";
    geomTest(a, b, exp, OverlayNG::UNION, 1);
}

/**
* Failure due to B hole collapsing and edges being labeled Exterior.
* They are coincident with an A hole edge, but because labeled E are not
* included in Intersection result.
*/
// xtestBCollapsedHoleEdgeLabelledExterior
template<>
template<>
void object::test<45> ()
{
    std::string a = "POLYGON ((309500 3477900, 309900 3477900, 309900 3477600, 309500 3477600, 309500 3477900), (309741.87561330193 3477680.6737848604, 309745.53718649445 3477677.607851833, 309779.0333599192 3477653.585555199, 309796.8051681937 3477642.143583868, 309741.87561330193 3477680.6737848604))";
    std::string b = "POLYGON ((309500 3477900, 309900 3477900, 309900 3477600, 309500 3477600, 309500 3477900), (309636.40806633036 3477777.2910157656, 309692.56085444096 3477721.966349552, 309745.53718649445 3477677.607851833, 309779.0333599192 3477653.585555199, 309792.0991800499 3477645.1734264474, 309779.03383125085 3477653.5853248164, 309745.53756275156 3477677.6076231804, 309692.5613257677 3477721.966119165, 309636.40806633036 3477777.2910157656))";
    std::string exp = "POLYGON ((309500 3477600, 309500 3477900, 309900 3477900, 309900 3477600, 309500 3477600), (309741.88 3477680.67, 309745.54 3477677.61, 309779.03 3477653.59, 309792.1 3477645.17, 309796.81 3477642.14, 309741.88 3477680.67))";
    geomTest(a, b, exp, OverlayNG::INTERSECTION, 100);
}




} // namespace tut
