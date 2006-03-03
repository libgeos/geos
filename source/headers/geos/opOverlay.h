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

#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace geos {
namespace operation { // geos.operation

/** \brief
 * Contains classes that perform a topological overlay to compute boolean spatial functions.
 * 
 * The Overlay Algorithm is used in spatial analysis methods for computing set-theoretic
 * operations (boolean combinations) of input {@link Geometry}s. The algorithm for
 * computing the overlay uses the intersection operations supported by topology graphs.
 * To compute an overlay it is necessary to explicitly compute the resultant graph formed
 * by the computed intersections.
 * 
 * The algorithm to compute a set-theoretic spatial analysis method has the following steps:
 * <UL>
 *   <LI>Build topology graphs of the two input geometries.  For each geometry all
 *       self-intersection nodes are computed and added to the graph.
 *   <LI>Compute nodes for all intersections between edges and nodes of the graphs.
 *   <LI>Compute the labeling for the computed nodes by merging the labels from the input graphs.
 *   <LI>Compute new edges between the compute intersection nodes.  Label the edges appropriately.
 *   <LI>Build the resultant graph from the new nodes and edges.
 *   <LI>Compute the labeling for isolated components of the graph.  Add the
 *       isolated components to the resultant graph.
 *   <LI>Compute the result of the boolean combination by selecting the node and edges
 *       with the appropriate labels. Polygonize areas and sew linear geometries together.
 * </UL>
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
namespace overlay { // geos.operation.overlay

class ElevationMatrix;
class ElevationMatrixCell;
class ElevationMatrixFilter;

class ElevationMatrixCell {
public:
	ElevationMatrixCell();
	~ElevationMatrixCell();
	void add(const Coordinate &c);
	void add(double z);
	double getAvg(void) const;
	double getTotal(void) const;
	std::string print() const;
private:
	std::set<double>zvals;
	double ztot;
};

/*
 * This is the CoordinateFilter used by ElevationMatrix.
 * filter_ro is used to add Geometry Coordinate's Z
 * values to the matrix.
 * filter_rw is used to actually elevate Geometries.
 */
class ElevationMatrixFilter: public CoordinateFilter
{
public:
	ElevationMatrixFilter(ElevationMatrix &em);
	~ElevationMatrixFilter();
	void filter_rw(Coordinate *c) const;
	void filter_ro(const Coordinate *c);
private:
	ElevationMatrix &em;
	double avgElevation;
};


/*
 */
class ElevationMatrix {
friend class ElevationMatrixFilter;
public:
	ElevationMatrix(const Envelope &extent, unsigned int rows,
		unsigned int cols);
	~ElevationMatrix();
	void add(const Geometry *geom);
	void elevate(Geometry *geom) const;
	// set Z value for each cell w/out one
	double getAvgElevation() const;
	ElevationMatrixCell &getCell(const Coordinate &c);
	const ElevationMatrixCell &getCell(const Coordinate &c) const;
	std::string print() const;
private:
	ElevationMatrixFilter filter;
	//void add(const CoordinateSequence *cs);
	void add(const Coordinate &c);
	Envelope env;
	unsigned int cols;
	unsigned int rows;
	double cellwidth;
	double cellheight;
	mutable bool avgElevationComputed;
	mutable double avgElevation;
	std::vector<ElevationMatrixCell>cells;
};

/// Computes the overlay of two Geometry. 
//
/// The overlay can be used to determine any
/// boolean combination of the geometries.
///
class OverlayOp: public GeometryGraphOperation {

public:

	/// The spatial functions supported by this class.
	//
	/// These operations implement various boolean combinations of
	/// the resultants of the overlay.
	///
	enum {
		INTERSECTION=1,
		UNION,
		DIFFERENCE,
		SYMDIFFERENCE
	};

	static Geometry* overlayOp(const Geometry *geom0, const Geometry *geom1,int opCode); //throw(TopologyException *);

	static bool isResultOfOp(geomgraph::Label *label,int opCode);

