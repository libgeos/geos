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
    double minX0 = p1.x < p2.x ? p1.x : p2.x;
    double minY0 = p1.y < p2.y ? p1.y : p2.y;
    double maxX0 = p1.x > p2.x ? p1.x : p2.x;
    double maxY0 = p1.y > p2.y ? p1.y : p2.y;

    double minX1 = q1.x < q2.x ? q1.x : q2.x;
    double minY1 = q1.y < q2.y ? q1.y : q2.y;
    double maxX1 = q1.x > q2.x ? q1.x : q2.x;
    double maxY1 = q1.y > q2.y ? q1.y : q2.y;

    double intMinX = minX0 > minX1 ? minX0 : minX1;
    double intMaxX = maxX0 < maxX1 ? maxX0 : maxX1;
    double intMinY = minY0 > minY1 ? minY0 : minY1;
    double intMaxY = maxY0 < maxY1 ? maxY0 : maxY1;

    double midx = (intMinX + intMaxX) / 2.0;
    double midy = (intMinY + intMaxY) / 2.0;

    // condition ordinate values by subtracting midpoint
    double p1x = p1.x - midx;
    double p1y = p1.y - midy;
    double p2x = p2.x - midx;
    double p2y = p2.y - midy;
    double q1x = q1.x - midx;
    double q1y = q1.y - midy;
    double q2x = q2.x - midx;
    double q2y = q2.y - midy;

    // unrolled computation using homogeneous coordinates eqn
    double px = p1y - p2y;
    double py = p2x - p1x;
    double pw = p1x * p2y - p2x * p1y;

    double qx = q1y - q2y;
    double qy = q2x - q1x;
    double qw = q1x * q2y - q2x * q1y;

    double x = py * qw - qy * pw;
    double y = qx * pw - px * qw;
    double w = px * qy - qx * py;

    double xInt = x/w;
    double yInt = y/w;
    geom::CoordinateXY rv;
    // check for parallel lines
    if (!std::isfinite(xInt) || !std::isfinite(yInt)) {
        rv.setNull();
        return rv;
    }
    // de-condition intersection point
    rv.x = xInt + midx;
    rv.y = yInt + midy;
    return rv;
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

