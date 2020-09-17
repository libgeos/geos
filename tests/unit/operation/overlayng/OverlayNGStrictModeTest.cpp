//
// Test Suite for geos::operation::overlayng::OverlayNGRobust class.
//
// Useful place for test cases raised by 3rd party software, that will be using
// Geometry->Intersection(), Geometry->Union(), etc, that call into the
// OverlayNGRobust utility class that bundles up different precision models
// and noders to provide a "best case" overlay for all inputs.
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
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayngstring_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode)
    {
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        OverlayNG ov(geom_a.get(), geom_b.get(), opCode);
        ov.setStrictMode(true);
        std::unique_ptr<Geometry> geom_result = ov.getResult();
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        OverlayNG ov(geom_a.get(), geom_b.get(), &pm, opCode);
        ov.setStrictMode(true);
        std::unique_ptr<Geometry> geom_result = ov.getResult();
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngstring_data> group;
typedef group::object object;

group test_overlayngstring_data("geos::operation::overlayng::OverlayNGStrictMode");

//
// Test Cases
//

// testPolygonTouchALPIntersection
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON ((10 10, 10 30, 30 30, 30 10, 10 10))";
    std::string b = "POLYGON ((40 10, 30 10, 35 15, 30 15, 30 20, 35 20, 25 30, 40 30, 40 10))";
    std::string expected = "POLYGON ((30 25, 25 30, 30 30, 30 25))";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

// testPolygonTouchALIntersection
template<>
template<>
void object::test<3> ()
{
    std::string a = "POLYGON ((10 30, 60 30, 60 10, 10 10, 10 30))";
    std::string b = "POLYGON ((10 50, 60 50, 60 30, 30 30, 10 10, 10 50))";
    std::string expected = "POLYGON ((30 30, 10 10, 10 30, 30 30))";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

// testPolygonTouchLPIntersection
template<>
template<>
void object::test<4> ()
{
    std::string a = "POLYGON ((10 10, 10 30, 30 30, 30 10, 10 10))";
    std::string b = "POLYGON ((40 25, 30 25, 30 20, 35 20, 30 15, 40 15, 40 25))";
    std::string expected = "LINESTRING (30 25, 30 20)";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

// testLineTouchLPIntersection
template<>
template<>
void object::test<5> ()
{
    std::string a = "LINESTRING (10 10, 20 10, 20 20, 30 10)";
    std::string b = "LINESTRING (10 10, 30 10)";
    std::string expected = "LINESTRING (10 10, 20 10)";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

// testPolygonResultMixedIntersection
template<>
template<>
void object::test<6> ()
{
    std::string a = "POLYGON ((10 30, 60 30, 60 10, 10 10, 10 30))";
    std::string b = "POLYGON ((10 50, 60 50, 60 30, 30 30, 10 10, 10 50))";
    std::string expected = "POLYGON ((30 30, 10 10, 10 30, 30 30))";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

// testPolygonResultLineIntersection
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON ((10 20, 20 20, 20 10, 10 10, 10 20))";
    std::string b = "POLYGON ((30 20, 30 10, 20 10, 20 20, 30 20))";
    std::string expected = "LINESTRING (20 20, 20 10)";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION);
}

/**
* Symmetric Difference is the one exception
* to the Strict Mode homogeneous output rule.
*/
// testPolygonLineSymDifference
template<>
template<>
void object::test<8> ()
{
    std::string a = "POLYGON ((10 20, 20 20, 20 10, 10 10, 10 20))";
    std::string b = "LINESTRING (15 15, 25 15)";
    std::string expected = "GEOMETRYCOLLECTION (POLYGON ((20 20, 20 15, 20 10, 10 10, 10 20, 20 20)), LINESTRING (20 15, 25 15))";
    testOverlay(a, b, expected, OverlayNG::SYMDIFFERENCE);
}

/**
* Check that result does not include collapsed line intersection
*/
// testPolygonIntersectionCollapse
template<>
template<>
void object::test<9> ()
{
    std::string a = "POLYGON ((1 1, 1 5, 3 5, 3 2, 9 1, 1 1))";
    std::string b = "POLYGON ((7 5, 9 5, 9 1, 7 1, 7 5))";
    std::string expected = "POLYGON EMPTY";
    testOverlay(a, b, expected, OverlayNG::INTERSECTION, 1.0);
}

// testPolygonUnionCollapse
template<>
template<>
void object::test<10> ()
{
    std::string a = "POLYGON ((1 1, 1 5, 3 5, 3 1.4, 7 1, 1 1))";
    std::string b = "POLYGON ((7 5, 9 5, 9 1, 7 1, 7 5))";
    std::string expected = "MULTIPOLYGON (((1 1, 1 5, 3 5, 3 1, 1 1)), ((7 1, 7 5, 9 5, 9 1, 7 1)))";
    testOverlay(a, b, expected, OverlayNG::UNION, 1.0);
}


// testPolygonLineUnion
template<>
template<>
void object::test<11> ()
{
    std::string a = "POLYGON ((10 20, 20 20, 20 10, 10 10, 10 20))";
    std::string b = "LINESTRING (15 15, 25 15)";
    std::string expected = "GEOMETRYCOLLECTION (POLYGON ((20 20, 20 15, 20 10, 10 10, 10 20, 20 20)), LINESTRING (20 15, 25 15))";
    testOverlay(a, b, expected, OverlayNG::UNION);
}


} // namespace tut
