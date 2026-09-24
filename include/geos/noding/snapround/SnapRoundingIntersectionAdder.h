/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <utility>

#include <geos/algorithm/LineIntersector.h> // for composition
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/SegmentIntersector.h>


// Forward declarations
namespace geos {
namespace geom {
class CoordinateXY;
class PrecisionModel;
}
namespace noding {
class SegmentString;
}
}

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

/**
 * Finds intersections between line segments which will be snap-rounded,
 * and adds them as nodes to the segments.
 *
 * Intersections are detected and computed using full precision.
 * Snapping takes place in a subsequent phase.
 *
 * The intersection points are recorded, so that HotPixels can be created for them.
 *
 * LineIntersector computes a proper intersection in double-double
 * arithmetic and returns the nearest double, which can lie across a
 * half-cell boundary of the snap-rounding grid from the double-double point.
 * The hot pixel and the segment nodes would then be placed in a grid cell
 * that does not contain the intersection.
 * The recorded point is therefore moved by one representable double
 * into the grid cell of the double-double point,
 * if the precision model rounds the moved point into that cell
 * and each moved ordinate stays within the range of both segments.
 * The range condition keeps the rounded intersection within the
 * envelope of each rounded segment. In particular, along a horizontal
 * or vertical segment the intersection keeps the segment's own ordinate.
 * Otherwise the point is recorded as computed.
 *
 * To avoid robustness issues with vertices which lie very close to line segments
 * a heuristic is used:
 * nodes are created if a vertex lies within a tolerance distance
 * of the interior of a segment.
 * The tolerance distance is chosen to be significantly below the snap-rounding grid size.
 * This has empirically proven to eliminate noding failures.
 */
class GEOS_DLL SnapRoundingIntersectionAdder: public SegmentIntersector { // implements SegmentIntersector

private:

    algorithm::LineIntersector li;
    geom::CoordinateSequence intersections;
    const geom::PrecisionModel& pm;
    double nearnessTol;

    /**
    * If an endpoint of one segment is near
    * the interior of the other segment, add it as an intersection.
    * EXCEPT if the endpoint is also close to a segment endpoint
    * (since this can introduce "zigs" in the linework).
    *
    * This resolves situations where
    * a segment A endpoint is extremely close to another segment B,
    * but is not quite crossing.  Due to robustness issues
    * in orientation detection, this can
    * result in the snapped segment A crossing segment B
    * without a node being introduced.
    */
    void processNearVertex(const geom::CoordinateSequence& seq0,
                           std::size_t ptIndex,
                           const geom::CoordinateSequence& seq1,
                           std::size_t segIndex,
                           SegmentString* edge);

    bool isNearSegmentInterior(const geom::CoordinateXY& p, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1) const;

public:

    /**
     * Creates an intersector which finds all snapped interior intersections,
     * and adds them as nodes.
     *
     * @param p_pm the precision model of the snap-rounding grid.
     *        It is referenced, not copied, and must outlive the intersector.
     * @param p_nearnessTol the intersection distance tolerance
     */
    SnapRoundingIntersectionAdder(const geom::PrecisionModel& p_pm, double p_nearnessTol)
        : SegmentIntersector()
        , intersections(geom::CoordinateSequence::XYZM(0))
        , pm(p_pm)
        , nearnessTol(p_nearnessTol)
    {}

    geom::CoordinateSequence getIntersections() { return std::move(intersections); };

    /**
    * This method is called by clients
    * of the {@link SegmentIntersector} class to process
    * intersections for two segments of the {@link SegmentString}s being intersected.
    * Note that some clients (such as MonotoneChains) may optimize away
    * this call for segment pairs which they have determined do not intersect
    * (e.g. by an disjoint envelope test).
    */
    void processIntersections(SegmentString* e0, std::size_t segIndex0, SegmentString* e1, std::size_t segIndex1) override;

    /**
    * Always process all intersections
    *
    */
    bool isDone() const override { return false; }


};

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos
