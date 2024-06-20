/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoveragePolygonValidator.h>

#include <geos/coverage/InvalidSegmentDetector.h>
#include <geos/coverage/CoveragePolygon.h>

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Location.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/PolygonExtracter.h>
#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/operation/valid/RepeatedPointRemover.h>


using geos::algorithm::locate::IndexedPointInAreaLocator;
using geos::algorithm::Orientation;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::LineSegment;
using geos::geom::Location;
using geos::geom::Polygon;
using geos::geom::util::PolygonExtracter;
using geos::noding::MCIndexSegmentSetMutualIntersector;
using geos::operation::valid::RepeatedPointRemover;

namespace geos {     // geos
namespace coverage { // geos.coverage


/* public static */
std::unique_ptr<Geometry>
CoveragePolygonValidator::validate(const Geometry* targetPolygon, std::vector<const Geometry*>& adjPolygons)
{
    CoveragePolygonValidator v(targetPolygon, adjPolygons);
    return v.validate();
}


/* public static */
std::unique_ptr<Geometry>
CoveragePolygonValidator::validate(const Geometry* targetPolygon, std::vector<const Geometry*>& adjPolygons, double gapWidth)
{
    CoveragePolygonValidator v(targetPolygon, adjPolygons);
    v.setGapWidth(gapWidth);
    return v.validate();
}


/* public */
CoveragePolygonValidator::CoveragePolygonValidator(
    const Geometry* geom,
    std::vector<const Geometry*>& p_adjGeoms)
    : targetGeom(geom)
    , adjGeoms(p_adjGeoms)
    , geomFactory(geom->getFactory())
{}


/* public */
void
CoveragePolygonValidator::setGapWidth(double p_gapWidth)
{
    gapWidth = p_gapWidth;
}


/* public */
std::unique_ptr<Geometry>
CoveragePolygonValidator::validate()
{
    std::vector<const Polygon*> adjPolygons = extractPolygons(adjGeoms);
    m_adjCovPolygons = toCoveragePolygons(adjPolygons);
    std::vector<CoverageRing*> targetRings = createRings(targetGeom);
    std::vector<CoverageRing*> adjRings = createRings(adjPolygons);

    /**
    * Mark matching segments first.
    * Matched segments are not considered for further checks.
    * This improves performance substantially for mostly-valid coverages.
    */
    Envelope targetEnv = *(targetGeom->getEnvelopeInternal());
    targetEnv.expandBy(gapWidth);

    checkTargetRings(targetRings, adjRings, targetEnv);

    return createInvalidLines(targetRings);
}

/* private static */
std::vector<std::unique_ptr<CoveragePolygon>> 
CoveragePolygonValidator::toCoveragePolygons(const std::vector<const Polygon*> polygons) {
    std::vector<std::unique_ptr<CoveragePolygon>> covPolys;
    for (const Polygon* poly : polygons) {
        covPolys.push_back( std::make_unique<CoveragePolygon>(poly) );
    }
    return covPolys;
}

/* private */
void
CoveragePolygonValidator::checkTargetRings(
    std::vector<CoverageRing*>& targetRings,
    std::vector<CoverageRing*>& adjRings,
    const Envelope& targetEnv)
{
    markMatchedSegments(targetRings, adjRings, targetEnv);

    /**
     * Short-circuit if target is fully known (matched or invalid).
     * This often happens in clean coverages,
     * when the target is surrounded by matching polygons.
     * It can also happen in invalid coverages
     * which have polygons which are duplicates,
     * or perfectly overlap other polygons.
     */
    if (CoverageRing::isKnown(targetRings))
        return;

    /**
     * Here target has at least one unmatched segment.
     * Do further checks to see if any of them are are invalid.
     */
    markInvalidInteractingSegments(targetRings, adjRings, gapWidth);
    markInvalidInteriorSegments(targetRings, m_adjCovPolygons);
}

/* private static */
std::vector<const Polygon*>
CoveragePolygonValidator::extractPolygons(std::vector<const Geometry*>& geoms)
{
    std::vector<const Polygon*> polygons;
    for (const Geometry* geom : geoms) {
        PolygonExtracter::getPolygons(*geom, polygons);
    }
    return polygons;
}


/* private */
std::unique_ptr<Geometry>
CoveragePolygonValidator::createEmptyResult()
{
    return geomFactory->createLineString();
}


/* private */
void
CoveragePolygonValidator::markMatchedSegments(
    std::vector<CoverageRing*>& targetRings,
    std::vector<CoverageRing*>& adjRngs,
    const Envelope& targetEnv)
{
    CoverageRingSegmentMap segmentMap;
    markMatchedSegments(targetRings, targetEnv, segmentMap);
    markMatchedSegments(adjRngs, targetEnv, segmentMap);
}


/* private */
void
CoveragePolygonValidator::markMatchedSegments(
    std::vector<CoverageRing*>& rings,
    const Envelope& envLimit,
    CoverageRingSegmentMap& segmentMap)
{
    for (CoverageRing* ring : rings) {
        for (std::size_t i = 0; i < ring->size() - 1; i++) {
            const Coordinate& p0 = ring->getCoordinate(i);
            const Coordinate& p1 = ring->getCoordinate(i + 1);

            //-- skip segments which lie outside the limit envelope
            if (! envLimit.intersects(p0, p1)) {
                continue;
            }
            //-- if segment keys match, mark them as matched (or invalid)
            CoverageRingSegment* seg = createCoverageRingSegment(ring, i);
            auto search = segmentMap.find(seg);
            if (search != segmentMap.end()) {
                CoverageRingSegment* segMatch = search->second;
                seg->match(segMatch);
            }
            else {
                segmentMap[seg] = seg;
            }
        }
    }
}


/* private */
CoveragePolygonValidator::CoverageRingSegment*
CoveragePolygonValidator::createCoverageRingSegment(CoverageRing* ring, std::size_t index)
{
    const Coordinate& p0 = ring->getCoordinate(index);
    const Coordinate& p1 = ring->getCoordinate(index + 1);

    if(ring->isInteriorOnRight()) {
        coverageRingSegmentStore.emplace_back(p0, p1, ring, index);
    }
    else {
        coverageRingSegmentStore.emplace_back(p1, p0, ring, index);
    }
    CoverageRingSegment& seg = coverageRingSegmentStore.back();
    return &seg;
}


/* private */
void
CoveragePolygonValidator::markInvalidInteractingSegments(
    std::vector<CoverageRing*>& targetRings,
    std::vector<CoverageRing*>& adjRings,
    double distanceTolerance)
{
    std::vector<const SegmentString*> targetSS;
    for (auto cr: targetRings) {
        targetSS.push_back(static_cast<SegmentString*>(cr));
    }
    std::vector<const SegmentString*> adjSS;
    for (auto cr: adjRings) {
        adjSS.push_back(static_cast<SegmentString*>(cr));
    }

    InvalidSegmentDetector detector(distanceTolerance);
    MCIndexSegmentSetMutualIntersector segSetMutInt(distanceTolerance);
    segSetMutInt.setBaseSegments(&targetSS);
    segSetMutInt.setSegmentIntersector(&detector);
    segSetMutInt.process(&adjSS);
}


/* private */
void
CoveragePolygonValidator::markInvalidInteriorSegments(
    std::vector<CoverageRing*>& targetRings,
    std::vector<std::unique_ptr<CoveragePolygon>>& adjCovPolygons )
{
    for (CoverageRing* ring : targetRings) {
        std::size_t stride = 1000;  //--  RING_SECTION_STRIDE;
        for (std::size_t i = 0; i < ring->size() - 1; i += stride) {
            std::size_t iEnd = i + stride;
            if (iEnd >= ring->size())
                iEnd = ring->size() - 1;
            markInvalidInteriorSection(*ring, i, iEnd, adjCovPolygons);
        }
    }
}

/* private */
void
CoveragePolygonValidator::markInvalidInteriorSection(
    CoverageRing& ring,
    std::size_t iStart, 
    std::size_t iEnd, 
    std::vector<std::unique_ptr<CoveragePolygon>>& adjCovPolygons )
{
    Envelope sectionEnv = ring.getEnvelope(iStart, iEnd);
    //TODO: is it worth indexing polygons?
    for (auto& adjPoly : adjCovPolygons) {
        if (adjPoly->intersectsEnv(sectionEnv)) {
            //-- test vertices in section
            for (auto i = iStart; i < iEnd; i++) {
                markInvalidInteriorSegment(ring, i, adjPoly.get());
            }
        }
    }
}

/* private */
void 
CoveragePolygonValidator::markInvalidInteriorSegment(
    CoverageRing& ring, std::size_t i, CoveragePolygon* adjPoly)
{
    //-- skip check for segments with known state.
    if (ring.isKnown(i))
        return;

    /**
     * Check if vertex is in interior of an adjacent polygon.
     * If so, the segments on either side are in the interior.
     * Mark them invalid, unless they are already matched.
     */
    const CoordinateXY& p = ring.getCoordinate(i);
    if (adjPoly->contains(p)) {
        ring.markInvalid(i);
        //-- previous segment may be interior (but may also be matched)
        auto iPrev = i == 0 ? ring.size()-2 : i-1;
        if (! ring.isKnown(iPrev))
            ring.markInvalid(iPrev);
    }
}

/* private */
std::unique_ptr<Geometry>
CoveragePolygonValidator::createInvalidLines(std::vector<CoverageRing*>& rings)
{
    std::vector<std::unique_ptr<LineString>> lines;
    for (CoverageRing* ring : rings) {
        ring->createInvalidLines(geomFactory, lines);
    }

    if (lines.size() == 0) {
        return createEmptyResult();
    }
    else if (lines.size() == 1) {
        return lines[0]->clone();
    }

    return geomFactory->createMultiLineString(std::move(lines));
}

/**********************************************************************************/

/* private */
std::vector<CoverageRing*>
CoveragePolygonValidator::createRings(const Geometry* geom)
{
    std::vector<const Polygon*> polygons;
    geom::util::PolygonExtracter::getPolygons(*geom, polygons);
    return createRings(polygons);
}

/* private */
std::vector<CoverageRing*>
CoveragePolygonValidator::createRings(std::vector<const Polygon*>& polygons)
{
    std::vector<CoverageRing*> rings;
    for (const Polygon* poly : polygons) {
        createRings(poly, rings);
    }
    return rings;
}

/* private */
void
CoveragePolygonValidator::createRings(
    const Polygon* poly,
    std::vector<CoverageRing*>& rings)
{
    // Create exterior shell ring
    addRing( poly->getExteriorRing(), true, rings);

    // Create hole rings
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        addRing( poly->getInteriorRingN(i), false, rings);
    }
}

/* private */
void
CoveragePolygonValidator::addRing(
    const LinearRing* ring,
    bool isShell,
    std::vector<CoverageRing*>& rings)
{
    if (ring->isEmpty())
        return;
    rings.push_back(createRing(ring, isShell));
}

/* private */
CoverageRing*
CoveragePolygonValidator::createRing(const LinearRing* ring, bool isShell)
{
    CoordinateSequence* pts = const_cast<CoordinateSequence*>(ring->getCoordinatesRO());
    if (pts->hasRepeatedOrInvalidPoints()) {
        CoordinateSequence* cleanPts = RepeatedPointRemover::removeRepeatedAndInvalidPoints(pts).release();
        localCoordinateSequences.emplace_back(cleanPts);
        pts = cleanPts;
    }
    bool isCCW = Orientation::isCCW(pts);
    bool isInteriorOnRight = isShell ? ! isCCW : isCCW;
    coverageRingStore.emplace_back(pts, isInteriorOnRight);
    CoverageRing& cRing = coverageRingStore.back();
    return &cRing;
}




} // namespace geos.coverage
} // namespace geos
