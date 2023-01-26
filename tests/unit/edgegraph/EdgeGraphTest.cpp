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
        return graph.addEdge(CoordinateXYZM(Coordinate(p0x, p0y)), CoordinateXYZM(Coordinate(p1x, p1y)));
    }

    std::unique_ptr<EdgeGraph> build(std::string wkt) {
        auto geo = reader_.read(wkt);
        auto geocol = dynamic_cast<const GeometryCollection *>(geo.get());
        ensure("could not cast input geometry to collection", geocol);
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
        ensure("checkEdge could not find edge", e);
    }

    void checkEdgeRing(EdgeGraph& graph, Coordinate& p, std::vector<Coordinate>& dest) {
        HalfEdge* e = graph.findEdge(p, dest[0]);
        HalfEdge* onext = e;
        std::size_t i = 0;
        do {
            ensure("checkEdgeRing failed", onext->dest().equals2D(dest[i++]));
            onext = onext->oNext();
        } while (onext != e);
    }

    void checkNextPrev(EdgeGraph& graph) {
        std::vector<const HalfEdge*> edges;
        graph.getVertexEdges(edges);
        for (const auto* e: edges) {
            ensure("checkNextPrev failed", e->next()->prev() == e);
        }
    }

    HalfEdge* findEdge(EdgeGraph& graph, double x1, double y1, double x2, double y2) {
        return graph.findEdge(Coordinate(x1, y1), Coordinate(x2, y2));
    }

    void checkNext(EdgeGraph& graph, double x1, double y1, double x2, double y2, double x3, double y3) {
        HalfEdge* e1 = findEdge(graph, x1, y1, x2, y2);
        HalfEdge* e2 = findEdge(graph, x2, y2, x3, y3);
        ensure("checkNext failed next()", e1->next() == e2);
        ensure("checkNext failed prev()", e2->prev() == e1);
    }

    void checkNextPrev(EdgeGraph& graph, double x1, double y1, double x2, double y2) {
        HalfEdge* e = findEdge(graph, x1, y1, x2, y2);
        ensure("checkNextPrev failed", e->next()->prev() == e);
    }

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

    checkNextPrev(*graph);

    checkNext(*graph, 1, 0, 0, 0, 0, 1);
    checkNext(*graph, 0, 1, 0, 0, -1, 0);
    checkNext(*graph, -1, 0, 0, 0, 1, 0);

    checkNextPrev(*graph, 1, 0, 0, 0);
    checkNextPrev(*graph, 0, 1, 0, 0);
    checkNextPrev(*graph, -1, 0, 0, 0);

    ensure("testNode findEdge failed", findEdge(*graph, 0, 0, 1, 0)->degree() == 3 );
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

//
// testRingGraph
//
template<>
template<>
void object::test<4> ()
{
    std::unique_ptr<EdgeGraph> graph = build("MULTILINESTRING ((10 10, 10 90), (10 90, 90 90), (90 90, 90 10), (90 10, 10 10))");
    HalfEdge* e = findEdge(*graph, 10, 10, 10, 90);
    HalfEdge* eNext = findEdge(*graph, 10, 90, 90, 90);
    ensure(e->next() == eNext);
    ensure(eNext->prev() == e);

    HalfEdge* eSym = findEdge(*graph, 10, 90, 10, 10);
    ensure(e->sym() == eSym);
    ensure(e->orig().equals2D(Coordinate(10, 10)));
    ensure(e->dest().equals2D(Coordinate(10, 90)));

    checkNextPrev(*graph);
}

// testSingleEdgeGraph
template<>
template<>
void object::test<5> ()
{
    std::unique_ptr<EdgeGraph> graph = build("MULTILINESTRING ((10 10, 20 20))");
    checkNextPrev(*graph);
}



} // namespace tut

