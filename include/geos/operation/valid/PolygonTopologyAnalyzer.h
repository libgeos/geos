/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/geom/Coordinate.h>
#include <geos/operation/valid/PolygonIntersectionAnalyzer.h>
#include <geos/operation/valid/PolygonRing.h>
#include <geos/noding/BasicSegmentString.h>

#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::CoordinateXY;
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::LinearRing;

class GEOS_DLL PolygonTopologyAnalyzer {

private:

    // const Geometry* inputGeom;
    bool isInvertedRingValid = false;
    PolygonIntersectionAnalyzer segInt;
    std::vector<PolygonRing*> polyRings;
    geom::CoordinateXY disconnectionPt;


    // holding area for PolygonRings and SegmentStrings so we
    // can pass around pointers with abandon
    std::deque<PolygonRing> polyRingStore;
    std::deque<noding::BasicSegmentString> segStringStore;
    // when building SegmentStrings we sometimes want
    // to use deduped CoordinateSequences so we will
    // keep the deduped ones here so they get cleaned
    // up when processing is complete
    std::vector<std::unique_ptr<CoordinateSequence>> coordSeqStore;

    PolygonRing* createPolygonRing(const LinearRing* p_ring);
    PolygonRing* createPolygonRing(const LinearRing* p_ring, int p_index, PolygonRing* p_shell);

    static const CoordinateXY&
    findNonEqualVertex(const LinearRing* ring, const CoordinateXY& p);

    /**
     * Tests whether a touching segment is interior to a ring.
     *
     * Preconditions:
     *
     *  * The segment does not cross the ring
     *  * The segment vertex p0 lies on the ring
     *  * The ring is valid
     *
     * This works for both shells and holes, but the caller must know
     * the ring role.
     *
     * @param p0 the first vertex of the segment
     * @param p1 the second vertex of the segment
     * @param ringPts the points of the ring
     * @return true if the segment is inside the ring.
     */
    static bool isIncidentSegmentInRing(const CoordinateXY* p0, const CoordinateXY* p1,
        const CoordinateSequence* ringPts);

    static const CoordinateXY& findRingVertexPrev(const CoordinateSequence* ringPts,
        std::size_t index, const CoordinateXY& node);

    static const CoordinateXY& findRingVertexNext(const CoordinateSequence* ringPts,
        std::size_t index, const CoordinateXY& node);

    static std::size_t ringIndexPrev(const CoordinateSequence* ringPts, std::size_t index);

    static std::size_t ringIndexNext(const CoordinateSequence* ringPts, std::size_t index);

    /**
     * Computes the index of the segment which intersects a given point.
     * @param ringPts the ring points
     * @param pt the intersection point
     * @return the intersection segment index, or -1 if no intersection is found
     */
    static std::size_t intersectingSegIndex(const CoordinateSequence* ringPts, const CoordinateXY* pt);

    std::vector<SegmentString*> createSegmentStrings(const Geometry* geom, bool isInvertedRingValid);

    std::vector<PolygonRing*> getPolygonRings(const std::vector<SegmentString*>& segStrings);

    SegmentString* createSegString(const LinearRing* ring, const PolygonRing* polyRing);

    // Declare type as noncopyable
    PolygonTopologyAnalyzer(const PolygonTopologyAnalyzer& other) = delete;
    PolygonTopologyAnalyzer& operator=(const PolygonTopologyAnalyzer& rhs) = delete;

public:

    /* public */
    PolygonTopologyAnalyzer(const Geometry* geom, bool p_isInvertedRingValid);

    /**
     * Finds a self-intersection (if any) in a LinearRing.
     *
     * @param ring the ring to analyze
     * @return a self-intersection point if one exists, or null
     */
    static CoordinateXY findSelfIntersection(const LinearRing* ring);

    /**
     * Tests whether a ring is nested inside another ring.
     *
     * Preconditions:
     *
     * * The rings do not cross (i.e. the test is wholly inside or outside the target)
     * * The rings may touch at discrete points only
     * * The target ring does not self-cross, but it may self-touch
     *
     * If the test ring start point is properly inside or outside, that provides the result.
     * Otherwise the start point is on the target ring,
     * and the incident start segment (accounting for repeated points) is
     * tested for its topology relative to the target ring.
     *
     * @param test the ring to test
     * @param target the ring to test against
     * @return true if the test ring lies inside the target ring
     */
    static bool
    isRingNested(const LinearRing* test,
        const LinearRing* target);

    bool hasInvalidIntersection() {
        return segInt.isInvalid();
    }

    int getInvalidCode() {
        return segInt.getInvalidCode();
    }

    const CoordinateXY& getInvalidLocation() {
        return segInt.getInvalidLocation();
    }

    /**
    * Tests whether the interior of the polygonal geometry is
    * disconnected.
    * If true, the disconnection location is available from
    * getDisconnectionLocation().
    *
    * @return true if the interior is disconnected
    */
    bool isInteriorDisconnected();

    const CoordinateXY& getDisconnectionLocation() const
    {
        return disconnectionPt;
    };


    /**
    * Tests whether any polygon with holes has a disconnected interior
    * by virtue of the holes (and possibly shell) forming a hole cycle.
    *
    * This is a global check, which relies on determining
    * the touching graph of all holes in a polygon.
    *
    * If inverted rings disconnect the interior
    * via a self-touch, this is checked by the PolygonIntersectionAnalyzer.
    * If inverted rings are part of a hole cycle
    * this is detected here as well.
    */
    void checkInteriorDisconnectedByHoleCycle();

    /**
    * Tests if an area interior is disconnected by a self-touching ring.
    * This must be evaluated after other self-intersections have been analyzed
    * and determined to not exist, since the logic relies on
    * the rings not self-crossing (winding).
    * <p>
    * If self-touching rings are not allowed,
    * then the self-touch will previously trigger a self-intersection error.
    */
    void checkInteriorDisconnectedBySelfTouch();

};


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
