/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/OverlayGraph.h>

#include <geos/algorithm/CircularArcs.h>
#include <geos/operation/overlayng/Edge.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CircularArc.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;

/**
* Creates a new graph for a set of noded, labelled {@link Edge}s.
*/
//std::vector<std::unique_ptr<Edge>> && edges
OverlayGraph::OverlayGraph()
{}

/*public*/
std::vector<OverlayEdge*>&
OverlayGraph::getEdges()
{
    return edges;
}

/*public*/
std::vector<OverlayEdge*>
OverlayGraph::getNodeEdges()
{
    std::vector<OverlayEdge*> nodeEdges;
    nodeEdges.reserve(nodeMap.size());
    for (const auto& nodeMapPair : nodeMap) {
        nodeEdges.push_back(nodeMapPair.second);
    }
    return nodeEdges;
}

/*public*/
OverlayEdge*
OverlayGraph::getNodeEdge(const Coordinate& nodePt) const
{
    const auto& it = nodeMap.find(nodePt);
    if (it == nodeMap.end()) {
        return nullptr;
    }
    return it->second;
}

/*public*/
std::vector<OverlayEdge*>
OverlayGraph::getResultAreaEdges()
{
    std::vector<OverlayEdge*> resultEdges;
    for (OverlayEdge* edge : getEdges()) {
        if (edge->isInResultArea()) {
            resultEdges.push_back(edge);
        }
    }
    return resultEdges;
}

/*public*/
OverlayEdge*
OverlayGraph::addEdge(Edge* edge)
{
    // CoordinateSequence* pts = = edge->getCoordinates().release();
    std::shared_ptr<const CoordinateSequence> pts = edge->releaseCoordinates();
    OverlayEdge* e = createEdgePair(pts, createOverlayLabel(edge), edge->isCurved());
#if GEOS_DEBUG
    std::cerr << "added edge: " << *e << std::endl;
#endif
    insert(e);
    insert(e->symOE());
    return e;
}

/*private*/
OverlayEdge*
OverlayGraph::createEdgePair(const std::shared_ptr<const CoordinateSequence>& pts, OverlayLabel *lbl, bool isCurved)
{
    OverlayEdge* e0 = createOverlayEdge(pts, lbl, true, isCurved);
    OverlayEdge* e1 = createOverlayEdge(pts, lbl, false, isCurved);
    e0->link(e1);
    return e0;
}

static CoordinateXY
getDirectionPoint(const CoordinateSequence& pts, bool forward, bool isCurved)
{
    if (isCurved) {
        assert(pts.size() >= 3);
        if (forward) {
            CircularArc arc(pts, 0);
            return arc.getDirectionPoint();
        } else {
            CircularArc arc(pts, pts.size() - 3);
            return algorithm::CircularArcs::getDirectionPoint(arc.getCenter(), arc.getRadius(), arc.theta2(), !arc.isCCW());
        }
    }

    assert(pts.size() >= 2);
    if (forward) {
        return pts.getAt<CoordinateXY>(1);
    }

    return pts.getAt<CoordinateXY>(pts.size() - 2);
}

/*private*/
OverlayEdge*
OverlayGraph::createOverlayEdge(const std::shared_ptr<const CoordinateSequence>& pts, OverlayLabel* lbl, bool direction, bool isCurved)
{
    assert(pts->size() >= 2);

    CoordinateXYZM origin;
    const CoordinateXY dirPt = getDirectionPoint(*pts, direction, isCurved);

    if (direction) {
        pts->getAt(0, origin);
    }
    else {
        pts->getAt(pts->size() - 1, origin);
    }
    ovEdgeQue.emplace_back(origin, dirPt, direction, lbl, pts, isCurved);
    OverlayEdge& ove = ovEdgeQue.back();
    return &ove;
}

/*public*/
OverlayLabel*
OverlayGraph::createOverlayLabel(const Edge* edge)
{
    // Instantiate OverlayLabel on the std::deque
    ovLabelQue.emplace_back();
    // Read back a reference
    OverlayLabel& ovl = ovLabelQue.back();
    // Initialize the reference with values from edge
    edge->populateLabel(ovl);
    // Return as pointer.
    return &ovl;
}

/*private*/
void
OverlayGraph::insert(OverlayEdge* e)
{
    edges.push_back(e);

    /**
     * If the edge origin node is already in the graph,
     * insert the edge into the star of edges around the node.
     * Otherwise, add a new node for the origin.
     */
    const auto& it = nodeMap.find(e->orig());
    if (it != nodeMap.end()) {
        // found in map
        OverlayEdge* nodeEdge = it->second;
        nodeEdge->insert(e);
    }
    else {
        nodeMap[e->orig()] = e;
    }
}

/*public friend*/
std::ostream&
operator<<(std::ostream& os, const OverlayGraph& og)
{
    os << "OGRPH " << std::endl << "NODEMAP [" << og.nodeMap.size() << "]";
    // pair<Coordinate, OverlayEdge&>
    for (const auto& pr: og.nodeMap) {
        os << std::endl << " ";
        os << pr.first << " ";
        os << *(pr.second);
    }
    os << std::endl;
    os << "EDGES [" << og.edges.size() << "]";
    // pair<Coordinate, OverlayEdge&>
    for (const auto& e: og.edges) {
        os << std::endl << " ";
        os << *e << " ";
    }
    os << std::endl;
    return os;
}




} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
