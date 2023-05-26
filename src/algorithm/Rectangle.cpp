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

#include <geos/algorithm/Rectangle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>

using geos::geom::CoordinateXY;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::GeometryFactory;
using geos::geom::LineSegment;
using geos::geom::LinearRing;
using geos::geom::Polygon;


namespace geos {
namespace algorithm { // geos.algorithm


/* public static */
std::unique_ptr<Polygon>
Rectangle::createFromSidePts(
    const CoordinateXY& baseRightPt,
    const CoordinateXY& baseLeftPt,
    const CoordinateXY& oppositePt,
    const CoordinateXY& leftSidePt,
    const CoordinateXY& rightSidePt,
    const GeometryFactory* factory)
{
    //-- deltas for the base segment provide slope
    double dx = baseLeftPt.x - baseRightPt.x;
    double dy = baseLeftPt.y - baseRightPt.y;
    // Assert: dx and dy are not both zero

    double baseC = computeLineEquationC(dx, dy, baseRightPt);
    double oppC = computeLineEquationC(dx, dy, oppositePt);
    double leftC = computeLineEquationC(-dy, dx, leftSidePt);
    double rightC = computeLineEquationC(-dy, dx, rightSidePt);

    //-- compute lines along edges of rectangle
    LineSegment baseLine = createLineForStandardEquation(-dy, dx, baseC);
    LineSegment oppLine = createLineForStandardEquation(-dy, dx, oppC);
    LineSegment leftLine = createLineForStandardEquation(-dx, -dy, leftC);
    LineSegment rightLine = createLineForStandardEquation(-dx, -dy, rightC);

    /**
     * Corners of rectangle are the intersections of the
     * base and opposite, and left and right lines.
     * The rectangle is constructed with CW orientation.
     * The first side of the constructed rectangle contains the base segment.
     *
     * If a corner coincides with a input point
     * the exact value is used to avoid numerical inaccuracy.
     */
    CoordinateXY p0 = rightSidePt.equals2D(baseRightPt) ? baseRightPt
        : baseLine.lineIntersection(rightLine);
    CoordinateXY p1 = leftSidePt.equals2D(baseLeftPt) ? baseLeftPt
        : baseLine.lineIntersection(leftLine);
    CoordinateXY p2 = leftSidePt.equals2D(oppositePt) ? oppositePt
        : oppLine.lineIntersection(leftLine);
    CoordinateXY p3 = rightSidePt.equals2D(oppositePt) ? oppositePt
        : oppLine.lineIntersection(rightLine);

    CoordinateSequence cs({ p0, p1, p2, p3, p0 });
    return factory->createPolygon(std::move(cs));
}



/* private static */
double
Rectangle::computeLineEquationC(double a, double b, const CoordinateXY& p)
{
    return a * p.y - b * p.x;
}


/* private static */
LineSegment
Rectangle::createLineForStandardEquation(double a, double b, double c)
{
    Coordinate p0;
    Coordinate p1;
    /*
    * Line equation is ax + by = c
    * Slope m = -a/b.
    * Y-intercept = c/b
    * X-intercept = c/a
    *
    * If slope is low, use constant X values; if high use Y values.
    * This handles lines that are vertical (b = 0, m = Inf )
    * and horizontal (a = 0, m = 0).
    */
    if (std::abs(b) > std::abs(a)) {
        //-- abs(m) < 1
        p0 = Coordinate(0.0, c/b);
        p1 = Coordinate(1.0, c/b - a/b);
    }
    else {
        //-- abs(m) >= 1
        p0 = Coordinate(c/a, 0.0);
        p1 = Coordinate(c/a - b/a, 1.0);
    }
    return LineSegment(p0, p1);
}



} // namespace geos.algorithm
} // namespace geos

