//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>
#include <util/NodingTestUtil.h>

// geos
#include <geos/noding/Noder.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/snapround/SnapRoundingNoder.h>
#include <geos/noding/snapround/SnapRoundingIntersectionAdder.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/util/LinearComponentExtracter.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::noding;
using namespace geos::noding::snapround;
using geos::noding::Noder;
using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::geom::util::LinearComponentExtracter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_snaproundingnoder_data {

    WKTReader r;
    WKTWriter w;

    void
    checkRounding(std::string& wkt, double scale, std::string& expected_wkt)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        PrecisionModel pm(scale);
        SnapRoundingNoder noder(&pm);
        std::unique_ptr<Geometry> result = geos::NodingTestUtil::nodeValidated(geom.get(), nullptr, &noder);

        // only check if expected was provided
        if (expected_wkt.size() == 0) return;

        std::unique_ptr<Geometry> expected = r.read(expected_wkt);

        // std::cout << std::endl << "result" << std::endl;
        // std::cout << std::endl << w.write(result.get()) << std::endl;
        // std::cout << std::endl << "expected" << std::endl;
        // std::cout << std::endl << w.write(expected.get()) << std::endl;

        ensure_equals_geometry_xyzm(result.get(), expected.get());
    }

    // Runs SnapRoundingIntersectionAdder on the first segments of the first
    // two lines in wkt, with the nearness tolerance of SnapRoundingNoder
    // (a hundredth of a grid cell), and returns the recorded intersections.
    CoordinateSequence
    addIntersections(const std::string& wkt, const PrecisionModel& pm)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::vector<const LineString*> lines;
        LinearComponentExtracter::getLines(*geom, lines);
        NodedSegmentString a(lines[0]->getCoordinates(), geom->hasZ(), geom->hasM(), nullptr);
        NodedSegmentString b(lines[1]->getCoordinates(), geom->hasZ(), geom->hasM(), nullptr);
        SnapRoundingIntersectionAdder adder(pm, 1.0 / pm.getScale() / 100.0);
        adder.processIntersections(&a, 0, &b, 0);
        return adder.getIntersections();
    }


    // test_snaproundingnoder_data() {}
};

typedef test_group<test_snaproundingnoder_data> group;
typedef group::object object;

group test_snaproundingnoder_group("geos::noding::snapround::SnapRoundingNoder");

//
// Test Cases
//

// testSimple
template<>
template<>
void object::test<1> ()
{
    std::string wkt = "MULTILINESTRING ((1 1, 9 2), (3 3, 3 0))";
    std::string expected = "MULTILINESTRING ((1 1, 3 1), (3 1, 9 2), (3 3, 3 1), (3 1, 3 0))";
    checkRounding(wkt, 1, expected);
}

// testSnappedDiagonalLine
template<>
template<>
void object::test<2> ()
{
    std::string wkt = "LINESTRING (2 3, 3 3, 3 2, 2 3)";
    std::string expected = "MULTILINESTRING ((2 3, 3 3), (2 3, 3 3), (3 2, 3 3), (3 2, 3 3))";
    checkRounding(wkt, 1, expected);
}

// testRingsWithParallelNarrowSpikes
template<>
template<>
void object::test<3> ()
{
    std::string wkt = "MULTILINESTRING ((1 3.3, 1.3 1.4, 3.1 1.4, 3.1 0.9, 1.3 0.9, 1 -0.2, 0.8 1.3, 1 3.3), (1 2.9, 2.9 2.9, 2.9 1.3, 1.7 1, 1.3 0.9, 1 0.4, 1 2.9))";
    std::string expected = "MULTILINESTRING ((1 3, 1 1), (1 1, 2 1), (2 1, 3 1), (3 1, 2 1), (2 1, 1 1), (1 1, 1 0), (1 0, 1 1), (1 1, 1 3), (1 3, 3 3, 3 1), (3 1, 2 1), (2 1, 1 1), (1 1, 1 0), (1 0, 1 1), (1 1, 1 3))";
    checkRounding(wkt, 1, expected);
}


