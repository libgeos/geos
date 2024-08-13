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

#include <geos/operation/relateng/DimensionLocation.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>


using geos::geom::Location;
using geos::geom::Dimension;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
int
DimensionLocation::locationArea(Location loc)
{
    switch (loc) {
        case Location::INTERIOR: return AREA_INTERIOR;
        case Location::BOUNDARY: return AREA_BOUNDARY;
        default:
            return EXTERIOR;
    }
}


/* public static */
int
DimensionLocation::locationLine(Location loc)
{
    switch (loc) {
        case Location::INTERIOR: return LINE_INTERIOR;
        case Location::BOUNDARY: return LINE_BOUNDARY;
        default:
            return EXTERIOR;
    }
}


/* public static */
int
DimensionLocation::locationPoint(Location loc)
{
    switch (loc) {
        case Location::INTERIOR: return POINT_INTERIOR;
        default:
            return EXTERIOR;
    }
}


/* public static */
Location
DimensionLocation::location(int dimLoc)
{
    switch (dimLoc) {
        case POINT_INTERIOR:
        case LINE_INTERIOR:
        case AREA_INTERIOR:
            return Location::INTERIOR;
        case LINE_BOUNDARY:
        case AREA_BOUNDARY:
            return Location::BOUNDARY;
        default:
            return Location::EXTERIOR;
    }
}


/* public static */
int
DimensionLocation::dimension(int dimLoc)
{
    switch (dimLoc) {
        case POINT_INTERIOR:
            return Dimension::P;
        case LINE_INTERIOR:
        case LINE_BOUNDARY:
            return Dimension::L;
        case AREA_INTERIOR:
        case AREA_BOUNDARY:
            return Dimension::A;
        default:
            return Dimension::False;
    }
}


/* public static */
int
DimensionLocation::dimension(int dimLoc, int exteriorDim)
{
    if (dimLoc == EXTERIOR)
        return exteriorDim;
    else
        return dimension(dimLoc);
}





} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




