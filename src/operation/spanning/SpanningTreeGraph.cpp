/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/spanning/SpanningTreeGraph.h>
#include <geos/operation/spanning/SpanningTreeEdge.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>

using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

SpanningTreeGraph::~SpanningTreeGraph()
{
    // PlanarGraph doesn't seem to own the objects, so we must delete them.
    for (auto* de : newDirEdges) delete de;
    for (auto* e : newEdges) delete e;
    for (auto* n : newNodes) delete n;
}

Node*
SpanningTreeGraph::getNode(const Coordinate& coordinate)
{
    Node* node = findNode(coordinate);
    if (node == nullptr) {
        node = new Node(coordinate);
        add(node);
        newNodes.push_back(node);
    }
    return node;
}

void
SpanningTreeGraph::addEdge(const LineString* lineString, std::size_t index)
{
    if (lineString->isEmpty()) return;
    const CoordinateSequence* coordinates = lineString->getCoordinatesRO();
    if (coordinates->size() <= 1) return;

    Coordinate startCoord = coordinates->getAt(0);
    Coordinate endCoord = coordinates->getAt(coordinates->size() - 1);

    Node* startNode = getNode(startCoord);
    Node* endNode = getNode(endCoord);

    // Create DirectedEdges
    DirectedEdge* dirEdge0 = new DirectedEdge(startNode, endNode, coordinates->getAt(1), true);
    DirectedEdge* dirEdge1 = new DirectedEdge(endNode, startNode, coordinates->getAt(coordinates->size() - 2), false);
    
    newDirEdges.push_back(dirEdge0);
    newDirEdges.push_back(dirEdge1);

    Edge* edge = new SpanningTreeEdge(lineString, index);
    edge->setDirectedEdges(dirEdge0, dirEdge1);
    
    newEdges.push_back(edge);

    add(edge);
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
