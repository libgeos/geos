/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2020 Martin Davis
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/ExactMaxInscribedCircle.java
 * https://github.com/locationtech/jts/commit/8d5ced1b784d232e1eecf5df4b71873e8822336a
 *
 **********************************************************************/

#include <geos/algorithm/construct/ExactMaxInscribedCircle.h>
#include <geos/algorithm/Angle.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Triangle.h>
#include <geos/util/IllegalArgumentException.h>

#include <typeinfo> // for dynamic_cast


using namespace geos::geom;


namespace geos {
namespace algorithm { // geos.algorithm
namespace construct { // geos.algorithm.construct


/* public static */
bool
ExactMaxInscribedCircle::isSupported(const Geometry* geom)
{
    /* isSimplePolygon() */
    /* TODO replace with geometryid test */
    const Polygon* polygon = dynamic_cast<const Polygon*>(geom);
    if (polygon == nullptr)
        return false;
    if (polygon->getNumInteriorRing() > 0)
        return false;

    if (isTriangle(polygon))
        return true;
    if (isQuadrilateral(polygon) && isConvex(polygon))
        return true;
    return false;
}

/* private static */
bool
ExactMaxInscribedCircle::isTriangle(const Polygon* polygon)
{
    return polygon->getNumPoints() == 4;
}

/* private static */
bool
ExactMaxInscribedCircle::isQuadrilateral(const Polygon* polygon)
{
    return polygon->getNumPoints() == 5;
}

/* public static */
std::pair<CoordinateXY, CoordinateXY>
ExactMaxInscribedCircle::computeRadius(const Polygon* polygon)
{
    const LinearRing* exRing = polygon->getExteriorRing();
    const CoordinateSequence* ring = exRing->getCoordinatesRO();
    if (exRing->getNumPoints() == 4)
        return computeTriangle(ring);
    else if (exRing->getNumPoints() == 5)
        return computeConvexQuadrilateral(ring);
    throw util::IllegalArgumentException("Input must be a triangle or convex quadrilateral");
}

/* private static */
std::pair<CoordinateXY, CoordinateXY>
ExactMaxInscribedCircle::computeTriangle(const CoordinateSequence* ring)
{
    CoordinateXY center = Triangle::inCentre(
        ring->getAt(0), ring->getAt(1), ring->getAt(2));
    LineSegment seg(ring->getAt(0), ring->getAt(1));
    CoordinateXY radius = seg.project(center);
    std::pair<CoordinateXY, CoordinateXY> result;
    result.first = center;
    result.second = radius;
    return result;
}

/* private static */
std::pair<CoordinateXY, CoordinateXY>
ExactMaxInscribedCircle::computeConvexQuadrilateral(const CoordinateSequence* ring)
{
    /* Ensure ring is clockwise */
    const CoordinateSequence* ringCW = ring;
    std::unique_ptr<CoordinateSequence> ringPtr; // storage if we have to reverse
    if (Orientation::isCCW(ring)) {
        ringPtr = ring->clone();
        ringPtr->reverse();
        ringCW = ringPtr.get();
    }

    double diameter = ringCW->getEnvelope().getDiameter();
    //-- expand diameter for robustness
    double diamWithTolerance = 2 * diameter;

    //-- compute corner bisectors
    std::array<LineSegment, 4> bisector = computeBisectors(ringCW, diamWithTolerance);
    //-- compute nodes and find interior one farthest from sides
    double maxDist = -1;
    CoordinateXY center;
    CoordinateXY radius;
    for (std::size_t i = 0; i < 4; i++) {
        LineSegment& b1 = bisector[i];
        std::size_t i2 = (i + 1) % 4;
        LineSegment& b2 = bisector[i2];

        CoordinateXY nodePt = b1.intersection(b2);

        if (nodePt.isNull()) {
            continue;
        }

        //-- only interior nodes are considered
        if (! isPointInConvexRing(ringCW, nodePt)) {
            continue;
        }

        //-- check if node is further than current max center
        CoordinateXY r = nearestEdgePt(ringCW, nodePt);
        double dist = nodePt.distance(r);
        if (maxDist < 0 || dist > maxDist) {
            center = nodePt;
            radius = r;
            maxDist = dist;
        }
    }
    std::pair<CoordinateXY, CoordinateXY> result;
    result.first = center;
    result.second = radius;
    return result;
}

/* private static */
std::array<LineSegment, 4>
ExactMaxInscribedCircle::computeBisectors(
    const CoordinateSequence* ptsCW, double diameter)
{
    std::array<LineSegment, 4> bisector;
    for (std::size_t i = 0; i < 4; i++) {
        bisector[i] = computeConvexBisector(ptsCW, i, diameter);
    }
    return bisector;
}

/* private static */
CoordinateXY
ExactMaxInscribedCircle::nearestEdgePt(const CoordinateSequence* ring,
    const CoordinateXY& pt)
{
    CoordinateXY nearestPt;
    CoordinateXY r;
    double minDist = -1;
    for (std::size_t i = 0; i < ring->size() - 1; i++) {
        LineSegment edge(ring->getAt(i), ring->getAt(i + 1));
        edge.closestPoint(pt, r);
        double dist = pt.distance(r);
        if (minDist < 0 || dist < minDist) {
            minDist = dist;
            nearestPt = r;
        }
    }
    return nearestPt;
}

/* private static */
LineSegment
ExactMaxInscribedCircle::computeConvexBisector(
    const CoordinateSequence* pts, std::size_t index, double len)
{
    const CoordinateXY& basePt = pts->getAt(index);
    std::size_t iPrev = index == 0 ? pts->size() - 2 : index - 1;
    std::size_t iNext = index >= pts->size() ? 0 : index + 1;
    const CoordinateXY& pPrev = pts->getAt(iPrev);
    const CoordinateXY& pNext = pts->getAt(iNext);
    if (! isConvex(pPrev, basePt, pNext))
        throw util::IllegalArgumentException("Input is not convex");
    double bisectAng = Angle::bisector(pPrev, basePt, pNext);
    CoordinateXY endPt = Angle::project(basePt, bisectAng, len);
    return LineSegment(basePt.x, basePt.y, endPt.x, endPt.y);
}

/* private static */
bool
ExactMaxInscribedCircle::isConvex(const Polygon* polygon)
{
    const LinearRing* shell = polygon->getExteriorRing();
    return isConvex(shell->getCoordinatesRO());
}

/* private static */
bool
ExactMaxInscribedCircle::isConvex(const CoordinateSequence* ring)
{
    /**
     * A ring cannot be all concave, so if it has a consistent
     * orientation it must be convex.
     */
    std::size_t n = ring->size();
    if (n < 4)
        return false;
    int ringOrient = 0;
    for (std::size_t i = 0; i < n - 1; i++) {
        std::size_t i1 = i + 1;
        std::size_t i2 = (i1 >= n - 1) ? 1 : i1 + 1;
        int orient = Orientation::index(
            ring->getAt(i),
            ring->getAt(i1),
            ring->getAt(i2));
        if (orient == Orientation::COLLINEAR)
            continue;
        if (ringOrient == 0) {
            ringOrient = orient;
        }
        else if (orient != ringOrient) {
            return false;
        }
    }
    return true;
}

/* private static */
bool
ExactMaxInscribedCircle::isConvex(const CoordinateXY& p0,
                                  const CoordinateXY& p1,
                                  const CoordinateXY& p2)
{
    return Orientation::CLOCKWISE == Orientation::index(p0, p1, p2);
}

/* private static */
bool
ExactMaxInscribedCircle::isPointInConvexRing(const CoordinateSequence* ringCW,
                                             const CoordinateXY& p)
{
    for (std::size_t i = 0; i < ringCW->size() - 1; i++) {
        const CoordinateXY& p0 = ringCW->getAt(i);
        const CoordinateXY& p1 = ringCW->getAt(i + 1);
        int orient = Orientation::index(p0, p1, p);
        if (orient == Orientation::COUNTERCLOCKWISE)
            return false;
    }
    return true;
}


} // namespace geos.algorithm.construct
} // namespace geos.algorithm
} // namespace geos
