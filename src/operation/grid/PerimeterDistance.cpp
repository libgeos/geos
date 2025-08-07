/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <stdexcept>

#include <geos/operation/grid/PerimeterDistance.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

namespace geos::operation::grid {

double
PerimeterDistance::getPerimeterDistance(double xmin, double ymin, double xmax, double ymax, double x, double y)
{
    if (x == xmin) {
        // Left
        return y - ymin;
    }

    if (y == ymax) {
        // Top
        return (ymax - ymin) + x - xmin;
    }

    if (x == xmax) {
        // Right
        return (xmax - xmin) + (ymax - ymin) + ymax - y;
    }

    if (y == ymin) {
        // Bottom
        return (xmax - xmin) + 2 * (ymax - ymin) + (xmax - x);
    }

    throw std::runtime_error("Cannot calculate perimeter distance for point not on boundary.");
}

double
PerimeterDistance::getPerimeterDistance(const geom::Envelope& b, const geom::CoordinateXY& c)
{
    return getPerimeterDistance(b.getMinX(), b.getMinY(), b.getMaxX(), b.getMaxY(), c.x, c.y);
}

double
PerimeterDistance::getPerimeterDistanceCCW(double measure1, double measure2, double perimeter)
{
    if (measure2 <= measure1) {
        return measure1 - measure2;
    }
    return perimeter + measure1 - measure2;
}

bool PerimeterDistance::isBetweenCCW(double x, double a, double b)
{
    if (a < b) {
        return x <= a || x >= b;
    }
    return x <= a && x >= b;
}


}
