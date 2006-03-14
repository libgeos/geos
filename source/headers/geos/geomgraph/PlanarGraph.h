/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_PLANARGRAPH_H
#define GEOS_GEOMGRAPH_PLANARGRAPH_H

#include <map>
#include <vector>
#include <memory>

#include <geos/geom/Coordinate.h>
#include <geos/geomgraph/PlanarGraph.h>
#include <geos/geomgraph/NodeMap.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace geomgraph {
		class Edge;
		class Node;
		class EdgeEnd;
		class NodeFactory;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
 * \brief
 * Represents a directed graph which is embeddable in a planar surface.
 * 
 * The computation of the IntersectionMatrix relies on the use of a structure
 * called a "topology graph".  The topology graph contains nodes and edges
 * corresponding to the nodes and line segments of a Geometry. Each
 * node and edge in the graph is labeled with its topological location
 * relative to the source geometry.
 * 
 * Note that there is no requirement that points of self-intersection
 * be a vertex.
 * Thus to obtain a correct topology graph, Geometry objects must be
 * self-noded before constructing their graphs.
 *
 * Two fundamental operations are supported by topology graphs:
 * 
 *  - Computing the intersections between all the edges and nodes of
 *    a single graph
 *  - Computing the intersections between the edges and nodes of two
 *    different graphs
 * 
 */
class PlanarGraph {
public:

	static void linkResultDirectedEdges(std::vector<Node*>* allNodes);
			// throw(TopologyException);

	PlanarGraph(const NodeFactory &nodeFact);

	PlanarGraph();

	virtual ~PlanarGraph();

	virtual std::vector<Edge*>::iterator getEdgeIterator();

	virtual std::vector<EdgeEnd*>* getEdgeEnds();

	virtual bool isBoundaryNode(int geomIndex, const geom::Coordinate& coord);

	virtual void add(EdgeEnd *e);

	virtual NodeMap::iterator getNodeIterator();

	virtual std::vector<Node*>* getNodes();

	virtual void getNodes(std::vector<Node*>&);

	virtual Node* addNode(Node *node);

	virtual Node* addNode(const geom::Coordinate& coord);

	virtual Node* find(geom::Coordinate& coord);

	virtual void addEdges(const std::vector<Edge*> &edgesToAdd);

	virtual void linkResultDirectedEdges();

	virtual void linkAllDirectedEdges();

	virtual EdgeEnd* findEdgeEnd(Edge *e);

	virtual Edge* findEdge(const geom::Coordinate& p0,
			const geom::Coordinate& p1);

	virtual Edge* findEdgeInSameDirection(const geom::Coordinate& p0,
			const geom::Coordinate& p1);

	virtual std::string printEdges();

	virtual NodeMap* getNodeMap();

protected:

	std::vector<Edge*> *edges;

	NodeMap *nodes;

	std::vector<EdgeEnd*> *edgeEndList;

	virtual void insertEdge(Edge *e);

private:

	bool matchInSameDirection(const geom::Coordinate& p0,
			const geom::Coordinate& p1,
			const geom::Coordinate& ep0,
			const geom::Coordinate& ep1);
};



} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/PlanarGraph.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_PLANARGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/14 15:46:54  strk
 * Added PlanarGraph::getNodes(vector&) func, to reduce useless heap allocations
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

