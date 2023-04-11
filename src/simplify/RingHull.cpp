/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/simplify/RingHull.h>
#include <geos/simplify/LinkedRing.h>
#include <geos/simplify/RingHullIndex.h>

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Triangle.h>
#include <geos/index/VertexSequencePackedRtree.h>
#include <geos/util.h>

using geos::algorithm::Orientation;
using geos::geom::Envelope;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::geom::Triangle;
using geos::index::VertexSequencePackedRtree;

namespace geos {
namespace simplify { // geos.simplify



/*
* Creates a new instance.
*
* @param ring the ring vertices to process
* @param isOuter whether the hull is outer or inner
*/
RingHull::RingHull(const LinearRing* p_ring, bool p_isOuter)
    : inputRing(p_ring)
    , vertex(inputRing->getCoordinates())
{
    init(*vertex, p_isOuter);
}

/* public */
void
RingHull::setMinVertexNum(std::size_t minVertexNum)
{
    targetVertexNum = static_cast<double>(minVertexNum);
}

/* public */
void
RingHull::setMaxAreaDelta(double maxAreaDelta)
{
    targetAreaDelta = maxAreaDelta;
}

/* public */
const Envelope*
RingHull::getEnvelope() const
{
    return inputRing->getEnvelopeInternal();
}

/* public */
std::unique_ptr<LinearRing>
RingHull::getHull(RingHullIndex& hullIndex)
{
    compute(hullIndex);
    std::unique_ptr<CoordinateSequence> hullPts = vertexRing->getCoordinates();
    // std::vector<Coordinate> hullPts;
    // std::copy(vrCoords.begin(), vrCoords.end(), hullPts.back_inserter());
    return inputRing->getFactory()->createLinearRing(std::move(hullPts));
}

/* private */
void
RingHull::init(CoordinateSequence& ring, bool isOuter)
{
    /**
     * Ensure ring is oriented according to outer/inner:
     * - outer, CW
     * - inner: CCW
     */
    bool orientCW = isOuter;
    if (orientCW == Orientation::isCCW(inputRing->getCoordinatesRO()))
    {
        ring.reverse();
    }

    vertexRing.reset(new LinkedRing(ring));
    vertexIndex.reset(new VertexSequencePackedRtree(ring));

    //-- remove duplicate final vertex
    vertexIndex->remove(ring.size() - 1);

    for (std::size_t i = 0; i < vertexRing->size(); i++) {
        addCorner(i, cornerQueue);
    }
}


/* private */
void
RingHull::addCorner(std::size_t i, Corner::PriorityQueue& queue)
{
    //-- convex corners are left untouched
    if (isConvex(*vertexRing, i))
      return;
    //-- corner is concave or flat - both can be removed
    Corner corner (i,
        vertexRing->prev(i),
        vertexRing->next(i),
        area(*vertexRing, i));
    queue.push(corner);
}

/* public static */
bool
RingHull::isConvex(const LinkedRing& vertexRing, std::size_t index)
{
    const Coordinate& pp = vertexRing.prevCoordinate(index);
    const Coordinate& p = vertexRing.getCoordinate(index);
    const Coordinate& pn = vertexRing.nextCoordinate(index);
    return Orientation::CLOCKWISE == Orientation::index(pp, p, pn);
}

/* public static */
double
RingHull::area(const LinkedRing& vertexRing, std::size_t index)
{
    const Coordinate& pp = vertexRing.prevCoordinate(index);
    const Coordinate& p = vertexRing.getCoordinate(index);
    const Coordinate& pn = vertexRing.nextCoordinate(index);
    return Triangle::area(pp, p, pn);
}

/* public */
void
RingHull::compute(RingHullIndex& hullIndex)
{
    while (! cornerQueue.empty()
        && vertexRing->size() > 3)
    {
        Corner corner = cornerQueue.top();
        cornerQueue.pop();
        //-- a corner may no longer be valid due to removal of adjacent corners
        if (corner.isRemoved(*vertexRing))
            continue;
        if (isAtTarget(corner)) {
            return;
        }
        //System.out.println(corner.toLineString(vertexList));
        /**
        * Corner is concave or flat - remove it if possible.
        */
        if (isRemovable(corner, hullIndex)) {
            removeCorner(corner, cornerQueue);
        }
    }
}

/* private */
bool
RingHull::isAtTarget(const Corner& corner)
{
    if (targetVertexNum >= 0) {
        double dVertexRingSize = static_cast<double>(vertexRing->size());
        return dVertexRingSize < targetVertexNum;
    }
    if (targetAreaDelta >= 0) {
        //-- include candidate corder to avoid overshooting target
        // (important for very small target area deltas)
        return areaDelta + corner.getArea() > targetAreaDelta;
    }
    //-- no target set
    return true;
}

/**
* Removes a corner by removing the apex vertex from the ring.
* Two new corners are created with apexes
* at the other vertices of the corner
* (if they are non-convex and thus removable).
*
* @param corner the corner to remove
* @param cornerQueue the corner queue
*/
/* private */
void
RingHull::removeCorner(const Corner& corner, Corner::PriorityQueue& queue)
{
    std::size_t index = corner.getIndex();
    std::size_t prev = vertexRing->prev(index);
    std::size_t next = vertexRing->next(index);
    vertexRing->remove(index);
    vertexIndex->remove(index);
    areaDelta += corner.getArea();

    //-- potentially add the new corners created
    addCorner(prev, queue);
    addCorner(next, queue);
}

/* private */
bool
RingHull::isRemovable(const Corner& corner, const RingHullIndex& hullIndex) const
{
    Envelope cornerEnv;
    corner.envelope(*vertexRing, cornerEnv);
    if (hasIntersectingVertex(corner, cornerEnv, this))
        return false;
    //-- no other rings to check
    if (hullIndex.size() == 0)
        return true;
    //-- check other rings for intersections
    std::vector<const RingHull*> queryResult = hullIndex.query(cornerEnv);
    for (const RingHull* hull : queryResult) {
      //-- this hull was already checked above
        if (hull == this)
            continue;
        if (hasIntersectingVertex(corner, cornerEnv, hull))
            return false;
    }
    return true;
}

/**
* Tests if any vertices in a hull intersect the corner triangle.
* Uses the vertex spatial index for efficiency.
*
* @param corner the corner vertices
* @param cornerEnv the envelope of the corner
* @param hull the hull to test
* @return true if there is an intersecting vertex
*/
/* private */
bool
RingHull::hasIntersectingVertex(
    const Corner& corner,
    const Envelope& cornerEnv,
    const RingHull* hull) const
{
    std::vector<std::size_t> result;
    hull->query(cornerEnv, result);
    for (std::size_t index : result)
    {
        //-- skip vertices of corner
        if (hull == this && corner.isVertex(index))
            continue;

        const Coordinate& v = hull->getCoordinate(index);
        //--- does corner triangle contain vertex?
        if (corner.intersects(v, *vertexRing))
            return true;
    }
    return false;
}

/* private */
const Coordinate&
RingHull::getCoordinate(std::size_t index) const
{
    return vertexRing->getCoordinate(index);
}


/* private */
void
RingHull::query(
    const Envelope& cornerEnv,
    std::vector<std::size_t>& result) const
{
    vertexIndex->query(cornerEnv, result);
}


void
RingHull::queryHull(const Envelope& queryEnv, std::vector<Coordinate>& pts)
{
    std::vector<std::size_t> result;
    vertexIndex->query(queryEnv, result);

    for (std::size_t index : result) {
        //-- skip if already removed
        if (! vertexRing->hasCoordinate(index))
            continue;
        const Coordinate& v = vertexRing->getCoordinate(index);
        pts.push_back(v);
    }
}

/* public */
std::unique_ptr<Polygon>
RingHull::toGeometry() const
{
    auto fact = GeometryFactory::create();
    std::unique_ptr<CoordinateSequence> coords = vertexRing->getCoordinates();
    // std::vector<Coordinate> coordCopy;
    // std::copy(coords.begin(), coords.end(), coordCopy.back_inserter());
    return fact->createPolygon(fact->createLinearRing(std::move(coords)));
}



// ------- Corner ------------------------------------------

/* public */
bool
RingHull::Corner::isVertex(std::size_t p_index) const
{
    return p_index == index
        || p_index == prev
        || p_index == next;
}

/* public */
std::size_t
RingHull::Corner::getIndex() const
{
    return index;
}

/* public */
double
RingHull::Corner::getArea() const
{
    return area;
}


/* public */
void
RingHull::Corner::envelope(const LinkedRing& ring, Envelope& env) const
{
    const Coordinate& pp = ring.getCoordinate(prev);
    const Coordinate& p = ring.getCoordinate(index);
    const Coordinate& pn = ring.getCoordinate(next);
    env.init(pp, pn);
    env.expandToInclude(p);
    return;
}

/* public */
bool
RingHull::Corner::intersects(const Coordinate& v, const LinkedRing& ring) const
{
    const Coordinate& pp = ring.getCoordinate(prev);
    const Coordinate& p = ring.getCoordinate(index);
    const Coordinate& pn = ring.getCoordinate(next);
    return Triangle::intersects(pp, p, pn, v);
}

/* public */
bool
RingHull::Corner::isRemoved(const LinkedRing& ring) const
{
    return ring.prev(index) != prev
        || ring.next(index) != next;
}

/* public */
std::unique_ptr<LineString>
RingHull::Corner::toLineString(const LinkedRing& ring)
{
    auto coords = detail::make_unique<CoordinateSequence>();
    coords->add(ring.getCoordinate(prev));
    coords->add(ring.getCoordinate(index));
    coords->add(ring.getCoordinate(next));
    auto gfact = GeometryFactory::create();
    return gfact->createLineString(std::move(coords));
}


} // namespace geos.simplify
} // namespace geos
