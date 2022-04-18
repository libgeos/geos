#include <geos/operation/linemerge/DirectedEdgeString.h>
#include <geos/operation/linemerge/LineMergeEdge.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/util.h>

using namespace geos::geom;
using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

DirectedEdgeString::DirectedEdgeString(const GeometryFactory* newFactory):
    factory(newFactory),
    directedEdges()
{
}

void
DirectedEdgeString::add(DirectedEdge* directedEdge)
{
    directedEdges.push_back(directedEdge);
}

LineString*
DirectedEdgeString::toLineString()
{
    return factory->createLineString(getCoordinates());
}

CoordinateSequence*
DirectedEdgeString::getCoordinates()
{
    auto coordinates = detail::make_unique<CoordinateArraySequence>();
    for(std::size_t i = 0, e = directedEdges.size(); i < e; ++i) {
        DirectedEdge* directedEdge = directedEdges[i];
        auto lme = detail::down_cast<LineMergeEdge*>(directedEdge->getEdge());
        coordinates->add(lme->getLine()->getCoordinatesRO(),
                         false,
                         directedEdge->getEdgeDirection());
    }
    return coordinates.release();
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
