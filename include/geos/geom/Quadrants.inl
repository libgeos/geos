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
 **********************************************************************
 *
 * Last port: geom/Quadrant.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_GEOM_QUADRANT_INL
#define GEOS_GEOM_QUADRANT_INL

#include <geos/geom/Quadrants.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/IllegalArgumentException.h>

#include <sstream>

namespace geos {
namespace geom {

/* public static */
INLINE Quadrant
Quadrants::quadrant(double dx, double dy)
{
    if(dx == 0.0 && dy == 0.0) {
        std::ostringstream s;
        s << "Cannot compute the quadrant for point ";
        s << "(" << dx << "," << dy << ")" << std::endl;
        throw geos::util::IllegalArgumentException(s.str());
    }
    if(dx >= 0) {
        if(dy >= 0) {
            return Quadrant::NE;
        }
        else {
            return Quadrant::SE;
        }
    }
    else {
        if(dy >= 0) {
            return Quadrant::NW;
        }
        else {
            return Quadrant::SW;
        }
    }
}

/* public static */
INLINE Quadrant
Quadrants::quadrant(const geom::Coordinate& p0, const geom::Coordinate& p1)
{
    if(p1.x == p0.x && p1.y == p0.y) {
        throw geos::util::IllegalArgumentException("Cannot compute the quadrant for two identical points " + p0.toString());
    }

    if(p1.x >= p0.x) {
        if(p1.y >= p0.y) {
            return Quadrant::NE;
        }
        else {
            return Quadrant::SE;
        }
    }
    else {
        if(p1.y >= p0.y) {
            return Quadrant::NW;
        }
        else {
            return Quadrant::SW;
        }
    }
}

/* public static */
INLINE bool
Quadrants::isOpposite(Quadrant quad1, Quadrant quad2)
{
    switch(quad1) {
        case Quadrant::NE: return quad2 == Quadrant::SW;
        case Quadrant::SE: return quad2 == Quadrant::NW;
        case Quadrant::SW: return quad2 == Quadrant::NE;
        case Quadrant::NW: return quad2 == Quadrant::SE;
        default:
            // never get here
            return false;
    }
}

/* public static */
INLINE bool
Quadrants::isNorthern(Quadrant quad)
{
    return quad == Quadrant::NE || quad == Quadrant::NW;
}

}
}

#endif

