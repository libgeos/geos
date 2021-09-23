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

using geos::geom::Coordinate;

namespace geos {
namespace triangulate {
namespace quadedge {

bool
TrianglePredicate::isInCircleNonRobust(
    const Coordinate& a, const Coordinate& b, const Coordinate& c,
    const Coordinate& p)
{
    bool isInCircle =
        (a.x * a.x + a.y * a.y) * triArea(b, c, p)
        - (b.x * b.x + b.y * b.y) * triArea(a, c, p)
        + (c.x * c.x + c.y * c.y) * triArea(a, b, p)
        - (p.x * p.x + p.y * p.y) * triArea(a, b, c)
        > 0;
    return isInCircle;
}

bool
TrianglePredicate::isInCircleNormalized(
    const Coordinate& a, const Coordinate& b, const Coordinate& c,
    const Coordinate& p)
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
    return (alift * bdxcdy + blift * cdxady + clift * adxbdy) >
           (alift * cdxbdy + blift * adxcdy + clift * bdxady);
}

double
TrianglePredicate::triArea(const Coordinate& a,
                           const Coordinate& b, const Coordinate& c)
{
    return (b.x - a.x) * (c.y - a.y)
           - (b.y - a.y) * (c.x - a.x);
}

bool
TrianglePredicate::isInCircleRobust(
    const Coordinate& a, const Coordinate& b, const Coordinate& c,
    const Coordinate& p)
{
    // This implementation is not robust, name is ported from JTS.
    return isInCircleNormalized(a, b, c, p);
}

} // namespace geos.triangulate.quadedge
} // namespace geos.triangulate
} // namespace geos
