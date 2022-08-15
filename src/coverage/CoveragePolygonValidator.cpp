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


using geos::algorithm::locate::IndexedPointInAreaLocator;
using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::LineSegment;
using geos::geom::Location;
using geos::geom::Polygon;
using geos::geom::util::PolygonExtracter;
using geos::noding::MCIndexSegmentSetMutualIntersector;


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
    const Geometry* targetPolygon,
    std::vector<const Geometry*>& adjPolygons)
    : targetGeom(targetPolygon)
    , adjGeoms(adjPolygons)
    , geomFactory(targetPolygon->getFactory())
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

    if (hasDuplicateGeom(targetGeom, adjPolygons)) {
        //TODO: convert to LineString copies
        return targetGeom->getBoundary();
    }

    std::vector<CoverageRing*> targetRings = CoverageRing::createRings(targetGeom, coverageRingStore);
    std::vector<CoverageRing*> adjRings = CoverageRing::createRings(adjPolygons, coverageRingStore);

    /**
    * Mark matching segments as valid first.
    * Valid segments are not considered for further checks.
    * This improves performance substantially for mostly-valid coverages.
    */
    Envelope targetEnv = *(targetGeom->getEnvelopeInternal());
    targetEnv.expandBy(gapWidth);
    markMatchedSegments(targetRings, adjRings, targetEnv);

    //-- check if target is fully matched and thus forms a clean coverage
    if (CoverageRing::isValid(targetRings))
        return createEmptyResult();

    findInvalidInteractingSegments(targetRings, adjRings, gapWidth);

    findInteriorSegments(targetRings, adjPolygons);

    return createInvalidLines(targetRings);
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
bool
CoveragePolygonValidator::hasDuplicateGeom(const Geometry* geom, std::vector<const Polygon*>& adjPolygons) const
{
    for (auto adjPoly : adjPolygons) {
        if (adjPoly->getEnvelopeInternal()->equals(geom->getEnvelopeInternal())) {
            if (adjPoly->equals(geom))
                return true;
        }
    }
    return false;
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
            CoverageRingSegment* seg = createCoverageRingSegment(ring, i);
            //-- skip segments which lie outside the limit envelope
            if (! envLimit.intersects(seg->p0, seg->p1)) {
                continue;
            }
            //-- if segments match, mark them valid
            auto search = segmentMap.find(seg);
            if (search != segmentMap.end()) {
                CoverageRingSegment* segMatch = search->second;
                segMatch->markValid();
                seg->markValid();
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
      coverageRingSegmentStore.emplace_back(p0, p1, ring, index);
      CoverageRingSegment& seg = coverageRingSegmentStore.back();
      return &seg;
}


/* private */
void
CoveragePolygonValidator::findInvalidInteractingSegments(
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
CoveragePolygonValidator::findInteriorSegments(
    std::vector<CoverageRing*>& targetRings,
    std::vector<const Polygon*>& adjPolygons)
{
    for (CoverageRing* ring : targetRings) {
        for (std::size_t i = 0; i < ring->size() - 1; i++) {
            //-- skip check for segments with known state.
            if (ring->isKnown(i))
                continue;

            /**
             * Check if vertex is in interior of an adjacent polygon.
             * If so, the segments on either side are in the interior.
             * Mark them invalid, unless they are already matched.
             */
            const Coordinate& p = ring->getCoordinate(i);
            if (isInteriorVertex(p, adjPolygons)) {
                ring->markInvalid(i);
                //-- previous segment may be interior (but may also be matched)
                std::size_t iPrev = i == 0 ? ring->size()-2 : i-1;
                if (! ring->isKnown(iPrev))
                    ring->markInvalid(iPrev);
            }
        }
    }
}


/* private */
bool
CoveragePolygonValidator::isInteriorVertex(const Coordinate& p,
    std::vector<const Polygon*>& adjPolygons)
{
    /**
     * There should not be too many adjacent polygons,
     * and hopefully not too many segments with unknown status
     * so a linear scan should not be too inefficient
     */
    //TODO: try a spatial index?
    for (std::size_t i = 0; i < adjPolygons.size(); i++) {
        const Polygon* adjPoly = adjPolygons[i];
        if (! adjPoly->getEnvelopeInternal()->intersects(p))
            continue;

        if (polygonContainsPoint(i, adjPoly, p))
            return true;
    }
    return false;
}


/* private */
bool
CoveragePolygonValidator::polygonContainsPoint(std::size_t index,
    const Polygon* poly, const Coordinate& pt)
{
    IndexedPointInAreaLocator* pia = getLocator(index, poly);
    return Location::INTERIOR == pia->locate(&pt);
}


/* private */
IndexedPointInAreaLocator*
CoveragePolygonValidator::getLocator(std::size_t index, const Polygon* poly)
{
    auto it = adjPolygonLocators.find(index);
    // found locator already constructed
    if (it != adjPolygonLocators.end()) {
        return (it->second).get();
    }
    // construct new locator for this polygon
    else {
        IndexedPointInAreaLocator* ipia = new IndexedPointInAreaLocator(*poly);
        adjPolygonLocators.emplace(std::piecewise_construct,
              std::forward_as_tuple(index),
              std::forward_as_tuple(ipia));
        return ipia;
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



} // namespace geos.coverage
} // namespace geos


