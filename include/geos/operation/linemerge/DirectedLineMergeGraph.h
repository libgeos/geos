#pragma once

#include <geos/export.h>
#include <geos/planargraph/PlanarGraph.h> // for inheritance

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class LineString;
class Coordinate;
}
namespace planargraph {
class Node;
class NodeMap;
}
}

namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

class GEOS_DLL DirectedLineMergeGraph : public planargraph::PlanarGraph {
private:
    planargraph::Node* getNode(const geom::Coordinate& coordinate);

    std::vector<planargraph::Node*> newNodes;

    std::vector<planargraph::DirectedEdge*> newDirEdges;

public:
    void addEdge(const geom::LineString* lineString);

    ~DirectedLineMergeGraph() override;
};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
