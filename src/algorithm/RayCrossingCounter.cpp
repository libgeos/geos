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
 **********************************************************************
 *
 * Last port: algorithm/RayCrossingCounter.java rev. 1.2 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/CircularArc.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Curve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/geom/SimpleCurve.h>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {
//
// private:
//

//
// protected:
//

//
// public:
//
/*static*/ geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const geom::CoordinateSequence& ring)
{
    RayCrossingCounter rcc(point);

    for(std::size_t i = 1, ni = ring.size(); i < ni; i++) {
        const geom::CoordinateXY& p1 = ring.getAt<geom::CoordinateXY>(i-1);;
        const geom::CoordinateXY& p2 = ring.getAt<geom::CoordinateXY>(i);

        rcc.countSegment(p1, p2);

        if(rcc.isOnSegment()) {
            return rcc.getLocation();
        }
    }
    return rcc.getLocation();
}

/*static*/ geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const std::vector<const geom::Coordinate*>& ring)
{
    RayCrossingCounter rcc(point);

    for(std::size_t i = 1, ni = ring.size(); i < ni; i++) {
        const geom::Coordinate& p1 = *ring[ i - 1 ];
        const geom::Coordinate& p2 = *ring[ i ];

        rcc.countSegment(p1, p2);

        if(rcc.isOnSegment()) {
            return rcc.getLocation();
        }
    }
    return rcc.getLocation();
}

geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const geom::Curve& ring)
{
    RayCrossingCounter rcc(point);

    for (std::size_t i = 0; i < ring.getNumCurves(); i++) {
        const geom::SimpleCurve* curve = ring.getCurveN(i);
        rcc.processSequence(*curve->getCoordinatesRO(), !curve->hasCurvedComponents());
    }

    return rcc.getLocation();
}

void
RayCrossingCounter::processSequence(const geom::CoordinateSequence& seq, bool isLinear)
{
    if (isOnSegment()) {
        return;
    }

    if (isLinear) {
        for(std::size_t i = 1; i < seq.size(); i++) {
            const geom::CoordinateXY& p1 = seq.getAt<geom::CoordinateXY>(i-1);;
            const geom::CoordinateXY& p2 = seq.getAt<geom::CoordinateXY>(i);

            countSegment(p1, p2);

            if (isOnSegment()) 	{
                return;
            }
        }
    } else {
        for (std::size_t i = 2; i < seq.size(); i += 2) {
            const geom::CoordinateXY& p1 = seq.getAt<geom::CoordinateXY>(i-2);
            const geom::CoordinateXY& p2 = seq.getAt<geom::CoordinateXY>(i-1);
            const geom::CoordinateXY& p3 = seq.getAt<geom::CoordinateXY>(i);

            countArc(p1, p2, p3);

            if (isOnSegment()) 	{
                return;
            }
        }
    }
}

void
RayCrossingCounter::countSegment(const geom::CoordinateXY& p1,
                                 const geom::CoordinateXY& p2)
{
    // For each segment, check if it crosses
    // a horizontal ray running from the test point in
    // the positive x direction.

    // check if the segment is strictly to the left of the test point
    if(p1.x < point.x && p2.x < point.x) {
        return;
    }

    // check if the point is equal to the current ring vertex
    if(point.x == p2.x && point.y == p2.y) {
        isPointOnSegment = true;
        return;
    }

    // For horizontal segments, check if the point is on the segment.
    // Otherwise, horizontal segments are not counted.
    if(p1.y == point.y && p2.y == point.y) {
        double minx = p1.x;
        double maxx = p2.x;

        if(minx > maxx) {
            minx = p2.x;
            maxx = p1.x;
        }

        if(point.x >= minx && point.x <= maxx) {
            isPointOnSegment = true;
        }

        return;
    }

    // Evaluate all non-horizontal segments which cross a horizontal ray
    // to the right of the test pt.
    // To avoid double-counting shared vertices, we use the convention that
    // - an upward edge includes its starting endpoint, and excludes its
    //   final endpoint
    // - a downward edge excludes its starting endpoint, and includes its
    //   final endpoint

    if((p1.y > point.y && p2.y <= point.y) || (p2.y > point.y && p1.y <= point.y)) {
        // For an upward edge, orientationIndex will be positive when p1->p2
        // crosses ray. Conversely, downward edges should have negative sign.
        int sign = CGAlgorithmsDD::orientationIndex(p1, p2, point);
        if(sign == 0) {
            isPointOnSegment = true;
            return;
        }

        if(p2.y < p1.y) {
            sign = -sign;
        }

        // The segment crosses the ray if the sign is strictly positive.
        if(sign > 0) {
            crossingCount++;
        }
    }
}

