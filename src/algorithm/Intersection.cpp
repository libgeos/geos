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
#include <geos/algorithm/Distance.h>
#include <geos/algorithm/Intersection.h>
#include <geos/algorithm/Orientation.h>

namespace geos {
namespace algorithm { // geos.algorithm


/* public static */
geom::CoordinateXY
Intersection::intersection(const geom::CoordinateXY& p1, const geom::CoordinateXY& p2,
                           const geom::CoordinateXY& q1, const geom::CoordinateXY& q2)
{
    return CGAlgorithmsDD::intersection(p1, p2, q1, q2);
}


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
/* public static */
geom::CoordinateXY
Intersection::intersectionLineSegment(
    const geom::CoordinateXY& line1,
    const geom::CoordinateXY& line2,
    const geom::CoordinateXY& seg1,
    const geom::CoordinateXY& seg2)
{
    int orientS1 = Orientation::index(line1, line2, seg1);
    if (orientS1 == 0)
        return seg1;

    int orientS2 = Orientation::index(line1, line2, seg2);
    if (orientS2 == 0)
        return seg2;

    /**
     * If segment lies completely on one side of the line, it does not intersect
     */
    if ((orientS1 > 0 && orientS2 > 0) || (orientS1 < 0 && orientS2 < 0)) {
        return geom::CoordinateXY::getNull();
    }

    /**
     * The segment intersects the line.
     * The full line-line intersection is used to compute the intersection point.
     */
    geom::CoordinateXY intPt = intersection(line1, line2, seg1, seg2);
    if (!intPt.isNull())
        return intPt;

    /**
     * Due to robustness failure it is possible the intersection computation will return null.
     * In this case choose the closest point
     */
    double dist1 = Distance::pointToLinePerpendicular(seg1, line1, line2);
    double dist2 = Distance::pointToLinePerpendicular(seg2, line1, line2);
    if (dist1 < dist2)
        return seg1;
    else
        return seg2;
}


} // namespace geos.algorithm
} // namespace geos

