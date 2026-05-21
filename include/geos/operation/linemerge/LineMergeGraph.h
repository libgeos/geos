/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
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

#pragma once

#include <geos/export.h>
#include <geos/planargraph/PlanarGraph.h> // for inheritance

#include <memory>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Curve;
class Coordinate;
}
namespace planargraph {
class Node;
class Edge;
class DirectedEdge;
}
}


namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

/** \brief
 * A planar graph of edges that is analyzed to sew the edges together.
 *
 * The <code>marked</code> flag on planargraph::Edge
 * and planargraph::Node indicates whether they have been
 * logically deleted from the graph.
 */
class GEOS_DLL LineMergeGraph: public planargraph::PlanarGraph {

private:

    planargraph::Node* getNode(const geom::CoordinateXY& coordinate);

    std::vector<std::unique_ptr<planargraph::Node>> newNodes;

    std::vector<std::unique_ptr<planargraph::Edge>> newEdges;

    std::vector<std::unique_ptr<planargraph::DirectedEdge>> newDirEdges;

public:

    LineMergeGraph();

    /** \brief
     * Adds an Edge, DirectedEdges, and Nodes for the given
     * LineString/CircularString/CompoundCurve representation of an edge.
     *
     * Empty lines or lines with all coordinates equal are not added.
     *
     * @param curve the curve to add to the graph
     */
    void addEdge(const geom::Curve* curve);

    ~LineMergeGraph() override;

private:
    // Declared as non-copyable
    LineMergeGraph(const LineMergeGraph& other) = delete;
    LineMergeGraph& operator=(const LineMergeGraph& rhs) = delete;

};
} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

