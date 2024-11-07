/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <unordered_set>

#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/export.h>

namespace geos {
namespace geom {
class CoordinateSequence;
class Geometry;
}
}

namespace geos {
namespace coverage { // geos::coverage

class CoverageBoundarySegmentFinder : public geos::geom::CoordinateSequenceFilter
{
    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using CoordinateSequenceFilter = geos::geom::CoordinateSequenceFilter;
    using Geometry = geos::geom::Geometry;
    using LineSegment = geos::geom::LineSegment;

public:

    CoverageBoundarySegmentFinder(LineSegment::UnorderedSet& segs)
        : m_boundarySegs(segs)
        {};

    bool isGeometryChanged() const override {
        return false;
    }

    bool isDone() const override {
        return false;
    }

    void filter_ro(const CoordinateSequence& seq, std::size_t i) override;


    static LineSegment::UnorderedSet
        findBoundarySegments(const std::vector<const Geometry*>& geoms);

    static bool isBoundarySegment(
        const LineSegment::UnorderedSet& boundarySegs,
        const CoordinateSequence* seq,
        std::size_t i);

private:

    static LineSegment
        createSegment(const CoordinateSequence& seq, std::size_t i);


    LineSegment::UnorderedSet& m_boundarySegs;


}; // CoverageBoundarySegmentFinder



} // geos::coverage
} // geos
