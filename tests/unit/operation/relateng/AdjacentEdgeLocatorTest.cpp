//
// Test Suite for geos::operation::relateng::AdjacentEdgeLocator class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/operation/relateng/AdjacentEdgeLocator.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_adjacentedgelocator_data {

    WKTReader r;
    // WKTWriter w;

    void
    checkLocation(const std::string& wkt, int x, int y, Location expectedLoc)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        AdjacentEdgeLocator ael(geom.get());
        Coordinate c(x, y);
        Location loc = ael.locate(&c);
        ensure_equals("Locations are not equal ", expectedLoc, loc);
    }
};

typedef test_group<test_adjacentedgelocator_data> group;
typedef group::object object;

group test_adjacentedgelocator_group("geos::operation::relateng::AdjacentEdgeLocator");

//
// Test Cases
//

// testAdjacent2
template<>
template<>
void object::test<1> ()
{
    checkLocation(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 5 9, 5 1, 1 1, 1 9)), POLYGON ((9 9, 9 1, 5 1, 5 9, 9 9)))",
        5, 5, Location::INTERIOR
        );
}

// testNonAdjacent
template<>
template<>
void object::test<2> ()
{
    checkLocation(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 4 9, 5 1, 1 1, 1 9)), POLYGON ((9 9, 9 1, 5 1, 5 9, 9 9)))",
        5, 5, Location::BOUNDARY
        );
}

// testAdjacent6WithFilledHoles
template<>
template<>
void object::test<3> ()
{
    checkLocation(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 5 9, 6 6, 1 5, 1 9), (2 6, 4 8, 6 6, 2 6)), POLYGON ((2 6, 4 8, 6 6, 2 6)), POLYGON ((9 9, 9 5, 6 6, 5 9, 9 9)), POLYGON ((9 1, 5 1, 6 6, 9 5, 9 1), (7 2, 6 6, 8 3, 7 2)), POLYGON ((7 2, 6 6, 8 3, 7 2)), POLYGON ((1 1, 1 5, 6 6, 5 1, 1 1)))",
        6, 6, Location::INTERIOR
        );
}

// testAdjacent5WithEmptyHole
template<>
template<>
void object::test<4> ()
{
    checkLocation(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 5 9, 6 6, 1 5, 1 9), (2 6, 4 8, 6 6, 2 6)), POLYGON ((2 6, 4 8, 6 6, 2 6)), POLYGON ((9 9, 9 5, 6 6, 5 9, 9 9)), POLYGON ((9 1, 5 1, 6 6, 9 5, 9 1), (7 2, 6 6, 8 3, 7 2)), POLYGON ((1 1, 1 5, 6 6, 5 1, 1 1)))",
        6, 6, Location::BOUNDARY
        );
}

// testContainedAndAdjacent
template<>
template<>
void object::test<5> ()
{
    std::string wkt("GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9)), POLYGON ((9 2, 2 2, 2 8, 9 8, 9 2)))");
    checkLocation(wkt,
        9, 5, Location::BOUNDARY
        );
    checkLocation(wkt,
        9, 8, Location::BOUNDARY
        );
}

/**
* Tests a bug caused by incorrect point-on-segment logic.
*/
// testDisjointCollineartemplate<>
template<>
template<>
void object::test<6> ()
{
    checkLocation(
        "GEOMETRYCOLLECTION (MULTIPOLYGON (((1 4, 4 4, 4 1, 1 1, 1 4)), ((5 4, 8 4, 8 1, 5 1, 5 4))))",
        2, 4, Location::BOUNDARY
        );
}





} // namespace tut
