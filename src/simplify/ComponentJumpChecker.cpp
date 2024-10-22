/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2023 Martin Davis <mtnclimb@gmail.com>
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/simplify/ComponentJumpChecker.h>
#include <geos/simplify/TaggedLineString.h>

#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>
#include <geos/util.h>

using geos::algorithm::RayCrossingCounter;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::geom::LineSegment;


namespace geos {
namespace simplify { // geos::simplify

/**
* Checks if a line section jumps a component if flattened.
*
* Assumes start <= end.
*
* @param line the line containing the section being flattened
* @param start start index of the section
* @param end end index of the section
* @param seg the flattening segment
* @return true if the flattened section jumps a component
*/
/*public*/
bool
ComponentJumpChecker::hasJump(
    const TaggedLineString* line,
    std::size_t start, std::size_t end,
    const LineSegment& seg) const
{
    Envelope sectionEnv = computeEnvelope(line, start, end);
    for (TaggedLineString* comp : components) {
      //-- don't test component against itself
        if (comp == line)
            continue;

        const Coordinate& compPt = comp->getComponentPoint();
        if (sectionEnv.intersects(compPt)) {
            if (hasJumpAtComponent(compPt, line, start, end, seg)) {
                return true;
            }
        }
    }
    return false;
}


/**
* Checks if two consecutive segments jumps a component if flattened.
* The segments are assumed to be consecutive.
* (so the seg1.p1 = seg2.p0).
* The flattening segment must be the segment between seg1.p0 and seg2.p1.
*
* @param line the line containing the section being flattened
* @param seg1 the first replaced segment
* @param seg2 the next replaced segment
* @param seg the flattening segment
* @return true if the flattened segment jumps a component
*/
/* public */
bool
ComponentJumpChecker::hasJump(
    const TaggedLineString* line,
    const LineSegment* seg1,
    const LineSegment* seg2,
    const LineSegment& seg) const
{
    Envelope sectionEnv = computeEnvelope(seg1, seg2);
    for (TaggedLineString* comp : components) {
        //-- don't test component against itself
        if (comp == line)
            continue;

        const Coordinate& compPt = comp->getComponentPoint();
        if (sectionEnv.intersects(compPt)) {
            if (hasJumpAtComponent(compPt, seg1, seg2, seg)) {
                return true;
            }
        }
    }
    return false;
}


/*private static*/
bool
ComponentJumpChecker::hasJumpAtComponent(
    const Coordinate& compPt,
    const TaggedLineString* line,
    std::size_t start, std::size_t end,
    const LineSegment& seg)
{
    std::size_t sectionCount = crossingCount(compPt, line, start, end);
    std::size_t segCount = crossingCount(compPt, seg);
    bool hasJump = sectionCount % 2 != segCount % 2;
    return hasJump;
}

/*private static*/
bool
ComponentJumpChecker::hasJumpAtComponent(
    const Coordinate& compPt,
    const LineSegment* seg1, const LineSegment* seg2,
    const LineSegment& seg)
{
    std::size_t sectionCount = crossingCount(compPt, seg1, seg2);
    std::size_t segCount = crossingCount(compPt, seg);
    bool hasJump = sectionCount % 2 != segCount % 2;
    return hasJump;
}

/*private static*/
std::size_t
ComponentJumpChecker::crossingCount(
    const Coordinate& compPt,
    const LineSegment& seg)
{
    RayCrossingCounter rcc(compPt);
    rcc.countSegment(seg.p0,  seg.p1);
    return rcc.getCount();
}

/*private static*/
std::size_t
ComponentJumpChecker::crossingCount(
    const Coordinate& compPt,
    const LineSegment* seg1, const LineSegment* seg2)
{
    RayCrossingCounter rcc(compPt);
    rcc.countSegment(seg1->p0,  seg1->p1);
    rcc.countSegment(seg2->p0,  seg2->p1);
    return rcc.getCount();
}

/*private static*/
std::size_t
ComponentJumpChecker::crossingCount(
    const Coordinate& compPt,
    const TaggedLineString* line,
    std::size_t start, std::size_t end)
{
    RayCrossingCounter rcc(compPt);
    for (std::size_t i = start; i < end; i++) {
        rcc.countSegment(line->getCoordinate(i), line->getCoordinate(i + 1));
    }
    return rcc.getCount();
}

/*private static*/
Envelope
ComponentJumpChecker::computeEnvelope(
    const LineSegment* seg1, const LineSegment* seg2)
{
    Envelope env;
    env.expandToInclude(seg1->p0);
    env.expandToInclude(seg1->p1);
    env.expandToInclude(seg2->p0);
    env.expandToInclude(seg2->p1);
    return env;
}

/*private static*/
Envelope
ComponentJumpChecker::computeEnvelope(
    const TaggedLineString* line,
    std::size_t start, std::size_t end)
{
    Envelope env;
    for (std::size_t i = start; i <= end; i++) {
        env.expandToInclude(line->getCoordinate(i));
    }
    return env;
}



} // namespace geos::simplify
} // namespace geos
