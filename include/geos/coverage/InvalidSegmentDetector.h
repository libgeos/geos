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

#pragma once


#include <geos/noding/SegmentIntersector.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
namespace noding {
class SegmentString;
}
namespace coverage {
class CoverageRing;
}
}


using geos::noding::SegmentIntersector;
using geos::noding::SegmentString;
using geos::geom::Coordinate;


namespace geos {      // geos.
namespace coverage { // geos.coverage

/**
 * Detects invalid coverage topology where ring segments interact.
 * The inputs to processIntersections(SegmentString, int, SegmentString, int)}
 * must be CoverageRing s.
 * If an invalid situation is detected the input target segment is
 * marked invalid using CoverageRing#markInvalid(int).
 *
 * This class assumes it is used with SegmentSetMutualIntersector,
 * so that segments in the same ring are not evaluated.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL InvalidSegmentDetector : public SegmentIntersector {

private:

    // Members
    double distanceTol;

    // Methods
    bool isInvalid(const Coordinate& tgt0, const Coordinate& tgt1,
        const Coordinate& adj0, const Coordinate& adj1,
        CoverageRing* adj, std::size_t indexAdj);

    bool isEqual(
        const Coordinate& t0, const Coordinate& t1,
        const Coordinate& adj0, const Coordinate& adj1);

    /**
    * Checks if the segments are collinear, or if the target segment
    * intersects the interior of the adjacent ring.
    * Segments which are collinear must be non-equal and hence invalid,
    * since matching segments have already been marked as valid and
    * are not passed to this code.
    *
    * @param tgt0
    * @param tgt1
    * @param adj0
    * @param adj1
    * @return
    */
    bool isCollinearOrInterior(
        const Coordinate& tgt0, const Coordinate& tgt1,
        const Coordinate& adj0, const Coordinate& adj1,
        CoverageRing* adj, std::size_t indexAdj);

    bool isInteriorSegment(
        const Coordinate& intVertex,
        const Coordinate& tgt0, const Coordinate& tgt1,
        CoverageRing* adj, std::size_t indexAdj);

    static bool isNearlyParallel(
        const Coordinate& p00, const Coordinate& p01,
        const Coordinate& p10, const Coordinate& p11,
        double distanceTol);


public:

    /**
    * Creates an invalid segment detector.
    */
    InvalidSegmentDetector() {};

    InvalidSegmentDetector(double p_distanceTol)
        : distanceTol(p_distanceTol) {};


    bool isDone() const override {
        // process all intersections
        return false;
    };

    /**
    * Process interacting segments.
    * The input order is important.
    * The adjacent segment is first, the target is second.
    * The inputs must be CoverageRing.
    */
    void processIntersections(
        SegmentString* ssAdj,    std::size_t iAdj,
        SegmentString* ssTarget, std::size_t iTarget) override;



};

} // namespace geos.coverage
} // namespace geos


