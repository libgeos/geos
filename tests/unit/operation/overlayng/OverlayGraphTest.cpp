//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/operation/overlayng/OverlayGraph.h>
#include <geos/operation/overlayng/Edge.h>
#include <geos/operation/overlayng/EdgeSourceInfo.h>
#include <geos/operation/overlayng/OverlayEdge.h>




using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlaygraph_data {

    WKTReader r;
    WKTWriter w;
    std::vector<std::unique_ptr<Edge>> edges;
    std::vector<EdgeSourceInfo> esis;

    void
    checkNodeValid(const OverlayEdge* e)
    {
        ensure("OverlayEdge is null!", e != nullptr);
        bool isNodeValid = e->isEdgesSorted();
        ensure("Found non-sorted edges around node.", isNodeValid);
    }

    void
    resetEdges()
    {
        edges.clear();
        esis.clear();
    }

    void
    addEdge(const char *wktLine)
    {
        std::unique_ptr<Geometry> geom = r.read(std::string(wktLine));
        LineString* line = dynamic_cast<LineString*>(geom.get());
        const CoordinateSequence* cs = line->getCoordinatesRO();
        std::unique_ptr<CoordinateSequence> csCopy(cs->clone());
        esis.emplace_back(0);
        const EdgeSourceInfo* esi = &(esis.back());
        std::unique_ptr<Edge> e(new Edge(std::move(csCopy), esi));
        edges.push_back(std::move(e));
    }


};

typedef test_group<test_overlaygraph_data> group;
typedef group::object object;

group test_overlaygraph_group("geos::overation::overlayng::OverlayGraph");

//
// Test Cases
//

//  testCCWAfterInserts
template<>
template<>
void object::test<1> ()
{
    resetEdges();
    addEdge("LINESTRING(50 39, 35 42, 37 30)");
    addEdge("LINESTRING(50 39, 50 60, 20 60)");
    addEdge("LINESTRING(50 39, 68 35)");
    OverlayGraph graph(std::move(edges));
    OverlayEdge* node = graph.getNodeEdge(Coordinate(50, 39));
    checkNodeValid(node);
}

//  Closed loop
template<>
template<>
void object::test<2> ()
{
    resetEdges();
    addEdge("LINESTRING(0 0, 5 -5, 10 0, 5 5, 0 0)");
    OverlayGraph graph(std::move(edges));
    OverlayEdge* node = graph.getNodeEdge(Coordinate(0, 0));
    checkNodeValid(node);
}

//  Diamond Pair
template<>
template<>
void object::test<3> ()
{
    resetEdges();
    addEdge("LINESTRING(0 0, 5 -5, 10 0)");
    addEdge("LINESTRING(0 0, 5 5, 10 0)");
    OverlayGraph graph(std::move(edges));
    OverlayEdge* node = graph.getNodeEdge(Coordinate(0, 0));
    checkNodeValid(node);
}

//  testCCWAfterInserts2
template<>
template<>
void object::test<4> ()
{
    resetEdges();
    addEdge("LINESTRING(50 200, 0 200)");
    addEdge("LINESTRING(50 200, 190 50, 50 50)");
    addEdge("LINESTRING(50 200, 200 200, 100 210, 0 200)");
    OverlayGraph graph(std::move(edges));
    OverlayEdge* node = graph.getNodeEdge(Coordinate(50, 200));
    checkNodeValid(node);
}


} // namespace tut
