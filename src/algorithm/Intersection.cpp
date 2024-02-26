/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cmath>
#include <vector>

#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/Intersection.h>

namespace geos {
namespace algorithm { // geos.algorithm


/* public static */
geom::Coordinate
Intersection::intersection(const geom::Coordinate& p1, const geom::Coordinate& p2,
                           const geom::Coordinate& q1, const geom::Coordinate& q2)
{
    return CGAlgorithmsDD::intersection(p1, p2, q1, q2);
}


} // namespace geos.algorithm
} // namespace geos

