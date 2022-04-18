#pragma once

#include <geos/export.h>
#include <geos/geom/LineString.h>
#include <geos/operation/linemerge/DirectedLineMergeGraph.h> // for composition

#include <memory>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
namespace planargraph {
class Node;
class DirectedEdge;
}
namespace operation {
namespace linemerge {
class DirectedEdgeString;
}
}
}

namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

class GEOS_DLL DirectedLineMerger {
private:
    DirectedLineMergeGraph graph;

    std::vector<std::unique_ptr<geom::LineString>> mergedLineStrings;
    std::vector<DirectedEdgeString*> edgeStrings;

    const geom::GeometryFactory* factory;

    void merge();

    void buildEdgeStringsForObviousStartNodes();
    void buildEdgeStringsForUnprocessedNodes();
    void buildEdgeStringsForUnprocessedEdges();

    void buildEdgeStringsStartingAt(
        planargraph::Node* node,
        bool stopAtMarkedNode);

    DirectedEdgeString* buildEdgeStringStartingWith(
        planargraph::DirectedEdge* start,
        bool stopAtMarkedNode);

public:
    DirectedLineMerger();
    ~DirectedLineMerger();

    void add(std::vector<const geom::Geometry*>* geometries);

    void add(const geom::Geometry* geometry);

    void add(const geom::LineString* lineString);

    std::vector<std::unique_ptr<geom::LineString>> getMergedLineStrings();

    // Declare type as noncopyable
    DirectedLineMerger(const DirectedLineMerger& other) = delete;
    DirectedLineMerger& operator=(const DirectedLineMerger& rhs) = delete;
};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
