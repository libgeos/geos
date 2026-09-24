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

#include <vector>

#include <geos/algorithm/LineIntersector.h> // for composition
#include <geos/geom/Coordinate.h> // for use in vector
#include <geos/geom/PrecisionModel.h> // for inlines (should drop)
#include <geos/noding/SegmentIntersector.h>


// Forward declarations
namespace geos {
namespace geom {
class PrecisionModel;
}
namespace noding {
class SegmentString;
class NodedSegmentString;
namespace snap {
class SnappingPointIndex;
}
}
}

namespace geos {
namespace noding { // geos::noding
namespace snap { // geos::noding::snap

/**
 * Finds intersections between line segments which are being snapped,
 * and adds them as nodes.
 *
 * With a snap tolerance of zero (or less) no snapping occurs:
 * crossing points are added as computed,
 * and a vertex is added as a node of another segment
 * only if it lies exactly on that segment.
 */
class GEOS_DLL SnappingIntersectionAdder: public SegmentIntersector { // implements SegmentIntersector

private:

    algorithm::LineIntersector li;
    double snapTolerance;
    SnappingPointIndex& snapPointIndex;

    /**
    * If an endpoint of one segment is near
    * the <i>interior</i> of the other segment, add it as an intersection.
    * EXCEPT if the endpoint is also close to a segment endpoint
    * (since this can introduce "zigs" in the linework).
    * <p>
    * This resolves situations where
    * a segment A endpoint is extremely close to another segment B,
    * but is not quite crossing.  Due to robustness issues
    * in orientation detection, this can
    * result in the snapped segment A crossing segment B
    * without a node being introduced.
    * <p>
    * This is also how the endpoints of collinear overlaps are noded,
    * at every snap tolerance:
    * the line intersection code adds only single-point intersections,
    * and is not applied to adjacent segments of the same string.
    */
    void processNearVertex(
        SegmentString* srcSS,
        std::size_t srcIndex,
        const geom::Coordinate& p,
        SegmentString* ss,
        std::size_t segIndex,
        const geom::Coordinate& p0,
        const geom::Coordinate& p1);

    /**
    * Tests if a vertex is near the interior of a segment.
    * With a positive snap tolerance the vertex must be closer than
    * the tolerance to the segment, and not that close to either segment endpoint.
    * With a snap tolerance of zero (or less) the vertex must lie
    * exactly on the segment, according to the robust orientation predicate,
    * and not be equal to either segment endpoint.
    *
    * @param p the vertex to test
    * @param p0 an endpoint of the segment
    * @param p1 an endpoint of the segment
    * @return true if the vertex is near the segment interior
    */
    bool isNearSegmentInterior(
        const geom::Coordinate& p,
        const geom::Coordinate& p0,
        const geom::Coordinate& p1) const;

    /**
    * Tests if segments are adjacent on the same SegmentString.
    * Closed segStrings require a check for the point shared by the beginning
    * and end segments.
    */
    static bool isAdjacent(SegmentString* ss0, std::size_t segIndex0, SegmentString* ss1, std::size_t segIndex1);


public:

    /**
    * Creates an intersector which finds intersections, snaps them,
    * and adds them as nodes.
    *
    * @param p_snapTolerance the snapping tolerance distance;
    *        zero (or less) disables snapping
    * @param p_snapPointIndex the snap point index
    */
    SnappingIntersectionAdder(double p_snapTolerance, SnappingPointIndex& p_snapPointIndex);

    /**
    * This method is called by clients
    * of the {@link SegmentIntersector} class to process
    * intersections for two segments of the {@link SegmentString}s being intersected.
    * Note that some clients (such as <code>MonotoneChain</code>s) may optimize away
    * this call for segment pairs which they have determined do not intersect
    * (e.g. by an disjoint envelope test).
    */
    void processIntersections(SegmentString* e0, std::size_t segIndex0, SegmentString* e1, std::size_t segIndex1) override;

    bool isDone() const override { return false; };


};

} // namespace geos::noding::snap
} // namespace geos::noding
} // namespace geos





