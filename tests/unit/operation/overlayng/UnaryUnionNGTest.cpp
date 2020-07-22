//
// Test Suite for geos::operation::unaryunionng::OverlayNG class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/UnaryUnionNG.h>

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
struct test_unaryunionng_data {

    WKTReader r;
    WKTWriter w;

    void
    checkUnaryUnion(const std::string& wkt, double scaleFactor, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> result = UnaryUnionNG::Union(geom.get(), pm);
        // std::string wkt_result = w.write(result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(result.get(), expected.get());
    }

};

typedef test_group<test_unaryunionng_data> group;
typedef group::object object;

group test_unaryunionng_group("geos::operation::overlayng::UnaryUnionNG");

//
// Test Cases
//

// testMultiPolygonNarrowGap
template<>
template<>
void object::test<1> ()
{
    checkUnaryUnion(
        "MULTIPOLYGON (((1 9, 5.7 9, 5.7 1, 1 1, 1 9)), ((9 9, 9 1, 6 1, 6 9, 9 9)))",
        1,
        "POLYGON ((1 9, 6 9, 9 9, 9 1, 6 1, 1 1, 1 9))"
        );
}

// testPolygonsRounded
template<>
template<>
void object::test<2> ()
{
    checkUnaryUnion(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 6 9, 6 1, 1 1, 1 9)), POLYGON ((9 1, 2 8, 9 9, 9 1)))",
        1,
        "POLYGON ((1 9, 6 9, 9 9, 9 1, 6 4, 6 1, 1 1, 1 9))"
        );
}

// testPolygonsOverlapping
template<>
template<>
void object::test<3> ()
{
    checkUnaryUnion(
        "GEOMETRYCOLLECTION (POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200)), POLYGON ((250 250, 250 150, 150 150, 150 250, 250 250)))",
        1,
        "POLYGON ((100 200, 150 200, 150 250, 250 250, 250 150, 200 150, 200 100, 100 100, 100 200))"
        );
}



} // namespace tut
