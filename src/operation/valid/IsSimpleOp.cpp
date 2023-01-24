/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 * Copyright (C) 2009 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/valid/IsSimpleOp.h>

#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/util/LinearComponentExtracter.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/operation/valid/RepeatedPointRemover.h>

#include <unordered_set>

using namespace geos::algorithm;
using namespace geos::noding;
using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace operation {
namespace valid {


/* public static */
bool
IsSimpleOp::isSimple(const Geometry& geom)
{
    IsSimpleOp op(geom);
    return op.isSimple();
}

/* public static */
CoordinateXY
IsSimpleOp::getNonSimpleLocation(const Geometry& geom)
{
    IsSimpleOp op(geom);
    return op.getNonSimpleLocation();
}

/* public */
void
IsSimpleOp::setFindAllLocations(bool isFindAll)
{
    isFindAllLocations = isFindAll;
}

/* public */
bool
IsSimpleOp::isSimple()
{
    compute();
    return isSimpleResult;
}

/* public */
CoordinateXY
IsSimpleOp::getNonSimpleLocation()
{
    compute();
    if (nonSimplePts.size() == 0) {
        CoordinateXY c;
        c.setNull();
        return c;
    }
    return nonSimplePts[0];
}


/* public */
const std::vector<CoordinateXY>&
IsSimpleOp::getNonSimpleLocations()
{
    compute();
    return nonSimplePts;
}

/* private */
void
IsSimpleOp::compute()
{
    if (computed || nonSimplePts.size() > 0)
        return;
    isSimpleResult = computeSimple(inputGeom);
    computed = true;
}

/* private */
bool
IsSimpleOp::computeSimple(const Geometry& geom)
{
    if (geom.isEmpty()) return true;
    switch(geom.getGeometryTypeId()) {
        case GEOS_MULTIPOINT:
            return isSimpleMultiPoint(dynamic_cast<const MultiPoint&>(geom));
        case GEOS_LINESTRING:
            return isSimpleLinearGeometry(geom);
        case GEOS_MULTILINESTRING:
            return isSimpleLinearGeometry(geom);
        case GEOS_LINEARRING:
            return isSimplePolygonal(geom);
        case GEOS_POLYGON:
            return isSimplePolygonal(geom);
        case GEOS_MULTIPOLYGON:
            return isSimplePolygonal(geom);
        case GEOS_GEOMETRYCOLLECTION:
            return isSimpleGeometryCollection(geom);
        // all other geometry types are simple by definition
        default:
            return true;
    }
}

/* private */
bool
IsSimpleOp::isSimpleMultiPoint(const MultiPoint& mp)
{
    if (mp.isEmpty()) return true;
    bool bIsSimple = true;
    std::unordered_set<CoordinateXY, Coordinate::HashCode> points;

    for (std::size_t i = 0; i < mp.getNumGeometries(); i++) {
        const Point* pt = mp.getGeometryN(i);
        const CoordinateXY* p = pt->getCoordinate();
        if (points.find(*p) != points.end()) {
            nonSimplePts.push_back(*p);
            bIsSimple = false;
            if (!isFindAllLocations)
                break;
        }
        else {
            points.emplace(*p);
        }
    }
    return bIsSimple;
}


/* private */
bool
IsSimpleOp::isSimplePolygonal(const Geometry& geom)
{
    bool bIsSimple = true;
    std::vector<const LineString*> rings;
    LinearComponentExtracter::getLines(geom, rings);

    for (const LineString* ring : rings) {
        if (! isSimpleLinearGeometry(*ring)) {
            bIsSimple = false;
            if (!isFindAllLocations)
                break;
        }
    }
    return bIsSimple;
}


/* private */
bool
IsSimpleOp::isSimpleGeometryCollection(const Geometry& geom)
{
    bool bIsSimple = true;
    for (std::size_t i = 0; i < geom.getNumGeometries(); i++) {
        const Geometry* comp = geom.getGeometryN(i);
        if (! computeSimple(*comp)) {
            bIsSimple = false;
            if (!isFindAllLocations)
                break;
        }
    }
    return bIsSimple;
}

/* private */
bool
IsSimpleOp::isSimpleLinearGeometry(const Geometry& geom)
{
    if (geom.isEmpty())
        return true;

    std::vector<SegmentString*> segStringsBare;
    auto noRepeatedPtSeqs = removeRepeatedPts(geom);
    auto segStrings = createSegmentStrings(noRepeatedPtSeqs);
    for (auto& ss: segStrings) {
        segStringsBare.push_back(ss.get());
    }
    NonSimpleIntersectionFinder segInt(isClosedEndpointsInInterior, isFindAllLocations, nonSimplePts);
    MCIndexNoder noder;
    noder.setSegmentIntersector(&segInt);
    noder.computeNodes(&segStringsBare);
    if (segInt.hasIntersection()) {
        return false;
    }
    return true;
}

/* private static */
std::vector<std::unique_ptr<CoordinateSequence>>
IsSimpleOp::removeRepeatedPts(const Geometry& geom)
{
    std::vector<std::unique_ptr<CoordinateSequence>> coordseqs;
    for (std::size_t i = 0, sz = geom.getNumGeometries(); i < sz; i++) {
        const LineString* line = dynamic_cast<const LineString*>(geom.getGeometryN(i));
        if (line) {
            auto ptsNoRepeat = RepeatedPointRemover::removeRepeatedPoints(line->getCoordinatesRO());
            coordseqs.emplace_back(ptsNoRepeat.release());
        }
    }
    return coordseqs;
}

/* private static */
std::vector<std::unique_ptr<SegmentString>>
IsSimpleOp::createSegmentStrings(std::vector<std::unique_ptr<CoordinateSequence>>& seqs)
{
    std::vector<std::unique_ptr<SegmentString>> segStrings;
    for (auto& seq : seqs) {
        BasicSegmentString* bss = new BasicSegmentString(
            seq.get(),
            nullptr);
        segStrings.emplace_back(static_cast<SegmentString*>(bss));
    }
    return segStrings;
}

// --------------------------------------------------------------------------------

/* public */
bool
IsSimpleOp::NonSimpleIntersectionFinder::hasIntersection() const
{
    return intersectionPts.size() > 0;
}

/* public */
void
IsSimpleOp::NonSimpleIntersectionFinder::processIntersections(
    SegmentString* ss0, std::size_t segIndex0,
    SegmentString* ss1, std::size_t segIndex1)
{

    // don't test a segment with itself
    bool isSameSegString = ss0 == ss1;
    bool isSameSegment = isSameSegString && segIndex0 == segIndex1;
    if (isSameSegment)
        return;

    const CoordinateXY& p00 = ss0->getCoordinate<CoordinateXY>(segIndex0);
    const CoordinateXY& p01 = ss0->getCoordinate<CoordinateXY>(segIndex0 + 1);
    const CoordinateXY& p10 = ss1->getCoordinate<CoordinateXY>(segIndex1);
    const CoordinateXY& p11 = ss1->getCoordinate<CoordinateXY>(segIndex1 + 1);

    bool hasInt = findIntersection(
        ss0, segIndex0, ss1, segIndex1,
        p00, p01, p10, p11);

    // found an intersection!
    if (hasInt) {
        const CoordinateXY& intPt = li.getIntersection(0);
        // don't save dupes
        for (auto& pt: intersectionPts) {
            if (intPt.equals2D(pt))
                return;
        }
        // do save new ones!
        intersectionPts.emplace_back(intPt);
    }
}

/* private */
bool
IsSimpleOp::NonSimpleIntersectionFinder::findIntersection(
    SegmentString* ss0, std::size_t segIndex0,
    SegmentString* ss1, std::size_t segIndex1,
    const CoordinateXY& p00, const CoordinateXY& p01,
    const CoordinateXY& p10, const CoordinateXY& p11)
{

    li.computeIntersection(p00, p01, p10, p11);
    if (! li.hasIntersection()) return false;

    /**
    * Check for an intersection in the interior of a segment.
    */
    bool hasInteriorInt = li.isInteriorIntersection();
    if (hasInteriorInt) return true;

    /**
    * Check for equal segments (which will produce two intersection points).
    * These also intersect in interior points, so are non-simple.
    * (This is not triggered by zero-length segments, since they
    * are filtered out by the MC index).
    */
    bool hasEqualSegments = li.getIntersectionNum() >= 2;
    if (hasEqualSegments) return true;

    /**
    * Following tests assume non-adjacent segments.
    */
    std::size_t segIndexDiff = segIndex1 > segIndex0
            ? segIndex1 - segIndex0
            : segIndex0 - segIndex1;
    bool isSameSegString = ss0 == ss1;
    bool isAdjacentSegment = isSameSegString && segIndexDiff <= 1;
    if (isAdjacentSegment)
        return false;

    /**
    * At this point there is a single intersection point
    * which is a vertex in each segString.
    * Classify them as endpoints or interior
    */
    bool isIntersectionEndpt0 = isIntersectionEndpoint(ss0, segIndex0, li, 0);
    bool isIntersectionEndpt1 = isIntersectionEndpoint(ss1, segIndex1, li, 1);

    bool hasInteriorVertexInt = ! (isIntersectionEndpt0 && isIntersectionEndpt1);
    if (hasInteriorVertexInt) return true;

    /**
    * Both intersection vertices must be endpoints.
    * Final check is if one or both of them is interior due
    * to being endpoint of a closed ring.
    * This only applies to different lines
    * (which avoids reporting ring endpoints).
    */
    if (isClosedEndpointsInInterior && !isSameSegString) {
        bool hasInteriorEndpointInt = ss0->isClosed() || ss1->isClosed();
        if (hasInteriorEndpointInt) return true;
    }
    return false;
}

/* private */
bool
IsSimpleOp::NonSimpleIntersectionFinder::isIntersectionEndpoint(
    const SegmentString* ss, std::size_t ssIndex,
    const LineIntersector& lineInter, std::size_t liSegmentIndex) const
{
    std::size_t vertexIndex = intersectionVertexIndex(lineInter, liSegmentIndex);
    /**
    * If the vertex is the first one of the segment, check if it is the start endpoint.
    * Otherwise check if it is the end endpoint.
    */
    if (vertexIndex == 0) {
        return ssIndex == 0;
    }
    else {
        return ssIndex + 2 == ss->size();
    }
}

/* private */
std::size_t
IsSimpleOp::NonSimpleIntersectionFinder::intersectionVertexIndex(
    const LineIntersector& lineInter,
    std::size_t segmentIndex) const
{
    const CoordinateXY& intPt = lineInter.getIntersection(0);
    const CoordinateXY* endPt0 = lineInter.getEndpoint(segmentIndex, 0);
    return intPt.equals2D(*endPt0) ? 0 : 1;
}

/* public */
bool
IsSimpleOp::NonSimpleIntersectionFinder::isDone() const
{
    if (isFindAll)
        return false;
    return intersectionPts.size() > 0;
}

// --------------------------------------------------------------------------------


} // geos.operation.valid
} // geos.operation
} // geos
