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

#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/Polygon.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/operation/valid/PolygonIntersectionAnalyzer.h>
#include <geos/operation/valid/PolygonRing.h>
#include <geos/operation/valid/PolygonTopologyAnalyzer.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/util/IllegalArgumentException.h>

using namespace geos::geom;
using geos::noding::SegmentString;

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid


/* public */
PolygonTopologyAnalyzer::PolygonTopologyAnalyzer(const Geometry* geom, bool p_isInvertedRingValid)
    : isInvertedRingValid(p_isInvertedRingValid)
    , segInt(p_isInvertedRingValid)
    , disconnectionPt(Coordinate::getNull())
{
    if (geom->isEmpty()){
        return;
    }
    // Code copied in from analyze()
    std::vector<SegmentString*> segStrings = createSegmentStrings(geom, p_isInvertedRingValid);
    polyRings = getPolygonRings(segStrings);
    // Code copied in from analyzeIntersections()
    noding::MCIndexNoder noder;
    noder.setSegmentIntersector(&segInt);
    noder.computeNodes(&segStrings);
    if (segInt.hasDoubleTouch()) {
        disconnectionPt = segInt.getDoubleTouchLocation();
    }
}


/* public static */
CoordinateXY
PolygonTopologyAnalyzer::findSelfIntersection(const LinearRing* ring)
{
    PolygonTopologyAnalyzer ata(ring, false);
    if (ata.hasInvalidIntersection())
        return ata.getInvalidLocation();
    return Coordinate::getNull();
}

/* public static */
bool
PolygonTopologyAnalyzer::isRingNested(const LinearRing* test,
    const LinearRing* target)
{
    const CoordinateXY& p0 = test->getCoordinatesRO()->getAt<CoordinateXY>(0);
    const CoordinateSequence* targetPts = target->getCoordinatesRO();
    Location loc = algorithm::PointLocation::locateInRing(p0, *targetPts);
    if (loc == Location::EXTERIOR) return false;
    if (loc == Location::INTERIOR) return true;

    /**
     * The start point is on the boundary of the ring.
     * Use the topology at the node to check if the segment
     * is inside or outside the ring.
     */
    const CoordinateXY& p1 = findNonEqualVertex(test, p0);
    return isIncidentSegmentInRing(&p0, &p1, targetPts);
}

/* private static */
const CoordinateXY&
PolygonTopologyAnalyzer::findNonEqualVertex(const LinearRing* ring, const CoordinateXY& p)
{
    std::size_t i = 1;
    const CoordinateSequence& ringPts = *ring->getCoordinatesRO();
    const CoordinateXY* next = &(ringPts.getAt<CoordinateXY>(i));
    while (next->equals2D(p) && i < ring->getNumPoints() - 1) {
        i += 1;
        next = &(ringPts.getAt<CoordinateXY>(i));
    }
    return ringPts.getAt<CoordinateXY>(i);
}

/* private static */
bool
PolygonTopologyAnalyzer::isIncidentSegmentInRing(
    const CoordinateXY* p0, const CoordinateXY* p1,
    const CoordinateSequence* ringPts)
{
    std::size_t index = intersectingSegIndex(ringPts, p0);
    const CoordinateXY* rPrev = &findRingVertexPrev(ringPts, index, *p0);
    const CoordinateXY* rNext = &findRingVertexNext(ringPts, index, *p0);
    /**
    * If ring orientation is not normalized, flip the corner orientation
    */
    bool isInteriorOnRight = ! algorithm::Orientation::isCCW(ringPts);
    if (! isInteriorOnRight) {
        const CoordinateXY* temp = rPrev;
        rPrev = rNext;
        rNext = temp;
    }
    return algorithm::PolygonNodeTopology::isInteriorSegment(p0, rPrev, rNext, p1);
}

/* private static */
const CoordinateXY&
PolygonTopologyAnalyzer::findRingVertexPrev(const CoordinateSequence* ringPts, std::size_t index, const CoordinateXY& node)
{
    std::size_t iPrev = index;
    const CoordinateXY* prev = &(ringPts->getAt<CoordinateXY>(iPrev));
    while (prev->equals2D(node)) {
      iPrev = ringIndexPrev(ringPts, iPrev);
      prev = &(ringPts->getAt<CoordinateXY>(iPrev));
    }
    return ringPts->getAt<CoordinateXY>(iPrev);
}

/* private static */
const CoordinateXY&
PolygonTopologyAnalyzer::findRingVertexNext(const CoordinateSequence* ringPts, std::size_t index, const CoordinateXY& node)
{
    //-- safe, since index is always the start of a ring segment
    std::size_t iNext = index + 1;
    const CoordinateXY* next = &(ringPts->getAt<CoordinateXY>(iNext));
    while (next->equals2D(node)) {
      iNext = ringIndexNext(ringPts, iNext);
      next = &(ringPts->getAt<CoordinateXY>(iNext));
    }
    return ringPts->getAt<CoordinateXY>(iNext);
}

/* private static */
std::size_t
PolygonTopologyAnalyzer::ringIndexPrev(const CoordinateSequence* ringPts, std::size_t index)
{
    if (index == 0) {
        return ringPts->getSize() - 2;
    }
    return index - 1;
}

/* private static */
std::size_t
PolygonTopologyAnalyzer::ringIndexNext(const CoordinateSequence* ringPts, std::size_t index)
{
    if (index >= ringPts->getSize() - 2) {
        return 0;
    }
    return index + 1;
}

