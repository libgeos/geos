/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
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

namespace geos::noding {

/// A NodablePath represents a PathString to which coordinates can be added.
class GEOS_DLL NodablePath {
public:
    virtual void addIntersection( const geom::Coordinate& intPt, size_t pathIndex) {
        addIntersection(geom::CoordinateXYZM{intPt}, pathIndex);
    }

    virtual void addIntersection( const geom::CoordinateXYM& intPt, size_t pathIndex) {
        addIntersection(geom::CoordinateXYZM{intPt}, pathIndex);
    }

    virtual void addIntersection(const geom::CoordinateXY& intPt, size_t pathIndex) {
        addIntersection(geom::CoordinateXYZM{intPt}, pathIndex);
    }

    virtual void addIntersection(const geom::CoordinateXYZM& intPt, size_t pathIndex) =0;
};

}