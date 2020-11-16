//
// Test Suite for handling of Z in geos::operation::overlayng::OverlayNG class.
//
//
// Ported from JTS
// modules/core/src/test/java/org/locationtech/jts/operation/overlayng/OverlyNGZTest.java
// 0524f550f76e32fb0c2f1badc3775d67a9fa9189
//

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

    void checkOverlay(int opCode, const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> a = r.read(wktA);
        std::unique_ptr<Geometry> b = r.read(wktB);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = OverlayNG::overlay(a.get(), b.get(), opCode);
        ensure_equals_geometry_xyz(expected.get(), result.get());
    }

    void checkIntersection(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::INTERSECTION, wktA, wktB, wktExpected);
    }

    void checkDifference(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::DIFFERENCE, wktA, wktB, wktExpected);
    }

};

typedef test_group<test_overlayngz_data> group;
typedef group::object object;

group test_overlayngz_group("geos::operation::overlayng::OverlayNGZ");

//
// Test Cases
//

// testLineIntersectionPointInterpolated
template<>
template<>
void object::test<1> ()
{
    checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 0 10 10)", "POINT(5 5 5)");
}

// testLineIntersectionPoint
template<>
template<>
void object::test<2> ()
{
    checkIntersection(
        "LINESTRING (0 0 0, 10 10 10)",
        "LINESTRING (10 0 0, 5 5 999, 0 10 10)",
        "POINT(5 5 999)"
    );
}

//  testLineDifferenceLineInterpolated
template<>
template<>
void object::test<3> ()
{
    checkDifference(
        "LINESTRING (0 0 0, 10 10 10)",
        "LINESTRING (5 5, 6 6)",
        "MULTILINESTRING ((0 0 0, 5 5 5), (6 6 6, 10 10 10))"
    );
}


} // namespace tut
