//
// Test Suite for handling of Z in geos::operation::overlayng::OverlayNG class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayngz_data {

    WKTReader r;

    void checkIntersection(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> a = r.read(wktA);
        std::unique_ptr<Geometry> b = r.read(wktB);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = OverlayNG::overlay(a.get(), b.get(), OverlayNG::INTERSECTION);
        ensure_equals_geometry(expected.get(), result.get());
    }

};

typedef test_group<test_overlayngz_data> group;
typedef group::object object;

group test_overlayngz_group("geos::operation::overlayng::OverlayNGZ");

//
// Test Cases
//

// testLineIntersectionInterpolated
template<>
template<>
void object::test<1> ()
{
    checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 0 10 10)", "POINT(5 5 5)");
}


} // namespace tut