/**
* This test checks the HotPixel test for overlapping horizontal line
* testHorizontalLinesWithMiddleNode
*/
template<>
template<>
void object::test<4> ()
{
    std::string wkt = "MULTILINESTRING ((2.5117493 49.0278625, 2.5144958 49.0278625), (2.511749 49.027863, 2.513123 49.027863, 2.514496 49.027863))";
    std::string expected = "MULTILINESTRING ((2.511749 49.027863, 2.513123 49.027863), (2.511749 49.027863, 2.513123 49.027863), (2.513123 49.027863, 2.514496 49.027863), (2.513123 49.027863, 2.514496 49.027863))";
    checkRounding(wkt, 1000000.0, expected);
}

// testSlantAndHorizontalLineWithMiddleNode
template<>
template<>
void object::test<5> ()
{
    std::string wkt = "MULTILINESTRING ((0.1565552 49.5277405, 0.1579285 49.5277405, 0.1593018 49.5277405), (0.1568985 49.5280838, 0.1589584 49.5273972))";
    std::string expected = "MULTILINESTRING ((0.156555 49.527741, 0.157928 49.527741), (0.156899 49.528084, 0.157928 49.527741), (0.157928 49.527741, 0.157929 49.527741, 0.159302 49.527741), (0.157928 49.527741, 0.158958 49.527397))";
    checkRounding(wkt, 1000000.0, expected);
}

// testNearbyCorner
template<>
template<>
void object::test<6> ()
{
    std::string wkt = "MULTILINESTRING ((0.2 1.1, 1.6 1.4, 1.9 2.9), (0.9 0.9, 2.3 1.7))";
    std::string expected = "MULTILINESTRING ((0 1, 1 1), (1 1, 2 1), (1 1, 2 1), (2 1, 2 2), (2 1, 2 2), (2 2, 2 3))";
    checkRounding(wkt, 1.0, expected);
}

// testNearbyShape
template<>
template<>
void object::test<7> ()
{
    std::string wkt = "MULTILINESTRING ((1.3 0.1, 2.4 3.9), (0 1, 1.53 1.48, 0 4))";
    std::string expected = "MULTILINESTRING ((1 0, 2 1), (2 1, 2 4), (0 1, 2 1), (2 1, 0 4))";
    checkRounding(wkt, 1.0, expected);
}

/**
* Currently fails, perhaps due to intersection lying right on a grid cell corner?
* Fixed by ensuring intersections are forced into segments
*/
// testIntOnGridCorner
template<>
template<>
void object::test<8> ()
{
    std::string wkt = "MULTILINESTRING ((4.30166242 45.53438188, 4.30166243 45.53438187), (4.3011475 45.5328371, 4.3018341 45.5348969))";
    std::string expected = "";
    checkRounding(wkt, 100000000, expected);
}

/**
* Currently fails, does not node correctly
*/
// testVertexCrossesLine
template<>
template<>
void object::test<9> ()
{
    std::string wkt = "MULTILINESTRING ((2.2164917 48.8864136, 2.2175217 48.8867569), (2.2175217 48.8867569, 2.2182083 48.8874435), (2.2182083 48.8874435, 2.2161484 48.8853836))";
    std::string expected = "";
    checkRounding(wkt, 1000000, expected);
}

/**
* Currently fails, does not node correctly.
* Fixed by NOT rounding lines extracted by Overlay
*/
// testVertexCrossesLine2
template<>
template<>
void object::test<10> ()
{
    std::string wkt = "MULTILINESTRING ((2.276916574988164 49.06082147500638, 2.2769165 49.0608215), (2.2769165 49.0608215, 2.2755432 49.0608215), (2.2762299 49.0615082, 2.276916574988164 49.06082147500638))";
    std::string expected = "";
    checkRounding(wkt, 1000000, expected);
}

/**
* Looks like a very short line is stretched between two grid points,
* and for some reason the node at one end is not inserted in a line snapped to it
*/
// testShortLineNodeNotAdded
template<>
template<>
void object::test<11> ()
{
    std::string wkt = "LINESTRING (2.1279144 48.8445282, 2.126884443750796 48.84555818124935, 2.1268845 48.8455582, 2.1268845 48.8462448)";
    std::string expected = "MULTILINESTRING ((2.127914 48.844528, 2.126885 48.845558), (2.126885 48.845558, 2.126884 48.845558), (2.126884 48.845558, 2.126885 48.845558), (2.126885 48.845558, 2.126885 48.846245))";
    checkRounding(wkt, 1000000, expected);
}

