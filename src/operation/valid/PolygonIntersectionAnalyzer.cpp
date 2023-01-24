/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/geom/Coordinate.h>
#include <geos/noding/SegmentString.h>
#include <geos/operation/valid/PolygonIntersectionAnalyzer.h>
#include <geos/operation/valid/PolygonRing.h>
#include <geos/operation/valid/TopologyValidationError.h>
#include <geos/util/IllegalStateException.h>

using geos::geom::Coordinate;
using geos::noding::SegmentString;


namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

/* public */
void
PolygonIntersectionAnalyzer::processIntersections(
    SegmentString* ss0, std::size_t segIndex0,
    SegmentString* ss1, std::size_t segIndex1)
{
    // don't test a segment with itself
    bool isSameSegString = ss0 == ss1;
    bool isSameSegment = isSameSegString && segIndex0 == segIndex1;
    if (isSameSegment) return;

    int code = findInvalidIntersection(ss0, segIndex0, ss1, segIndex1);
    /**
     * Ensure that invalidCode is only set once,
     * since the short-circuiting in {@link SegmentIntersector} is not guaranteed
     * to happen immediately.
     */
    if (code != TopologyValidationError::oNoInvalidIntersection) {
        invalidCode = code;
        invalidLocation = li.getIntersection(0);
    }
}

/* private */
int
PolygonIntersectionAnalyzer::findInvalidIntersection(
    const SegmentString* ss0, std::size_t segIndex0,
    const SegmentString* ss1, std::size_t segIndex1)
{
    const CoordinateXY& p00 = ss0->getCoordinate<CoordinateXY>(segIndex0);
    const CoordinateXY& p01 = ss0->getCoordinate<CoordinateXY>(segIndex0 + 1);
    const CoordinateXY& p10 = ss1->getCoordinate<CoordinateXY>(segIndex1);
    const CoordinateXY& p11 = ss1->getCoordinate<CoordinateXY>(segIndex1 + 1);

    li.computeIntersection(p00, p01, p10, p11);

    if (! li.hasIntersection()) {
        return TopologyValidationError::oNoInvalidIntersection;
    }

    bool isSameSegString = (ss0 == ss1);

    /**
     * Check for an intersection in the interior of both segments.
     * Collinear intersections by definition contain an interior intersection.
     * They occur in either a zero-width spike or gore,
     * or adjacent rings.
     */
    if (li.isProper() || li.getIntersectionNum() >= 2) {
        return TopologyValidationError::eSelfIntersection;
    }

    /**
     * Now know there is exactly one intersection,
     * at a vertex of at least one segment.
     */
    Coordinate intPt = li.getIntersection(0);

    /**
     * If segments are adjacent the intersection must be their common endpoint.
     * (since they are not collinear).
     * This is valid.
     */
    bool isAdjacentSegments = isSameSegString && isAdjacentInRing(ss0, segIndex0, segIndex1);
    // Assert: intersection is an endpoint of both segs
    if (isAdjacentSegments) return TopologyValidationError::oNoInvalidIntersection;

    /**
     * Under OGC semantics, rings cannot self-intersect.
     * So the intersection is invalid.
     */
    if (isSameSegString && ! isInvertedRingValid) {
        return TopologyValidationError::eRingSelfIntersection;
    }

    /**
     * Optimization: don't analyze intPts at the endpoint of a segment.
     * This is because they are also start points, so don't need to be
     * evaluated twice.
     * This simplifies following logic, by removing the segment endpoint case.
     */
    if (intPt.equals2D(p01) || intPt.equals2D(p11))
        return TopologyValidationError::oNoInvalidIntersection;

    /**
     * Check topology of a vertex intersection.
     * The ring(s) must not cross.
     */
    const CoordinateXY* e00 = &p00;
    const CoordinateXY* e01 = &p01;
    if (intPt.equals2D(p00)) {
        e00 = &(prevCoordinateInRing(ss0, segIndex0));
        e01 = &p01;
    }
    const CoordinateXY* e10 = &p10;
    const CoordinateXY* e11 = &p11;
    if (intPt.equals2D(p10)) {
        e10 = &(prevCoordinateInRing(ss1, segIndex1));
        e11 = &p11;
    }
    bool hasCrossing = algorithm::PolygonNodeTopology::isCrossing(&intPt, e00, e01, e10, e11);
    if (hasCrossing) {
        return TopologyValidationError::eSelfIntersection;
    }

    /**
     * If allowing inverted rings, record a self-touch to support later checking
     * that it does not disconnect the interior.
     */
    if (isSameSegString && isInvertedRingValid) {
        addSelfTouch(ss0, intPt, e00, e01, e10, e11);
    }

    /**
     * If the rings are in the same polygon
     * then record the touch to support connected interior checking.
     *
     * Also check for an invalid double-touch situation,
     * if the rings are different.
     */
    bool isDoubleTouch = addDoubleTouch(ss0, ss1, intPt);
    if (isDoubleTouch && ! isSameSegString) {
        m_hasDoubleTouch = true;
        doubleTouchLocation = intPt;
        // TODO: for poly-hole or hole-hole touch, check if it has bad topology.  If so return invalid code
    }

    return TopologyValidationError::oNoInvalidIntersection;
}


/* private */
bool
PolygonIntersectionAnalyzer::addDoubleTouch(
    const SegmentString* ss0, const SegmentString* ss1,
    const CoordinateXY& intPt)
{
    return PolygonRing::addTouch(
        const_cast<PolygonRing*>(static_cast<const PolygonRing*>(ss0->getData())),
        const_cast<PolygonRing*>(static_cast<const PolygonRing*>(ss1->getData())),
        intPt);
}

/* private */
void
PolygonIntersectionAnalyzer::addSelfTouch(
    const SegmentString* ss, const CoordinateXY& intPt,
    const CoordinateXY* e00, const CoordinateXY* e01,
    const CoordinateXY* e10, const CoordinateXY* e11)
{
    const PolygonRing* constPolyRing = static_cast<const PolygonRing*>(ss->getData());
    PolygonRing* polyRing = const_cast<PolygonRing*>(constPolyRing);
    if (polyRing == nullptr) {
        throw util::IllegalStateException("SegmentString missing PolygonRing data when checking self-touches");
    }
    polyRing->addSelfTouch(intPt, e00, e01, e10, e11);
}

/* private */
const CoordinateXY&
PolygonIntersectionAnalyzer::prevCoordinateInRing(
    const SegmentString* ringSS, std::size_t segIndex) const
{
    std::size_t prevIndex;
    if (segIndex == 0) {
        prevIndex = ringSS->size() - 2;
    }
    else {
        prevIndex = segIndex - 1;
    }
    return ringSS->getCoordinate<CoordinateXY>(prevIndex);
}

/* private */
bool
PolygonIntersectionAnalyzer::isAdjacentInRing(const SegmentString* ringSS,
    std::size_t segIndex0, std::size_t segIndex1) const
{
    std::size_t delta = segIndex0 > segIndex1
                        ? segIndex0 - segIndex1
                        : segIndex1 - segIndex0 ;

    if (delta <= 1) return true;
    /**
     * A string with N vertices has maximum segment index of N-2.
     * If the delta is at least N-2, the segments must be
     * at the start and end of the string and thus adjacent.
     */
    if (delta >= ringSS->size() - 2) return true;
    return false;
}



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
