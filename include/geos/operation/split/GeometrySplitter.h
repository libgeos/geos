/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences LLC
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

namespace geos::geom {
class Curve;
class Geometry;
class Point;
}

namespace geos::operation::split {

class GEOS_DLL GeometrySplitter {

public:
    /** Split a geometry.
     *
     * The geometry to be split must be a (Multi)LineString or (Multi)Polygon.
     *
     * The split geometry may contain any number of (Multi)Point, (Multi)LineString, or
     * (Multi)Polygon elements.
     *
     * @param geom geometry to be split
     * @param splitGeom "blade" and/or points defining where the geometry will be split
     * @return a GeometryCollection containing the split parts
     */
    static std::unique_ptr<geom::Geometry>
    split(const geom::Geometry& geom, const geom::Geometry& splitGeom);

private:

    static std::unique_ptr<geom::Geometry>
    splitLinealWithEdge(const geom::Geometry& geom, const geom::Geometry& edge);

    static std::unique_ptr<geom::Geometry>
    splitPolygonalWithEdge(const geom::Geometry& geom, const geom::Geometry& edge);

    static std::unique_ptr<geom::Geometry>
    splitCurveWithPoint(const geom::Curve& g, const geom::Point& point);

    static std::unique_ptr<geom::Geometry>
    splitAtPoints(const geom::Geometry& geom, const geom::Geometry& splitPoints);

    class SplitWithPointTransformer;

    static constexpr double POINT_TO_LINE_TOLERANCE = 1e-10;
};

}