/**
* This test will fail if the diagonals of hot pixels are not checked.
* Note that the nearby vertex is far enough from the long segment
* to avoid being snapped as an intersection.
*/
// testDiagonalNotNodedRightUp
template<>
template<>
void object::test<12> ()
{
    std::string wkt = "MULTILINESTRING ((0 0, 10 10), ( 0 2, 4.55 5.4, 9 10 ))";
    std::string expected = "";
    checkRounding(wkt, 1, expected);
}

/**
* Same diagonal test but flipped to test other diagonal
*/
// testDiagonalNotNodedLeftUp
template<>
template<>
void object::test<13> ()
{
    std::string wkt = "MULTILINESTRING ((10 0, 0 10), ( 10 2, 5.45 5.45, 1 10 ))";
    std::string expected = "";
    checkRounding(wkt, 1, expected);
}


/**
* Original full-precision diagonal line case
*/
// testDiagonalNotNodedOriginal
template<>
template<>
void object::test<14> ()
{
    std::string wkt = "MULTILINESTRING (( 2.45167 48.96709, 2.45768 48.9731 ), (2.4526978 48.968811, 2.4537277 48.9691544, 2.4578476 48.9732742))";
    std::string expected = "";
    checkRounding(wkt, 100000, expected);
}

/**
* An A vertex lies very close to a B segment.
* The vertex is snapped across the segment, but the segment is not noded.
* FIXED by adding intersection detection for near vertices to segments
*/
// testNearVertexNotNoded
template<>
template<>
void object::test<15> ()
{
    std::string wkt = "MULTILINESTRING ((2.4829102 48.8726807, 2.4830818249999997 48.873195575, 2.4839401 48.8723373), ( 2.4829102 48.8726807, 2.4832535 48.8737106 ))";
    std::string expected = "";
    checkRounding(wkt, 100000000, expected);
}

// testLoopBackCreatesNode
template<>
template<>
void object::test<16> ()
{
    std::string wkt = "LINESTRING (2 2, 5 2, 8 4, 5 6, 4.8 2.3, 2 5)";
    std::string expected = "MULTILINESTRING ((2 2, 5 2), (5 2, 8 4, 5 6, 5 2), (5 2, 2 5))";
    checkRounding(wkt, 1, expected);
}

/**
* Complex lines are snapped to a simpler arrangement
*/
// testHammerheads
template<>
template<>
void object::test<17> ()
{
    std::string wkt =      "MULTILINESTRING ((1 3.3, 1.3 1.4, 3.1 1.4, 3.1 0.9, 1.3 0.9, 1 -0.2, 0.8 1.3, 1 3.3), (1 2.9, 2.9 2.9, 2.9 1.3, 1.7 1, 1.3 0.9, 1 0.4, 1 2.9))";
    std::string expected = "MULTILINESTRING ((1 3, 1 1), (1 1, 2 1), (2 1, 3 1), (3 1, 2 1), (2 1, 1 1), (1 1, 1 0), (1 0, 1 1), (1 1, 1 3), (1 3, 3 3, 3 1), (3 1, 2 1), (2 1, 1 1), (1 1, 1 0), (1 0, 1 1), (1 1, 1 3))";
    checkRounding(wkt, 1.0, expected);
}

// Z preserved for simple intersection
template<>
template<>
void object::test<18> ()
{
    std::string wkt = "MULTILINESTRING Z ((1 1 0, 9 2 60), (3 3 0, 3 0 36))";
    std::string expected = "MULTILINESTRING Z ((1 1 0, 3 1 18), (3 1 18, 9 2 60), (3 3 0, 3 1 18), (3 1 18, 3 0 36))";
    checkRounding(wkt, 1, expected); // intersection point of (3 1.25) is snapped to (3 1) but Z interpolation is done at (3 1.25)
}

