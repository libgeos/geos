#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "graph.h"
#include "geosAlgorithm.h"

using namespace std;

class OverlayOp {
public:
	enum {
		INTERSECTION=1,
		UNION,
		DIFFERENCE,
		SYMDIFFERENCE
	};
	PlanarGraph* getGraph() {return NULL;}
	static bool isResultOfOp(Label *l,int o) {return false;}
	bool isCoveredByLA(Coordinate& c) {return false;}
	bool isCoveredByA(Coordinate& c) {return false;}
	Geometry* getArgGeometry(int i) {return NULL;}
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
	DirectedEdge* getNext(DirectedEdge *de);
	void setEdgeRing(DirectedEdge *de,EdgeRing *er);
};

/**
 * A MaximalEdgeRing is a ring of edges which may contain nodes of degree > 2.
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
	void add(PlanarGraph *graph);
	/**
	* Add a set of edges and nodes, which form a graph.
	* The graph is assumed to contain one or more polygons,
	* possibly with holes.
	*/
	void add(vector<DirectedEdge*> *dirEdges,vector<Node*> *nodes);
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
	* @return null, if all the rings are holes
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
	* @return null if no containing EdgeRing is found
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
	Node* createNode(Coordinate& coord);
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

#endif
