/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2022 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/TPVWSimplifier.h>
#include <geos/coverage/Corner.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>

#include <geos/simplify/LinkedLine.h>

using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::MultiLineString;
using geos::simplify::LinkedLine;


namespace geos {
namespace coverage { // geos.coverage


typedef TPVWSimplifier::Edge Edge;
typedef TPVWSimplifier::EdgeIndex EdgeIndex;


/* public static */
std::unique_ptr<MultiLineString>
TPVWSimplifier::simplify(
    const MultiLineString* lines,
    double distanceTolerance)
{
    TPVWSimplifier simp(lines, distanceTolerance);
    std::unique_ptr<MultiLineString> result = simp.simplify();
    return result;
}


/* public static */
std::unique_ptr<MultiLineString>
TPVWSimplifier::simplify(
    const MultiLineString* p_lines,
    std::vector<bool>& p_freeRings,
    const MultiLineString* p_constraintLines,
    double distanceTolerance)
{
    TPVWSimplifier simp(p_lines, distanceTolerance);
    simp.setFreeRingIndices(p_freeRings);
    simp.setConstraints(p_constraintLines);
    std::unique_ptr<MultiLineString> result = simp.simplify();
    return result;
}


/* public */
TPVWSimplifier::TPVWSimplifier(
    const MultiLineString* lines,
    double distanceTolerance)
    : inputLines(lines)
    , areaTolerance(distanceTolerance*distanceTolerance)
    , geomFactory(inputLines->getFactory())
    , constraintLines(nullptr)
    {}


/* private */
void
TPVWSimplifier::setConstraints(const MultiLineString* constraints)
{
    constraintLines = constraints;
}

/* public */
void
TPVWSimplifier::setFreeRingIndices(std::vector<bool>& freeRing)
{
    //XXX Assert: bit set has same size as number of lines.
    isFreeRing = freeRing;
}

/* private */
std::unique_ptr<MultiLineString>
TPVWSimplifier::simplify()
{
    std::vector<bool> emptyList;
    std::vector<Edge> edges = createEdges(inputLines, isFreeRing);
    std::vector<Edge> constraintEdges = createEdges(constraintLines, emptyList);

    EdgeIndex edgeIndex;
    edgeIndex.add(edges);
    edgeIndex.add(constraintEdges);

    std::vector<std::unique_ptr<LineString>> result;
    for (auto& edge : edges) {
        std::unique_ptr<CoordinateSequence> ptsSimp = edge.simplify(edgeIndex);
        auto ls = geomFactory->createLineString(std::move(ptsSimp));
        result.emplace_back(ls.release());
    }
    return geomFactory->createMultiLineString(std::move(result));
}

/* private */
std::vector<Edge>
TPVWSimplifier::createEdges(
    const MultiLineString* lines,
    std::vector<bool>& freeRing)
{
    std::vector<Edge> edges;

    if (lines == nullptr)
        return edges;

    for (std::size_t i = 0; i < lines->getNumGeometries(); i++) {
        const LineString* line = lines->getGeometryN(i);
        bool isFree = freeRing.empty() ? false : freeRing[i];
        edges.emplace_back(line, isFree, areaTolerance);
    }
    return edges;
}

/************************************************************************/

TPVWSimplifier::Edge::Edge(const LineString* p_inputLine, bool p_isFreeRing, double p_areaTolerance)
    : areaTolerance(p_areaTolerance)
    , isFreeRing(p_isFreeRing)
    , envelope(p_inputLine->getEnvelopeInternal())
    , nbPts(p_inputLine->getNumPoints())
    , linkedLine(*(p_inputLine->getCoordinatesRO()))
    , vertexIndex(*(p_inputLine->getCoordinatesRO()))
    , minEdgeSize(p_inputLine->getCoordinatesRO()->isRing() ? 3 : 2)
{
    // linkedLine = new LinkedLine(pts);
    // minEdgeSize = linkedLine.isRing() ? 3 : 2;
    // vertexIndex = new VertexSequencePackedRtree(pts);
    //-- remove ring duplicate final vertex
    if (linkedLine.isRing()) {
        vertexIndex.remove(nbPts-1);
    }
}

/* private */
const Coordinate&
TPVWSimplifier::Edge::getCoordinate(std::size_t index) const
{
    return linkedLine.getCoordinate(index);
}

/* public */
const Envelope*
TPVWSimplifier::Edge::getEnvelopeInternal() const
{
    return envelope;
}

/* public */
std::size_t
TPVWSimplifier::Edge::size() const
{
    return linkedLine.size();
}

/* private */
std::unique_ptr<CoordinateSequence>
TPVWSimplifier::Edge::simplify(EdgeIndex& edgeIndex)
{
    Corner::PriorityQueue cornerQueue;
    createQueue(cornerQueue);
    while (! cornerQueue.empty() && size() > minEdgeSize) {
        //Corner corner = cornerQueue.poll();
        Corner corner = cornerQueue.top();
        cornerQueue.pop();

        //-- a corner may no longer be valid due to removal of adjacent corners
        if (corner.isRemoved())
            continue;
        //System.out.println(corner.toLineString(edge));
        //-- done when all small corners are removed
        if (corner.getArea() > areaTolerance)
            break;
        if (isRemovable(corner, edgeIndex) ) {
            removeCorner(corner, cornerQueue);
        }
    }
    return linkedLine.getCoordinates();
}

/* private */
void
TPVWSimplifier::Edge::createQueue(Corner::PriorityQueue& cornerQueue)
{
    std::size_t minIndex = (linkedLine.isRing() && isFreeRing) ? 0 : 1;
    std::size_t maxIndex = nbPts - 1;
    for (std::size_t i = minIndex; i < maxIndex; i++) {
        addCorner(i, cornerQueue);
    }
    return;
}

/* private */
void
TPVWSimplifier::Edge::addCorner(
    std::size_t i,
    Corner::PriorityQueue& cornerQueue)
{
    if (isFreeRing || (i != 0 && i != nbPts-1)) {
        Corner corner(&linkedLine, i);
        if (corner.getArea() <= areaTolerance) {
            cornerQueue.push(corner);
        }
    }
}

/* private */
bool
TPVWSimplifier::Edge::isRemovable(
    Corner& corner,
    EdgeIndex& edgeIndex) const
{
    Envelope cornerEnv = corner.envelope();
    //-- check nearby lines for violating intersections
    //-- the query also returns this line for checking
    std::vector<const Edge*> edgeHits = edgeIndex.query(cornerEnv);
    for (const Edge* edge : edgeHits) {
        if (hasIntersectingVertex(corner, cornerEnv, *edge))
            return false;
    //-- check if corner base equals line (2-pts)
    //-- if so, don't remove corner, since that would collapse to the line
        if (edge != this && edge->size() == 2) {
            // TODO xxxxxx make linkedLine coordinates local
            // to linkedline and return a reference, update
            // simplify to clone reference at final step
            auto linePts = edge->linkedLine.getCoordinates();
            if (corner.isBaseline(linePts->getAt(0), linePts->getAt(1)))
                return false;
        }
    }
    return true;
}


/* private */
bool
TPVWSimplifier::Edge::hasIntersectingVertex(
    const Corner& corner,
    const Envelope& cornerEnv,
    const Edge& edge) const
{
    std::vector<std::size_t> result = edge.query(cornerEnv);
    for (std::size_t index : result) {

        const Coordinate& v = edge.getCoordinate(index);
        // ok if corner touches another line - should only happen at endpoints
        if (corner.isVertex(v))
            continue;

        //--- does corner triangle contain vertex?
        if (corner.intersects(v))
            return true;
    }
    return false;
}

/* private */
std::vector<std::size_t>
TPVWSimplifier::Edge::query(const Envelope& cornerEnv) const
{
    std::vector<std::size_t> result;
    vertexIndex.query(cornerEnv, result);
    return result;
}


/* private */
void
TPVWSimplifier::Edge::removeCorner(
    Corner& corner,
    Corner::PriorityQueue& cornerQueue)
{
    std::size_t index = corner.getIndex();
    std::size_t prev = linkedLine.prev(index);
    std::size_t next = linkedLine.next(index);
    linkedLine.remove(index);
    vertexIndex.remove(index);

    //-- potentially add the new corners created
    addCorner(prev, cornerQueue);
    addCorner(next, cornerQueue);
}

/************************************************************************/


/* public */
void
TPVWSimplifier::EdgeIndex::add(std::vector<Edge>& edges)
{
    for (Edge& edge : edges) {
        index.insert(&edge);
    }
}

/* public */
std::vector<const Edge*>
TPVWSimplifier::EdgeIndex::query(const Envelope& queryEnv)
{
    std::vector<const Edge*> hits;
    index.query(queryEnv, hits);
    return hits;
}



} // geos.coverage
} // geos
