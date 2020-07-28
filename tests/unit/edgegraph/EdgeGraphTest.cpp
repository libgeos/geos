#include <tut/tut.hpp>
// geos
#include <geos/edgegraph/EdgeGraph.h>
#include <geos/edgegraph/HalfEdge.h>
#include <geos/edgegraph/EdgeGraphBuilder.h>
#include <geos/geom/Envelope.h>
#include <geos/io/WKTReader.h>

using namespace geos::edgegraph;
using namespace geos::geom;

namespace tut {

// dummy data, not used
struct test_edgegraph_data {

    geos::io::WKTReader reader_;

    HalfEdge* addEdge(EdgeGraph& graph, double p0x, double p0y, double p1x, double p1y) {
        return graph.addEdge(Coordinate(p0x, p0y), Coordinate(p1x, p1y));
    }

    std::unique_ptr<EdgeGraph> build(std::string wkt) {
        auto geo = reader_.read(wkt);
        auto geocol = dynamic_cast<const GeometryCollection *>(geo.get());
        ensure(geocol);
        return EdgeGraphBuilder::build(geocol);
    }

    void checkNodeValid(HalfEdge* e) {
        bool isNodeValid = e->isEdgesSorted();
        ensure("Found non-sorted edges around node", isNodeValid);
    }

    void checkNodeValid(EdgeGraph& graph, Coordinate& p0, Coordinate& p1) {
        HalfEdge* e = graph.findEdge(p0, p1);
        bool isNodeValid = e->isEdgesSorted();
        ensure("Found non-sorted edges around node", isNodeValid);
    }

    void checkEdge(EdgeGraph& graph, Coordinate& p0, Coordinate& p1) {
        HalfEdge* e = graph.findEdge(p0, p1);
        ensure(e);
    }

    void checkEdgeRing(EdgeGraph& graph, Coordinate& p, std::vector<Coordinate>& dest) {
        HalfEdge* e = graph.findEdge(p, dest[0]);
        HalfEdge* onext = e;
        int i = 0;
        do {
            ensure(onext->dest().equals2D(dest[i++]));
            onext = onext->oNext();
        } while (onext != e);
    }


#if 0
    void testQuery(std::string& wktInput, double tolerance, const Envelope& queryEnv, std::string& wktExpected, bool includeRepeated) {
        KdTree index(tolerance);
        // Read input and fill tree with it
        auto geo = reader_.read(wktInput);
        std::unique_ptr<CoordinateSequence> coords = geo->getCoordinates();
        for (std::size_t i = 0; i < coords->size(); i++) {
            index.insert(coords->getAt(i));
        }
        // Read expected output into vector of coordinates
        auto geoExpected = reader_.read(wktExpected);
        auto csExpected = geoExpected->getCoordinates();
        std::vector<Coordinate> expectedCoord;
        csExpected->toVector(expectedCoord);
        // Read tree into vector of coordinates
        std::unique_ptr<std::vector<Coordinate>> result = KdTree::toCoordinates(*(index.query(queryEnv)), includeRepeated);

        std::sort(result->begin(), result->end());
        std::sort(expectedCoord.begin(), expectedCoord.end());

        ensure("Result count not equal to expected count", result->size() == expectedCoord.size());
        ensure("Expected result coordinates not found", *result == expectedCoord);
    }
#endif

};

using group = test_group<test_edgegraph_data>;
using object = group::object;

group test_edgegraph_group("geos::edgegraph::EdgeGraph");

//
// testNode
//
template<>
template<>
void object::test<1> ()
{
    std::unique_ptr<EdgeGraph> graph = build("GEOMETRYCOLLECTION(LINESTRING(0 0, 1 0), LINESTRING(0 0, 0 1), LINESTRING(0 0, -1 0))");
    std::vector<Coordinate>coords = { Coordinate(1, 0), Coordinate(0, 1), Coordinate(-1, 0) };

    Coordinate p0(0,0);
    Coordinate p1(1,0);
    checkEdgeRing(*graph, p0, coords);
    checkNodeValid(*graph, p0, p1);
    checkEdge(*graph, p0, p1);
}

//
// testCCWAfterInserts
//
template<>
template<>
void object::test<2> ()
{
    EdgeGraph graph;
    HalfEdge* e1 = addEdge(graph, 50, 39, 35, 42);
    addEdge(graph, 50, 39, 50, 60);
    addEdge(graph, 50, 39, 68, 35);
    checkNodeValid(e1);
}

//
// testCCWAfterInserts2
//
template<>
template<>
void object::test<3> ()
{
    EdgeGraph graph;
    HalfEdge* e1 = addEdge(graph, 50, 200, 0, 200);
    addEdge(graph, 50, 200, 190, 50);
    addEdge(graph, 50, 200, 200, 200);
    checkNodeValid(e1);
}


} // namespace tut