/* private static */
std::size_t
PolygonTopologyAnalyzer::intersectingSegIndex(const CoordinateSequence* ringPts,
    const CoordinateXY* pt)
{
    algorithm::LineIntersector li;
    for (std::size_t i = 0; i < ringPts->size() - 1; i++) {
      li.computeIntersection(*pt, ringPts->getAt<CoordinateXY>(i), ringPts->getAt<CoordinateXY>(i+1));
      if (li.hasIntersection()) {
        //-- check if pt is the start point of the next segment
        if (pt->equals2D(ringPts->getAt<CoordinateXY>(i + 1))) {
          return i + 1;
        }
        return i;
      }
    }
    throw util::IllegalArgumentException("Segment vertex does not intersect ring");
}

/* public */
bool
PolygonTopologyAnalyzer::isInteriorDisconnected()
{
    /**
     * May already be set by a double-touching hole
     */
    if (!disconnectionPt.isNull()) {
        return true;
    }
    if (isInvertedRingValid) {
        checkInteriorDisconnectedBySelfTouch();
        if (!disconnectionPt.isNull()) {
            return true;
        }
    }
    checkInteriorDisconnectedByHoleCycle();
    if (!disconnectionPt.isNull()) {
        return true;
    }
    return false;
}


/* public */
void
PolygonTopologyAnalyzer::checkInteriorDisconnectedBySelfTouch()
{
    if (! polyRings.empty()) {
        const CoordinateXY* dPt = PolygonRing::findInteriorSelfNode(polyRings);
        if (dPt)
            disconnectionPt = *dPt;
    }
}


/* public */
void
PolygonTopologyAnalyzer::checkInteriorDisconnectedByHoleCycle()
{
    /**
    * PolyRings will be null for empty, no hole or LinearRing inputs
    */
    if (! polyRings.empty()) {
        const CoordinateXY* dPt = PolygonRing::findHoleCycleLocation(polyRings);
        if (dPt)
            disconnectionPt = *dPt;
    }
}


/* private static */
std::vector<SegmentString*>
PolygonTopologyAnalyzer::createSegmentStrings(const Geometry* geom, bool bIsInvertedRingValid)
{
    std::vector<SegmentString*> segStrings;
    int typeId = geom->getGeometryTypeId();
    if (typeId == GEOS_LINEARRING) {
        const LinearRing* ring = static_cast<const LinearRing*>(geom);
        segStrings.push_back(createSegString(ring, nullptr));
        return segStrings;
    }
    if (! (typeId == GEOS_POLYGON || typeId == GEOS_MULTIPOLYGON)) {
        throw util::IllegalArgumentException("Cannot process non-polygonal input");
    }
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        const Polygon* poly = static_cast<const Polygon*>(geom->getGeometryN(i));
        if (poly->isEmpty()) continue;
        bool hasHoles = poly->getNumInteriorRing() > 0;

        //--- polygons with no holes do not need connected interior analysis
        PolygonRing* shellRing = nullptr;
        if (hasHoles || bIsInvertedRingValid) {
            shellRing = createPolygonRing(poly->getExteriorRing());
        }
        segStrings.push_back(createSegString(poly->getExteriorRing(), shellRing));

        for (std::size_t j = 0 ; j < poly->getNumInteriorRing(); j++) {
            const LinearRing* hole = poly->getInteriorRingN(j);
            if (hole->isEmpty()) continue;
            PolygonRing* holeRing = createPolygonRing(hole, static_cast<int>(j), shellRing);
            segStrings.push_back(createSegString(hole, holeRing));
        }
    }
    return segStrings;
}


/* private */
PolygonRing*
PolygonTopologyAnalyzer::createPolygonRing(const LinearRing* p_ring)
{
    polyRingStore.emplace_back(p_ring);
    return &(polyRingStore.back());
}


/* private */
PolygonRing*
PolygonTopologyAnalyzer::createPolygonRing(const LinearRing* p_ring, int p_index, PolygonRing* p_shell)
{
    polyRingStore.emplace_back(p_ring, p_index, p_shell);
    return &(polyRingStore.back());
}


/* private static */
std::vector<PolygonRing*>
PolygonTopologyAnalyzer::getPolygonRings(const std::vector<SegmentString*>& segStrings)
{
    std::vector<PolygonRing*> polygonRings;
    for (SegmentString* ss : segStrings) {

        PolygonRing* polyRing = const_cast<PolygonRing*>(static_cast<const PolygonRing*>(ss->getData()));
        if (polyRing != nullptr) {
            polygonRings.push_back(polyRing);
        }
    }
    return polygonRings;
}


/* private static */
SegmentString*
PolygonTopologyAnalyzer::createSegString(const LinearRing* ring, const PolygonRing* polyRing)
{
    // Let the input LinearRing retain ownership of the
    // CoordinateSequence, and pass it directly into the BasicSegmentString
    // constructor.
    CoordinateSequence* pts = const_cast<CoordinateSequence*>(ring->getCoordinatesRO());

    // Repeated points must be removed for accurate intersection detection
    // So, in this case we create a de-duped copy of the CoordinateSequence
    // and manage the lifecycle locally. This we pass on to the SegmentString
    if (pts->hasRepeatedPoints()) {
        std::unique_ptr<CoordinateSequence> newPts = RepeatedPointRemover::removeRepeatedPoints(pts);
        pts = newPts.get();
        coordSeqStore.emplace_back(newPts.release());
    }

    // Allocate the BasicSegmentString in the store and return a
    // pointer into the store. This way we don't have to track the
    // individual SegmentStrings, they just go away when the
    // PolygonTopologyAnalyzer deallocates.
    segStringStore.emplace_back(pts, polyRing);
    SegmentString* ss = static_cast<SegmentString*>(&(segStringStore.back()));
    return ss;
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
