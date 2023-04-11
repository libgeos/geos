/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2023 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <map>

#include <geos/coverage/CoverageBoundarySegmentFinder.h>
#include <geos/coverage/CoverageEdge.h>
#include <geos/coverage/CoverageRingEdges.h>
#include <geos/coverage/VertexCounter.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/util/IllegalStateException.h>
#include <geos/constants.h>


using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineSegment;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::geom::MultiPolygon;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public */
std::vector<CoverageEdge*>
CoverageRingEdges::selectEdges(std::size_t ringCount) const
{
    std::vector<CoverageEdge*> result;
    for (CoverageEdge* edge : m_edges) {
        if (edge->getRingCount() == ringCount) {
            result.push_back(edge);
        }
    }
    return result;
}


/* private */
void
CoverageRingEdges::build()
{
    Coordinate::UnorderedSet nodes = findNodes(m_coverage);
    LineSegment::UnorderedSet boundarySegs = CoverageBoundarySegmentFinder::findBoundarySegments(m_coverage);
    Coordinate::UnorderedSet boundaryNodes = findBoundaryNodes(boundarySegs);
    nodes.insert(boundaryNodes.begin(), boundaryNodes.end());

    std::map<LineSegment, CoverageEdge*> uniqueEdgeMap;
    for (const Geometry* geom : m_coverage) {
        for (std::size_t ipoly = 0; ipoly < geom->getNumGeometries(); ipoly++) {
            const Polygon* poly = static_cast<const Polygon*>(geom->getGeometryN(ipoly));
            //-- extract shell
            const LinearRing* shell = poly->getExteriorRing();
            addRingEdges(shell, nodes, boundarySegs, uniqueEdgeMap);
            //-- extract holes
            for (std::size_t ihole = 0; ihole < poly->getNumInteriorRing(); ihole++) {
                const LinearRing* hole = poly->getInteriorRingN(ihole);
                addRingEdges(hole, nodes, boundarySegs, uniqueEdgeMap);
            }
        }
    }
}

/* private */
void
CoverageRingEdges::addRingEdges(
    const LinearRing* ring,
    Coordinate::UnorderedSet& nodes,
    LineSegment::UnorderedSet& boundarySegs,
    std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap)
{
    addBoundaryNodes(ring, boundarySegs, nodes);
    std::vector<CoverageEdge*> ringEdges = extractRingEdges(ring, uniqueEdgeMap, nodes);
    m_ringEdgesMap[ring] = ringEdges;
}

/* private */
void
CoverageRingEdges::addBoundaryNodes(
    const LinearRing* ring,
    LineSegment::UnorderedSet& boundarySegs,
    Coordinate::UnorderedSet& nodes)
{
    const CoordinateSequence* seq = ring->getCoordinatesRO();
    bool isBdyLast = CoverageBoundarySegmentFinder::isBoundarySegment(boundarySegs, seq, seq->size() - 2);
    bool isBdyPrev = isBdyLast;
    for (std::size_t i = 0; i < seq->size() - 1; i++) {
        bool isBdy = CoverageBoundarySegmentFinder::isBoundarySegment(boundarySegs, seq, i);
        if (isBdy != isBdyPrev) {
            const Coordinate& nodePt = seq->getAt(i);
            nodes.insert(nodePt);
        }
        isBdyPrev = isBdy;
    }
}

/* private */
std::vector<CoverageEdge*>
CoverageRingEdges::extractRingEdges(
    const LinearRing* ring,
    std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap,
    Coordinate::UnorderedSet& nodes)
{
    std::vector<CoverageEdge*> ringEdges;
    std::size_t first = findNextNodeIndex(ring, NO_COORD_INDEX, nodes);
    if (first == NO_COORD_INDEX) {
        //-- ring does not contain a node, so edge is entire ring
        CoverageEdge* edge = createEdge(ring, uniqueEdgeMap);
        ringEdges.push_back(edge);
    }
    else {
        std::size_t start = first;
        std::size_t end = start;
        do {
            end = findNextNodeIndex(ring, start, nodes);
            CoverageEdge* edge = createEdge(ring, start, end, uniqueEdgeMap);
            ringEdges.push_back(edge);
            start = end;
        } while (end != first);
    }
    return ringEdges;
}

