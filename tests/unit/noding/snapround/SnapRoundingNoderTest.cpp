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

        ensure_equals_geometry(result.get(), expected.get());
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


} // namespace tut
