/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/quadedge/TrianglePredicate.java r524
 *
 **********************************************************************/

#include <geos/triangulate/quadedge/TrianglePredicate.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>

using geos::geom::Coordinate;

namespace geos {
namespace triangulate {
namespace quadedge {

geom::Location
TrianglePredicate::isInCircleNonRobust(
    const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
    const CoordinateXY& p)
{
    auto det =
        (a.x * a.x + a.y * a.y) * triArea(b, c, p)
        - (b.x * b.x + b.y * b.y) * triArea(a, c, p)
        + (c.x * c.x + c.y * c.y) * triArea(a, b, p)
        - (p.x * p.x + p.y * p.y) * triArea(a, b, c);

    return det > 0 ? geom::Location::EXTERIOR : (det < 0 ? geom::Location::INTERIOR : geom::Location::BOUNDARY);
}

geom::Location
TrianglePredicate::isInCircleNormalized(
    const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
    const CoordinateXY& p)
{
    // Unfortunately this implementation is not robust either. For robust one see:
    // https://www.cs.cmu.edu/~quake/robust.html
    // https://www.cs.cmu.edu/afs/cs/project/quake/public/code/predicates.c

    long double adx = static_cast<long double>(a.x) - static_cast<long double>(p.x);
    long double ady = static_cast<long double>(a.y) - static_cast<long double>(p.y);
    long double bdx = static_cast<long double>(b.x) - static_cast<long double>(p.x);
    long double bdy = static_cast<long double>(b.y) - static_cast<long double>(p.y);
    long double cdx = static_cast<long double>(c.x) - static_cast<long double>(p.x);
    long double cdy = static_cast<long double>(c.y) - static_cast<long double>(p.y);

    long double bdxcdy = bdx * cdy;
    long double cdxbdy = cdx * bdy;
    long double alift = adx * adx + ady * ady;

    long double cdxady = cdx * ady;
    long double adxcdy = adx * cdy;
    long double blift = bdx * bdx + bdy * bdy;

    long double adxbdy = adx * bdy;
    long double bdxady = bdx * ady;
    long double clift = cdx * cdx + cdy * cdy;

    long double A = (alift * bdxcdy + blift * cdxady + clift * adxbdy);
    long double B = (alift * cdxbdy + blift * adxcdy + clift * bdxady);

    if (A < B) {
        return geom::Location::EXTERIOR;
    } else if (A == B) {
        return geom::Location::BOUNDARY;
    } else {
        return geom::Location::INTERIOR;
    }
}

double
TrianglePredicate::triArea(const CoordinateXY& a,
                           const CoordinateXY& b, const CoordinateXY& c)
{
    return (b.x - a.x) * (c.y - a.y)
           - (b.y - a.y) * (c.x - a.x);
}

geom::Location
TrianglePredicate::isInCircleRobust(
    const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
    const CoordinateXY& p)
{
    // This implementation is not robust, name is ported from JTS.
    return isInCircleNormalized(a, b, c, p);
}

} // namespace geos.triangulate.quadedge
} // namespace geos.triangulate
} // namespace geos
