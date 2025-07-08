//
// Test Suite for geos::coverage::CoverageRingEdges class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageRingEdges.h>
#include <geos/coverage/CoverageEdge.h>

using geos::coverage::CoverageRingEdges;
using geos::coverage::CoverageEdge;
using geos::geom::MultiLineString;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coverageringedges_data
 {

    WKTReader r;
    WKTWriter w;

    test_coverageringedges_data() {
    }

    void
    checkEdges(const std::string& wkt, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::vector<const Geometry*> polygons = toArray(*geom);
        CoverageRingEdges cov(polygons);
        std::vector<CoverageEdge*>& edges = cov.getEdges();
        std::unique_ptr<Geometry> edgeLines = toArray(edges, geom->getFactory());
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        ensure_equals_geometry(edgeLines.get(), expected.get());
    }

    void
    checkEdgesSelected(const std::string& wkt, std::size_t ringCount, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        auto polygons = toArray(*geom);
        CoverageRingEdges covEdges(polygons);
        auto edges = covEdges.selectEdges(ringCount);
        auto edgeLines = toArray(edges, geom->getFactory());
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        ensure_equals_geometry(edgeLines.get(), expected.get());
    }

    std::unique_ptr<Geometry>
    toArray(std::vector<CoverageEdge*>& edges, const GeometryFactory* geomFactory)
    {
        std::vector<std::unique_ptr<LineString>> lines;
        for (auto* edge : edges) {
            lines.push_back(edge->toLineString(geomFactory));
        }
        return geomFactory->createMultiLineString(std::move(lines));
    }

    std::vector<const Geometry*>
    toArray(const Geometry& geom)
    {
        std::vector<const Geometry*> geoms;
        for (std::size_t i = 0; i < geom.getNumGeometries(); i++) {
            geoms.push_back(geom.getGeometryN(i));
        }
        return geoms;
    }
};


typedef test_group<test_coverageringedges_data> group;
typedef group::object object;

group test_coverageringedges_data("geos::coverage::CoverageRingEdges");


// testTwoAdjacent
template<>
template<>
void object::test<1> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))",
        "MULTILINESTRING ((1 6, 1 1, 9 1, 9 6, 6 5), (1 6, 1 9, 6 9, 6 5), (1 6, 6 5))"
    );
}

// testTwoAdjacentWithFilledHole
template<>
template<>
void object::test<2> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1), (2 4, 4 4, 4 2, 2 2, 2 4)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)), POLYGON ((4 2, 2 2, 2 4, 4 4, 4 2)))",
        "MULTILINESTRING ((1 6, 1 1, 9 1, 9 6, 6 5), (1 6, 1 9, 6 9, 6 5), (1 6, 6 5), (2 4, 2 2, 4 2, 4 4, 2 4))"
    );
}

// testHolesAndFillWithDifferentEndpoints
template<>
template<>
void object::test<3> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (POLYGON ((0 10, 10 10, 10 0, 0 0, 0 10), (1 9, 4 8, 9 9, 9 1, 1 1, 1 9)), POLYGON ((9 9, 1 1, 1 9, 4 8, 9 9)), POLYGON ((1 1, 9 9, 9 1, 1 1)))",
        "MULTILINESTRING ((0 10, 0 0, 10 0, 10 10, 0 10), (1 1, 1 9, 4 8, 9 9), (1 1, 9 1, 9 9), (1 1, 9 9))"
    );
}

// testTouchingSquares
template<>
template<>
void object::test<4> ()
{
    std::string wkt = "MULTIPOLYGON (((2 7, 2 8, 3 8, 3 7, 2 7)), ((1 6, 1 7, 2 7, 2 6, 1 6)), ((0 7, 0 8, 1 8, 1 7, 0 7)), ((0 5, 0 6, 1 6, 1 5, 0 5)), ((2 5, 2 6, 3 6, 3 5, 2 5)))";
    checkEdgesSelected(wkt, 1,
        "MULTILINESTRING ((1 6, 0 6, 0 5, 1 5, 1 6), (1 6, 1 7), (1 6, 2 6), (1 7, 0 7, 0 8, 1 8, 1 7), (1 7, 2 7), (2 6, 2 5, 3 5, 3 6, 2 6), (2 6, 2 7), (2 7, 2 8, 3 8, 3 7, 2 7))");
    checkEdgesSelected(wkt, 2,
        "MULTILINESTRING EMPTY");
}

// testAdjacentSquares
template<>
template<>
void object::test<5> ()
{
    std::string wkt = "GEOMETRYCOLLECTION (POLYGON ((1 3, 2 3, 2 2, 1 2, 1 3)), POLYGON ((3 3, 3 2, 2 2, 2 3, 3 3)), POLYGON ((3 1, 2 1, 2 2, 3 2, 3 1)), POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1)))";
    checkEdgesSelected(wkt, 1,
        "MULTILINESTRING ((1 2, 1 1, 2 1), (1 2, 1 3, 2 3), (2 1, 3 1, 3 2), (2 3, 3 3, 3 2))");
    checkEdgesSelected(wkt, 2,
        "MULTILINESTRING ((1 2, 2 2), (2 1, 2 2), (2 2, 2 3), (2 2, 3 2))");
}

// testMultiPolygons
template<>
template<>
void object::test<6> ()
{
    checkEdges(
        "GEOMETRYCOLLECTION (MULTIPOLYGON (((5 9, 2.5 7.5, 1 5, 5 5, 5 9)), ((5 5, 9 5, 7.5 2.5, 5 1, 5 5))), MULTIPOLYGON (((5 9, 6.5 6.5, 9 5, 5 5, 5 9)), ((1 5, 5 5, 5 1, 3.5 3.5, 1 5))))",
        "MULTILINESTRING ((1 5, 2.5 7.5, 5 9), (1 5, 3.5 3.5, 5 1), (1 5, 5 5), (5 1, 5 5), (5 1, 7.5 2.5, 9 5), (5 5, 5 9), (5 5, 9 5), (5 9, 6.5 6.5, 9 5))"
    );
}

} // namespace tut