bool
RayCrossingCounter::shouldCountCrossing(const geom::CircularArc& arc, const geom::CoordinateXY& q) {
    // To avoid double-counting shared vertices, we count an intersection point if
    // a) is in the interior of the arc
    // b) is at the starting point of the arc, and the arc is directed upward at that point
    // c) is at the ending point of the arc is directed downward at that point
    if (q.equals2D(arc.p0)) {
        return arc.isUpwardAtPoint(q);
    } else if (q.equals2D(arc.p2)) {
        return !arc.isUpwardAtPoint(q);
    } else {
        return true;
    }
}

/// Return an array of 0-2 intersection points between an arc and a horizontal
/// ray extending righward from a point. If fewer than 2 intersection points exist,
/// some Coordinates in the returned array will be equal to CoordinateXY::getNull().
std::array<geom::CoordinateXY, 2>
RayCrossingCounter::pointsIntersectingHorizontalRay(const geom::CircularArc& arc, const geom::CoordinateXY& origin) {
    const auto& c = arc.getCenter();
    const auto& R = arc.getRadius();

    auto dx = std::sqrt(R*R - std::pow(origin.y - c.y, 2) );

    // Find two points where the horizontal line intersects the circle
    // that is coincident with this arc.
    // Problem: because of floating-point errors, these
    // constructed points may not actually like on the circle.
    CoordinateXY intPt1{c.x - dx, origin.y};
    CoordinateXY intPt2{c.x + dx, origin.y};

    // Solution (best we have for now)
    // Snap computed points to points that define the arc
    double eps = 1e-14;

    for (const CoordinateXY& pt : arc ) {
        if (origin.y == pt.y) {
            if (intPt1.distance(pt) < eps) {
                intPt1 = pt;
            }
            if (intPt2.distance(pt) < eps) {
                intPt2 = pt;
            }
        }
    }

    std::array<CoordinateXY, 2> ret { CoordinateXY::getNull(), CoordinateXY::getNull() };

    std::size_t pos = 0;
    if (intPt1.x >= origin.x && arc.containsPointOnCircle(intPt1)) {
        ret[pos++] = intPt1;
    }
    if (intPt2.x >= origin.x && arc.containsPointOnCircle(intPt2)) {
        ret[pos++] = intPt2;
    }

    return ret;
}

void
RayCrossingCounter::countArc(const CoordinateXY& p1,
                             const CoordinateXY& p2,
                             const CoordinateXY& p3)
{
    // For each arc, check if it crosses
    // a horizontal ray running from the test point in
    // the positive x direction.
    geom::CircularArc arc(p1, p2, p3);

    // If the arc is degenerate, process it is two line segments
    if (arc.isLinear()) {
        countSegment(p1, p2);
        countSegment(p2, p3);
        return;
    }

    // Check if the arc is strictly to the left of the test point
    geom::Envelope arcEnvelope;
    CircularArcs::expandEnvelope(arcEnvelope, p1, p2, p3);

    if (arcEnvelope.getMaxX() < point.x) {
        return;
    }

    // Evaluate all arcs whose enveleope is to the right of the test point.
    if (arcEnvelope.getMaxY() >= point.y && arcEnvelope.getMinY() <= point.y) {
        if (arc.containsPoint(point)) {
            isPointOnSegment = true;
            return;
        }

        auto intPts = pointsIntersectingHorizontalRay(arc, point);
        if (!intPts[0].isNull() && shouldCountCrossing(arc, intPts[0])) {
            crossingCount++;
        }
        if (!intPts[1].isNull() && shouldCountCrossing(arc, intPts[1])) {
            crossingCount++;
        }
    }
}

geom::Location
RayCrossingCounter::getLocation() const
{
    if(isPointOnSegment) {
        return geom::Location::BOUNDARY;
    }

    // The point is in the interior of the ring if the number
    // of X-crossings is odd.
    if((crossingCount % 2) == 1) {
        return geom::Location::INTERIOR;
    }

    return geom::Location::EXTERIOR;
}


bool
RayCrossingCounter::isPointInPolygon() const
{
    return getLocation() != geom::Location::EXTERIOR;
}


} // geos::algorithm
} // geos
