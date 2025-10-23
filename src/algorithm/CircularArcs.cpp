/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Angle.h>
#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Quadrant.h>
#include <geos/math/DD.h>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {

std::array<geom::CoordinateXY, 3>
CircularArcs::createArc(const geom::CoordinateXY& center, double radius, double start, double end, bool ccw)
{
    double mid = getMidpointAngle(start, end, ccw);

    if (ccw) {
        return {
            createPoint(center, radius, start),
            createPoint(center, radius, mid),
            createPoint(center, radius, end),
        };
    } else {
        return {
            createPoint(center, radius, end),
            createPoint(center, radius, mid),
            createPoint(center, radius, start),
        };
    }
}

CoordinateXY
CircularArcs::getMidpoint(const CoordinateXY& p0, const CoordinateXY& p2, const CoordinateXY& center, double radius, bool isCCW)
{
    double start = getAngle(p0, center);
    double stop = getAngle(p2, center);
    double mid = getMidpointAngle(start, stop, isCCW);
    return createPoint(center, radius, mid);
}

double
CircularArcs::getAngle(const CoordinateXY& p, const CoordinateXY& center)
{
    return std::atan2(p.y - center.y, p.x - center.x);
}

double
CircularArcs::getMidpointAngle(double theta0, double theta2, bool isCCW)
{
    if (!isCCW) {
        return getMidpointAngle(theta2, theta0, true);
    }

    double mid = (theta0 + theta2) / 2;
    if (!Angle::isWithinCCW(mid, theta0, theta2)) {
        mid += MATH_PI;
    }

    return mid;
}

CoordinateXY
CircularArcs::createPoint(const CoordinateXY& center, double radius, double theta)
{
    return { center.x + radius* std::cos(theta), center.y + radius* std::sin(theta) };
}

template<typename T>
CoordinateXY getCenterImpl(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    // Circumcenter formulas from Graphics Gems III
    T p0x{p0.x};
    T p0y{p0.y};
    T p1x{p1.x};
    T p1y{p1.y};
    T p2x{p2.x};
    T p2y{p2.y};

    T ax = p1x - p2x;
    T ay = p1y - p2y;
    T bx = p2x - p0x;
    T by = p2y - p0y;
    T cx = p0x - p1x;
    T cy = p0y - p1y;

    T d1 = -(bx*cx + by*cy);
    T d2 = -(cx*ax + cy*ay);
    T d3 = -(ax*bx + ay*by);

    T e1 = d2*d3;
    T e2 = d3*d1;
    T e3 = d1*d2;
    T e = e1 + e2 + e3;

    T G3x = p0.x + p1.x + p2.x;
    T G3y = p0.y + p1.y + p2.y;
    T Hx = (e1*p0.x + e2*p1.x + e3*p2.x) / e;
    T Hy = (e1*p0.y + e2*p1.y + e3*p2.y) / e;

    T rx = 0.5*(G3x - Hx);
    T ry = 0.5*(G3y - Hy);

    if constexpr (std::is_same_v<T, math::DD>) {
        return {rx.doubleValue(), ry.doubleValue()};
    } else {
        return {rx, ry};
    }
}

CoordinateXY
CircularArcs::getCenter(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    if (p0.equals2D(p2)) {
        // Closed circle
        return { 0.5*(p0.x + p1.x), 0.5*(p0.y + p1.y) };
    }

    return getCenterImpl<double>(p0, p1, p2);
}

void
CircularArcs::expandEnvelope(geom::Envelope& e, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
                             const geom::CoordinateXY& p2)
{
    using geom::Quadrant;

    e.expandToInclude(p0);
    e.expandToInclude(p1);
    e.expandToInclude(p2);

    CoordinateXY center = getCenter(p0, p1, p2);

    // zero-length arc
    if (center.equals2D(p0) || center.equals2D(p1)) {
        return;
    }

    // collinear
    if (std::isnan(center.x)) {
        return;
    }

    //* 1 | 0
    //* --+--
    //* 2 | 3
    const auto pa0 = Quadrant::pseudoAngle(center, p0);
    const auto pa1 = Quadrant::pseudoAngle(center, p1);
    const auto pa2 = Quadrant::pseudoAngle(center, p2);

    auto q0 = static_cast<int>(pa0);
    auto q2 = static_cast<int>(pa2);
    double R = center.distance(p1);

    if (q0 == q2) {
        // Start and end quadrants are the same. Either the arc crosses all
        // the axes, or none of the axes.

        const bool isBetween = pa1 > std::min(pa0, pa2) && pa1 < std::max(pa0, pa2);

        if (!isBetween) {
            e.expandToInclude({center.x, center.y + R});
            e.expandToInclude({center.x - R, center.y});
            e.expandToInclude({center.x, center.y - R});
            e.expandToInclude({center.x + R, center.y});
        }

        return;
    }

    auto orientation = Orientation::index(p0, p1, p2);

    if (orientation == Orientation::CLOCKWISE) {
        std::swap(q0, q2);
    }

    for (auto q = q0 + 1; (q % 4) != ((q2+1) % 4); q++) {
        switch (q % 4) {
        case Quadrant::NW:
            e.expandToInclude({center.x, center.y + R});
            break;
        case Quadrant::SW:
            e.expandToInclude({center.x - R, center.y});
            break;
        case Quadrant::SE:
            e.expandToInclude({center.x, center.y - R});
            break;
        case Quadrant::NE:
            e.expandToInclude({center.x + R, center.y});
            break;
        }
    }
}

}
}
