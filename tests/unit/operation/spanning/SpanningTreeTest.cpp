//
// Test Suite for geos::operation::spanning::SpanningTree class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/spanning/SpanningTree.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryCollection.h>
// std
#include <memory>
#include <string>
#include <vector>
#include <numeric>
#include <set>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_spanning_data {
    geos::io::WKTReader wktreader;

    test_spanning_data()
        : wktreader()
    {
    }

    std::unique_ptr<geos::geom::Geometry>
    readWKT(const std::string& inputWKT)
    {
        return std::unique_ptr<geos::geom::Geometry>(wktreader.read(inputWKT));
    }
    
    std::vector<const geos::geom::LineString*>
    toLines(const geos::geom::Geometry* geom) {
        std::vector<const geos::geom::LineString*> lines;
        if (const geos::geom::GeometryCollection* gc = dynamic_cast<const geos::geom::GeometryCollection*>(geom)) {
            for (std::size_t i = 0; i < gc->getNumGeometries(); ++i) {
                if (const geos::geom::LineString* ls = dynamic_cast<const geos::geom::LineString*>(gc->getGeometryN(i))) {
                    lines.push_back(ls);
                }
            }
        }
        return lines;
    }
};

typedef test_group<test_spanning_data> group;
typedef group::object object;

group test_spanning_group("geos::operation::spanning::SpanningTree");

//
// Test Cases
//

// Basic triangle test
template<> template<>
void object::test<1>()
{
    // Triangle with side lengths 10, ~11.18, ~11.18
    // MST should pick the shortest edge (10) and one of the others.
    // Lengths:
    // 0: (0 0, 10 0) -> 10
    // 1: (10 0, 5 10) -> sqrt(25+100) = 11.18
    // 2: (5 10, 0 0) -> sqrt(25+100) = 11.18
    
    std::string wkt = "MULTILINESTRING((0 0, 10 0), (10 0, 5 10), (5 10, 0 0))";
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get());

    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);

    ensure_equals(result.size(), 3u);
    
    // Count edges in MST (should be 2)
    int count = 0;
    for (int r : result) {
        if (r > 0) count++;
    }
    ensure_equals(count, 2);
    
    // Edge 0 (length 10) must be included
    ensure(result[0] > 0);
    
    // Component IDs should be the same (connected graph)
    int compId = result[0];
    for (int r : result) {
        if (r > 0) ensure_equals(r, compId);
    }
}

// Disconnected graph
template<> template<>
void object::test<2>()
{
    std::string wkt = "MULTILINESTRING((0 0, 10 0), (20 0, 30 0))";
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get());

    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);

    ensure_equals(result.size(), 2u);
    ensure(result[0] > 0);
    ensure(result[1] > 0);
    
    // Should be different components
    ensure(result[0] != result[1]);
}

// Complex graph
template<> template<>
void object::test<3>()
{
    std::string wkt = "MULTILINESTRING("
        "(0 0, 10 0), (10 0, 10 10), (10 10, 0 10), (0 10, 0 0), " // 0-3: Sides (10)
        "(0 0, 10 10), (10 0, 0 10))"; // 4-5: Diagonals (~14.14)
    
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get());
    
    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    
    ensure_equals(result.size(), 6u);
    
    int count = 0;
    std::set<int> components;
    for (int r : result) {
        if (r > 0) {
            count++;
            components.insert(r);
        }
    }
    // A square with 2 diagonals has 4 vertices. MST should have 3 edges.
    // Wait, 4 edges for square + 2 diagonals = 6 edges.
    // Vertices: 4.
    // MST edges = Vertices - 1 = 3.
    // result should have 3 non-zero entries.
    ensure_equals(count, 3);
    
    // Should be 1 component
    ensure_equals(components.size(), 1u);
    
    // Diagonals should not be included as they are longer
    ensure_equals(result[4], 0);
    ensure_equals(result[5], 0);
}

// Empty input
template<> template<>
void object::test<4>()
{
    std::vector<const geos::geom::LineString*> lines;
    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    ensure(result.empty());
}

// Null pointers and empty geometries
template<> template<>
void object::test<5>()
{
    auto factory = geos::geom::GeometryFactory::create();
    auto empty = factory->createLineString();
    
    std::vector<const geos::geom::LineString*> lines;
    lines.push_back(nullptr);
    lines.push_back(empty.get());
    
    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    
    ensure_equals(result.size(), 2u);
    ensure_equals(result[0], 0);
    ensure_equals(result[1], 0);
}

// Short lines (zero length) - should be ignored
template<> template<>
void object::test<6>()
{
    std::string wkt = "MULTILINESTRING((0 0, 0 0))";
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get());
    
    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    
    ensure_equals(result.size(), 1u);
    ensure_equals(result[0], 0);
}

// Loops (start == end)
template<> template<>
void object::test<7>()
{
    std::string wkt = "LINESTRING(0 0, 10 10, 0 0)";
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get()); // wait, toLines expects a collection
    
    // Manual setup if needed, but MULTILINESTRING is easier
    wkt = "MULTILINESTRING((0 0, 10 10, 0 0))";
    geom = readWKT(wkt);
    lines = toLines(geom.get());

    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    
    ensure_equals(result.size(), 1u);
    // A single loop doesn't form a tree with 2 nodes? 
    // Actually, start and end are the same node. So 1 node, 1 edge.
    // Kruskal: find(start) == find(end), so not included.
    ensure_equals(result[0], 0);
}

// Multi-edges (two edges between same nodes)
template<> template<>
void object::test<8>()
{
    std::string wkt = "MULTILINESTRING((0 0, 10 0), (0 0, 10 0))";
    auto geom = readWKT(wkt);
    auto lines = toLines(geom.get());

    std::vector<int> result;
    geos::operation::spanning::SpanningTree::mst(lines, result);
    
    ensure_equals(result.size(), 2u);
    // One should be in, one out.
    int count = 0;
    for (int r : result) if (r > 0) count++;
    ensure_equals(count, 1);
}

} // namespace tut
