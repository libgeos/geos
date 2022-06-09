//
// Test Suite for geos::operation::coverageunionng::OverlayNG class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/CoverageUnion.h>

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
struct test_coverageunionng_data {

    WKTReader r;
    WKTWriter w;

    void
    checkCoverageUnion(const std::string& wkt, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = CoverageUnion::geomunion(geom.get());
        // std::string wkt_result = w.write(result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(result.get(), expected.get());
    }

};

typedef test_group<test_coverageunionng_data> group;
typedef group::object object;

group test_coverageunionng_group("geos::operation::overlayng::CoverageUnionNG");

//
// Test Cases
//

// testFilledHole
template<>
template<>
void object::test<1> ()
{
    checkCoverageUnion(
        "MULTIPOLYGON (((100 200, 200 200, 200 100, 100 100, 100 200), (120 180, 180 180, 180 120, 120 120, 120 180)), ((180 120, 120 120, 120 180, 180 180, 180 120)))",
        "POLYGON ((200 200, 200 100, 100 100, 100 200, 200 200))"
        );
}

// test3Squares
template<>
template<>
void object::test<2> ()
{
    checkCoverageUnion(
        "MULTIPOLYGON (((1 4, 3 4, 3 2, 1 2, 1 4)), ((5 4, 5 2, 3 2, 3 4, 5 4)), ((7 4, 7 2, 5 2, 5 4, 7 4)))",
        "POLYGON ((3 4, 5 4, 7 4, 7 2, 5 2, 3 2, 1 2, 1 4, 3 4))"
        );
}

} // namespace tut
