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

#ifndef GEOS_OP_UNION_COVERAGEUNION_H
#define GEOS_OP_UNION_COVERAGEUNION_H

#include <geos/geom/LineSegment.h>
#include <geos/geom/Geometry.h>

#include <memory>
#include <unordered_set>

namespace geos {
    namespace geom {
        class Polygon;
        class LineString;
        class GeometryFactory;
    }
}

namespace geos {
namespace operation {
namespace geounion {

    class GEOS_DLL CoverageUnion {
    public:
        static std::unique_ptr<geom::Geometry> Union(const geom::Geometry* geom);

    private:
        CoverageUnion() = default;

        void extractSegments(const geom::Polygon* geom);
        void extractSegments(const geom::Geometry* geom);
        void extractSegments(const geom::LineString* geom);

        std::unique_ptr<geom::Geometry> polygonize(const geom::GeometryFactory* gf);

        struct SegmentHash {
            size_t operator()(const geom::LineSegment & s) const {
                // FIXME define a real hash function, and probably move this class elsewhere.
                return 1;
            }
        };

        std::unordered_set<geos::geom::LineSegment, SegmentHash> segments;
    };

}
}
}

#endif