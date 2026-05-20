/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/linemerge/LineMergeGraph.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/operation/linemerge/LineMergeGraph.h>
#include <geos/operation/linemerge/LineMergeEdge.h>
#include <geos/operation/linemerge/LineMergeDirectedEdge.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/planargraph/Node.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <memory>

#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif


//using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

LineMergeGraph::LineMergeGraph() = default;

void
LineMergeGraph::addEdge(const LineString* lineString)
{
    if(lineString->isEmpty()) {
        return;
    }

#if GEOS_DEBUG
    std::cerr << "Adding LineString " << lineString->toString() << std::endl;
#endif

    auto coordinates = valid::RepeatedPointRemover::removeRepeatedPoints(lineString->getCoordinatesRO());

    std::size_t nCoords = coordinates->size(); // virtual call..

    // don't add lines with all coordinates equal
    if(nCoords <= 1) {
        return;
    }

    const CoordinateXY& startCoordinate = coordinates->getAt<CoordinateXY>(0);
    const CoordinateXY& endCoordinate = coordinates->getAt<CoordinateXY>(nCoords - 1);

    planargraph::Node* startNode = getNode(startCoordinate);
    planargraph::Node* endNode = getNode(endCoordinate);
#if GEOS_DEBUG
    std::cerr << " startNode: " << *startNode << std::endl;
    std::cerr << " endNode: " << *endNode << std::endl;
#endif

    const CoordinateXY& dirPt0 = coordinates->getAt<CoordinateXY>(1);
    newDirEdges.push_back(std::make_unique<LineMergeDirectedEdge>(startNode, endNode, dirPt0, true));
    auto* directedEdge0 = newDirEdges.back().get();

    const CoordinateXY& dirPt1 = coordinates->getAt<CoordinateXY>(nCoords - 2);
    newDirEdges.push_back(std::make_unique<LineMergeDirectedEdge>(endNode, startNode, dirPt1, false));
    auto* directedEdge1 = newDirEdges.back().get();

    newEdges.push_back(std::make_unique<LineMergeEdge>(lineString));
    planargraph::Edge* edge = newEdges.back().get();
    edge->setDirectedEdges(directedEdge0, directedEdge1);

#if GEOS_DEBUG
    std::cerr << " planargraph::Edge: " << *edge << std::endl;
#endif

    add(edge);

#if GEOS_DEBUG
    std::cerr << " After addition to the graph:" << std::endl;
    std::cerr << "  startNode: " << *startNode << std::endl;
    std::cerr << "  endNode: " << *endNode << std::endl;
#endif

}

planargraph::Node*
LineMergeGraph::getNode(const CoordinateXY& coordinate)
{
    planargraph::Node* node = findNode(coordinate);
    if(node == nullptr) {
        newNodes.push_back(std::make_unique<planargraph::Node>(coordinate));
        node = newNodes.back().get();
        add(node);
    }
    return node;
}

LineMergeGraph::~LineMergeGraph() = default;

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
