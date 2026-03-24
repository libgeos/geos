//
// Test Suite for geos::operation::spanning::SpanningTree class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/spanning/SpanningTree.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Curve.h>
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
    
    std::vector<const geos::geom::Curve*>
    toCurves(const geos::geom::Geometry* geom) {
        std::vector<const geos::geom::Curve*> curves;
        if (const geos::geom::GeometryCollection* gc = dynamic_cast<const geos::geom::GeometryCollection*>(geom)) {
            for (std::size_t i = 0; i < gc->getNumGeometries(); ++i) {
                if (const geos::geom::Curve* c = dynamic_cast<const geos::geom::Curve*>(gc->getGeometryN(i))) {
                    curves.push_back(c);
                }
            }
        } else if (const geos::geom::Curve* c = dynamic_cast<const geos::geom::Curve*>(geom)) {
            curves.push_back(c);
        }
        return curves;
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
    auto curves = toCurves(geom.get());

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);

    ensure_equals(result.size(), 3u);
    
    // Count edges in MST (should be 2)
    int count = 0;
    for (std::size_t r : result) {
        if (r > 0) count++;
    }
    ensure_equals(count, 2);
    
    // Edge 0 (length 10) must be included
    ensure(result[0] > 0);
    
    // Component IDs should be the same (connected graph)
    std::size_t compId = result[0];
    for (std::size_t r : result) {
        if (r > 0) ensure_equals(r, compId);
    }
}

// Disconnected graph
template<> template<>
void object::test<2>()
{
    std::string wkt = "MULTILINESTRING((0 0, 10 0), (20 0, 30 0))";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);

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
    auto curves = toCurves(geom.get());
    
    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    
    ensure_equals(result.size(), 6u);
    
    int count = 0;
    std::set<std::size_t> components;
    for (std::size_t r : result) {
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
    ensure_equals(result[4], 0u);
    ensure_equals(result[5], 0u);
}

// Empty input
template<> template<>
void object::test<4>()
{
    std::vector<const geos::geom::Curve*> curves;
    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    ensure(result.empty());
}

// Null pointers and empty geometries
template<> template<>
void object::test<5>()
{
    auto factory = geos::geom::GeometryFactory::create();
    auto empty = factory->createLineString();
    
    std::vector<const geos::geom::Curve*> curves;
    curves.push_back(nullptr);
    curves.push_back(empty.get());
    
    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    
    ensure_equals(result.size(), 2u);
    ensure_equals(result[0], 0u);
    ensure_equals(result[1], 0u);
}

// Short lines (zero length) - should be ignored
template<> template<>
void object::test<6>()
{
    std::string wkt = "MULTILINESTRING((0 0, 0 0))";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());
    
    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    
    ensure_equals(result.size(), 1u);
    ensure_equals(result[0], 0u);
}

// Loops (start == end)
template<> template<>
void object::test<7>()
{
    std::string wkt = "MULTILINESTRING((0 0, 10 10, 0 0))";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    
    ensure_equals(result.size(), 1u);
    // A single loop doesn't form a tree with 2 nodes? 
    // Actually, start and end are the same node. So 1 node, 1 edge.
    // Kruskal: find(start) == find(end), so not included.
    ensure_equals(result[0], 0u);
}

// Multi-edges (two edges between same nodes)
template<> template<>
void object::test<8>()
{
    std::string wkt = "MULTILINESTRING((0 0, 10 0), (0 0, 10 0))";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    
    ensure_equals(result.size(), 2u);
    // One should be in, one out.
    int count = 0;
    for (std::size_t r : result) if (r > 0) count++;
    ensure_equals(count, 1);
}

// Mixed Curved geometries
template<> template<>
void object::test<9>()
{
    std::string wkt = "GEOMETRYCOLLECTION("
        "CIRCULARSTRING(0 0, 5 5, 10 0),"
        "LINESTRING(10 0, 10 10),"
        "COMPOUNDCURVE(CIRCULARSTRING(10 10, 5 15, 0 10), (0 10, 0 0))"
        ")";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());
    ensure_equals(curves.size(), 3u);

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);

    int count = 0;
    for (std::size_t r : result) if (r > 0) count++;
    
    // Debug:
    // for (std::size_t i = 0; i < curves.size(); ++i) {
    //    std::cout << "Curve " << i << " length=" << curves[i]->getLength() << " result=" << result[i] << std::endl;
    // }

    // Geometries:
    // C0: (0,0) -> (10,0) via (5,5). Length: PI*5 ~= 15.7
    // C1: (10,0) -> (10,10). Length: 10
    // C2: (10,10) -> (0,0) via (5,15) and (0,10). 
    //     Part 1: (10,10) -> (0,10) via (5,15). Length: PI*5 ~= 15.7
    //     Part 2: (0,10) -> (0,0). Length: 10
    //     Total C2 length ~= 25.7
    
    // Nodes: (0,0), (10,0), (10,10). 
    // Wait, (0,10) is NOT a node because it's INTERNAL to COMPOUNDCURVE C2.
    // So we have 3 nodes: N0(0,0), N1(10,0), N2(10,10).
    // Edges:
    // E0: N0-N1 (Length 15.7)
    // E1: N1-N2 (Length 10)
    // E2: N2-N0 (Length 25.7)
    
    // MST should pick E1 (10) and E0 (15.7). Total 2 edges.
    // My expectation of 3 was wrong because I thought there were 4 nodes.
    ensure_equals(count, 2);
}

// Simple square
template<> template<>
void object::test<10>()
{
    std::string wkt = "MULTILINESTRING((0 0,1 0),(1 1,1 0),(0 0,1 1),(0 0,0 1),(1 1,0 1),(1 0,0 1))";
    auto geom = readWKT(wkt);
    auto curves = toCurves(geom.get());

    std::vector<std::size_t> result;
    geos::operation::spanning::SpanningTree::mst(curves, result);
    ensure_equals(result.size(), 6u);
    // MULTILINESTRING((0 0,1 0),(1 1,1 0),(0 0,0 1))

    int count = 0;
    for (std::size_t r : result) {
        //std::cout << r << std::endl;
        if (r > 0) count++;
    }
    ensure_equals(count, 3);
}


} // namespace tut