	/// This method will handle arguments of Location.NULL correctly
	//
	/// @return true if the locations correspond to the opCode
	///
	static bool isResultOfOp(int loc0,int loc1,int opCode);

	/// Construct an OverlayOp with the given Geometry args.
	// 
	/// Ownership of passed args will remain to caller, and
	/// the OverlayOp won't change them in any way.
	///
	OverlayOp(const Geometry *g0, const Geometry *g1);

	virtual ~OverlayOp();

	Geometry* getResultGeometry(int funcCode);
		// throw(TopologyException *);

	geomgraph::PlanarGraph& getGraph() { return graph; }

	/** \brief
	 * This method is used to decide if a point node should be included
	 * in the result or not.
	 *
	 * @return true if the coord point is covered by a result Line
	 * or Area geometry
	 */
	bool isCoveredByLA(const Coordinate& coord);

	/** \brief
	 * This method is used to decide if an L edge should be included
	 * in the result or not.
	 *
	 * @return true if the coord point is covered by a result Area geometry
	 */
	bool isCoveredByA(const Coordinate& coord);

	/*
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */

protected:

	/** \brief
	 * Insert an edge from one of the noded input graphs.
	 *
	 * Checks edges that are inserted to see if an
	 * identical edge already exists.
	 * If so, the edge is not inserted, but its label is merged
	 * with the existing edge.
	 */
	void insertUniqueEdge(geomgraph::Edge *e);

private:

	algorithm::PointLocator ptLocator;

	const GeometryFactory *geomFact;

	Geometry *resultGeom;

	geomgraph::PlanarGraph graph;

	geomgraph::EdgeList edgeList;

	std::vector<Polygon*> *resultPolyList;

	std::vector<LineString*> *resultLineList;

	std::vector<Point*> *resultPointList;

	void computeOverlay(int opCode); // throw(TopologyException *);

	void insertUniqueEdges(std::vector<geomgraph::Edge*> *edges);

	/*
	 * If either of the GeometryLocations for the existing label is
	 * exactly opposite to the one in the labelToMerge,
	 * this indicates a dimensional collapse has happened.
	 * In this case, convert the label for that Geometry to a Line label
	 */
	//Not needed
	//void checkDimensionalCollapse(geomgraph::Label labelToMerge, geomgraph::Label existingLabel);

	/*
	 * Update the labels for edges according to their depths.
	 * For each edge, the depths are first normalized.
	 * Then, if the depths for the edge are equal,
	 * this edge must have collapsed into a line edge.
	 * If the depths are not equal, update the label
	 * with the locations corresponding to the depths
	 * (i.e. a depth of 0 corresponds to a Location of EXTERIOR,
	 * a depth of 1 corresponds to INTERIOR)
	 */
	void computeLabelsFromDepths();

	/*
	 * If edges which have undergone dimensional collapse are found,
	 * replace them with a new edge which is a L edge
	 */
	void replaceCollapsedEdges();

	/*
	 * Copy all nodes from an arg geometry into this graph.
	 * The node label in the arg geometry overrides any previously
	 * computed label for that argIndex.
	 * (E.g. a node may be an intersection node with
	 * a previously computed label of BOUNDARY,
	 * but in the original arg Geometry it is actually
	 * in the interior due to the Boundary Determination Rule)
	 */
	void copyPoints(int argIndex);

	/*
	 * Compute initial labelling for all DirectedEdges at each node.
	 * In this step, DirectedEdges will acquire a complete labelling
	 * (i.e. one with labels for both Geometries)
	 * only if they
	 * are incident on a node which has edges for both Geometries
	 */
	void computeLabelling(); // throw(TopologyException *);

	/*
	 * For nodes which have edges from only one Geometry incident on them,
	 * the previous step will have left their dirEdges with no
	 * labelling for the other Geometry. 
	 * However, the sym dirEdge may have a labelling for the other
	 * Geometry, so merge the two labels.
	 */
	void mergeSymLabels();

	void updateNodeLabelling();

