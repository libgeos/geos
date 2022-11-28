/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/Edge.h>
#include <geos/operation/overlayng/EdgeSourceInfo.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/io/WKBWriter.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;
using geos::util::GEOSException;

/*public*/
Edge::Edge(std::unique_ptr<CoordinateSequence>&& p_pts, const EdgeSourceInfo* info)
    : aDim(OverlayLabel::DIM_UNKNOWN)
    , aDepthDelta(0)
    , aIsHole(false)
    , bDim(OverlayLabel::DIM_UNKNOWN)
    , bDepthDelta(0)
    , bIsHole(false)
    , pts(std::move(p_pts))
{
    copyInfo(info);
}

/*public static*/
bool
Edge::isCollapsed(const CoordinateSequence* pts) {
    std::size_t sz = pts->size();
    if (sz < 2)
        return true;
    // zero-length line
    if (pts->getAt(0).equals2D(pts->getAt(1)))
        return true;
    // TODO: is pts > 2 with equal points ever expected?
    if (sz > 2) {
        if (pts->getAt(sz-1).equals2D(pts->getAt(sz - 2)))
            return true;
    }
    return false;
}

/**
* Populates the label for an edge resulting from an input geometry.
*  - If the edge is not part of the input, the label is left as NOT_PART
*  - If input is an Area and the edge is on the boundary
* (which may include some collapses),
* edge is marked as an AREA edge and side locations are assigned
*  - If input is an Area and the edge is collapsed
* (depth delta = 0),
* the label is set to COLLAPSE.
* The location will be determined later
* by evaluating the final graph topology.
*  - If input is a Line edge is set to a LINE edge.
* For line edges the line location is not significant
* (since there is no parent area for which to determine location).
*/
/*private*/
void
Edge::initLabel(OverlayLabel& lbl, uint8_t geomIndex, int dim, int depthDelta, bool p_isHole)
{
    int dimLabel = labelDim(dim, depthDelta);

    switch (dimLabel) {
        case OverlayLabel::DIM_NOT_PART: {
            lbl.initNotPart(geomIndex);
            break;
        }
        case OverlayLabel::DIM_BOUNDARY: {
            lbl.initBoundary(geomIndex, locationLeft(depthDelta), locationRight(depthDelta), p_isHole);
            break;
        }
        case OverlayLabel::DIM_COLLAPSE: {
            lbl.initCollapse(geomIndex, p_isHole);
            break;
        }
        case OverlayLabel::DIM_LINE: {
            lbl.initLine(geomIndex);
            break;
        }
    }
}

/*public friend*/
std::ostream&
operator<<(std::ostream& os, const Edge& e)
{
    auto gf = GeometryFactory::create();
    auto cs = e.getCoordinatesRO();
    auto line = gf->createLineString(cs->clone());
    io::WKBWriter w;
    w.writeHEX(*line, os);
    return os;
}

bool EdgeComparator(const Edge* a, const Edge* b)
{
    return a->compareTo(*b);
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


