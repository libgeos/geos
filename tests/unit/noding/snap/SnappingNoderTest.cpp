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
#include <geos/geom/util/LinearComponentExtracter.h>
#include <geos/noding/snap/SnappingNoder.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::noding;
using namespace geos::noding::snap;
using geos::noding::Noder;
using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::geom::util::LinearComponentExtracter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_snappingnoder_data {

    WKTReader r;

    void
    checkRounding(const std::string& wkt1, const std::string& wkt2, double snapDist, std::string& expected_wkt)
    {
        std::unique_ptr<Geometry> geom1 = r.read(wkt1);
        std::unique_ptr<Geometry> geom2(nullptr);
        if (wkt2.size() > 0) {
            geom2 = r.read(wkt2);
        }
        SnappingNoder noder(snapDist);
        std::unique_ptr<Geometry> result = geos::NodingTestUtil::nodeValidated(geom1.get(), geom2.get(), &noder);

        // only check if expected was provided
        if (expected_wkt.size() == 0) return;

        std::unique_ptr<Geometry> expected = r.read(expected_wkt);
        ensure_equals_geometry(expected.get(), result.get());
    }

};

typedef test_group<test_snappingnoder_data> group;
typedef group::object object;

group test_snappingnoder_group("geos::noding::snap::SnappingNoder");

//
// Test Cases
//


//  testOverlappingLinesWithNearVertex
template<>
template<>
void object::test<1> ()
{
    std::string wkt1 = "LINESTRING (100 100, 300 100)";
    std::string wkt2 = "LINESTRING (200 100.1, 400 100)";
    std::string expected = "MULTILINESTRING ((100 100, 200 100.1), (200 100.1, 300 100), (200 100.1, 300 100), (300 100, 400 100))";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testSnappedVertex
template<>
template<>
void object::test<2> ()
{
    std::string wkt1 = "LINESTRING (100 100, 200 100, 300 100)";
    std::string wkt2 = "LINESTRING (200 100.3, 400 110)";
    std::string expected = "MULTILINESTRING ((100 100, 200 100), (200 100, 300 100), (200 100, 400 110))";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testSelfSnap
template<>
template<>
void object::test<3> ()
{
    std::string wkt1 = "LINESTRING (100 200, 100 100, 300 100, 200 99.3, 200 0)";
    std::string wkt2 = "";
    std::string expected = "MULTILINESTRING ((100 200, 100 100, 200 99.3), (200 99.3, 300 100), (300 100, 200 99.3), (200 99.3, 200 0))";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testLineCondensePointsp
template<>
template<>
void object::test<4> ()
{
    std::string wkt1 = "LINESTRING (1 1, 1.3 1, 1.6 1, 1.9 1, 2.2 1, 2.5 1, 2.8 1, 3.1 1, 3.5 1, 4 1)";
    std::string wkt2 = "";
    std::string expected = "LINESTRING (1 1, 2.2 1, 3.5 1)";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testLineDensePointsSelfSnap
template<>
template<>
void object::test<5> ()
{
    std::string wkt1 = "LINESTRING (1 1, 1.3 1, 1.6 1, 1.9 1, 2.2 1, 2.5 1, 2.8 1, 3.1 1, 3.5 1, 4.8 1, 3.8 3.1, 2.5 1.1, 0.5 3.1)";
    std::string wkt2 = "";
    std::string expected = "MULTILINESTRING ((1 1, 2.2 1), (2.2 1, 3.5 1, 4.8 1, 3.8 3.1, 2.2 1), (2.2 1, 1 1), (1 1, 0.5 3.1))";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testAlmostCoincidentEdge
template<>
template<>
void object::test<6> ()
{
    std::string wkt1 = "MULTILINESTRING ((698400.5682737827 2388494.3828697307, 698402.3209180075 2388497.0819257903, 698415.3598714538 2388498.764371397, 698413.5003455497 2388495.90071853, 698400.5682737827 2388494.3828697307), (698231.847335025 2388474.57994264, 698440.416211779 2388499.05985776, 698432.582638943 2388300.28294705, 698386.666515791 2388303.40346027, 698328.29462841 2388312.88889197, 698231.847335025 2388474.57994264))";
    std::string wkt2 = "";
    std::string expected = "MULTILINESTRING ((698231.847335025 2388474.57994264, 698328.29462841 2388312.88889197, 698386.666515791 2388303.40346027, 698432.582638943 2388300.28294705, 698440.416211779 2388499.05985776, 698413.5003455497 2388495.90071853), (698231.847335025 2388474.57994264, 698400.5682737827 2388494.3828697307), (698400.5682737827 2388494.3828697307, 698402.3209180075 2388497.0819257903, 698415.3598714538 2388498.764371397, 698413.5003455497 2388495.90071853), (698400.5682737827 2388494.3828697307, 698413.5003455497 2388495.90071853), (698400.5682737827 2388494.3828697307, 698413.5003455497 2388495.90071853))";
    checkRounding(wkt1, wkt2, 1, expected);
}

//  testAlmostCoincidentines
template<>
template<>
void object::test<7> ()
{
    std::string wkt1 = "MULTILINESTRING ((698413.5003455497 2388495.90071853, 698400.5682737827 2388494.3828697307), (698231.847335025 2388474.57994264, 698440.416211779 2388499.05985776))";
    std::string wkt2 = "";
    std::string expected = "MULTILINESTRING ((698231.847335025 2388474.57994264, 698400.5682737827 2388494.3828697307), (698400.5682737827 2388494.3828697307, 698413.5003455497 2388495.90071853), (698400.5682737827 2388494.3828697307, 698413.5003455497 2388495.90071853), (698413.5003455497 2388495.90071853, 698440.416211779 2388499.05985776))";
    checkRounding(wkt1, wkt2, 1, expected);
}






} // namespace tut
