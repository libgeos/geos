/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/locate/PointOnGeometryLocator.h> // inherited

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class Coordinate;
class Surface;
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace locate { // geos::algorithm::locate

/** \brief
 * Computes the location of points relative to a polygonal
 * [Geometry](@ref geom::Geometry), using a simple `O(n)` algorithm.
 *
 * The algorithm used reports if a point lies in the interior, exterior,
 * or exactly on the boundary of the Geometry.
 *
 * Instance methods are provided to implement the interface `PointInAreaLocator`.
 * However, they provide no performance advantage over the class methods.
 *
 * This algorithm is suitable for use in cases where only a few points will be tested.
 * If many points will be tested, IndexedPointInAreaLocator may provide better performance.
 */
class GEOS_DLL SimplePointInAreaLocator : public PointOnGeometryLocator {

public:

    static geom::Location locate(const geom::CoordinateXY& p,
                                 const geom::Geometry* geom);

    /** \brief
     * Determines the Location of a point in a [Surface](@ref geom::Surface).
     *
     * The return value is one of:
     *
     * - geom::Location::INTERIOR
     *   if the point is in the geometry interior
     * - geom::Location::BOUNDARY
     *   if the point lies exactly on the boundary
     * - geom::Location::EXTERIOR
     *   if the point is outside the geometry
     *
     * Computes `geom::Location::BOUNDARY` if the point lies exactly
     * on the polygon boundary.
     *
     * @param p the point to test
     * @param poly the geometry to test
     * @return the Location of the point in the polygon
     */
    static geom::Location locatePointInSurface(const geom::CoordinateXY& p,
                                               const geom::Surface& poly);

    /** \brief
     * Determines whether a point is contained in a [Geometry](@ref geom::Geometry),
     * or lies on its boundary.
     *
     * This is a convenience method for
     *
     *      Location::EXTERIOR != locate(p, geom)
     *
     * @param p the point to test
     * @param geom the geometry to test
     * @return true if the point lies in or on the geometry
     */
    static bool isContained(const geom::CoordinateXY& p,
                            const geom::Geometry* geom);

    static bool isAnyPointContained(const geom::Geometry& pt, const geom::Geometry& areaGeom);

    static bool isEveryPointContained(const geom::Geometry& pt, const geom::Geometry& areaGeom);

    SimplePointInAreaLocator(const geom::Geometry* p_g)
        : g(p_g)
    { }

    SimplePointInAreaLocator(const geom::Geometry& p_g)
        : g(&p_g)
    { }

    geom::Location
    locate(const geom::CoordinateXY* p) override
    {
        return locate(*p, g);
    }

private:

    static geom::Location locateInGeometry(const geom::CoordinateXY& p,
                                           const geom::Geometry* geom);

    const geom::Geometry* g;

};

} // geos::algorithm::locate
} // geos::algorithm
} // geos


