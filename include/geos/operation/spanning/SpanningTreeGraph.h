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

#pragma once

#include <geos/export.h>
#include <geos/planargraph/PlanarGraph.h> // for inheritance

#include <vector>
#include <cstddef> // for std::size_t

// Forward declarations
namespace geos {
namespace geom {
class Curve;
class Coordinate;
class Geometry;
}
namespace planargraph {
class Node;
class Edge;
class DirectedEdge;
}
}


namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

/** \brief
 * A planar graph of edges used to find a spanning tree.
 */
class GEOS_DLL SpanningTreeGraph: public planargraph::PlanarGraph {

private:
    planargraph::Node* getNode(const geom::Coordinate& coordinate);
    
    std::vector<planargraph::Node*> newNodes;
    std::vector<planargraph::Edge*> newEdges;
    std::vector<planargraph::DirectedEdge*> newDirEdges;

public:

    /** \brief
     * Adds an Edge, DirectedEdges, and Nodes for the given
     * Curve representation of an edge.
     *
     * @param curve the curve to add to the graph
     * @param index the original index of the curve
     */
    void addEdge(const geom::Curve* curve, std::size_t index);
    
    ~SpanningTreeGraph() override;
};
} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
