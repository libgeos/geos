/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/linemerge/EdgeString.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/operation/linemerge/EdgeString.h>
#include <geos/operation/linemerge/LineMergeEdge.h>
#include <geos/operation/linemerge/LineMergeDirectedEdge.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>

#include <vector>


using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

/**
 * Constructs an EdgeString with the given factory used to convert
 * this EdgeString to a LineString
 */
EdgeString::EdgeString(const GeometryFactory* newFactory):
    factory(newFactory),
    directedEdges()
{
}

/**
 * Adds a directed edge which is known to form part of this line.
 */
void
EdgeString::add(LineMergeDirectedEdge* directedEdge)
{
    directedEdges.push_back(directedEdge);
}

std::unique_ptr<CoordinateSequence>
EdgeString::getCoordinates() const
{
    int forwardDirectedEdges = 0;
    int reverseDirectedEdges = 0;

    bool resultHasZ = false;
    bool resultHasM = false;

    for (const LineMergeDirectedEdge* directedEdge : directedEdges) {
        const LineMergeEdge* lme = detail::down_cast<LineMergeEdge*>(directedEdge->getEdge());

        resultHasZ |= lme->getLine()->hasZ();
        resultHasM |= lme->getLine()->hasM();
    }

    auto coordinates = std::make_unique<CoordinateSequence>(0, resultHasZ, resultHasM);

    bool lastPointMissingZ = false;
    bool lastPointMissingM = false;

    for (const LineMergeDirectedEdge* directedEdge : directedEdges) {
        if(directedEdge->getEdgeDirection()) {
            forwardDirectedEdges++;
        }
        else {
            reverseDirectedEdges++;
        }

        const LineMergeEdge* lme = detail::down_cast<LineMergeEdge*>(directedEdge->getEdge());
        const CoordinateSequence* seq = lme->getLine()->getCoordinatesRO();

        if (lastPointMissingZ && seq->hasZ()) {
            const double z = directedEdge->getEdgeDirection() ? seq->getZ(0) : seq->getZ(seq->getSize() - 1);
            coordinates->setZ(coordinates->getSize() - 1, z);
        }
        if (lastPointMissingM && seq->hasM()) {
            const double m = directedEdge->getEdgeDirection() ? seq->getM(0) : seq->getM(seq->getSize() - 1);
            coordinates->setM(coordinates->getSize() - 1, m);
        }

        coordinates->add(*seq,
                         false,
                         directedEdge->getEdgeDirection());

        lastPointMissingZ = resultHasZ && !seq->hasZ();
        lastPointMissingM = resultHasM && !seq->hasM();
    }

    if(reverseDirectedEdges > forwardDirectedEdges) {
        coordinates->reverse();
    }

    return coordinates;
}

/*
 * Converts this EdgeString into a new LineString.
 */
std::unique_ptr<LineString>
EdgeString::toLineString() const
{
    return factory->createLineString(getCoordinates());
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
