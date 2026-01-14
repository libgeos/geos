/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Area.java @ 2017-09-04
 *
 **********************************************************************/

#include <cmath>
#include <vector>

#include <geos/algorithm/Area.h>
#include <geos/geom/CircularArc.h>
#include <geos/geom/Curve.h>
#include <geos/geom/SimpleCurve.h>
#include <geos/util/IllegalArgumentException.h>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm { // geos.algorithm

template<typename T>
double signedRingArea(const T& ring)
{
    std::size_t rlen = ring.size();
    if(rlen < 3) {
        return 0.0;
    }

    double sum = 0.0;
    /*
     * Based on the Shoelace formula.
     * http://en.wikipedia.org/wiki/Shoelace_formula
     */
    double x0 = ring[0].x;
    for(std::size_t i = 1; i < rlen - 1; i++) {
        double x = ring[i].x - x0;
        double y1 = ring[i + 1].y;
        double y2 = ring[i - 1].y;
        sum += x * (y2 - y1);
    }
    return sum / 2.0;

}

/* public static */
double
Area::ofRing(const std::vector<geom::Coordinate>& ring)
{
    return std::abs(ofRingSigned(ring));
}

double
Area::ofRing(const std::vector<geom::CoordinateXY>& ring)
{
    return std::abs(signedRingArea(ring));
}

/* public static */
double
Area::ofRing(const geom::CoordinateSequence* ring)
{
    return std::abs(ofRingSigned(ring));
}

/* public static */
double
Area::ofRingSigned(const std::vector<geom::Coordinate>& ring)
{
    return signedRingArea(ring);
}

/* public static */
double
Area::ofRingSigned(const geom::CoordinateSequence* ring)
{
    std::size_t n = ring->size();
    if(n < 3) {
        return 0.0;
    }
    /*
     * Based on the Shoelace formula.
     * http://en.wikipedia.org/wiki/Shoelace_formula
     */
    CoordinateXY p0, p1, p2;
    p1 = ring->getAt<CoordinateXY>(0);
    p2 = ring->getAt<CoordinateXY>(1);
    double x0 = p1.x;
    p2.x -= x0;
    double sum = 0.0;
    for(std::size_t i = 1; i < n - 1; i++) {
        p0.y = p1.y;
        p1.x = p2.x;
        p1.y = p2.y;
        p2 = ring->getAt<CoordinateXY>(i + 1);
        p2.x -= x0;
        sum += p1.x * (p0.y - p2.y);
    }
    return sum / 2.0;
}

double
Area::ofClosedCurve(const geom::Curve& ring) {
    if (!ring.isClosed()) {
        throw util::IllegalArgumentException("Argument is not closed");
    }

    double sum = 0;

    for (std::size_t i = 0; i < ring.getNumCurves(); i++) {
        const geom::SimpleCurve& section = *ring.getCurveN(i);

        if (section.isEmpty()) {
            continue;
        }

        const geom::CoordinateSequence& coords = *section.getCoordinatesRO();

        if (section.isCurved()) {
            for (std::size_t j = 2; j < coords.size(); j += 2) {
                const CoordinateXY& p0 = coords.getAt<CoordinateXY>(j-2);
                const CoordinateXY& p1 = coords.getAt<CoordinateXY>(j-1);
                const CoordinateXY& p2 = coords.getAt<CoordinateXY>(j);

                double triangleArea = 0.5*(p0.x*p2.y - p2.x*p0.y);
                sum += triangleArea;

                geom::CircularArc arc(coords, j-2);
                if (arc.isLinear()) {
                    continue;
                }

                double circularSegmentArea = arc.getArea();

                if (algorithm::Orientation::index(p0, p2, p1) == algorithm::Orientation::CLOCKWISE) {
                    sum += circularSegmentArea;
                } else {
                    sum -= circularSegmentArea;
                }
            }
        } else {
            for (std::size_t j = 1; j < coords.size(); j++) {
                const CoordinateXY& p0 = coords.getAt<CoordinateXY>(j-1);
                const CoordinateXY& p1 = coords.getAt<CoordinateXY>(j);

                double triangleArea = 0.5*(p0.x*p1.y - p1.x*p0.y);
                sum += triangleArea;
            }
        }
    }

    return std::abs(sum);
}

} // namespace geos.algorithm
} //namespace geos

