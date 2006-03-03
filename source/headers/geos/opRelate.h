/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OPRELATE_H
#define GEOS_OPRELATE_H

#include <memory>
#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>

namespace geos {
namespace operation { // geos.operation

/** \brief
 * Contains classes to implement the computation of the spatial relationships of <CODE>Geometry</CODE>s.
 * 
 * The <code>relate</code> algorithm computes the <code>IntersectionMatrix</code> describing the
 * relationship of two <code>Geometry</code>s.  The algorithm for computing <code>relate</code>
 * uses the intersection operations supported by topology graphs.  Although the <code>relate</code>
 * result depends on the resultant graph formed by the computed intersections, there is
 * no need to explicitly compute the entire graph.
 * It is sufficient to compute the local structure of the graph
 * at each intersection node.
 * <P>
 * The algorithm to compute <code>relate</code> has the following steps:
 * <UL>
 *   <LI>Build topology graphs of the two input geometries. For each geometry
 *       all self-intersection nodes are computed and added to the graph.
 *   <LI>Compute nodes for all intersections between edges and nodes of the graphs.
 *   <LI>Compute the labeling for the computed nodes by merging the labels from the input graphs.
 *   <LI>Compute the labeling for isolated components of the graph (see below)
 *   <LI>Compute the <code>IntersectionMatrix</code> from the labels on the nodes and edges.
 * </UL>
 * 
 * <H3>Labeling isolated components</H3>
 * 
 * Isolated components are components (edges or nodes) of an input <code>Geometry</code> which
 * do not contain any intersections with the other input <code>Geometry</code>.  The
 * topological relationship of these components to the other input <code>Geometry</code>
 * must be computed in order to determine the complete labeling of the component.  This can
 * be done by testing whether the component lies in the interior or exterior of the other
 * <code>Geometry</code>.  If the other <code>Geometry</code> is 1-dimensional, the isolated
 * component must lie in the exterior (since otherwise it would have an intersection with an
 * edge of the <code>Geometry</code>).  If the other <code>Geometry</code> is 2-dimensional,
 * a Point-In-Polygon test can be used to determine whether the isolated component is in the
 * interior or exterior.
 * 
 * <h2>Package Specification</h2>
 * 
 * <ul>
 *   <li>Java Topology Suite Technical Specifications
 *   <li><A HREF="http://www.opengis.org/techno/specs.htm">
 *       OpenGIS Simple Features Specification for SQL</A>
 * </ul>
 * 
 */
namespace relate { // geos.operation.relate

/*
 * Represents a node in the topological graph used to compute spatial
 * relationships.
 */
class RelateNode: public geomgraph::Node {
public:
	RelateNode(const Coordinate& coord, geomgraph::EdgeEndStar *edges);
	virtual ~RelateNode();
	void updateIMFromEdges(IntersectionMatrix *im);
protected:
	void computeIM(IntersectionMatrix *im);
};

/*
 * Computes the geomgraph::EdgeEnd objects which arise from a noded geomgraph::Edge.
 */
class EdgeEndBuilder {
public:
	EdgeEndBuilder() {}

	std::vector<geomgraph::EdgeEnd*> *computeEdgeEnds(std::vector<geomgraph::Edge*> *edges);
	void computeEdgeEnds(geomgraph::Edge *edge,std::vector<geomgraph::EdgeEnd*> *l);

protected:

	void createEdgeEndForPrev(geomgraph::Edge *edge,
			std::vector<geomgraph::EdgeEnd*> *l,
			geomgraph::EdgeIntersection *eiCurr,
			geomgraph::EdgeIntersection *eiPrev);

	void createEdgeEndForNext(geomgraph::Edge *edge,
			std::vector<geomgraph::EdgeEnd*> *l,
			geomgraph::EdgeIntersection *eiCurr,
			geomgraph::EdgeIntersection *eiNext);
};

/*
 * Contains all geomgraph::EdgeEnd objectss which start at the same point
 * and are parallel.
 */
class EdgeEndBundle: public geomgraph::EdgeEnd {
public:
	EdgeEndBundle(geomgraph::EdgeEnd *e);
	virtual ~EdgeEndBundle();
	geomgraph::Label *getLabel();
//Iterator iterator() //Not needed
	std::vector<geomgraph::EdgeEnd*>* getEdgeEnds();
	void insert(geomgraph::EdgeEnd *e);
	void computeLabel() ; 
	void updateIM(IntersectionMatrix *im);
	std::string print();
protected:
	std::vector<geomgraph::EdgeEnd*> *edgeEnds;
	void computeLabelOn(int geomIndex);
	void computeLabelSides(int geomIndex);
	void computeLabelSide(int geomIndex,int side);
};

/*
 * An ordered list of EdgeEndBundle objects around a RelateNode.
 * They are maintained in CCW order (starting with the positive x-axis)
 * around the node
 * for efficient lookup and topology building.
 */
class EdgeEndBundleStar: public geomgraph::EdgeEndStar {
public:

	EdgeEndBundleStar() {}
	
	virtual ~EdgeEndBundleStar();
	void insert(geomgraph::EdgeEnd *e);
	void updateIM(IntersectionMatrix *im);
};

/*
 * Used by the geomgraph::NodeMap in a RelateNodeGraph to create RelateNode objects.
 */
class RelateNodeFactory: public geomgraph::NodeFactory {
public:
	geomgraph::Node* createNode(const Coordinate &coord) const;
	static const geomgraph::NodeFactory &instance();
private:
	RelateNodeFactory() {};
};

/*
 * Implements the simple graph of Nodes and geomgraph::EdgeEnd which is all that is
 * required to determine topological relationships between Geometries.
 * Also supports building a topological graph of a single Geometry, to
 * allow verification of valid topology.
 * 
 * It is <b>not</b> necessary to create a fully linked
 * PlanarGraph to determine relationships, since it is sufficient
 * to know how the Geometries interact locally around the nodes.
 * In fact, this is not even feasible, since it is not possible to compute
 * exact intersection points, and hence the topology around those nodes
 * cannot be computed robustly.
 * The only Nodes that are created are for improper intersections;
 * that is, nodes which occur at existing vertices of the Geometries.
 * Proper intersections (e.g. ones which occur between the interior of
 * line segments)
 * have their topology determined implicitly, without creating a geomgraph::Node object
 * to represent them.
 *
 */
class RelateNodeGraph {
public:
	RelateNodeGraph();
	virtual ~RelateNodeGraph();
//	Iterator getNodeIterator();
	std::map<Coordinate*,geomgraph::Node*,CoordinateLessThen> &getNodeMap();

	void build(geomgraph::GeometryGraph *geomGraph);

	void computeIntersectionNodes(geomgraph::GeometryGraph *geomGraph,
			int argIndex);

	void copyNodesAndLabels(geomgraph::GeometryGraph *geomGraph,int argIndex);

	void insertEdgeEnds(std::vector<geomgraph::EdgeEnd*> *ee);

private:

	geomgraph::NodeMap *nodes;
};

/*
 * Computes the topological relationship between two Geometries.
 *
 * RelateComputer does not need to build a complete graph structure to compute
 * the IntersectionMatrix.  The relationship between the geometries can
 * be computed by simply examining the labelling of edges incident on each node.
 * 
 * RelateComputer does not currently support arbitrary GeometryCollections.
 * This is because GeometryCollections can contain overlapping Polygons.
 * In order to correct compute relate on overlapping Polygons, they
 * would first need to be noded and merged (if not explicitly, at least
 * implicitly).
 *
 */
class RelateComputer {
friend class Unload;
public:
	//RelateComputer();
	virtual ~RelateComputer();
	RelateComputer(std::vector<geomgraph::GeometryGraph*> *newArg);
	IntersectionMatrix* computeIM();
private:

	static const algorithm::LineIntersector* li;

	static const algorithm::PointLocator* ptLocator;

	/// the arg(s) of the operation
	std::vector<geomgraph::GeometryGraph*> *arg; 

	geomgraph::NodeMap nodes;

	/// this intersection matrix will hold the results compute for the relate
	IntersectionMatrix *im;

	std::vector<geomgraph::Edge*> isolatedEdges;

	/// the intersection point found (if any)
	Coordinate invalidPoint;

	void insertEdgeEnds(std::vector<geomgraph::EdgeEnd*> *ee);

	void computeProperIntersectionIM(geomgraph::index::SegmentIntersector *intersector,
			IntersectionMatrix *imX);

	void copyNodesAndLabels(int argIndex);
	void computeIntersectionNodes(int argIndex);
	void labelIntersectionNodes(int argIndex);
	void computeDisjointIM(IntersectionMatrix *imX);
	void labelNodeEdges();
	void updateIM(IntersectionMatrix *imX);
	void labelIsolatedEdges(int thisIndex,int targetIndex);
	void labelIsolatedEdge(geomgraph::Edge *e,int targetIndex, const Geometry *target);
	void labelIsolatedNodes();
	void labelIsolatedNode(geomgraph::Node *n,int targetIndex);
};

/*
 * Implements the relate() operation on Geometry.
 * 
 * WARNING: The current implementation of this class will compute a result for
 * GeometryCollections.  However, the semantics of this operation are
 * not well-defined and the value returned may not represent
 * an appropriate notion of relate.
 */
class RelateOp: public GeometryGraphOperation {
public:
	static IntersectionMatrix* relate(const Geometry *a,const Geometry *b);
	RelateOp(const Geometry *g0, const Geometry *g1);
	virtual ~RelateOp();
	IntersectionMatrix* getIntersectionMatrix();
private:
	RelateComputer relateComp;
};

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.7  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.4  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.3  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.15  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.14  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.13  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