	/*
	 * Incomplete nodes are nodes whose labels are incomplete.
	 * (e.g. the location for one Geometry is NULL).
	 * These are either isolated nodes,
	 * or nodes which have edges from only a single Geometry incident
	 * on them.
	 *
	 * Isolated nodes are found because nodes in one graph which
	 * don't intersect nodes in the other are not completely
	 * labelled by the initial process of adding nodes to the nodeList.
	 * To complete the labelling we need to check for nodes that
	 * lie in the interior of edges, and in the interior of areas.
	 * 
	 * When each node labelling is completed, the labelling of the
	 * incident edges is updated, to complete their labelling as well.
	 */
	void labelIncompleteNodes();

	/*
	 * Label an isolated node with its relationship to the target geometry.
	 */
	void labelIncompleteNode(geomgraph::Node *n,int targetIndex);

	/*
	 * Find all edges whose label indicates that they are in the result
	 * area(s), according to the operation being performed. 
	 * Since we want polygon shells to be
	 * oriented CW, choose dirEdges with the interior of the result
	 * on the RHS.
	 * Mark them as being in the result.
	 * Interior Area edges are the result of dimensional collapses.
	 * They do not form part of the result area boundary.
	 */
	void findResultAreaEdges(int opCode);

	/*
	 * If both a dirEdge and its sym are marked as being in the result,
	 * cancel them out.
	 */
	void cancelDuplicateResultEdges();

	/*
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const Coordinate& coord,std::vector<Geometry*> *geomList);

	/*
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const Coordinate& coord,std::vector<Polygon*> *geomList);

	/*
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const Coordinate& coord,std::vector<LineString*> *geomList);

	/*
	 * Build a Geometry containing all Geometries in the given vectors.
	 * Takes element's ownership, vector control is left to caller. 
	 */
	Geometry* computeGeometry(std::vector<Point*> *nResultPointList,
                              std::vector<LineString*> *nResultLineList,
                              std::vector<Polygon*> *nResultPolyList);

	/* Caches for memory management */
	std::vector<geomgraph::Edge *>dupEdges;

	/*
	 * Merge Z values of node with those of the segment or vertex in
	 * the given Polygon it is on.
	 */
	int mergeZ(geomgraph::Node *n, const Polygon *poly) const;

	/*
	 * Merge Z values of node with those of the segment or vertex in
	 * the given LineString it is on.
	 * @returns 1 if an intersection is found, 0 otherwise.
	 */
	int mergeZ(geomgraph::Node *n, const LineString *line) const;

	/*
	 * Average Z of input geometries
	 */
	double avgz[2];
	bool avgzcomputed[2];

	double getAverageZ(int targetIndex);
	static double getAverageZ(const Polygon *poly);

	ElevationMatrix *elevationMatrix;

};

/*
 * A ring of {@link Edge}s with the property that no node
 * has degree greater than 2.  These are the form of rings required
 * to represent polygons under the OGC SFS spatial data model.
 *
 * @see com.vividsolutions.jts.operation.overlay.MaximalEdgeRing
 */
class MinimalEdgeRing: public geomgraph::EdgeRing {
public:
	// CGAlgorithms argument obsoleted
	MinimalEdgeRing(geomgraph::DirectedEdge *start, const GeometryFactory *geometryFactory);
	//virtual ~MinimalEdgeRing();
	inline geomgraph::DirectedEdge* getNext(geomgraph::DirectedEdge *de);
	inline void setEdgeRing(geomgraph::DirectedEdge *de,geomgraph::EdgeRing *er);
};

// INLINES
void MinimalEdgeRing::setEdgeRing(geomgraph::DirectedEdge *de,geomgraph::EdgeRing *er) {
	de->setMinEdgeRing(er);
}
geomgraph::DirectedEdge* MinimalEdgeRing::getNext(geomgraph::DirectedEdge *de) {
	return de->getNextMin();
}

/*
 * A ring of {@link edges} which may contain nodes of degree > 2.
 * A MaximalEdgeRing may represent two different spatial entities:
 * <ul>
 * <li>a single polygon possibly containing inversions (if the ring is oriented CW)
 * <li>a single hole possibly containing exversions (if the ring is oriented CCW)
 * </ul>
 * If the MaximalEdgeRing represents a polygon,
 * the interior of the polygon is strongly connected.
 * <p>
 * These are the form of rings used to define polygons under some spatial data models.
 * However, under the OGC SFS model, {@link MinimalEdgeRings} are required.
 * A MaximalEdgeRing can be converted to a list of MinimalEdgeRings using the
 * {@link #buildMinimalRings() } method.
 *
 * @see com.vividsolutions.jts.operation.overlay.MinimalEdgeRing
 */

class MaximalEdgeRing: public geomgraph::EdgeRing {

public:

