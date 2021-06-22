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

#include <geos/noding/SegmentIntersector.h>
#include <geos/algorithm/LineIntersector.h>


#include <geos/export.h>

#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
namespace noding {
class SegmentString;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Coordinate;
using geos::noding::SegmentString;

class GEOS_DLL PolygonIntersectionAnalyzer : public noding::SegmentIntersector {

private:

    algorithm::LineIntersector li;
    std::vector<Coordinate> intersectionPts;
    bool hasProperInt = false;
    bool m_hasIntersection = false;
    bool m_hasDoubleTouch = false;
    bool isInvertedRingValid;

    bool findInvalidIntersection(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1);

    bool addDoubleTouch(SegmentString* ss0, SegmentString* ss1,
        const Coordinate* intPt);

    void addSelfTouch(
        SegmentString* ss, const Coordinate* intPt,
        const Coordinate* e00, const Coordinate* e01,
        const Coordinate* e10, const Coordinate* e11);

    /**
    * For a segment string for a ring, gets the coordinate
    * previous to the given index (wrapping if the index is 0)
    *
    * @param ringSS the ring segment string
    * @param segIndex the segment index
    * @return the coordinate previous to the given segment
    */
    const Coordinate& prevCoordinateInRing(const SegmentString* ringSS, std::size_t segIndex) const;

    /**
    * Tests if two segments in a closed {@link SegmentString} are adjacent.
    * This handles determining adjacency across the start/end of the ring.
    *
    * @param ringSS the segment string
    * @param segIndex0 a segment index
    * @param segIndex1 a segment index
    * @return true if the segments are adjacent
    */
    bool isAdjacentInRing(SegmentString* ringSS, std::size_t segIndex0, std::size_t segIndex1) const;


public:

    PolygonIntersectionAnalyzer(bool p_isInvertedRingValid)
        : isInvertedRingValid(p_isInvertedRingValid)
        {};


    bool isDone() const override
    {
        return m_hasIntersection || m_hasDoubleTouch;
    }

    const Coordinate* getIntersectionLocation() const
    {
        if (intersectionPts.size() == 0) return nullptr;
        return &(intersectionPts[0]);
    }

    bool hasDoubleTouch() const
    {
        return m_hasDoubleTouch;
    }

    bool hasIntersection() const
    {
        return ! intersectionPts.empty();
    }

    /**
    * Creates a new finder, allowing for the mode where inverted rings are valid.
    *
    * @param isInvertedRingValid true if inverted rings are valid.
    */
    void processIntersections(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1) override;
};


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