// M preserved for simple intersection
template<>
template<>
void object::test<19> ()
{
    std::string wkt = "MULTILINESTRING M ((1 1 0, 9 2 60), (3 3 0, 3 0 36))";
    std::string expected = "MULTILINESTRING M ((1 1 0, 3 1 18), (3 1 18, 9 2 60), (3 3 0, 3 1 18), (3 1 18, 3 0 36))";
    checkRounding(wkt, 1, expected); // intersection point of (3 1.25) is snapped to (3 1) but M interpolation is done at (3 1.25)
}


// Regression test: a proper intersection lies just across a half-cell boundary
// from its nearest double. Rounding the nearest double puts the hot pixel and the
// segment nodes in the neighbouring cell, and the snapped polygon boundaries cross
// without a node, which the validating noder reports.
template<>
template<>
void object::test<20>()
{
    set_test_name("Near-half-grid intersection is noded at fixed precision");
    std::string wkt = "GEOMETRYCOLLECTION (POLYGON ((-48.404225529999998 -5.0067209500000001, -48.404226645700533 -5.0066261829361647, -48.404226071884779 -5.0066747824165594, -48.404225529999998 -5.0067209500000001)), POLYGON ((-48.404225526798065 -5.006720954048145, -48.404228013904287 -5.0065102959894476, -48.404975132381281 -5.0041457218802918, -48.404225526798065 -5.006720954048145)))";
    std::string expected = "MULTILINESTRING ((-48.40422553 -5.00672095, -48.404225530085 -5.006720942756), (-48.404225530085 -5.006720942756, -48.404225530085 -5.006720942755), (-48.404225530085 -5.006720942755, -48.4042266457 -5.006626183023), (-48.4042266457 -5.006626183023, -48.404226645701 -5.006626182936), (-48.404226645701 -5.006626182936, -48.4042266457 -5.006626183023), (-48.4042266457 -5.006626183023, -48.404226071885 -5.006674782417, -48.404226066361 -5.006675253063), (-48.404226066361 -5.006675253063, -48.404225530085 -5.006720942755), (-48.404225530085 -5.006720942755, -48.404225530085 -5.006720942756), (-48.404225530085 -5.006720942756, -48.40422553 -5.00672095), (-48.404225526798 -5.006720954048, -48.404226066361 -5.006675253063), (-48.404226066361 -5.006675253063, -48.4042266457 -5.006626183023), (-48.4042266457 -5.006626183023, -48.404226645701 -5.006626182936), (-48.404226645701 -5.006626182936, -48.404228013904 -5.006510295989, -48.404975132381 -5.00414572188, -48.404225530085 -5.006720942755), (-48.404225530085 -5.006720942755, -48.404225530085 -5.006720942756), (-48.404225530085 -5.006720942756, -48.404225526798 -5.006720954048))";
    checkRounding(wkt, 1e12, expected);
}

// Unit test of the grid-size rounding of an intersection. A scale of 0.1 is a
// grid size of 10. The intersection lies exactly on the negative half-cell
// boundary -1000000000000005, which rounds towards positive infinity, as
// PrecisionModel::makePrecise rounds it. Rounding with the scale 0.1, whose
// double is slightly more than 0.1, would give -1000000000000010 instead.
// The nearest double is the tie itself, so the result is the same without
// the cell correction.
template<>
template<>
void object::test<21>()
{
    set_test_name("Negative half-grid intersection uses the grid-size tie rule");
    PrecisionModel pm(0.1);
    ensure_equals("half-grid tie", pm.makePrecise(-1000000000000005.0), -1000000000000000.0, 0.0);
    auto intersections = addIntersections("MULTILINESTRING ((-1000000000000006 -10, -1000000000000004 10), (-1000000000000006 10, -1000000000000004 -10))", pm);
    ensure_equals("one intersection", intersections.size(), std::size_t(1));
    ensure_equals("chosen cell", pm.makePrecise(intersections.getAt<CoordinateXYZM>(0).x),
                  -1000000000000000.0, 0.0);
}

