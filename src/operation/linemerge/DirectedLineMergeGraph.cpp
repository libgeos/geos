#include <geos/operation/linemerge/DirectedLineMergeGraph.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineString.h>
#include <geos/operation/linemerge/LineMergeEdge.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/planargraph/Node.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;
using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

void
DirectedLineMergeGraph::addEdge(const LineString* lineString)
{
    if(lineString->isEmpty()) {
        return;
    }

#if GEOS_DEBUG
    std::cerr << "Adding LineString " << lineString->toString() << std::endl;
#endif

    auto coordinates = valid::RepeatedPointRemover::removeRepeatedPoints(
        lineString->getCoordinatesRO());

    std::size_t nCoords = coordinates->size();

    // don't add lines with all coordinates equal
    if(nCoords <= 1) {
        return;
    }

    const Coordinate& startCoordinate = coordinates->getAt(0);
    const Coordinate& endCoordinate = coordinates->getAt(nCoords - 1);

    Node* startNode = getNode(startCoordinate);
    Node* endNode = getNode(endCoordinate);
#if GEOS_DEBUG
    std::cerr << " startNode: " << *startNode << std::endl;
    std::cerr << " endNode: " << *endNode << std::endl;
#endif

    // NOTE: we do not add Edge to graph, but add DirectedEdge instead
    // using Edge as LineString container

    DirectedEdge* directedEdge = new DirectedEdge(
        startNode, endNode, coordinates->getAt(1), true);
    newDirEdges.push_back(directedEdge);

    Edge* edge = new LineMergeEdge(lineString);
    newEdges.push_back(edge);
    directedEdge->setEdge(edge);

    add(directedEdge);

    // add DirectedEdge to node, instead of calling Edge::setDirectedEdges
    startNode->addOutEdge(directedEdge);
}

Node*
DirectedLineMergeGraph::getNode(const Coordinate& coordinate)
{
    Node* node = findNode(coordinate);
    if(node == nullptr) {
        node = new Node(coordinate);
        newNodes.push_back(node);
        add(node);
    }
    return node;
}

DirectedLineMergeGraph::~DirectedLineMergeGraph()
{
    unsigned int i;
    for(i = 0; i < newNodes.size(); i++) {
        delete newNodes[i];
    }
    for(i = 0; i < newEdges.size(); i++) {
        delete newEdges[i];
    }
    for(i = 0; i < newDirEdges.size(); i++) {
        delete newDirEdges[i];
    }
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
