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

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

namespace geos {
namespace algorithm { // geos::algorithm

/**
 * \class InteriorPointPoint
 * \brief
 * Computes a point in the interior of an point geometry.
 *
 * Algorithm:
 *
 * Find a point which is closest to the centroid of the geometry.
 */
class GEOS_DLL InteriorPointPoint {
private:

    bool hasInterior;

    geom::CoordinateXY centroid;

    double minDistance;

    geom::Coordinate interiorPoint;

    /**
     * Tests the point(s) defined by a Geometry for the best inside point.
     * If a Geometry is not of dimension 0 it is not tested.
     * @param geom the geometry to add
     */
    void add(const geom::Geometry* geom);

    void add(const geom::CoordinateXY* point);

public:

    InteriorPointPoint(const geom::Geometry* g);

    ~InteriorPointPoint() {}

    bool getInteriorPoint(geom::CoordinateXY& ret) const;

};

} // namespace geos::algorithm
} // namespace geos