// Regression test: moving an intersection into the cell of the exact
// intersection keeps the Z and M values interpolated by LineIntersector.
template<>
template<>
void object::test<22>()
{
    set_test_name("Corrected grid cell preserves intersection Z and M");
    PrecisionModel pm(1e12);
    auto intersections = addIntersections("GEOMETRYCOLLECTION (POLYGON ZM ((-48.404225529999998 -5.0067209500000001 0 100, -48.404226645700533 -5.0066261829361647 10 120, -48.404226071884779 -5.0066747824165594 10 120, -48.404225529999998 -5.0067209500000001 0 100)), POLYGON ZM ((-48.404225526798065 -5.006720954048145 20 200, -48.404228013904287 -5.0065102959894476 30 220, -48.404975132381281 -5.0041457218802918 30 220, -48.404225526798065 -5.006720954048145 20 200)))", pm);
    ensure_equals("one intersection", intersections.size(), std::size_t(1));
    const auto& coord = intersections.getAt<CoordinateXYZM>(0);
    ensure_equals("corrected cell", pm.makePrecise(coord.x), -48.4042266457, 0.0);
    ensure_equals("Z at corrected node", coord.z, 17.2494, 1e-4);
    ensure_equals("M at corrected node", coord.m, 164.4988, 1e-4);
}

// Pins the cell of an intersection at a coarse scale. The exact intersection
// is just left of -0.5, but its nearest double is the half-cell boundary -0.5
// itself, which rounds to 0. The node is placed in the cell of the exact
// intersection, -1. Without the correction the geometry is also noded validly,
// with the node at (0 0), so this test pins the choice of cell rather than
// preventing a noding failure.
template<>
template<>
void object::test<23>()
{
    set_test_name("Intersection hidden below a half-grid double rounds left");
    std::string wkt = "MULTILINESTRING ((-1 -1, 0 1), (-1 1, 0 -1.0000000000000002))";
    PrecisionModel pm(1);
    auto intersections = addIntersections(wkt, pm);
    ensure_equals("one intersection", intersections.size(), std::size_t(1));
    ensure_equals("chosen cell", pm.makePrecise(intersections.getAt<CoordinateXYZM>(0).x), -1.0, 0.0);
    std::string expected = "MULTILINESTRING ((-1 -1, -1 0), (-1 0, 0 1), (-1 1, -1 0), (-1 0, 0 -1))";
    checkRounding(wkt, 1, expected);
}

// Regression test: along a horizontal or vertical segment the intersection is
// rounded as the segment's own vertices are. The segment's ordinate
// 377804.96362807497 lies below the half-cell boundary 377804.963628075, but
// PrecisionModel::makePrecise rounds it up to 377804.96362808, because the
// product with the scale rounds onto the boundary in double arithmetic.
// Moving the node into the cell below would bend the snapped segment into a V.
template<>
template<>
void object::test<24>()
{
    set_test_name("Horizontal and vertical segments stay straight through a node");
    std::string horizontal = "MULTILINESTRING ((123456.25 377804.593628075, 123456.75 377805.33362807496), (123455 377804.96362807497, 123458 377804.96362807497))";
    std::string horizontalExpected = "MULTILINESTRING ((123456.25 377804.59362808, 123456.5 377804.96362808), (123456.5 377804.96362808, 123456.75 377805.33362808), (123455 377804.96362808, 123456.5 377804.96362808), (123456.5 377804.96362808, 123458 377804.96362808))";
    checkRounding(horizontal, 1e8, horizontalExpected);
    std::string vertical = "MULTILINESTRING ((377804.593628075 123456.25, 377805.33362807496 123456.75), (377804.96362807497 123455, 377804.96362807497 123458))";
    std::string verticalExpected = "MULTILINESTRING ((377804.59362808 123456.25, 377804.96362808 123456.5), (377804.96362808 123456.5, 377805.33362808 123456.75), (377804.96362808 123455, 377804.96362808 123456.5), (377804.96362808 123456.5, 377804.96362808 123458))";
    checkRounding(vertical, 1e8, verticalExpected);
}

