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
#include <geos/operation/valid/TopologyValidationError.h>


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
    bool m_hasDoubleTouch = false;
    bool isInvertedRingValid = false;
    int invalidCode = TopologyValidationError::oNoInvalidIntersection;
    Coordinate invalidLocation;
    Coordinate doubleTouchLocation;

    int findInvalidIntersection(
        const SegmentString* ss0, std::size_t segIndex0,
        const SegmentString* ss1, std::size_t segIndex1);

    bool addDoubleTouch(
        const SegmentString* ss0, const SegmentString* ss1,
        const Coordinate& intPt);

    void addSelfTouch(
        const SegmentString* ss, const Coordinate& intPt,
        const Coordinate* e00, const Coordinate* e01,
        const Coordinate* e10, const Coordinate* e11);

    const Coordinate& prevCoordinateInRing(
        const SegmentString* ringSS, std::size_t segIndex) const;

    bool isAdjacentInRing(const SegmentString* ringSS,
        std::size_t segIndex0, std::size_t segIndex1) const;


public:

    /**
    * Creates a new finder, allowing for the mode where inverted rings are valid.
    *
    * @param isInvertedRingValid true if inverted rings are valid.
    */
    PolygonIntersectionAnalyzer(bool p_isInvertedRingValid)
        : isInvertedRingValid(p_isInvertedRingValid)
        , invalidLocation(Coordinate::getNull())
        , doubleTouchLocation(Coordinate::getNull())
        {}

    void processIntersections(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1) override;

    bool isDone() const override {
        return isInvalid() || m_hasDoubleTouch;
    };

    bool isInvalid() const
    {
        return invalidCode >= 0;
    };

    int getInvalidCode() const
    {
        return invalidCode;
    };

    const Coordinate& getInvalidLocation() const
    {
        return invalidLocation;
    };

    bool hasDoubleTouch() const
    {
        return m_hasDoubleTouch;
    };

    const Coordinate& getDoubleTouchLocation() const
    {
        return doubleTouchLocation;
    };

};


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

