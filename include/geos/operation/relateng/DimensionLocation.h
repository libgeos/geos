/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Location.h>
#include <geos/export.h>

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


class GEOS_DLL DimensionLocation {
    using Location = geos::geom::Location;

public:

    enum DimensionLocationType {
        EXTERIOR = 2,           // == Location.EXTERIOR
        POINT_INTERIOR = 103,
        LINE_INTERIOR = 110,
        LINE_BOUNDARY = 111,
        AREA_INTERIOR = 120,
        AREA_BOUNDARY = 121
    };

    static int locationArea(Location loc);

    static int locationLine(Location loc);

    static int locationPoint(Location loc);

    static Location location(int dimLoc);

    static int dimension(int dimLoc);

    static int dimension(int dimLoc, int exteriorDim);

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

