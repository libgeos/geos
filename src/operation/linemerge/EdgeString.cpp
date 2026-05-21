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
#include <geos/geom/util/CurveBuilder.h>
#include <geos/util.h>

#include <cmath>
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

static void
addCoordinates(geom::util::CurveBuilder& curveBuilder, const SimpleCurve* curve, bool forward)
{
    if (curve->isEmpty()) {
        return;
    }

    const CoordinateSequence& srcCoords = *curve->getCoordinatesRO();

    // Patch Z value in last coordinate, if needed
    if (curveBuilder.hasActiveSequence() && curve->hasZ()) {
        CoordinateSequence& dstCoords = curveBuilder.getSeq(curveBuilder.isCurved());
        if (std::isnan(dstCoords.getZ(dstCoords.getSize() - 1))) {
            dstCoords.setZ(dstCoords.getSize() - 1, forward ? srcCoords.getZ(0) : srcCoords.getZ(srcCoords.getSize() - 1));
        }
    }

    // Patch M value in last coordinate, if needed
    if (curveBuilder.hasActiveSequence() && curve->hasM()) {
        CoordinateSequence& dstCoords = curveBuilder.getSeq(curveBuilder.isCurved());
        if (std::isnan(dstCoords.getM(dstCoords.getSize() - 1))) {
            dstCoords.setM(dstCoords.getSize() - 1, forward ? srcCoords.getM(0) : srcCoords.getM(srcCoords.getSize() - 1));
        }
    }

    const bool isCurved = curve->getGeometryTypeId() == GEOS_CIRCULARSTRING;
    CoordinateSequence& dstCoords = curveBuilder.getSeq(isCurved);
    dstCoords.add(srcCoords, false, forward);
}

std::unique_ptr<Curve>
EdgeString::getGeometry() const
{
    int forwardDirectedEdges = 0;
    int reverseDirectedEdges = 0;

    bool resultHasZ = false;
    bool resultHasM = false;

    for (const LineMergeDirectedEdge* directedEdge : directedEdges) {
        const LineMergeEdge* lme = detail::down_cast<LineMergeEdge*>(directedEdge->getEdge());

        resultHasZ |= lme->getCurve()->hasZ();
        resultHasM |= lme->getCurve()->hasM();
    }

    geom::util::CurveBuilder curveBuilder(*factory, resultHasZ, resultHasM);
    curveBuilder.setOutputLinearRing(false);

    for (const LineMergeDirectedEdge* directedEdge : directedEdges) {
        const bool isForward = directedEdge->getEdgeDirection();

        if (isForward) {
            forwardDirectedEdges++;
        }
        else {
            reverseDirectedEdges++;
        }

        const LineMergeEdge* lme = detail::down_cast<LineMergeEdge*>(directedEdge->getEdge());
        const Curve* curve = lme->getCurve();

        if (curve->getGeometryTypeId() == geom::GEOS_COMPOUNDCURVE) {
            if (isForward) {
                for (std::size_t i = 0; i < curve->getNumCurves(); i++) {
                    const SimpleCurve* sc = curve->getCurveN(i);
                    addCoordinates(curveBuilder, sc, isForward);
                }
            } else {
                for (std::size_t i = curve->getNumCurves(); i > 0; i--) {
                    const SimpleCurve* sc = curve->getCurveN(i - 1);
                    addCoordinates(curveBuilder, sc, isForward);
                }
            }
        } else {
            const SimpleCurve* sc = detail::down_cast<const SimpleCurve*>(curve);
            addCoordinates(curveBuilder, sc, directedEdge->getEdgeDirection());
        }
    }

    auto result = curveBuilder.getGeometry();

    if(reverseDirectedEdges > forwardDirectedEdges) {
        return result->reverse();
    }

    return result;
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
