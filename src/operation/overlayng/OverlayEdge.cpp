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

#include <geos/operation/overlayng/MaximalEdgeRing.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/OverlayLabel.h>
#include <geos/operation/overlayng/OverlayEdgeRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>


using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;
using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public*/
std::unique_ptr<CoordinateSequence>
OverlayEdge::getCoordinatesOriented()
{
    if (direction) {
        return pts->clone();
    }
    std::unique_ptr<CoordinateSequence> ptsCopy = pts->clone();
    ptsCopy->reverse();
    return ptsCopy;
}

/**
* Adds the coordinates of this edge to the given list,
* in the direction of the edge.
* Duplicate coordinates are removed
* (which means that this is safe to use for a path
* of connected edges in the topology graph).
*
* @param coords the coordinate list to add to
*/
/*public*/
void
OverlayEdge::addCoordinates(CoordinateSequence* coords) const
{
    bool isFirstEdge = coords->size() > 0;
    if (direction) {
        std::size_t startIndex = 1;
        if (isFirstEdge) {
            startIndex = 0;
        }
        coords->add(*pts, startIndex, pts->size() - 1, false);
    }
    else { // is backward
        int startIndex = (int)(pts->size()) - 2;
        if (isFirstEdge) {
            startIndex = (int)(pts->size()) - 1;
        }
        for (int i = startIndex; i >= 0; i--) {
            coords->add(*pts, static_cast<std::size_t>(i), static_cast<std::size_t>(i), false);
        }
    }
}
/*public friend*/
std::ostream&
operator<<(std::ostream& os, const OverlayEdge& oe)
{
    os << "OE( " << oe.orig();
    if (oe.pts->size() > 2) {
        os << ", " << oe.directionPt();
    }
    os << " .. " << oe.dest() << " ) ";
    oe.label->toString(oe.direction, os);
    os << oe.resultSymbol();
    os << " / Sym: ";
    oe.symOE()->getLabel()->toString(oe.symOE()->direction, os);
    os << oe.symOE()->resultSymbol();
    return os;
}

/*public*/
std::string
OverlayEdge::resultSymbol() const
{
    if (isInResultArea()) return std::string(" resA");
    if (isInResultLine()) return std::string(" resL");
    return std::string("");
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


