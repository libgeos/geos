/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/TaggedLineStringSimplifier.java r536 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/simplify/ComponentJumpChecker.h>
#include <geos/simplify/LineSegmentIndex.h>
#include <geos/simplify/TaggedLineStringSimplifier.h>
#include <geos/simplify/TaggedLineString.h>
#include <geos/simplify/TaggedLineSegment.h>
#include <geos/util.h>

#include <algorithm>
#include <cassert>
#include <memory>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using geos::geom::LineSegment;
using geos::geom::Coordinate;
using geos::geom::LineString;
using geos::algorithm::LineIntersector;
using geos::algorithm::Orientation;

namespace geos {
namespace simplify { // geos::simplify

/*public*/
TaggedLineStringSimplifier::TaggedLineStringSimplifier(
    LineSegmentIndex* nInputIndex,
    LineSegmentIndex* nOutputIndex,
    const ComponentJumpChecker* crossChecker)
    : inputIndex(nInputIndex)
    , outputIndex(nOutputIndex)
    , jumpChecker(crossChecker)
    , li(new LineIntersector())
    , line(nullptr)
    , linePts(nullptr)
{}


/*public*/
void
TaggedLineStringSimplifier::simplify(TaggedLineString* nLine, double distanceTolerance)
{
    assert(nLine);
    line = nLine;

    linePts = line->getParentCoordinates();
    assert(linePts);

#if GEOS_DEBUG
    std::cerr << "TaggedLineStringSimplifier[" << this << "] "
              << " TaggedLineString[" << line << "] "
              << " has " << linePts->size() << " coords in input"
              << std::endl;
#endif

    if(linePts->isEmpty()) {
        return;
    }
    simplifySection(0, linePts->size() - 1, 0, distanceTolerance);

    if(line->isRing() && linePts->isRing()) {
        simplifyRingEndpoint(distanceTolerance);
    }
}

/*private*/
void
TaggedLineStringSimplifier::simplifySection(std::size_t i,
        std::size_t j, std::size_t depth, double distanceTolerance)
{
    depth += 1;

#if GEOS_DEBUG
    std::cerr << "TaggedLineStringSimplifier[" << this << "] "
              << " simplifying section " << i << "-" << j
              << std::endl;
#endif

    if((i + 1) == j) {

#if GEOS_DEBUG
        std::cerr << "single segment, no flattening"
                  << std::endl;
#endif
        std::unique_ptr<TaggedLineSegment> newSeg(new
                                             TaggedLineSegment(*(line->getSegment(i))));

        line->addToResult(std::move(newSeg));
        // leave this segment in the input index, for efficiency
        return;
    }

    bool isValidToSimplify = true;

    /*
     * Following logic ensures that there is enough points in the
     * output line.
     * If there is already more points than the minimum, there's
     * nothing to check.
     * Otherwise, if in the worst case there wouldn't be enough points,
     * don't flatten this segment (which avoids the worst case scenario)
     */
    if(line->getResultSize() < line->getMinimumSize()) {
        std::size_t worstCaseSize = depth + 1;
        if(worstCaseSize < line->getMinimumSize()) {
            isValidToSimplify = false;
        }
    }

    double distance;

    // pass distance by ref
    std::size_t furthestPtIndex = findFurthestPoint(linePts, i, j, distance);

#if GEOS_DEBUG
    std::cerr << "furthest point " << furthestPtIndex
              << " at distance " << distance
              << std::endl;
#endif

    // flattening must be less than distanceTolerance
    if(distance > distanceTolerance) {
        isValidToSimplify = false;
    }

    if (isValidToSimplify) {
        // test if flattened section would cause intersection or jump
        LineSegment flatSeg(linePts->getAt(i), linePts->getAt(j));
        isValidToSimplify = isTopologyValid(line, i, j, flatSeg);
    }

    if(isValidToSimplify) {

        std::unique_ptr<TaggedLineSegment> newSeg = flatten(i, j);

#if GEOS_DEBUG
        std::cerr << "isValidToSimplify, adding seg "
                  << newSeg->p0 << ", " << newSeg->p1
                  << " to TaggedLineSegment[" << line << "] result "
                  << std::endl;
#endif

        line->addToResult(std::move(newSeg));
        return;
    }

    simplifySection(i, furthestPtIndex, depth, distanceTolerance);
    simplifySection(furthestPtIndex, j, depth, distanceTolerance);
}

/*private*/
void
TaggedLineStringSimplifier::simplifyRingEndpoint(double distanceTolerance)
{
    if (line->getResultSize() > line->getMinimumSize()) {
        const auto* firstSeg = static_cast<LineSegment*>(line->getResultSegments().front());
        const auto* lastSeg = static_cast<LineSegment*>(line->getResultSegments().back());

        LineSegment simpSeg(lastSeg->p0, firstSeg->p1);
        const Coordinate& endPt = firstSeg->p0;
        if (simpSeg.distance(endPt) <= distanceTolerance &&
            isTopologyValid(line, firstSeg, lastSeg, simpSeg))
        {
            line->removeRingEndpoint();
        }
    }
}

/*private*/
std::unique_ptr<TaggedLineSegment>
TaggedLineStringSimplifier::flatten(std::size_t start, std::size_t end)
{
    // make a new segment for the simplified geometry
    const Coordinate& p0 = linePts->getAt(start);
    const Coordinate& p1 = linePts->getAt(end);
    std::unique_ptr<TaggedLineSegment> newSeg(new TaggedLineSegment(p0, p1));
    // update the indexes
    outputIndex->add(newSeg.get());
    remove(line, start, end);
    return newSeg;
}

/*private*/
bool
TaggedLineStringSimplifier::isTopologyValid(
    const TaggedLineString* lineIn,
    std::size_t sectionStart, std::size_t sectionEnd,
    const LineSegment& flatSeg)
{
    if (hasOutputIntersection(flatSeg))
        return false;
    if (hasInputIntersection(lineIn, sectionStart, sectionEnd, flatSeg))
        return false;
    if (jumpChecker->hasJump(lineIn, sectionStart, sectionEnd, flatSeg))
        return false;
    return true;
}

/*private*/
bool
TaggedLineStringSimplifier::isTopologyValid(
    const TaggedLineString* lineIn,
    const LineSegment* seg1, const LineSegment* seg2,
    const LineSegment& flatSeg)
{
    //-- if segments are already flat, topology is unchanged and so is valid
    //-- (otherwise, output and/or input intersection test would report false positive)
    if (isCollinear(seg1->p0, flatSeg))
        return true;
    if (hasOutputIntersection(flatSeg))
        return false;
    if (hasInputIntersection(flatSeg))
        return false;
    if (jumpChecker->hasJump(lineIn, seg1, seg2, flatSeg))
        return false;
    return true;
}

/*private*/
bool
TaggedLineStringSimplifier::isCollinear(
    const Coordinate& pt,
    const LineSegment& seg) const
{
    return Orientation::COLLINEAR == seg.orientationIndex(pt);
}

/*private*/
bool
TaggedLineStringSimplifier::hasOutputIntersection(
    const LineSegment& flatSeg)
{
    //std::unique_ptr<std::vector<LineSegment*>>
    auto querySegs = outputIndex->query(&flatSeg);

    for(const LineSegment* querySeg : *querySegs) {
        if(hasInvalidIntersection(*querySeg, flatSeg)) {
            return true;
        }
    }

    return false;
}

/*private*/
bool
TaggedLineStringSimplifier::hasInvalidIntersection(
    const LineSegment& seg0,
    const LineSegment& seg1) const
{
    if(seg0.equalsTopo(seg1))
        return true;
    li->computeIntersection(seg0.p0, seg0.p1, seg1.p0, seg1.p1);
    return li->isInteriorIntersection();
}

/*private*/
bool
TaggedLineStringSimplifier::hasInputIntersection(const LineSegment& flatSeg)
{
    return hasInputIntersection(nullptr, 0, 0, flatSeg);
}

/*private*/
bool
TaggedLineStringSimplifier::hasInputIntersection(
    const TaggedLineString* parentLine,
    const std::size_t excludeStart, const std::size_t excludeEnd,
    const LineSegment& flatSeg)
{
    const auto& querySegs = inputIndex->query(&flatSeg);

    for(const LineSegment* ls : *querySegs) {
        const TaggedLineSegment* querySeg = static_cast<const TaggedLineSegment*>(ls);

        if (hasInvalidIntersection(*ls, flatSeg)) {
            /**
             * Ignore the intersection if the intersecting segment is part of the section being collapsed
             * to the candidate segment
             */
            if (parentLine != nullptr &&
                isInLineSection(line, excludeStart, excludeEnd, querySeg)) {
                continue;
            }
            return true;
        }
    }

    return false;
}

/*static private*/
bool
TaggedLineStringSimplifier::isInLineSection(
    const TaggedLineString* lineIn,
    const std::size_t excludeStart, const std::size_t excludeEnd,
    const TaggedLineSegment* seg)
{
    // not in this line
    if(seg->getParent() != lineIn->getParent()) {
        return false;
    }

    std::size_t segIndex = seg->getIndex();
    if (excludeStart <= excludeEnd) {
        //-- section is contiguous
        if (segIndex >= excludeStart && segIndex < excludeEnd)
            return true;
    }
    else {
        //-- section wraps around the end of a ring
        if (segIndex >= excludeStart || segIndex <= excludeEnd)
            return true;
    }
    return false;
}

/*private*/
void
TaggedLineStringSimplifier::remove(const TaggedLineString* p_line,
                                   std::size_t start,
                                   std::size_t end)
{
    assert(end <= p_line->getSegments().size());
    assert(start < end); // I'm not sure this should always be true

    for(std::size_t i = start; i < end; i++) {
        const TaggedLineSegment* seg = p_line->getSegment(i);
        inputIndex->remove(seg);
    }
}

/*private static*/
std::size_t
TaggedLineStringSimplifier::findFurthestPoint(
    const geom::CoordinateSequence* pts,
    std::size_t i, std::size_t j,
    double& maxDistance)
{
    LineSegment seg(pts->getAt(i), pts->getAt(j));
#if GEOS_DEBUG
    std::cerr << __FUNCTION__ << "segment " << seg
              << std::endl;
#endif
    double maxDist = -1.0;
    std::size_t maxIndex = i;
    for(std::size_t k = i + 1; k < j; k++) {
        const Coordinate& midPt = pts->getAt(k);
        double distance = seg.distance(midPt);
#if GEOS_DEBUG
        std::cerr << "dist to " << midPt
                  << ": " << distance
                  << std::endl;
#endif
        if(distance > maxDist) {
#if GEOS_DEBUG
            std::cerr << "this is max"
                      << std::endl;
#endif
            maxDist = distance;
            maxIndex = k;
        }
    }
    maxDistance = maxDist;
    return maxIndex;
}


} // namespace geos::simplify
} // namespace geos
