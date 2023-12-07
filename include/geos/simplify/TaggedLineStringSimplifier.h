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
 **********************************************************************
 *
 * NOTES: This class can be optimized to work with vector<Coordinate*>
 *        rather then with CoordinateSequence
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <cstddef>
#include <vector>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace geom {
class CoordinateSequence;
class Coordinate;
class LineSegment;
}
namespace simplify {
class TaggedLineSegment;
class TaggedLineString;
class LineSegmentIndex;
class ComponentJumpChecker;
}
}

using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;
using geos::geom::LineSegment;


namespace geos {
namespace simplify { // geos::simplify


/** \brief
 * Simplifies a TaggedLineString, preserving topology
 * (in the sense that no new intersections are introduced).
 * Uses the recursive Douglas-Peucker algorithm.
 *
 */
class GEOS_DLL TaggedLineStringSimplifier {

public:

    TaggedLineStringSimplifier(LineSegmentIndex* inputIndex,
                               LineSegmentIndex* outputIndex,
                               const ComponentJumpChecker* jumpChecker);

    /**
     * Simplifies the given {@link TaggedLineString}
     * using the distance tolerance specified.
     *
     * @param line the linestring to simplify
     * @param distanceTolerance simplification tolerance
     */
    void simplify(TaggedLineString* line, double distanceTolerance);


private:

    // externally owned
    LineSegmentIndex* inputIndex;

    // externally owned
    LineSegmentIndex* outputIndex;

    const ComponentJumpChecker* jumpChecker;

    std::unique_ptr<algorithm::LineIntersector> li;

    /// non-const as segments are possibly added to it
    TaggedLineString* line;

    const CoordinateSequence* linePts;

    void simplifySection(std::size_t i, std::size_t j, std::size_t depth, double distanceTolerance);

    void simplifyRingEndpoint(double distanceTolerance);

    static std::size_t findFurthestPoint(
        const CoordinateSequence* pts,
        std::size_t i, std::size_t j,
        double& maxDistance);

    bool isTopologyValid(
        const TaggedLineString* lineIn,
        std::size_t sectionStart, std::size_t sectionEnd,
        const LineSegment& flatSeg);

    bool isTopologyValid(
        const TaggedLineString* lineIn,
        const LineSegment* seg1, const LineSegment* seg2,
        const LineSegment& flatSeg);

    bool hasInputIntersection(const LineSegment& flatSeg);

    bool hasInputIntersection(
        const TaggedLineString* lineIn,
        std::size_t excludeStart, std::size_t excludeEnd,
        const LineSegment& flatSeg);

    bool isCollinear(const Coordinate& pt, const LineSegment& seg) const;

    bool hasOutputIntersection(const LineSegment& flatSeg);

    bool hasInvalidIntersection(
        const LineSegment& seg0,
        const LineSegment& seg1) const;


    std::unique_ptr<TaggedLineSegment> flatten(
        std::size_t start, std::size_t end);

    /** \brief
     * Tests whether a segment is in a section of a TaggedLineString.
     * Sections may wrap around the endpoint of the line, 
     * to support ring endpoint simplification.
     * This is indicated by excludedStart > excludedEnd
     *
     * @param line line to be checked for the presence of `seg`
     * @param excludeStart  the index of the first segment in the excluded section  
     * @param excludeEnd the index of the last segment in the excluded section
     * @param seg segment to look for in `line`
     * @return true if the test segment intersects some segment in the line not in the excluded section
     */
    static bool isInLineSection(
        const TaggedLineString* line,
        const std::size_t excludeStart, const std::size_t excludeEnd,
        const TaggedLineSegment* seg);

    /** \brief
     * Remove the segs in the section of the line
     *
     * @param line
     * @param start
     * @param end
     */
    void remove(const TaggedLineString* line,
                std::size_t start,
                std::size_t end);

};


} // namespace geos::simplify
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
