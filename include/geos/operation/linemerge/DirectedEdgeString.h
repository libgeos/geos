#pragma once

#include <geos/export.h>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class CoordinateSequence;
class LineString;
}
namespace planargraph {
class DirectedEdge;
}
}

namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

class GEOS_DLL DirectedEdgeString {
private:
    const geom::GeometryFactory* factory;
    std::vector<planargraph::DirectedEdge*> directedEdges;

    geom::CoordinateSequence* getCoordinates();

public:
    DirectedEdgeString(const geom::GeometryFactory* newFactory);

    ~DirectedEdgeString() = default;

    void add(planargraph::DirectedEdge* directedEdge);

    geom::LineString* toLineString();
};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