	// CGAlgorithms arg is obsoleted
	MaximalEdgeRing(geomgraph::DirectedEdge *start, const GeometryFactory *geometryFactory);

	virtual ~MaximalEdgeRing();
	geomgraph::DirectedEdge* getNext(geomgraph::DirectedEdge *de);
	void setEdgeRing(geomgraph::DirectedEdge* de,geomgraph::EdgeRing* er);
	std::vector<MinimalEdgeRing*>* buildMinimalRings();
	void linkDirectedEdgesForMinimalEdgeRings();
};

/*
 * Constructs {@link Point}s from the nodes of an overlay graph.
 */
class PointBuilder {
private:

	OverlayOp *op;
	const GeometryFactory *geometryFactory;
	void extractNonCoveredResultNodes(int opCode);

	/*
	 * Converts non-covered nodes to Point objects and adds them to
	 * the result.
	 *
	 * A node is covered if it is contained in another element Geometry
	 * with higher dimension (e.g. a node point might be contained in
	 * a polygon, in which case the point can be eliminated from
	 * the result).
	 *
	 * @param n the node to test
	 */
	void filterCoveredNodeToPoint(const geomgraph::Node *);

	/// Allocated a construction time, but not owned.
	/// Make sure you take ownership of it, getting 
	/// it from build()
	std::vector<Point*> *resultPointList;

public:

	PointBuilder(OverlayOp *newOp,
			const GeometryFactory *newGeometryFactory,
			algorithm::PointLocator *newPtLocator=NULL)
		:
		op(newOp),
		geometryFactory(newGeometryFactory),
		resultPointList(new std::vector<Point *>())
	{}

	/*
	 * @return a list of the Points in the result of the specified
	 * overlay operation
	 */
	std::vector<Point*>* build(int opCode);
};

/*
 * Forms JTS LineStrings out of a the graph of {@link geomgraph::DirectedEdge}s
 * created by an {@link OverlayOp}.
 *
 */
class LineBuilder {
public:
	LineBuilder(OverlayOp *newOp, const GeometryFactory *newGeometryFactory, algorithm::PointLocator *newPtLocator);
	~LineBuilder();

	/**
	 * @return a list of the LineStrings in the result of the specified overlay operation
	 */
	std::vector<LineString*>* build(int opCode);

	/**
	 * Find and mark L edges which are "covered" by the result area (if any).
	 * L edges at nodes which also have A edges can be checked by checking
	 * their depth at that node.
	 * L edges at nodes which do not have A edges can be checked by doing a
	 * point-in-polygon test with the previously computed result areas.
	 */
	void collectLineEdge(geomgraph::DirectedEdge *de,int opCode,std::vector<geomgraph::Edge*>* edges);

	/**
	 * Collect edges from Area inputs which should be in the result but
	 * which have not been included in a result area.
	 * This happens ONLY:
	 * 
	 *  -  during an intersection when the boundaries of two
	 *     areas touch in a line segment
	 *  -   OR as a result of a dimensional collapse.
	 * 
	 */
	void collectBoundaryTouchEdge(geomgraph::DirectedEdge *de,int opCode,std::vector<geomgraph::Edge*>* edges);

private:
	OverlayOp *op;
	const GeometryFactory *geometryFactory;
	algorithm::PointLocator *ptLocator;
	std::vector<geomgraph::Edge*> lineEdgesList;
	std::vector<LineString*>* resultLineList;
	void findCoveredLineEdges();
	void collectLines(int opCode);
	void buildLines(int opCode);
	void labelIsolatedLines(std::vector<geomgraph::Edge*> *edgesList);

