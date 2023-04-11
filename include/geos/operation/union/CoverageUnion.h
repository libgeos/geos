/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/LineSegment.h>
#include <geos/geom/Geometry.h>

#include <memory>

namespace geos {
    namespace geom {
        class Polygon;
        class LineString;
        class LinearRing;
        class GeometryFactory;
    }
}

using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::geom::LineString;
using geos::geom::LinearRing;
using geos::geom::LineSegment;


namespace geos {
namespace operation {
namespace geounion {

    class GEOS_DLL CoverageUnion {
    public:
        static std::unique_ptr<Geometry> Union(const Geometry* geom);

    private:
        CoverageUnion() = default;

        void extractRings(const Polygon* geom);
        void extractRings(const Geometry* geom);
        void extractSegments(const LineString* geom);
        void sortRings();

        std::unique_ptr<Geometry> polygonize(const GeometryFactory* gf);

        std::vector<const LinearRing*> rings;
        // std::unordered_set<geos::geom::LineSegment, geos::geom::LineSegment::HashCode> segments;
        LineSegment::UnorderedSet segments;
        static constexpr double AREA_PCT_DIFF_TOL = 1e-6;
    };

}
}
}
