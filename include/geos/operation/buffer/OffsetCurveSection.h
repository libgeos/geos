/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2021 Martin Davis
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class GeometryFactory;
class LineString;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/**
 * Models a section of a raw offset curve,
 * starting at a given location along the raw curve.
 * The location is a decimal number, with the integer part
 * containing the segment index and the fractional part
 * giving the fractional distance along the segment.
 * The location of the last section segment
 * is also kept, to allow optimizing joining sections together.
 *
 * @author mdavis
 */
class GEOS_DLL OffsetCurveSection {

private:

    std::unique_ptr<CoordinateSequence> sectionPts;
    double location;
    double locLast;

    bool isEndInSameSegment(double nextLoc) const;


public:

    OffsetCurveSection(std::unique_ptr<CoordinateSequence> && secPts, double pLoc, double pLocLast)
        : sectionPts(std::move(secPts))
        , location(pLoc)
        , locLast(pLocLast)
        {};

    const CoordinateSequence* getCoordinates() const;
    std::unique_ptr<CoordinateSequence> releaseCoordinates();

    double getLocation() const { return location; };

    /**
    * Joins section coordinates into a LineString.
    * Join vertices which lie in the same raw curve segment
    * are removed, to simplify the result linework.
    *
    * @param sections the sections to join
    * @param geomFactory the geometry factory to use
    * @return the simplified linestring for the joined sections
    */
    static std::unique_ptr<Geometry> toLine(
        std::vector<std::unique_ptr<OffsetCurveSection>>& sections,
        const GeometryFactory* geomFactory);

    static std::unique_ptr<Geometry> toGeometry(
        std::vector<std::unique_ptr<OffsetCurveSection>>& sections,
        const GeometryFactory* geomFactory);

    static std::unique_ptr<OffsetCurveSection> create(
        const CoordinateSequence* srcPts,
        std::size_t start, std::size_t end,
        double loc, double locLast);

    static bool OffsetCurveSectionComparator(
        const std::unique_ptr<OffsetCurveSection>& a,
        const std::unique_ptr<OffsetCurveSection>& b);

};


} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