	/**
	 * Label an isolated node with its relationship to the target geometry.
	 */
	void labelIsolatedLine(geomgraph::Edge *e,int targetIndex);

	/*
	 * If the given CoordinateSequence has mixed 3d/2d vertexes
	 * set Z for all vertexes missing it.
	 * The Z value is interpolated between 3d vertexes and copied
	 * from a 3d vertex to the end.
	 */
	void propagateZ(CoordinateSequence *cs);
};

/*
 * Forms {@link Polygon}s out of a graph of {@link geomgraph::DirectedEdge}s.
 * The edges to use are marked as being in the result Area.
 * <p>
 *
 */
class PolygonBuilder {
public:

	// CGAlgorithms argument is unused
	PolygonBuilder(const GeometryFactory *newGeometryFactory);

	~PolygonBuilder();
	/**
	* Add a complete graph.
	* The graph is assumed to contain one or more polygons,
	* possibly with holes.
	*/
	void add(geomgraph::PlanarGraph *graph); // throw(TopologyException *);
	/**
	* Add a set of edges and nodes, which form a graph.
	* The graph is assumed to contain one or more polygons,
	* possibly with holes.
	*/
	void add(std::vector<geomgraph::DirectedEdge*> *dirEdges,std::vector<geomgraph::Node*> *nodes); // throw(TopologyException *);
  	std::vector<Geometry*>* getPolygons();
	bool containsPoint(const Coordinate& p);
private:
	const GeometryFactory *geometryFactory;
	std::vector<geomgraph::EdgeRing*> shellList;

	/**
	 * for all DirectedEdges in result, form them into MaximalEdgeRings
	 */
	std::vector<MaximalEdgeRing*>* buildMaximalEdgeRings(
		std::vector<geomgraph::DirectedEdge*> *dirEdges);

	std::vector<MaximalEdgeRing*>* buildMinimalEdgeRings(
		std::vector<MaximalEdgeRing*> *maxEdgeRings,
		std::vector<geomgraph::EdgeRing*> *newShellList,
		std::vector<geomgraph::EdgeRing*> *freeHoleList);
	/**
	 * This method takes a list of MinimalEdgeRings derived from a
	 * MaximalEdgeRing, and tests whether they form a Polygon. 
	 * This is the case if there is a single shell
	 * in the list.  In this case the shell is returned.
	 * The other possibility is that they are a series of connected
	 * holes, in which case no shell is returned.
	 *
	 * @return the shell geomgraph::EdgeRing, if there is one
	 * @return NULL, if all the rings are holes
	 */
	geomgraph::EdgeRing* findShell(std::vector<MinimalEdgeRing*>* minEdgeRings);

	/**
	 * This method assigns the holes for a Polygon (formed from a list of
	 * MinimalEdgeRings) to its shell.
	 * Determining the holes for a MinimalEdgeRing polygon serves two
	 * purposes:
	 * 
	 *  - it is faster than using a point-in-polygon check later on.
	 *  - it ensures correctness, since if the PIP test was used the point
	 *    chosen might lie on the shell, which might return an incorrect
	 *    result from the PIP test
	 */
	void placePolygonHoles(geomgraph::EdgeRing *shell,
		std::vector<MinimalEdgeRing*> *minEdgeRings);

	/**
	 * For all rings in the input list,
	 * determine whether the ring is a shell or a hole
	 * and add it to the appropriate list.
	 * Due to the way the DirectedEdges were linked,
	 * a ring is a shell if it is oriented CW, a hole otherwise.
	 */
	void sortShellsAndHoles(std::vector<MaximalEdgeRing*> *edgeRings,
		std::vector<geomgraph::EdgeRing*> *newShellList,
		std::vector<geomgraph::EdgeRing*> *freeHoleList);

