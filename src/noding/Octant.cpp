/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/Octant.java rev. 1.2 (JTS-1.7)
 *
 **********************************************************************/

#include <cmath>
#include <sstream>

#include <geos/util/IllegalArgumentException.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>

//
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*public static*/
int
Octant::octant(double dx, double dy)
{
    if(dx == 0.0 && dy == 0.0) {
        std::ostringstream s;
        s << "Cannot compute the octant for point ( " << dx << ", " << dy << " )";
        throw util::IllegalArgumentException(s.str());
    }

    double adx = std::fabs(dx);
    double ady = std::fabs(dy);

    if(dx >= 0) {
        if(dy >= 0) {
            if(adx >= ady) {
                return 0;
            }
            else {
                return 1;
            }
        }
        else { // dy < 0
            if(adx >= ady) {
                return 7;
            }
            else {
                return 6;
            }
        }
    }
    else { // dx < 0
        if(dy >= 0) {
            if(adx >= ady) {
                return 3;
            }
            else {
                return 2;
            }
        }
        else { // dy < 0
            if(adx >= ady) {
                return 4;
            }
            else {
                return 5;
            }
        }
    }

}

/*public static*/
int
Octant::octant(const CoordinateXY& p0, const CoordinateXY& p1)
{
    double dx = p1.x - p0.x;
    double dy = p1.y - p0.y;

    return octant(dx, dy);
}

} // namespace geos.noding
} // namespace geos
