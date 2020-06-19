//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>

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

    // void
    // checkRounding(std::string& wkt, double scale, std::string& expected_wkt)
    // {
    //     std::unique_ptr<Geometry> geom = r.read(wkt);
    //     PrecisionModel pm(scale);
    //     SnapRoundingNoder noder(&pm);
    //     std::unique_ptr<Geometry> result = nodeValidated(geom.get(), nullptr, noder);

    //     // only check if expected was provided
    //     if (expected_wkt.size() == 0) return;

    //     std::unique_ptr<Geometry> expected = r.read(expected_wkt);
    //     ensure_equals_geometry(expected.get(), result.get());
    // }


    // test_snappingnoder_data() {}
};

typedef test_group<test_snappingnoder_data> group;
typedef group::object object;

group test_snappingnoder_group("geos::noding::snap::SnappingNoder");

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
    // checkRounding(wkt, 1, expected);
}






} // namespace tut
