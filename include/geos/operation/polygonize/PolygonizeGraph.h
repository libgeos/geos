/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 ~    Vicky Vergara
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
 * Modernized to c++11: on 2018
 * Last port: operation/polygonize/PolygonizeGraph.java rev. 6/138 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H
#define GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H

#include <geos/export.h>

#include <geos/planargraph/PlanarGraph.h>  // for inheritance

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
// warning C4251: needs to have dll-interface to be used by clients of class
#pragma warning(disable: 4251)
#endif

// Forward declarations

namespace geos {
namespace geom {
	class LineString;
	class GeometryFactory;
	class Coordinate;
	class CoordinateSequence;
}  // namespace geom
namespace planargraph {
	class Node;
	class Edge;
	class DirectedEdge;
}  // namespace planargraph
}  // namespace geos

namespace geos {
namespace operation {
namespace polygonize {


// Forward declarations
class EdgeRing;
class PolygonizeDirectedEdge;

/** \brief
 * Represents a planar graph of edges that can be used to compute a
 * polygonization, and implements the algorithms to compute the
 * EdgeRings formed by the graph.
 *
 * The marked flag on DirectedEdge is used to indicate that a directed edge
 * has be logically deleted from the graph.
 *
 */
class GEOS_DLL PolygonizeGraph: public planargraph::PlanarGraph {
 public:
	/**
	 * \brief
	 * Deletes all edges at a node
	 */
	void deleteAllEdges(planargraph::Node *node);

	/**
	 * \brief
	 * Create a new polygonization graph.
	 */
	explicit PolygonizeGraph(const geom::GeometryFactory *newFactory);

	/**
	 * \brief
	 * Destroy a polygonization graph.
	 */
	~PolygonizeGraph() override;

	/**
	 * \brief
	 * Add a LineString forming an edge of the polygon graph.
	 * @param line the line to add
	 */
	void addEdge(const geom::LineString *line);

	/**
	 * \brief
	 * Computes the EdgeRings formed by the edges in this graph.
	 *
	 * @param edgeRingList : the EdgeRing found by the
	 * 	polygonization process will be pushed here.
	 *
	 */
	void getEdgeRings(std::vector<EdgeRing*>& edgeRingList);

	/**
	 * \brief
	 * Finds and removes all cut edges from the graph.
	 *
	 * @param cutLines : the list of the LineString forming the removed
	 *                   cut edges will be pushed here.
	 *
	 * TODO: document ownership of the returned LineStrings
	 */
	void deleteCutEdges(std::vector<const geom::LineString*> &cutLines);

	/** \brief
	 * Marks all edges from the graph which are "dangles".
	 *
	 * Dangles are which are incident on a node with degree 1.
	 * This process is recursive, since removing a dangling edge
	 * may result in another edge becoming a dangle.
	 * In order to handle large recursion depths efficiently,
	 * an explicit recursion stack is used
	 *
	 * @param dangleLines : the LineStrings that formed dangles will
	 *                      be push_back'ed here
	 */
	void deleteDangles(std::vector<const geom::LineString*> &dangleLines);

 private:
	int getDegreeNonDeleted(planargraph::Node *node) const;

	int getDegree(planargraph::Node *node, long label) const;

	const geom::GeometryFactory *factory;

	planargraph::Node* getNode(const geom::Coordinate& pt);

	void computeNextCWEdges();

	/**
	 * \brief
	 * Convert the maximal edge rings found by the initial graph traversal
	 * into the minimal edge rings required by JTS polygon topology rules.
	 *
	 * @param ringEdges
	 * 	the list of start edges for the edgeRings to convert.
	 *
	 */
	void convertMaximalToMinimalEdgeRings(
			std::vector<PolygonizeDirectedEdge*> ringEdges);

	/**
	 * \brief
	 * Finds all nodes in a maximal edgering
	 * which are self-intersection nodes
	 *
	 * @param startDE
	 * @param label
	 * @param intNodes : intersection nodes found will be pushed here
	 *                   the vector won't be cleared before pushing.
	 */
	std::vector<planargraph::Node*>
	findIntersectionNodes(
		 	PolygonizeDirectedEdge *startDE,
			long label);

	/**
	 * Finds and labels all edgerings in the graph.
	 *
	 * The edge rings are labelling with unique integers.
	 * The labelling allows detecting cut edges.
	 *
	 * @param dirEdgesIn  a list of the DirectedEdges in the graph
	 * @result vector that contains each ring found
	 */
	std::vector<PolygonizeDirectedEdge*>
	findLabeledEdgeRings(
			std::vector<planargraph::DirectedEdge*> &dirEdgesIn);

	void label(std::vector<planargraph::DirectedEdge*> &dirEdges, long label);

	void computeNextCWEdges(planargraph::Node *node);

	/**
	 * \brief
	 * Computes the next edge pointers going CCW around the given node,
	 * for the given edgering label.
	 * This algorithm has the effect of converting maximal edgerings
	 * into minimal edgerings
	 */
	void computeNextCCWEdges(planargraph::Node *node, long label);

	/**
	 * \brief
	 * Traverse a ring of DirectedEdges, accumulating them into a list.
	 * This assumes that all dangling directed edges have been removed
	 * from the graph, so that there is always a next dirEdge.
	 *
	 * @param startDE the DirectedEdge to start traversing at
	 * @result a vector of the DirectedEdge that form a ring
	 */
	std::vector<planargraph::DirectedEdge*>
	findDirEdgesInRing(PolygonizeDirectedEdge *startDE) const;

	/* not const because has side effect of saving the Edge Ring found */
	EdgeRing* findEdgeRing(PolygonizeDirectedEdge *startDE);

	/* These are for memory management */
	/* created as PolygonizeEdge but saved as Edge*/
	std::vector<planargraph::Edge *> newEdges;
	/* created as PolygonizeDirectedEdge but saved as DirectedEdge*/
	std::vector<planargraph::DirectedEdge *> newDirEdges;
	std::vector<planargraph::Node *> newNodes;
	std::vector<EdgeRing *> newEdgeRings;
	std::vector<geom::CoordinateSequence *> newCoords;
};

}  // namespace polygonize
}  // namespace operation
}  // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H
