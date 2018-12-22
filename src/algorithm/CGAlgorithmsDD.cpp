/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mateusz Loskot <mateusz@loskot.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithmsDD.java r789 (JTS-1.14)
 *
 **********************************************************************/


#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/IllegalArgumentException.h>
#include <sstream>

using namespace geos::geom;
using namespace geos::algorithm;

namespace {

/**
 * A value which is safely greater than the relative round-off
 * error in double-precision numbers
 */
double constexpr DP_SAFE_EPSILON =  1e-15;

inline int OrientationDD(DD const& dd)
{
    static DD const zero(0.0);
    if (dd < zero)
        return CGAlgorithmsDD::RIGHT;

    if (dd > zero)
        return CGAlgorithmsDD::LEFT;

    return CGAlgorithmsDD::STRAIGHT;
}

// inline std::string ToStringDD(DD const& dd)
// {
//     return dd.ToString();
// }
}

namespace geos {
namespace algorithm { // geos::algorithm

int CGAlgorithmsDD::orientationIndex(const Coordinate& p1,
                                     const Coordinate& p2,
                                     const Coordinate& q)
{
    if (ISNAN(q.x) || ISNAN(q.y) || !FINITE(q.x) || !FINITE(q.y)) {
        throw util::IllegalArgumentException("CGAlgorithmsDD::orientationIndex encountered NaN/Inf numbers");
    }

    // fast filter for orientation index
    // avoids use of slow extended-precision arithmetic in many cases
    int index = orientationIndexFilter(p1, p2, q);
    if (index <= 1)
        return index;

    // normalize coordinates
    DD dx1 = DD(p2.x) + DD(-p1.x);
    DD dy1 = DD(p2.y) + DD(-p1.y);
    DD dx2 = DD(q.x) + DD(-p2.x);
    DD dy2 = DD(q.y) + DD(-p2.y);

    // sign of determinant - inlined for performance
    DD mx1y2(dx1 * dy2);
    DD my1x2(dy1 * dx2);
    DD d = mx1y2 - my1x2;
    return OrientationDD(d);
}

int CGAlgorithmsDD::signOfDet2x2(DD &x1, DD &y1, DD &x2, DD &y2)
{
    DD mx1y2(x1 * y2);
    DD my1x2(y1 * x2);
    DD d = mx1y2 - my1x2;
    return OrientationDD(d);
}

int CGAlgorithmsDD::signOfDet2x2(double dx1, double dy1, double dx2, double dy2)
{
    if (ISNAN(dx1)   || ISNAN(dy1)   || ISNAN(dx2)   || ISNAN(dy2) ||
        !FINITE(dx1) || !FINITE(dy1) || !FINITE(dx2) || !FINITE(dy2)) {
        throw util::IllegalArgumentException("CGAlgorithmsDD::signOfDet2x2 encountered NaN/Inf numbers");
    }
    DD x1(dx1);
    DD y1(dy1);
    DD x2(dx2);
    DD y2(dy2);
    return CGAlgorithmsDD::signOfDet2x2(x1, y1, x2, y2);
}

int CGAlgorithmsDD::orientationIndexFilter(const Coordinate& pa,
        const Coordinate& pb,
        const Coordinate& pc)
{
    double detsum;
    double const detleft = (pa.x - pc.x) * (pb.y - pc.y);
    double const detright = (pa.y - pc.y) * (pb.x - pc.x);
    double const det = detleft - detright;

    if (detleft > 0.0) {
        if (detright <= 0.0) {
            return orientation(det);
        }
        else {
            detsum = detleft + detright;
        }
    }
    else if (detleft < 0.0) {
        if (detright >= 0.0) {
            return orientation(det);
        }
        else {
            detsum = -detleft - detright;
        }
    }
    else {
        return orientation(det);
    }

    double const errbound = DP_SAFE_EPSILON * detsum;
    if ((det >= errbound) || (-det >= errbound)) {
        return orientation(det);
    }
    return CGAlgorithmsDD::FAILURE;
}

void CGAlgorithmsDD::intersection(const Coordinate& p1, const Coordinate& p2,
                                  const Coordinate& q1, const Coordinate& q2,
                                  Coordinate &rv)
{
    DD q1x(q1.x);
    DD q1y(q1.y);
    DD q2x(q2.x);
    DD q2y(q2.y);

    DD p1x(p1.x);
    DD p1y(p1.y);
    DD p2x(p2.x);
    DD p2y(p2.y);

    DD qdy = q2y - q1y;
    DD qdx = q2x - q1x;

    DD pdy = p2y - p1y;
    DD pdx = p2x - p1x;

    DD denom = (qdy * pdx) - (qdx * pdy);

    /**
     * Cases:
     * - denom is 0 if lines are parallel
     * - intersection point lies within line segment p if fracP is between 0 and 1
     * - intersection point lies within line segment q if fracQ is between 0 and 1
     */
    DD numx1 = (q2x - q1x) * (p1y - q1y);
    DD numx2 = (q2y - q1y) * (p1x - q1x);
    DD numx = numx1 - numx2;
    DD fracP = numx / denom;

    DD x = p1x + (p2x - p1x) * fracP;

    DD numy1 = (p2x - p1x) * (p1y - q1y);
    DD numy2 = (p2y - p1y) * (p1x - q1x);
    DD numy = numy1 - numy2;
    DD fracQ = numy / denom;
    DD y = q1y + (q2y - q1y) * fracQ;

    rv.x = x.ToDouble();
    rv.y = y.ToDouble();
}


} // namespace geos::algorithm
} // namespace geos


