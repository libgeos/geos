/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
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


#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "operation.h"
#include "graph.h"
#include "geosAlgorithm.h"

using namespace std;

namespace geos {

class OverlayOp: public GeometryGraphOperation {
public:
	/**
	* The spatial functions supported by this class.
	* These operations implement various boolean combinations of the resultants of the overlay.
	*/
	enum {
		INTERSECTION=1,
		UNION,
		DIFFERENCE,
		SYMDIFFERENCE
	};
	static Geometry* overlayOp(const Geometry *geom0, const Geometry *geom1,int opCode) throw(TopologyException *);
	static bool isResultOfOp(Label *label,int opCode);
	/**
	* This method will handle arguments of Location.NULL correctly
	*
	* @return true if the locations correspond to the opCode
	*/
	static bool isResultOfOp(int loc0,int loc1,int opCode);
	OverlayOp(const Geometry *g0, const Geometry *g1);
	virtual ~OverlayOp();
	Geometry* getResultGeometry(int funcCode) throw(TopologyException *);
	PlanarGraph* getGraph();
	/**
	* This method is used to decide if a point node should be included in the result or not.
	*
	* @return true if the coord point is covered by a result Line or Area geometry
	*/
	bool isCoveredByLA(const Coordinate& coord);
	/**
	* This method is used to decide if an L edge should be included in the result or not.
	*
	* @return true if the coord point is covered by a result Area geometry
	*/
	bool isCoveredByA(const Coordinate& coord);
	/**
	* @return true if the coord is located in the interior or boundary of
	* a geometry in the list.
	*/
protected:
	/**
	* Insert an edge from one of the noded input graphs.
	* Checks edges that are inserted to see if an
	* identical edge already exists.
	* If so, the edge is not inserted, but its label is merged
	* with the existing edge.
	*/
	void insertUniqueEdge(Edge *e);
private:
	PointLocator *ptLocator;
	GeometryFactory *geomFact;
	Geometry *resultGeom;
	PlanarGraph *graph;
	EdgeList *edgeList;
	vector<Polygon*> *resultPolyList;
	vector<LineString*> *resultLineList;
	vector<Point*> *resultPointList;
	void computeOverlay(int opCode) throw(TopologyException *);
	void insertUniqueEdges(vector<Edge*> *edges);
	/**
	* If either of the GeometryLocations for the existing label is
	* exactly opposite to the one in the labelToMerge,
	* this indicates a dimensional collapse has happened.
	* In this case, convert the label for that Geometry to a Line label
	*/
	//Not needed
	//void checkDimensionalCollapse(Label labelToMerge, Label existingLabel);
	/**
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
	/**
	* If edges which have undergone dimensional collapse are found,
	* replace them with a new edge which is a L edge
	*/
	void replaceCollapsedEdges();
	/**
	* Copy all nodes from an arg geometry into this graph.
	* The node label in the arg geometry overrides any previously computed
	* label for that argIndex.
	* (E.g. a node may be an intersection node with
	* a previously computed label of BOUNDARY,
	* but in the original arg Geometry it is actually
	* in the interior due to the Boundary Determination Rule)
	*/
	void copyPoints(int argIndex);
	/**
	* Compute initial labelling for all DirectedEdges at each node.
	* In this step, DirectedEdges will acquire a complete labelling
	* (i.e. one with labels for both Geometries)
	* only if they
	* are incident on a node which has edges for both Geometries
	*/
	void computeLabelling() throw(TopologyException *);
	/**
	* For nodes which have edges from only one Geometry incident on them,
	* the previous step will have left their dirEdges with no labelling for the other
	* Geometry.  However, the sym dirEdge may have a labelling for the other
	* Geometry, so merge the two labels.
	*/
	void mergeSymLabels();
	void updateNodeLabelling();
	/**
	* Incomplete nodes are nodes whose labels are incomplete.
	* (e.g. the location for one Geometry is NULL).
	* These are either isolated nodes,
	* or nodes which have edges from only a single Geometry incident on them.
	*
	* Isolated nodes are found because nodes in one graph which don't intersect
	* nodes in the other are not completely labelled by the initial process
	* of adding nodes to the nodeList.
	* To complete the labelling we need to check for nodes that lie in the
	* interior of edges, and in the interior of areas.
	* <p>
	* When each node labelling is completed, the labelling of the incident
	* edges is updated, to complete their labelling as well.
	*/
	void labelIncompleteNodes();
	/**
	* Label an isolated node with its relationship to the target geometry.
	*/
	void labelIncompleteNode(Node *n,int targetIndex);
	/**
	* Find all edges whose label indicates that they are in the result area(s),
	* according to the operation being performed.  Since we want polygon shells to be
	* oriented CW, choose dirEdges with the interior of the result on the RHS.
	* Mark them as being in the result.
	* Interior Area edges are the result of dimensional collapses.
	* They do not form part of the result area boundary.
	*/
	void findResultAreaEdges(int opCode);
	/**
	* If both a dirEdge and its sym are marked as being in the result, cancel
	* them out.
	*/
	void cancelDuplicateResultEdges();
	bool isCovered(const Coordinate& coord,vector<Geometry*> *geomList);
	bool isCovered(const Coordinate& coord,vector<Polygon*> *geomList);
	bool isCovered(const Coordinate& coord,vector<LineString*> *geomList);
	Geometry* computeGeometry(vector<Point*> *nResultPointList,
                              vector<LineString*> *nResultLineList,
                              vector<Polygon*> *nResultPolyList);
};

/**
 * A MinimalEdgeRing is a ring of edges with the property that no node
 * has degree greater than 2.  These are the form of rings required
 * to represent polygons under the OGC SFS spatial data model.
 *
 * @version 1.2
 * @see com.vividsolutions.jts.operation.overlay.MaximalEdgeRing
 */
class MinimalEdgeRing: public EdgeRing {
public:
	MinimalEdgeRing(DirectedEdge *start,GeometryFactory *geometryFactory,CGAlgorithms *cga);
	virtual ~MinimalEdgeRing();
	DirectedEdge* getNext(DirectedEdge *de);
	void setEdgeRing(DirectedEdge *de,EdgeRing *er);
};

/**
 * A MaximalEdgeRing is a ring of edges which may contain nodes of degree>2.
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
 * @version 1.2
 * @see com.vividsolutions.jts.operation.overlay.MinimalEdgeRing
 */
class MaximalEdgeRing: public EdgeRing {
public:
	MaximalEdgeRing(DirectedEdge *start, GeometryFactory *geometryFactory, CGAlgorithms *cga);
	virtual ~MaximalEdgeRing();
	DirectedEdge* getNext(DirectedEdge *de);
	void setEdgeRing(DirectedEdge* de,EdgeRing* er);
	vector<MinimalEdgeRing*>* buildMinimalRings();
	void linkDirectedEdgesForMinimalEdgeRings();
};

class PointBuilder {
private:
	OverlayOp *op;
	GeometryFactory *geometryFactory;
	PointLocator *ptLocator;
	vector<Node*>* collectNodes(int opCode);
	/**
	* This method simplifies the resultant Geometry by finding and eliminating
	* "covered" points.
	* A point is covered if it is contained in another element Geometry
	* with higher dimension (e.g. a point might be contained in a polygon,
	* in which case the point can be eliminated from the resultant).
	*/
	vector<Point*>* simplifyPoints(vector<Node*>* resultNodeList);
public:
	PointBuilder(OverlayOp *newOp,GeometryFactory *newGeometryFactory,PointLocator *newPtLocator);
	/**
	* @return a list of the Points in the result of the specified overlay operation
	*/
	vector<Point*>* build(int opCode);
};

/**
 * LineBuilder forms JTS LineStrings out of a the graph of directed edges
 * created by an OverlayGraph.
 * <p>
 * LineBuilder is a friend class of OverlayGraph.
 *
 * @version 1.2
 */
class LineBuilder {
public:
	LineBuilder(OverlayOp *newOp,GeometryFactory *newGeometryFactory,PointLocator *newPtLocator);
	~LineBuilder();
	/**
	* @return a list of the LineStrings in the result of the specified overlay operation
	*/
	vector<LineString*>* build(int opCode);
	/**
	* Find and mark L edges which are "covered" by the result area (if any).
	* L edges at nodes which also have A edges can be checked by checking
	* their depth at that node.
	* L edges at nodes which do not have A edges can be checked by doing a
	* point-in-polygon test with the previously computed result areas.
	*/
	void collectLineEdge(DirectedEdge *de,int opCode,vector<Edge*>* edges);
	/**
	* Collect edges from Area inputs which should be in the result but
	* which have not been included in a result area.
	* This happens ONLY:
	* <ul>
	* <li>during an intersection when the boundaries of two
	* areas touch in a line segment
	* <li> OR as a result of a dimensional collapse.
	* </ul>
	*/
	void collectBoundaryTouchEdge(DirectedEdge *de,int opCode,vector<Edge*>* edges);
private:
	OverlayOp *op;
	GeometryFactory *geometryFactory;
	PointLocator *ptLocator;
	vector<Edge*>* lineEdgesList;
	vector<LineString*>* resultLineList;
	void findCoveredLineEdges();
	void collectLines(int opCode);
	void buildLines(int opCode);
	void labelIsolatedLines(vector<Edge*> *edgesList);
	/**
	* Label an isolated node with its relationship to the target geometry.
	*/
	void labelIsolatedLine(Edge *e,int targetIndex);
};

/**
 * PolygonBuilder forms Polygons out of a graph of {@link DirectedEdge}s.
 * The edges to use are marked as being in the result Area.
 * <p>
 * PolygonBuilder is a friend class of OverlayGraph.
 *
 * @version 1.2
 */
class PolygonBuilder {
public:
	PolygonBuilder(GeometryFactory *newGeometryFactory,CGAlgorithms *newCga);
	~PolygonBuilder();
	/**
	* Add a complete graph.
	* The graph is assumed to contain one or more polygons,
	* possibly with holes.
	*/
	void add(PlanarGraph *graph) throw(TopologyException *);
	/**
	* Add a set of edges and nodes, which form a graph.
	* The graph is assumed to contain one or more polygons,
	* possibly with holes.
	*/
	void add(vector<DirectedEdge*> *dirEdges,vector<Node*> *nodes)
		throw(TopologyException *);
  	vector<Polygon*>* getPolygons();
	bool containsPoint(Coordinate& p);
private:
	GeometryFactory *geometryFactory;
	CGAlgorithms *cga;
//	List dirEdgeList; //Doesn't seem to be used at all
//	NodeMap *nodes;
	vector<EdgeRing*> *shellList;
	/**
	* for all DirectedEdges in result, form them into MaximalEdgeRings
	*/
	vector<MaximalEdgeRing*>* buildMaximalEdgeRings(vector<DirectedEdge*> *dirEdges);
	vector<MaximalEdgeRing*>* buildMinimalEdgeRings(vector<MaximalEdgeRing*> *maxEdgeRings,
												vector<EdgeRing*> *newShellList,
												vector<EdgeRing*> *freeHoleList);
	/**
	* This method takes a list of MinimalEdgeRings derived from a MaximalEdgeRing,
	* and tests whether they form a Polygon.  This is the case if there is a single shell
	* in the list.  In this case the shell is returned.
	* The other possibility is that they are a series of connected holes, in which case
	* no shell is returned.
	*
	* @return the shell EdgeRing, if there is one
	* @return NULL, if all the rings are holes
	*/
	EdgeRing* findShell(vector<MinimalEdgeRing*>* minEdgeRings);
	/**
	* This method assigns the holes for a Polygon (formed from a list of
	* MinimalEdgeRings) to its shell.
	* Determining the holes for a MinimalEdgeRing polygon serves two purposes:
	* <ul>
	* <li>it is faster than using a point-in-polygon check later on.
	* <li>it ensures correctness, since if the PIP test was used the point
	* chosen might lie on the shell, which might return an incorrect result from the
	* PIP test
	* </ul>
	*/
	void placePolygonHoles(EdgeRing *shell,vector<MinimalEdgeRing*> *minEdgeRings);
	/**
	* For all rings in the input list,
	* determine whether the ring is a shell or a hole
	* and add it to the appropriate list.
	* Due to the way the DirectedEdges were linked,
	* a ring is a shell if it is oriented CW, a hole otherwise.
	*/
	void sortShellsAndHoles(vector<MaximalEdgeRing*> *edgeRings,
												vector<EdgeRing*> *newShellList,
												vector<EdgeRing*> *freeHoleList);
	/**
	* This method determines finds a containing shell for all holes
	* which have not yet been assigned to a shell.
	* These "free" holes should
	* all be <b>properly</b> contained in their parent shells, so it is safe to use the
	* <code>findEdgeRingContaining</code> method.
	* (This is the case because any holes which are NOT
	* properly contained (i.e. are connected to their
	* parent shell) would have formed part of a MaximalEdgeRing
	* and been handled in a previous step).
	*/
	void placeFreeHoles(vector<EdgeRing*>* newShellList, vector<EdgeRing*> *freeHoleList);
	/**
	* Find the innermost enclosing shell EdgeRing containing the argument EdgeRing, if any.
	* The innermost enclosing ring is the <i>smallest</i> enclosing ring.
	* The algorithm used depends on the fact that:
	* <br>
	*  ring A contains ring B iff envelope(ring A) contains envelope(ring B)
	* <br>
	* This routine is only safe to use if the chosen point of the hole
	* is known to be properly contained in a shell
	* (which is guaranteed to be the case if the hole does not touch its shell)
	*
	* @return containing EdgeRing, if there is one
	* @return NULL if no containing EdgeRing is found
	*/
	EdgeRing* findEdgeRingContaining(EdgeRing *testEr,vector<EdgeRing*> *newShellList);
	vector<Polygon*>* computePolygons(vector<EdgeRing*> *newShellList);
	/**
	* Checks the current set of shells (with their associated holes) to
	* see if any of them contain the point.
	*/

};


class OverlayNodeFactory: public NodeFactory {
public:
//	OverlayNodeFactory() {};
	Node* createNode(Coordinate coord);
};

class EdgeSetNoder {
private:
	LineIntersector *li;
	vector<Edge*>* inputEdges;
public:
	EdgeSetNoder(LineIntersector *newLi);
	~EdgeSetNoder();
	void addEdges(vector<Edge*> *edges);
	vector<Edge*>* getNodedEdges();
};
}
#endif