	/**
	 * This method determines finds a containing shell for all holes
	 * which have not yet been assigned to a shell.
	 * These "free" holes should
	 * all be <b>properly</b> contained in their parent shells, so it
	 * is safe to use the
	 * <code>findEdgeRingContaining</code> method.
	 * (This is the case because any holes which are NOT
	 * properly contained (i.e. are connected to their
	 * parent shell) would have formed part of a MaximalEdgeRing
	 * and been handled in a previous step).
	 */
	void placeFreeHoles(std::vector<geomgraph::EdgeRing*>* newShellList,
		std::vector<geomgraph::EdgeRing*> *freeHoleList);

	/**
	 * Find the innermost enclosing shell geomgraph::EdgeRing containing the
	 * argument geomgraph::EdgeRing, if any.
	 * The innermost enclosing ring is the <i>smallest</i> enclosing ring.
	 * The algorithm used depends on the fact that:
	 *
	 * ring A contains ring B iff envelope(ring A)
	 * contains envelope(ring B)
	 * 
	 * This routine is only safe to use if the chosen point of the hole
	 * is known to be properly contained in a shell
	 * (which is guaranteed to be the case if the hole does not touch
	 * its shell)
	 *
	 * @return containing geomgraph::EdgeRing, if there is one
	 * @return NULL if no containing geomgraph::EdgeRing is found
	 */
	geomgraph::EdgeRing* findEdgeRingContaining(geomgraph::EdgeRing *testEr,
		std::vector<geomgraph::EdgeRing*> *newShellList);

	std::vector<Geometry*>* computePolygons(std::vector<geomgraph::EdgeRing*> *newShellList);

	/**
	 * Checks the current set of shells (with their associated holes) to
	 * see if any of them contain the point.
	 */

};


/*
 * Creates nodes for use in the geomgraph::PlanarGraph constructed during
 * overlay operations.
 *
 */
class OverlayNodeFactory: public geomgraph::NodeFactory {
public:
	geomgraph::Node* createNode(const Coordinate &coord) const;
	static const geomgraph::NodeFactory &instance();
};

/*
 * Nodes a set of edges.
 * Takes one or more sets of edges and constructs a
 * new set of edges consisting of all the split edges created by
 * noding the input edges together
 */
class EdgeSetNoder {
private:
	algorithm::LineIntersector *li;
	std::vector<geomgraph::Edge*>* inputEdges;
public:
	EdgeSetNoder(algorithm::LineIntersector *newLi)
		:
		li(newLi),
		inputEdges(new std::vector<geomgraph::Edge*>())
	{}

	~EdgeSetNoder() {
		delete inputEdges; // TODO: avoid heap allocation
	}

	void addEdges(std::vector<geomgraph::Edge*> *edges);
	std::vector<geomgraph::Edge*>* getNodedEdges();
};


} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.21  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.20  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.19  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.18  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.17  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.16  2005/12/08 00:03:51  strk
 * LineBuilder::lineEdgesList made a real vector, rather then pointer (private member).
 * Small optimizations in LineBuilder loops, cleanups in LineBuilder class dox.
 *
 * Revision 1.15  2005/11/21 16:03:20  strk
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
 * Revision 1.14  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.13  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.12  2005/07/11 12:17:47  strk
 * Cleaned up syntax
 *
 * Revision 1.11  2005/06/28 01:07:02  strk
 * improved extraction of result points in overlay op
 *
 * Revision 1.10  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.9  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.8  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.7  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.6  2004/11/22 15:51:52  strk
 * Added interpolation of containing geometry's average Z for point_in_poly case.
 *
 * Revision 1.5  2004/11/20 18:17:26  strk
 * Added Z propagation for overlay lines output.
 *
 * Revision 1.4  2004/11/20 17:16:10  strk
 * Handled Z merging for point on polygon boundary case.
 *
 * Revision 1.3  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.18  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.17  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.16  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.15  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.14  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.13  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.11  2003/11/12 16:14:56  strk
 * Added some more throw specifications and cleanup on exception (leaks removed).
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