/* private */
CoverageEdge*
CoverageRingEdges::createEdge(
    const LinearRing* ring,
    std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap)
{
    CoverageEdge* edge;
    LineSegment edgeKey = CoverageEdge::key(ring);
    auto result = uniqueEdgeMap.find(edgeKey);
    if (result != uniqueEdgeMap.end()) {
        edge = result->second;
    }
    // if (uniqueEdgeMap.containsKey(edgeKey)) {
    //   edge = uniqueEdgeMap.get(edgeKey);
    // }
    else {
        std::unique_ptr<CoverageEdge> edge_ptr = CoverageEdge::createEdge(ring);
        edge = edge_ptr.release();
        m_edgeStore.emplace_back(edge);
        m_edges.push_back(edge);
        uniqueEdgeMap[edgeKey] = edge;
    }
    edge->incRingCount();
    return edge;
}

/* private */
CoverageEdge*
CoverageRingEdges::createEdge(
    const LinearRing* ring,
    std::size_t start, std::size_t end,
    std::map<LineSegment, CoverageEdge*>& uniqueEdgeMap)
{
    CoverageEdge* edge;
    LineSegment edgeKey = (end == start) ? CoverageEdge::key(ring) : CoverageEdge::key(ring, start, end);
    // if (uniqueEdgeMap.containsKey(edgeKey)) {
    //     edge = uniqueEdgeMap.get(edgeKey);
    // }
    auto result = uniqueEdgeMap.find(edgeKey);
    if (result != uniqueEdgeMap.end()) {
        edge = result->second;
    }
    else {
        std::unique_ptr<CoverageEdge> edge_ptr = CoverageEdge::createEdge(ring, start, end);
        edge = edge_ptr.release();
        m_edgeStore.emplace_back(edge);
        m_edges.push_back(edge);
        uniqueEdgeMap[edgeKey] = edge;
    }
    edge->incRingCount();
    return edge;
}

/* private */
std::size_t
CoverageRingEdges::findNextNodeIndex(
    const LinearRing* ring,
    std::size_t start,
    Coordinate::UnorderedSet& nodes) const
{
    std::size_t index = start;
    bool isScanned0 = false;
    do {
        index = next(index, ring);
        if (index == 0) {
            if (start == NO_COORD_INDEX && isScanned0) {
                return NO_COORD_INDEX;
            }
            isScanned0 = true;
        }
        const Coordinate& pt = ring->getCoordinatesRO()->getAt(index);
        if (nodes.find(pt) != nodes.end()) {
            return index;
        }
    } while (index != start);
    return NO_COORD_INDEX;
}

/* private static */
std::size_t
CoverageRingEdges::next(std::size_t index, const LinearRing* ring)
{
    if (index == NO_COORD_INDEX) return 0;
    index = index + 1;
    if (index >= ring->getNumPoints() - 1)
        index = 0;
    return index;
}


/* private */
Coordinate::UnorderedSet
CoverageRingEdges::findNodes(std::vector<const Geometry*>& coverage)
{
    std::map<Coordinate, std::size_t> vertexCount;
    VertexCounter::count(coverage, vertexCount);
    Coordinate::UnorderedSet nodes;
    // for (Coordinate v : vertexCount.keySet()) {
    //     if (vertexCount.get(v) > 2) {
    //         nodes.add(v);
    //     }
    // }
    for (const auto &mapPair : vertexCount) {
        const Coordinate& v = mapPair.first;
        std::size_t count = mapPair.second;
        if (count > 2)
            nodes.insert(v);
    }
    return nodes;
}


/* private */
Coordinate::UnorderedSet
CoverageRingEdges::findBoundaryNodes(LineSegment::UnorderedSet& lineSegments)
{
    std::map<Coordinate, std::size_t> counter;
    for (const LineSegment& line : lineSegments) {
        // counter.put(line.p0, counter.getOrDefault(line.p0, 0) + 1);
        // counter.put(line.p1, counter.getOrDefault(line.p1, 0) + 1);
        auto search0 = counter.find(line.p0);
        if (search0 != counter.end()) {
            counter[line.p0] = search0->second + 1;
        }
        else {
            counter[line.p0] = 0;
        }

        auto search1 = counter.find(line.p1);
        if (search1 != counter.end()) {
            counter[line.p1] = search1->second + 1;
        }
        else {
            counter[line.p1] = 0;
        }
    }

    Coordinate::UnorderedSet nodes;
    for (const auto& c : counter) {
        const Coordinate& v = c.first;
        std::size_t count = c.second;
        if (count > 2)
            nodes.insert(v);
    }
    return nodes;

    // return counter.entrySet().stream()
    //     .filter(e->e.getValue()>2)
    //     .map(Map.Entry::getKey).collect(Collectors.toSet());
}


