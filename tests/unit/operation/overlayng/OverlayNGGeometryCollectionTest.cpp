//
// Test Suite for geos::operation::overlayng::OverlayNG class for GeometryCollections.

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
struct test_overlaynggc_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode)
    {
        PrecisionModel pm;
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

    void
    testIntersection(const std::string& a, const std::string& b, const std::string& expected)
    {
        testOverlay(a, b, expected, OverlayNG::INTERSECTION);
    }

    void
    testUnion(const std::string& a, const std::string& b, const std::string& expected)
    {
        testOverlay(a, b, expected, OverlayNG::UNION);
    }
};

typedef test_group<test_overlaynggc_data> group;
typedef group::object object;

group test_overlaynggc_group("geos::operation::overlayng::OverlayNGGeometryCollection");

//
// Test Cases
//

//  testSimpleA_mP
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON ((0 0, 0 1, 1 1, 0 0))";
    std::string b = "GEOMETRYCOLLECTION ( MULTIPOINT ((0 0), (99 99)) )";
    testIntersection(a, b,
        "POINT (0 0)");
    testUnion(a, b,
        "GEOMETRYCOLLECTION (POINT (99 99), POLYGON ((0 0, 0 1, 1 1, 0 0)))");
}

//  testSimpleP_mP
template<>
template<>
void object::test<2> ()
{
    std::string a = "POINT(0 0)";
    std::string b = "GEOMETRYCOLLECTION ( MULTIPOINT ((0 0), (99 99)) )";
    testIntersection(a, b,
        "POINT (0 0)");
    testUnion(a, b,
        "MULTIPOINT ((0 0), (99 99))");
}

//  testSimpleP_mL
template<>
template<>
void object::test<3> ()
{
    std::string a = "POINT(5 5)";
    std::string b = "GEOMETRYCOLLECTION ( MULTILINESTRING ((1 9, 9 1), (1 1, 9 9)) )";
    testIntersection(a, b,
        "POINT (5 5)");
    testUnion(a, b,
        "MULTILINESTRING ((1 1, 5 5), (1 9, 5 5), (5 5, 9 1), (5 5, 9 9))");
}

//  testSimpleP_mA
template<>
template<>
void object::test<4> ()
{
    std::string a = "POINT(5 5)";
    std::string b = "GEOMETRYCOLLECTION ( MULTIPOLYGON (((1 1, 1 5, 5 5, 5 1, 1 1)), ((9 9, 9 5, 5 5, 5 9, 9 9))) )";
    testIntersection(a, b,
        "POINT (5 5)");
    testUnion(a, b,
        "MULTIPOLYGON (((1 1, 1 5, 5 5, 5 1, 1 1)), ((9 9, 9 5, 5 5, 5 9, 9 9)))");
}

//  testSimpleP_AA
template<>
template<>
void object::test<5> ()
{
    std::string a = "POINT(5 5)";
    std::string b = "GEOMETRYCOLLECTION ( POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1)), POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9)) )";
    testIntersection(a, b,
        "POINT (5 5)");
    testUnion(a, b,
        "MULTIPOLYGON (((1 1, 1 5, 5 5, 5 1, 1 1)), ((9 9, 9 5, 5 5, 5 9, 9 9)))");
    }

//  testSimpleL_AA
template<>
template<>
void object::test<6> ()
{
    std::string a = "LINESTRING (0 0, 10 10)";
    std::string b = "GEOMETRYCOLLECTION ( POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1)), POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9)) )";
    testIntersection(a, b,
        "MULTILINESTRING ((1 1, 5 5), (5 5, 9 9))");
    testUnion(a, b,
        "GEOMETRYCOLLECTION (LINESTRING (0 0, 1 1), LINESTRING (9 9, 10 10), POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1)), POLYGON ((5 5, 5 9, 9 9, 9 5, 5 5)))");
    }

//  testSimpleA_AA
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON ((2 8, 8 8, 8 2, 2 2, 2 8))";
    std::string b = "GEOMETRYCOLLECTION ( POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1)), POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9)) )";
    testIntersection(a, b,
        "MULTIPOLYGON (((2 2, 2 5, 5 5, 5 2, 2 2)), ((5 5, 5 8, 8 8, 8 5, 5 5)))");
    testUnion(a, b,
        "POLYGON ((1 1, 1 5, 2 5, 2 8, 5 8, 5 9, 9 9, 9 5, 8 5, 8 2, 5 2, 5 1, 1 1))");
}

} // namespace tut
