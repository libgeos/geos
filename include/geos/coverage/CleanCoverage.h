/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/noding/BasicSegmentString.h>

#include <deque>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class GeometryFactory;
class LineString;
class LinearRing;
class Polygon;
}
}

namespace geos {     // geos.
namespace coverage { // geos.coverage

class GEOS_DLL CleanCoverage {

    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using Polygon = geos::geom::Polygon;
    using LineString = geos::geom::LineString;
    using LinearRing = geos::geom::LinearRing;

private:

    // Members





public:

    CleanCoverage();


    class CleanArea {
    public:
    private:

    }

    class MergeStrategy {
    public:
    private:

    }

    class AreaMergeStrategy : public MergeStrategy {
    public:
    private:

    }

    class IndexMergeStrategy : public MergeStrategy {
    public:
    private:

    }

};

} // namespace geos.coverage
} // namespace geos





