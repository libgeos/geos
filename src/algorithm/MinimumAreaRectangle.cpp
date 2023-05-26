/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/MinimumAreaRectangle.h>
#include <geos/algorithm/ConvexHull.h>
#include <geos/algorithm/Rectangle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/constants.h>

using namespace geos::geom;


namespace geos {
namespace algorithm { // geos.algorithm


/* public static */
std::unique_ptr<Geometry>
MinimumAreaRectangle::getMinimumRectangle(const Geometry* geom)
{
    MinimumAreaRectangle mar(geom);
    return mar.getMinimumRectangle();
}


/* private */
std::unique_ptr<Geometry>
MinimumAreaRectangle::getMinimumRectangle()
{
    if (m_inputGeom->isEmpty()) {
        return m_inputGeom->getFactory()->createPolygon();
    }
    if (m_isConvex) {
        return computeConvex(m_inputGeom);
    }
    ConvexHull cvh(m_inputGeom);
    std::unique_ptr<Geometry> convexGeom = cvh.getConvexHull();

    return computeConvex(convexGeom.get());
}


/* private */
std::unique_ptr<Geometry>
MinimumAreaRectangle::computeConvex(const Geometry* convexGeom)
{
    const CoordinateSequence* convexHullPts;
    switch (convexGeom->getGeometryTypeId()) {
    case GEOS_POLYGON:
    {
        auto poly = static_cast<const Polygon*>(convexGeom);
        convexHullPts = poly->getExteriorRing()->getCoordinatesRO();
        break;
    }
    case GEOS_LINESTRING:
    {
        auto line = static_cast<const LineString*>(convexGeom);
        convexHullPts = line->getCoordinatesRO();
        break;
    }
    case GEOS_POINT:
    {
        auto pt = static_cast<const Point*>(convexGeom);
        convexHullPts = pt->getCoordinatesRO();
        break;
    }
    default:
        throw util::IllegalArgumentException("computeConvex called with unsupported geometry type");
    }

    // special cases for lines or points or degenerate rings
    switch (convexHullPts->size())
    {
    case 1:
        return m_inputGeom->getFactory()->createPoint(convexHullPts->getAt<CoordinateXY>(0));
    case 2:
    case 3:
        return computeMaximumLine(convexHullPts, m_inputGeom->getFactory());
    default:
        // TODO: ensure ring is CW
        return computeConvexRing(convexHullPts);
    }
}


/* private */
std::unique_ptr<Polygon>
MinimumAreaRectangle::computeConvexRing(const CoordinateSequence* ring)
{
    // Assert: ring is oriented CW

    double minRectangleArea = DoubleMax;
    std::size_t minRectangleBaseIndex = NO_COORD_INDEX;
    std::size_t minRectangleDiamIndex = NO_COORD_INDEX;
    std::size_t minRectangleLeftIndex = NO_COORD_INDEX;
    std::size_t minRectangleRightIndex = NO_COORD_INDEX;

    //-- start at vertex after first one
    std::size_t diameterIndex = 1;
    std::size_t leftSideIndex = 1;
    std::size_t rightSideIndex = NO_COORD_INDEX; // initialized once first diameter is found

    LineSegment segBase;
    LineSegment segDiam;
    // for each segment, find the next vertex which is at maximum distance
    for (std::size_t i = 0; i < ring->size() - 1; i++) {
        segBase.p0 = ring->getAt<CoordinateXY>(i);
        segBase.p1 = ring->getAt<CoordinateXY>(i + 1);
        diameterIndex = findFurthestVertex(ring, segBase, diameterIndex, 0);

        const CoordinateXY& diamPt = ring->getAt<CoordinateXY>(diameterIndex);
        CoordinateXY diamBasePt = segBase.project(diamPt);
        segDiam.p0 = diamBasePt;
        segDiam.p1 = diamPt;

        leftSideIndex = findFurthestVertex(ring, segDiam, leftSideIndex, 1);

        //-- init the max right index
        if (i == 0) {
            rightSideIndex = diameterIndex;
        }
        rightSideIndex = findFurthestVertex(ring, segDiam, rightSideIndex, -1);

        double rectWidth = segDiam.distancePerpendicular(ring->getAt<CoordinateXY>(leftSideIndex))
                         + segDiam.distancePerpendicular(ring->getAt<CoordinateXY>(rightSideIndex));
        double rectArea = segDiam.getLength() * rectWidth;

        if (rectArea < minRectangleArea) {
            minRectangleArea = rectArea;
            minRectangleBaseIndex = i;
            minRectangleDiamIndex = diameterIndex;
            minRectangleLeftIndex = leftSideIndex;
            minRectangleRightIndex = rightSideIndex;
        }
    }
    return Rectangle::createFromSidePts(
        ring->getAt<CoordinateXY>(minRectangleBaseIndex),
        ring->getAt<CoordinateXY>(minRectangleBaseIndex + 1),
        ring->getAt<CoordinateXY>(minRectangleDiamIndex),
        ring->getAt<CoordinateXY>(minRectangleLeftIndex),
        ring->getAt<CoordinateXY>(minRectangleRightIndex),
        m_inputGeom->getFactory());
}


/* private */
std::size_t
MinimumAreaRectangle::findFurthestVertex(
    const CoordinateSequence* pts,
    const LineSegment& baseSeg,
    std::size_t startIndex, int orient)
{
    double maxDistance = orientedDistance(baseSeg, pts->getAt<CoordinateXY>(startIndex), orient);
    double nextDistance = maxDistance;
    std::size_t maxIndex = startIndex;
    std::size_t nextIndex = maxIndex;
    //-- rotate "caliper" while distance from base segment is non-decreasing
    while (isFurtherOrEqual(nextDistance, maxDistance, orient)) {
        maxDistance = nextDistance;
        maxIndex = nextIndex;

        nextIndex = getNextIndex(pts, maxIndex);
        if (nextIndex == startIndex)
            break;
        nextDistance = orientedDistance(baseSeg, pts->getAt<CoordinateXY>(nextIndex), orient);
    }
    return maxIndex;
}


/* private */
bool
MinimumAreaRectangle::isFurtherOrEqual(
    double d1, double d2,
    int orient)
{
    switch (orient) {
        case 0: return std::abs(d1) >= std::abs(d2);
        case 1: return d1 >= d2;
        case -1: return d1 <= d2;
    }
    throw util::IllegalArgumentException("Invalid orientation index");
}


/* private static */
double
MinimumAreaRectangle::orientedDistance(
    const LineSegment& seg,
    const CoordinateXY& p,
    int orient)
{
    double dist = seg.distancePerpendicularOriented(p);
    if (orient == 0) {
        return std::abs(dist);
    }
    return dist;
}


/* private static */
std::size_t
MinimumAreaRectangle::getNextIndex(
    const CoordinateSequence* ring,
    std::size_t index)
{
    if (index == NO_COORD_INDEX) index = 0;
    else index = index + 1;

    if (index >= ring->size() - 1)
        index = 0;
    return index;
}


/* private static */
std::unique_ptr<LineString>
MinimumAreaRectangle::computeMaximumLine(
    const CoordinateSequence* pts,
    const GeometryFactory* factory)
{
    //-- find max and min pts for X and Y
    CoordinateXY ptMinX; ptMinX.setNull();
    CoordinateXY ptMaxX; ptMaxX.setNull();
    CoordinateXY ptMinY; ptMinY.setNull();
    CoordinateXY ptMaxY; ptMaxY.setNull();
    for (const CoordinateXY& p : pts->items<CoordinateXY>()) {
        if (ptMinX.isNull() || p.x < ptMinX.x) ptMinX = p;
        if (ptMaxX.isNull() || p.x > ptMaxX.x) ptMaxX = p;
        if (ptMinY.isNull() || p.y < ptMinY.y) ptMinY = p;
        if (ptMaxY.isNull() || p.y > ptMaxY.y) ptMaxY = p;
    }

    CoordinateXY p0 = ptMinX;
    CoordinateXY p1 = ptMaxX;

    //-- line is vertical - use Y pts
    if (p0.x == p1.x) {
        p0 = ptMinY;
        p1 = ptMaxY;
    }
    CoordinateSequence cs({ p0, p1 });
    return factory->createLineString(std::move(cs));
}



} // namespace geos.algorithm
} // namespace geos
