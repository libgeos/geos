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

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateSequenceFilter;
using geos::geom::Geometry;
using geos::geom::LineSegment;

namespace geos {
namespace coverage { // geos::coverage

class CoverageBoundarySegmentFinder : public CoordinateSequenceFilter
{


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
