/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Triangle.h>
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/Angle.h>

using geos::algorithm::Angle;
using geos::algorithm::Orientation;

namespace geos {
namespace geom { // geos::geom


bool
Triangle::isIsoceles()
{
    double len0 = p1.distance(p2);
    double len1 = p0.distance(p2);
    double len2 = p0.distance(p1);
    if (len0 == len1 || len1 == len2 || len2 == len0)
        return true;
    else
        return false;
}

void
Triangle::inCentre(CoordinateXY& result)
{
    // the lengths of the sides, labelled by their opposite vertex
    double len0 = p1.distance(p2);
    double len1 = p0.distance(p2);
    double len2 = p0.distance(p1);
    double circum = len0 + len1 + len2;
    double inCentreX = (len0 * p0.x + len1 * p1.x + len2 * p2.x)  / circum;
    double inCentreY = (len0 * p0.y + len1 * p1.y + len2 * p2.y)  / circum;

    result = CoordinateXY(inCentreX, inCentreY);
}

void
Triangle::circumcentre(CoordinateXY& result)
{
    double cx = p2.x;
    double cy = p2.y;
    double ax = p0.x - cx;
    double ay = p0.y - cy;
    double bx = p1.x - cx;
    double by = p1.y - cy;

    double denom = 2 * det(ax, ay, bx, by);
    double numx = det(ay, ax * ax + ay * ay, by, bx * bx + by * by);
    double numy = det(ax, ax * ax + ay * ay, bx, bx * bx + by * by);

    double ccx = cx - numx / denom;
    double ccy = cy + numy / denom;

    result = CoordinateXY(ccx, ccy);
}


double
Triangle::circumradius(
    const CoordinateXY& a,
    const CoordinateXY& b,
    const CoordinateXY& c)
{
    double A = a.distance(b);
    double B = b.distance(c);
    double C = c.distance(a);
    double triArea = area(a, b, c);
    if (triArea == 0.0)
        return std::numeric_limits<double>::infinity();

    return (A * B * C) / (4 * triArea);
}


void
Triangle::circumcentreDD(CoordinateXY& result)
{
    result = algorithm::CGAlgorithmsDD::circumcentreDD(p0, p1, p2);
}

/* public static */
const CoordinateXY
Triangle::circumcentre(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    Triangle t(p0, p1, p2);
    CoordinateXY c;
    t.circumcentre(c);
    return c;
}

/* private */
double
Triangle::det(double m00, double m01, double m10, double m11) const
{
    return m00 * m11 - m01 * m10;
}


/* public static */
bool
Triangle::isAcute(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    if (!Angle::isAcute(a, b, c))
        return false;
    if (!Angle::isAcute(b, c, a))
        return false;
    if (!Angle::isAcute(c, a, b))
        return false;
    return true;
}


/* public static */
bool
Triangle::isCCW(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    return Orientation::COUNTERCLOCKWISE == Orientation::index(a, b, c);
}


/* public static */
bool
Triangle::intersects(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c, const CoordinateXY& p)
{
    int exteriorIndex = isCCW(a, b, c) ?
        Orientation::CLOCKWISE : Orientation::COUNTERCLOCKWISE;
    if (exteriorIndex == Orientation::index(a, b, p))
        return false;
    if (exteriorIndex == Orientation::index(b, c, p))
        return false;
    if (exteriorIndex == Orientation::index(c, a, p))
        return false;
    return true;
}


/* public static */
double
Triangle::length(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    return a.distance(b) + b.distance(c) + c.distance(a);
}

/* public */
double
Triangle::length() const
{
    return length(p0, p1, p2);
}

/* public static */
double
Triangle::area(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    return std::abs(((c.x - a.x) * (b.y - a.y) - (b.x - a.x) * (c.y - a.y)) / 2);
}

/* public */
double
Triangle::area() const
{
    return area(p0, p1, p2);
}

/* public static */
double
Triangle::longestSideLength(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    double lenAB = a.distance(b);
    double lenBC = b.distance(c);
    double lenCA = c.distance(a);
    double maxLen = lenAB;
    if (lenBC > maxLen)
        maxLen = lenBC;
    if (lenCA > maxLen)
        maxLen = lenCA;
    return maxLen;
}



} // namespace geos::geom
} // namespace geos
