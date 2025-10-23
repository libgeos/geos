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

#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Quadrant.h>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {


CoordinateXY
CircularArcs::getCenter(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    if (p0.equals2D(p2)) {
        // Closed circle
        return { 0.5*(p0.x + p1.x), 0.5*(p0.y + p1.y) };
    }

    // Circumcenter formulas from Graphics Gems III
    CoordinateXY a{p1.x - p2.x, p1.y - p2.y};
    CoordinateXY b{p2.x - p0.x, p2.y - p0.y};
    CoordinateXY c{p0.x - p1.x, p0.y - p1.y};

    double d1 = -(b.x*c.x + b.y*c.y);
    double d2 = -(c.x*a.x + c.y*a.y);
    double d3 = -(a.x*b.x + a.y*b.y);

    double e1 = d2*d3;
    double e2 = d3*d1;
    double e3 = d1*d2;
    double e = e1 + e2 + e3;

    CoordinateXY G3{p0.x + p1.x + p2.x, p0.y + p1.y + p2.y};
    CoordinateXY H {(e1*p0.x + e2*p1.x + e3*p2.x) / e, (e1*p0.y + e2*p1.y + e3*p2.y) / e};

    CoordinateXY center = {0.5*(G3.x - H.x), 0.5*(G3.y - H.y)};

    return center;
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