// Regression test: an intersection with a horizontal or vertical segment is
// moved into the cell of the exact intersection along that segment, and keeps
// the segment's own ordinate across it. The exact intersection with y = 0 is
// just left of x = -0.5, whose nearest double -0.5 rounds to 0.
template<>
template<>
void object::test<25>()
{
    set_test_name("Intersection with an axis-parallel segment is moved along it");
    PrecisionModel pm(1);

    auto horizontal = addIntersections("MULTILINESTRING ((-1 -1, 0 1.0000000000000002), (-1 0, 0 0))", pm);
    ensure_equals("one intersection with horizontal", horizontal.size(), std::size_t(1));
    const auto& h = horizontal.getAt<CoordinateXYZM>(0);
    ensure_equals("x in exact cell", pm.makePrecise(h.x), -1.0, 0.0);
    ensure_equals("y on horizontal segment", h.y, 0.0, 0.0);

    auto vertical = addIntersections("MULTILINESTRING ((-1 -1, 1.0000000000000002 0), (0 -1, 0 0))", pm);
    ensure_equals("one intersection with vertical", vertical.size(), std::size_t(1));
    const auto& v = vertical.getAt<CoordinateXYZM>(0);
    ensure_equals("x on vertical segment", v.x, 0.0, 0.0);
    ensure_equals("y in exact cell", pm.makePrecise(v.y), -1.0, 0.0);
}

// Regression test with a grid size. A scale of 0.1 is a grid size of 10.
// The exact intersection is just left of x = 1005, whose nearest double 1005
// rounds to 1010. The node belongs to the cell at 1000.
template<>
template<>
void object::test<26>()
{
    set_test_name("Near-half-grid intersection is noded with a grid size");
    std::string wkt = "MULTILINESTRING ((1000 -100, 1010 100), (1000 100, 1010 -100.00000000000001))";
    std::string expected = "MULTILINESTRING ((1000 -100, 1000 0), (1000 0, 1010 100), (1000 100, 1000 0), (1000 0, 1010 -100))";
    checkRounding(wkt, 0.1, expected);
}

// Unit test of the case in which one representable double is not enough.
// The exact intersection lies in the cell at -13.45017351223. Its nearest double
// -13.4501735122295 and the next double below both round to -13.450173512229,
// because their products with the scale round onto the half-cell boundary.
// The intersection is then recorded exactly as LineIntersector computed it.
template<>
template<>
void object::test<27>()
{
    set_test_name("Intersection is kept when one double does not reach its cell");
    PrecisionModel pm(1e12);
    std::string wkt = "MULTILINESTRING ((48.00000000000059 -13.450173512243548, 48.0000000000005 -13.450173512223339), (48.00000000000589 -13.450173512245257, 47.99999999997482 -13.450173512153995))";
    auto intersections = addIntersections(wkt, pm);
    ensure_equals("one intersection", intersections.size(), std::size_t(1));
    const auto& recorded = intersections.getAt<CoordinateXYZM>(0);

    auto geom = r.read(wkt);
    std::vector<const LineString*> lines;
    LinearComponentExtracter::getLines(*geom, lines);
    geos::algorithm::LineIntersector li;
    li.computeIntersection(*lines[0]->getCoordinatesRO(), 0, *lines[1]->getCoordinatesRO(), 0);
    ensure("proper intersection", li.isProper());
    ensure_equals("x as computed", recorded.x, li.getIntersection(0).x, 0.0);
    ensure_equals("y as computed", recorded.y, li.getIntersection(0).y, 0.0);
}

// Regression test for a grid finer than a double: at a scale of 1e12 near
// x = 5e5, the cell index is about 5e17, beyond 2^53, so it is not exactly
// representable as a double. The x-ordinate of this crossing rounds to
// 501606.63344328245, the nearest double to the grid value of the exact
// cell, and must not be moved to a neighbouring double.
template<>
template<>
void object::test<28>()
{
    set_test_name("Intersection keeps its cell on a grid finer than a double");
    PrecisionModel pm(1e12);
    auto intersections = addIntersections("MULTILINESTRING ((501628.9755162752 6000739.718938188, 501606.2368510821 6000711.289381552), (501608.39187310886 6000711.1006511, 501591.03273743734 6000717.85878518))", pm);
    ensure_equals("one intersection", intersections.size(), std::size_t(1));
    ensure_equals("x in exact cell", pm.makePrecise(intersections.getAt<CoordinateXYZM>(0).x), 501606.63344328245, 0.0);
}

} // namespace tut
