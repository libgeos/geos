//
// Test Suite for geos::coverage::CoverageEdges class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageEdges.h>
#include <geos/geom/Geometry.h>

using geos::coverage::CoverageEdges;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coverageedges_data
 {

    WKTReader r;
    WKTWriter w;

    test_coverageedges_data() {
    }

    void
    checkEdges(const std::string& wkt, int type, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<Geometry> edgeLines = CoverageEdges::GetEdges(geom.get(), type);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        ensure_equals_geometry(edgeLines.get(), expected.get());
    }
};


typedef test_group<test_coverageedges_data> group;
typedef group::object object;

group test_coverageedges_data("geos::coverage::CoverageEdges");


// testTwoAdjacentAll
template<>
template<>
void object::test<1> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))",
        0,
        "MULTILINESTRING ((1 6, 1 1, 9 1, 9 6, 6 5), (1 6, 1 9, 6 9, 6 5), (1 6, 6 5))"
    );
}

// testTwoAdjacentInterior
template<>
template<>
void object::test<2> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))",
        2, // INTERIOR
        "MULTILINESTRING ((1 6, 6 5))"
    );
}

// testTwoAdjacentExterior
template<>
template<>
void object::test<3> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))",
        1, // EXTERIOR
        "MULTILINESTRING ((1 6, 1 1, 9 1, 9 6, 6 5), (1 6, 1 9, 6 9, 6 5))"
    );
}

// testAdjacentSquaresInterior
template<>
template<>
void object::test<4> ()
{
    std::string wkt = "GEOMETRYCOLLECTION (POLYGON ((1 3, 2 3, 2 2, 1 2, 1 3)), POLYGON ((3 3, 3 2, 2 2, 2 3, 3 3)), POLYGON ((3 1, 2 1, 2 2, 3 2, 3 1)), POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1)))";
    checkEdges(wkt, 2, // INTERIOR
        "MULTILINESTRING ((1 2, 2 2), (2 1, 2 2), (2 2, 2 3), (2 2, 3 2))");
}

// testTouchingAtPointInterior
template<>
template<>
void object::test<5> ()
{
    // Two polygons touching only at a point (1 1)
    std::string wkt = "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0)), POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1)))";
    checkEdges(wkt, 2, // INTERIOR
        "MULTILINESTRING EMPTY");
}

// testTouchingAtPointExterior
template<>
template<>
void object::test<6> ()
{
    // Two polygons touching only at a point (1 1)
    std::string wkt = "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0)), POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1)))";
    checkEdges(wkt, 1, // EXTERIOR
        "MULTILINESTRING ((0 0, 0 1, 1 1, 1 0, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
}

} // namespace tut
