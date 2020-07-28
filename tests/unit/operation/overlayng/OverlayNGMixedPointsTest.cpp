//
// Test Suite for geos::operation::overlayng::OverlayMixedPoints class.

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
struct test_overlayngmixedpoints_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngmixedpoints_data> group;
typedef group::object object;

group test_overlayngmixedpoints_group("geos::operation::overlayng::OverlayNGMixedPoints");

//
// Test Cases
//

// testSimpleLineIntersection
template<>
template<>
void object::test<1> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "POINT (5 1)";
    std::string exp = "POINT (5 1)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testLinePointInOutIntersection
template<>
template<>
void object::test<2> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "MULTIPOINT ((5 1), (15 1))";
    std::string exp = "POINT (5 1)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}
// testSimpleLineUnion
template<>
template<>
void object::test<3> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "POINT (5 1)";
    std::string exp = "LINESTRING (1 1, 9 1)";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}
// testSimpleLineDifference
template<>
template<>
void object::test<4> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "POINT (5 1)";
    std::string exp = "LINESTRING (1 1, 9 1)";
    testOverlay(a, b, exp, OverlayNG::DIFFERENCE, 1);
}
// testSimpleLineSymDifference
template<>
template<>
void object::test<5> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "POINT (5 1)";
    std::string exp = "LINESTRING (1 1, 9 1)";
    testOverlay(a, b, exp, OverlayNG::SYMDIFFERENCE, 1);
}
// testLinePointSymDifference
template<>
template<>
void object::test<6> ()
{
    std::string a = "LINESTRING (1 1, 9 1)";
    std::string b = "POINT (15 1)";
    std::string exp = "GEOMETRYCOLLECTION (LINESTRING (1 1, 9 1), POINT (15 1))";
    testOverlay(a, b, exp, OverlayNG::SYMDIFFERENCE, 1);
}

// testPolygonInsideIntersection
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON ((4 2, 6 2, 6 0, 4 0, 4 2))";
    std::string b = "POINT (5 1)";
    std::string exp = "POINT (5 1)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}
// testPolygonDisjointIntersection
template<>
template<>
void object::test<8> ()
{
    std::string a = "POLYGON ((4 2, 6 2, 6 0, 4 0, 4 2))";
    std::string b = "POINT (15 1)";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testPointEmptyLinestringUnion
template<>
template<>
void object::test<9> ()
{
    std::string a = "LINESTRING EMPTY";
    std::string b = "POINT (10 10)";
    std::string exp = "POINT (10 10)";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

// testLinestringEmptyPointUnion
template<>
template<>
void object::test<10> ()
{
    std::string a = "LINESTRING (10 10, 20 20)";
    std::string b = "POINT EMPTY";
    std::string exp = "LINESTRING (10 10, 20 20)";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

/**
* Result is empty because Line is not rounded.
*/
// testPointLineIntersectionPrec
template<>
template<>
void object::test<11> ()
{
    std::string a = "POINT (10.1 10.4)";
    std::string b = "LINESTRING (9.6 10, 20.1 19.6)";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}




} // namespace tut
