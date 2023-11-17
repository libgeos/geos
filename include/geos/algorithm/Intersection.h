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

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

namespace geos {
namespace algorithm {


/** \brief
 * Functions to compute intersection points between lines and line segments.
 *
 * In general it is not possible to compute
 * the intersection point of two lines exactly, due to numerical roundoff.
 * This is particularly true when the lines are nearly parallel.
 * These routines uses numerical conditioning on the input values
 * to ensure that the computed value is very close to the correct value.
 *
 * The Z-ordinate is ignored, and not populated.
 */
class GEOS_DLL Intersection {

public:

/** \brief
 * Computes the intersection point of two lines.
 * If the lines are parallel or collinear this case is detected
 * and <code>null</code> is returned.
 *
 * @param p1 an endpoint of line 1
 * @param p2 an endpoint of line 1
 * @param q1 an endpoint of line 2
 * @param q2 an endpoint of line 2
 * @return the intersection point between the lines, if there is one,
 * or null if the lines are parallel or collinear
 *
 * @see CGAlgorithmsDD#intersection(Coordinate, Coordinate, Coordinate, Coordinate)
 */
static geom::CoordinateXY intersection(const geom::CoordinateXY& p1, const geom::CoordinateXY& p2,
                                       const geom::CoordinateXY& q1, const geom::CoordinateXY& q2);

/**
* Computes the intersection point of a line and a line segment (if any).
* There will be no intersection point if:
*
*  * the segment does not intersect the line
*  * the line or the segment are degenerate (have zero length)
*
* If the segment is collinear with the line the first segment endpoint is returned.
*
* @param line1 a point on the line
* @param line2 a point on the line
* @param seg1 an endpoint of the line segment
* @param seg2 an endpoint of the line segment
* @return the intersection point, or null if it is not possible to find an intersection
*/
static geom::CoordinateXY intersectionLineSegment(
    const geom::CoordinateXY& line1,
    const geom::CoordinateXY& line2,
    const geom::CoordinateXY& seg1,
    const geom::CoordinateXY& seg2);


};


} // namespace geos::algorithm
} // namespace geos

