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

#include <map>

#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Coordinate.h>
#include <geos/export.h>

namespace geos {
namespace geom {
class CoordinateSequence;
class Geometry;
}
}

namespace geos {
namespace coverage { // geos::coverage

class VertexRingCounter : public geos::geom::CoordinateSequenceFilter
{
    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;

public:

    VertexRingCounter(std::map<Coordinate, std::size_t>& counts)
        : vertexCounts(counts)
        {};

    bool isGeometryChanged() const override {
        return false;
    }

    bool isDone() const override {
        return false;
    }

    void filter_ro(const CoordinateSequence& seq, std::size_t i) override;

    static void count(
        const std::vector<const Geometry*>& geoms,
        std::map<Coordinate, std::size_t>& counts);

private:

    std::map<Coordinate, std::size_t>& vertexCounts;

}; // VertexRingCounter



} // geos::coverage
} // geos
