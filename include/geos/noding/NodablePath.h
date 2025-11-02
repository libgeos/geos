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
    //virtual void addIntersection( const geom::Coordinate& intPt, int segmentIndex) =0;
    //virtual void addIntersection( const geom::CoordinateXYM& intPt, int segmentIndex) =0;
public:
    virtual void addInt(const geom::CoordinateXY& intPt, size_t pathIndex) {
        addIntersection(geom::CoordinateXYZM{intPt}, pathIndex);
    }

    virtual void addIntersection(geom::CoordinateXYZM intPt, size_t pathIndex) =0;
};

}