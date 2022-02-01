/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/buffer/OffsetCurve.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>
#include <geos/operation/buffer/SegmentMCIndex.h>
#include <geos/operation/buffer/BufferOp.h>

#include <geos/algorithm/Distance.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/GeometryMapper.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>

#include <cassert>

using namespace geos::index::chain;
using namespace geos::geom;
using geos::geom::util::GeometryMapper;

namespace geos {
namespace operation {
namespace buffer {

/* public static */
std::unique_ptr<Geometry>
OffsetCurve::getCurve(const Geometry& geom, double distance)
{
    OffsetCurve oc(geom, distance);
    return oc.getCurve();
}


/* public static */
std::unique_ptr<Geometry>
OffsetCurve::getCurve(const Geometry& geom,
    double dist,
    int quadSegs,
    BufferParameters::JoinStyle joinStyle,
    double mitreLimit)
{
    BufferParameters bufParms;
    if (quadSegs >= 0) bufParms.setQuadrantSegments(quadSegs);
    if (joinStyle >= 0) bufParms.setJoinStyle(joinStyle);
    if (mitreLimit >= 0) bufParms.setMitreLimit(mitreLimit);
    OffsetCurve oc(geom, dist, bufParms);
    return oc.getCurve();
}


/* public */
std::unique_ptr<Geometry>
OffsetCurve::getCurve()
{
    GeometryMapper::mapOp GetCurveMapOp = [this](const Geometry& geom)->std::unique_ptr<Geometry> {

        if (geom.getGeometryTypeId() == GEOS_POINT) return nullptr;
        if (geom.getGeometryTypeId() == GEOS_POLYGON) {
            auto boundary = geom.buffer(distance)->getBoundary();

            if (boundary->getGeometryTypeId() == GEOS_LINEARRING) {
                const LinearRing& ring = static_cast<const LinearRing&>(geom);
                auto ringCs = ring.getCoordinatesRO();
                std::unique_ptr<Geometry> ls(geom.getFactory()->createLineString(*ringCs));
                return ls;
            }
            return boundary;
        }
        return computeCurve(static_cast<const LineString&>(geom), distance);
    };

    return GeometryMapper::flatMap(inputGeom, 1, GetCurveMapOp);
}


/* public static */
void
OffsetCurve::rawOffset(const LineString& geom, double dist,
    std::vector<CoordinateSequence*>& lineList)
{
    BufferParameters bp;
    rawOffset(geom, dist, bp, lineList);
    return;
}


/* public static */
void
OffsetCurve::rawOffset(const LineString& geom, double distance, BufferParameters& bufParams,
    std::vector<CoordinateSequence*>& lineList)
{
    OffsetCurveBuilder ocb(geom.getFactory()->getPrecisionModel(), bufParams);
    ocb.getOffsetCurve(geom.getCoordinatesRO(), distance, lineList);
    return;
}


/* private */
std::unique_ptr<LineString>
OffsetCurve::computeCurve(const LineString& lineGeom, double p_distance)
{
    //-- first handle special/simple cases
    if (lineGeom.getNumPoints() < 2 || lineGeom.getLength() == 0.0) {
        return geomFactory->createLineString();
    }
    if (lineGeom.getNumPoints() == 2) {
        return offsetSegment(lineGeom.getCoordinatesRO(), p_distance);
    }

    std::vector<CoordinateSequence*> rawOffsetLines;
    rawOffset(lineGeom, p_distance, bufferParams, rawOffsetLines);
    if (rawOffsetLines.empty() || rawOffsetLines[0]->size() == 0) {
        for (auto* cs: rawOffsetLines)
            delete cs;
        return geomFactory->createLineString();
    }
    /**
     * Note: If the raw offset curve has no
     * narrow concave angles or self-intersections it could be returned as is.
     * However, this is likely to be a less frequent situation,
     * and testing indicates little performance advantage,
     * so not doing this.
     */

    std::unique_ptr<Polygon> bufferPoly = getBufferOriented(lineGeom, p_distance, bufferParams);

    //-- first try matching shell to raw curve
    const CoordinateSequence* shell = bufferPoly->getExteriorRing()->getCoordinatesRO();
    std::unique_ptr<LineString> offsetCurve = computeCurve(shell, rawOffsetLines);
    if (! offsetCurve->isEmpty() || bufferPoly->getNumInteriorRing() == 0) {
        for (auto* cs: rawOffsetLines)
            delete cs;
        return offsetCurve;
    }

    //-- if shell didn't work, try matching to largest hole
    auto longestHole = extractLongestHole(*bufferPoly);
    const CoordinateSequence* holePts = longestHole ? longestHole->getCoordinatesRO() : nullptr;
    offsetCurve = computeCurve(holePts, rawOffsetLines);
    for (auto* cs: rawOffsetLines)
        delete cs;
    return offsetCurve;
}

