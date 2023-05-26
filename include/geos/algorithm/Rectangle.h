/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023  Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/LineSegment.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class GeometryFactory;
class Polygon;
}
}

using geos::geom::CoordinateXY;
using geos::geom::GeometryFactory;
using geos::geom::LineSegment;
using geos::geom::Polygon;

namespace geos {
namespace algorithm {

class GEOS_DLL Rectangle {

public:

    /**
    * Creates a rectangular {@link Polygon} from a base segment
    * defining the position and orientation of one side of the rectangle, and
    * three points defining the locations of the line segments
    * forming the opposite, left and right sides of the rectangle.
    * The base segment and side points must be presented so that the
    * rectangle has CW orientation.
    *
    * The rectangle corners are computed as intersections of
    * lines, which generally cannot produce exact values.
    * If a rectangle corner is determined to coincide with a side point
    * the side point value is used to avoid numerical inaccuracy.
    *
    * The first side of the constructed rectangle contains the base segment.
    *
    * @param baseRightPt the right point of the base segment
    * @param baseLeftPt the left point of the base segment
    * @param oppositePt the point defining the opposite side
    * @param leftSidePt the point defining the left side
    * @param rightSidePt the point defining the right side
    * @param factory the geometry factory to use
    * @return the rectangular polygon
    */
    static std::unique_ptr<Polygon>
        createFromSidePts(
            const CoordinateXY& baseRightPt,
            const CoordinateXY& baseLeftPt,
            const CoordinateXY& oppositePt,
            const CoordinateXY& leftSidePt,
            const CoordinateXY& rightSidePt,
            const GeometryFactory* factory);

private:

    /**
    * Computes the constant C in the standard line equation Ax + By = C
    * from A and B and a point on the line.
    *
    * @param a the X coefficient
    * @param b the Y coefficient
    * @param p a point on the line
    * @return the constant C
    */
    static double
        computeLineEquationC(double a, double b, const CoordinateXY& p);

    static LineSegment
        createLineForStandardEquation(double a, double b, double c);

};


} // namespace geos::algorithm
} // namespace geos


