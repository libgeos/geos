/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://libgeos.org
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

#pragma once

#include <geos/export.h>
#include <vector>
#include <memory>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class LineSegment;
}
namespace simplify {
class TaggedLineString;
}
}

using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::LineSegment;

namespace geos {
namespace simplify { // geos::simplify


class GEOS_DLL ComponentJumpChecker {

private:

    const std::vector<TaggedLineString*>& components;

    static bool hasJumpAtComponent(
        const Coordinate& compPt,
        const TaggedLineString* line,
        std::size_t start, std::size_t end,
        const LineSegment& seg);

    static bool hasJumpAtComponent(
        const Coordinate& compPt,
        const LineSegment* seg1, const LineSegment* seg2,
        const LineSegment& seg);

    static std::size_t crossingCount(
        const Coordinate& compPt,
        const LineSegment& seg);

    static std::size_t crossingCount(
        const Coordinate& compPt,
        const LineSegment* seg1, const LineSegment* seg2);

    std::size_t static  crossingCount(
        const Coordinate& compPt,
        const TaggedLineString* line,
        std::size_t start, std::size_t end);

    static Envelope computeEnvelope(
        const LineSegment* seg1, const LineSegment* seg2);

    static Envelope computeEnvelope(
        const TaggedLineString* line,
        std::size_t start, std::size_t end);


public:

    ComponentJumpChecker(const std::vector<TaggedLineString*>& taggedLines)
        : components(taggedLines)
    {}

    bool hasJump(
        const TaggedLineString* line,
        std::size_t start, std::size_t end,
        const LineSegment& seg) const;

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
    bool hasJump(
        const TaggedLineString* line,
        const LineSegment* seg1,
        const LineSegment* seg2,
        const LineSegment& seg) const;

};

} // namespace geos::simplify
} // namespace geos