 /* private */
std::unique_ptr<LineString>
OffsetCurve::offsetSegment(const CoordinateSequence* pts, double p_distance)
{
    LineSegment ls(pts->getAt(0), pts->getAt(1));
    LineSegment offsetSeg = ls.offset(p_distance);
    std::vector<Coordinate> coords = { offsetSeg.p0, offsetSeg.p1 };
    return geomFactory->createLineString(std::move(coords));
}

/* private static */
std::unique_ptr<Polygon>
OffsetCurve::getBufferOriented(const LineString& geom, double p_distance, BufferParameters& bufParms)
{
    std::unique_ptr<Geometry> buffer = BufferOp::bufferOp(&geom, std::abs(p_distance), bufParms);
    std::unique_ptr<Polygon> bufferPoly = extractMaxAreaPolygon(*buffer);
    //-- for negative distances (Right of input) reverse buffer direction to match offset curve
    if (p_distance < 0) {
        bufferPoly = bufferPoly->reverse();
    }
    return bufferPoly;
}


/* private static */
std::unique_ptr<Polygon>
OffsetCurve::extractMaxAreaPolygon(const Geometry& geom)
{
    const std::size_t numGeometries = geom.getNumGeometries();
    if (numGeometries == 1) {
        const Polygon& poly = static_cast<const Polygon&>(geom);
        return poly.clone();
    }

    assert(numGeometries > 1);
    const Polygon* maxPoly = static_cast<const Polygon*>(geom.getGeometryN(0));
    double maxArea = maxPoly->getArea();
    for (std::size_t i = 1; i < numGeometries; i++) {
        const Polygon* poly = static_cast<const Polygon*>(geom.getGeometryN(i));
        double area = poly->getArea();
        if (area > maxArea) {
            maxPoly = poly;
            maxArea = area;
        }
    }
    return maxPoly->clone();
}

/* private static */
std::unique_ptr<LinearRing>
OffsetCurve::extractLongestHole(const Polygon& poly)
{
    const LinearRing* largestHole = nullptr;
    double maxLen = -1;
    for (std::size_t i = 0; i < poly.getNumInteriorRing(); i++) {
        const LinearRing* hole = poly.getInteriorRingN(i);
        double len = hole->getLength();
        if (len > maxLen) {
            largestHole = hole;
            maxLen = len;
        }
    }
    return largestHole ? largestHole->clone() : nullptr;
}

/* private */
std::unique_ptr<LineString>
OffsetCurve::computeCurve(const CoordinateSequence* bufferPts, std::vector<CoordinateSequence*>& rawOffsetList)
{
    std::vector<bool> isInCurve;
    isInCurve.resize(bufferPts->size() - 1, false);

    SegmentMCIndex segIndex(bufferPts);

    int curveStart = -1;
    CoordinateSequence* cs = rawOffsetList[0];
    for (std::size_t i = 0; i < cs->size() - 1; i++) {
        int index = markMatchingSegments(
                        cs->getAt(i), cs->getAt(i+1),
                        segIndex, bufferPts, isInCurve);
        if (curveStart < 0) {
            curveStart = index;
        }
    }
    std::vector<Coordinate> curvePts;
    extractSection(bufferPts, curveStart, isInCurve, curvePts);
    return geomFactory->createLineString(std::move(curvePts));
}

void
OffsetCurve::MatchCurveSegmentAction::select(const MonotoneChain& mc, std::size_t segIndex)
{
    (void)mc; // Quiet unused variable warning

    /**
    * A curveRingPt segment may match all or only a portion of a single raw segment.
    * There may be multiple curve ring segs that match along the raw segment.
    * The one closest to the segment start is recorded as the offset curve start.
    */
    double frac = subsegmentMatchFrac(bufferPts->getAt(segIndex), bufferPts->getAt(segIndex+1), p0, p1, matchDistance);
    //-- no match
    if (frac < 0) return;

    isInCurve[segIndex] = true;

    //-- record lowest index
    if (minFrac < 0 || frac < minFrac) {
        minFrac = frac;
        minCurveIndex = static_cast<int>(segIndex);
    }
}

/* private */
int
OffsetCurve::markMatchingSegments(
    const Coordinate& p0, const Coordinate& p1,
    SegmentMCIndex& segIndex, const CoordinateSequence* bufferPts,
    std::vector<bool>& isInCurve)
{
    Envelope matchEnv(p0, p1);
    matchEnv.expandBy(matchDistance);
    MatchCurveSegmentAction action(p0, p1, bufferPts, matchDistance, isInCurve);
    segIndex.query(&matchEnv, action);
    return action.getMinCurveIndex();
}


/* private static */
double
OffsetCurve::subsegmentMatchFrac(const Coordinate& p0, const Coordinate& p1,
      const Coordinate& seg0, const Coordinate& seg1, double matchDistance)
{
    if (matchDistance < algorithm::Distance::pointToSegment(p0, seg0, seg1))
        return -1;
    if (matchDistance < algorithm::Distance::pointToSegment(p1, seg0, seg1))
        return -1;
    //-- matched - determine position as fraction
    LineSegment seg(seg0, seg1);
    return seg.segmentFraction(p0);
}


/* private static */
void
OffsetCurve::extractSection(const CoordinateSequence* ring, int iStartIndex,
        std::vector<bool>& isExtracted, std::vector<Coordinate>& extractedPoints)
{
    if (iStartIndex < 0)
        return;

    CoordinateList coordList;
    std::size_t startIndex = static_cast<std::size_t>(iStartIndex);
    std::size_t i = startIndex;
    do {
        coordList.insert(coordList.end(), ring->getAt(i), false);
        if (! isExtracted[i]) {
            break;
        }
        i = next(i, ring->size() - 1);
    } while (i != startIndex);
    //-- handle case where every segment is extracted
    if (isExtracted[i]) {
        coordList.insert(coordList.end(), ring->getAt(i), false);
    }

    //-- if only one point found return empty LineString
    if (coordList.size() == 1)
        return;

    std::copy(coordList.begin(), coordList.end(),
              std::back_inserter(extractedPoints));

    return;
}

/* private static */
std::size_t
OffsetCurve::next(std::size_t i, std::size_t size) {
    i += 1;
    return (i < size) ? i : 0;
}




} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