/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageRingEdges::buildCoverage() const
{
    std::vector<std::unique_ptr<Geometry>> result;
    for (const Geometry* geom : m_coverage) {
        result.push_back(buildPolygonal(geom));
    }
    return result;
}

/* private */
std::unique_ptr<Geometry>
CoverageRingEdges::buildPolygonal(const Geometry* geom) const
{
    if (geom->getGeometryTypeId() == geom::GEOS_MULTIPOLYGON) {
        return buildMultiPolygon(static_cast<const MultiPolygon*>(geom));
    }
    else {
        return buildPolygon(static_cast<const Polygon*>(geom));
    }
}

/* private */
std::unique_ptr<Geometry>
CoverageRingEdges::buildMultiPolygon(const MultiPolygon* geom) const
{
    // Polygon[] polys = new Polygon[geom.getNumGeometries()];
    std::vector<std::unique_ptr<Polygon>> polys;
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        const Polygon* poly = static_cast<const Polygon*>(geom->getGeometryN(i));
        polys.push_back(buildPolygon(poly));
    }
    return geom->getFactory()->createMultiPolygon(std::move(polys));
}

/* private */
std::unique_ptr<Polygon>
CoverageRingEdges::buildPolygon(const Polygon* polygon) const
{
    std::size_t numRings = polygon->getNumInteriorRing();
    std::unique_ptr<LinearRing> shell = buildRing(polygon->getExteriorRing());
    if (numRings == 0) {
        return polygon->getFactory()->createPolygon(std::move(shell));
    }
    std::vector<std::unique_ptr<LinearRing>> holes;
    for (std::size_t i = 0; i < numRings; i++) {
        const LinearRing* hole = polygon->getInteriorRingN(i);
        auto newHole = buildRing(hole);
        holes.emplace_back(newHole.release());
    }
    return polygon->getFactory()->createPolygon(std::move(shell), std::move(holes));
}


/* private */
std::unique_ptr<LinearRing>
CoverageRingEdges::buildRing(const LinearRing* ring) const
{
    const std::vector<CoverageEdge*>* ringEdges;

    // List<CoverageEdge> ringEdges = m_ringEdgesMap.get(ring);
    auto result = m_ringEdgesMap.find(ring);
    if (result == m_ringEdgesMap.end()) {
        // return nullptr;
        throw util::IllegalStateException("buildRing");
    }
    else {
        ringEdges = &(result->second);
    }

    // CoordinateList ptsList = new CoordinateList();
    std::unique_ptr<CoordinateSequence> pts(new CoordinateSequence());
    Coordinate nullPt = Coordinate::getNull();
    for (std::size_t i = 0; i < ringEdges->size(); i++) {
        Coordinate& lastPt = pts->isEmpty() ? nullPt : pts->back();
        bool dir = isEdgeDirForward(*ringEdges, i, lastPt);
        const CoordinateSequence* ringCs = ringEdges->at(i)->getCoordinates();
        pts->add(*ringCs, false, dir);
    }
    return ring->getFactory()->createLinearRing(std::move(pts));
}

/* private */
bool
CoverageRingEdges::isEdgeDirForward(
    const std::vector<CoverageEdge*>& ringEdges,
    std::size_t index,
    const Coordinate& prevPt) const
{
    std::size_t size = ringEdges.size();
    if (size <= 1) return true;
    if (index == 0) {
        //-- if only 2 edges, first one can keep orientation
        if (size == 2)
            return true;
        const Coordinate& endPt0 = ringEdges[0]->getEndCoordinate();
        return endPt0.equals2D(ringEdges[1]->getStartCoordinate())
            || endPt0.equals2D(ringEdges[1]->getEndCoordinate());
    }
    //-- previous point determines required orientation
    return prevPt.equals2D(ringEdges[index]->getStartCoordinate());
}



} // namespace geos.coverage
} // namespace geos


