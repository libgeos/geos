#ifndef GEOS_OPVALID_H
#define GEOS_OPVALID_H

#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "opRelate.h"
#include "indexSweepline.h"
#include "indexQuadtree.h"

class SimpleNestedRingTester {
public:
	SimpleNestedRingTester(GeometryGraph *newGraph);
	~SimpleNestedRingTester();
	void add(LinearRing *ring);
	Coordinate& getNestedPoint();
	bool isNonNested();
private:
	static CGAlgorithms *cga;
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Coordinate nestedPt;
};

class TopologyValidationError {
public:
	enum {
		ERROR,
		REPEATED_POINT,
		HOLE_OUTSIDE_SHELL,
		NESTED_HOLES,
		DISCONNECTED_INTERIOR,
		SELF_INTERSECTION,
		RING_SELF_INTERSECTION,
		NESTED_SHELLS,
		DUPLICATE_RINGS
	};

TopologyValidationError(int newErrorType,Coordinate newPt);
TopologyValidationError(int newErrorType);
Coordinate& getCoordinate();
string getMessage();
string toString();
private:
	static string errMsg[];
	int errorType;
	Coordinate pt;
};

class RepeatedPointTester {
public:
	RepeatedPointTester() {};
	Coordinate& getCoordinate();
	bool hasRepeatedPoint(Geometry *g);
	bool hasRepeatedPoint(CoordinateList *coord);
private:
	Coordinate repeatedCoord;
	bool hasRepeatedPoint(Polygon *p);
	bool hasRepeatedPoint(GeometryCollection *gc);
};

class ConsistentAreaTester {
private:
	static LineIntersector *li;
	GeometryGraph *geomGraph;
	RelateNodeGraph *nodeGraph;
	// the intersection point found (if any)
	Coordinate invalidPoint;
	/**
	* Check all nodes to see if their labels are consistent.
	* If any are not, return false
	*/
	bool isNodeEdgeAreaLabelsConsistent();
public:
	ConsistentAreaTester(GeometryGraph *newGeomGraph);
	~ConsistentAreaTester();
	/**
	* @return the intersection point, or <code>null</code> if none was found
	*/
	Coordinate& getInvalidPoint();
	bool isNodeConsistentArea();
	/**
	* Checks for two duplicate rings in an area.
	* Duplicate rings are rings that are topologically equal
	* (that is, which have the same sequence of points up to point order).
	* If the area is topologically consistent (determined by calling the
	* <code>isNodeConsistentArea</code>,
	* duplicate rings can be found by checking for EdgeBundles which contain
	* more than one EdgeEnd.
	* (This is because topologically consistent areas cannot have two rings sharing
	* the same line segment, unless the rings are equal).
	* The start point of one of the equal rings will be placed in
	* invalidPoint.
	*
	* @return true if this area Geometry is topologically consistent but has two duplicate rings
	*/
	bool hasDuplicateRings();
};


class SweeplineNestedRingTester {
public:
	SweeplineNestedRingTester(GeometryGraph *newGraph);
	~SweeplineNestedRingTester();
	Coordinate& getNestedPoint();
	void add(LinearRing* ring);
	bool isNonNested();
	class OverlapAction: public SweepLineOverlapAction {
	public:
		bool isNonNested;
		OverlapAction(SweeplineNestedRingTester *p);
		void overlap(SweepLineInterval *s0, SweepLineInterval *s1);
	private:
		SweeplineNestedRingTester *parent;
	};
private:
	static CGAlgorithms *cga;
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Envelope *totalEnv;
	SweepLineIndex *sweepLine;
	Coordinate nestedPt;
	void buildIndex();
	bool isInside(LinearRing *innerRing,LinearRing *searchRing);
};

class QuadtreeNestedRingTester {
public:
	QuadtreeNestedRingTester(GeometryGraph *newGraph);
	~QuadtreeNestedRingTester();
	Coordinate& getNestedPoint();
	void add(LinearRing *ring);
	bool isNonNested();
private:
	static CGAlgorithms *cga;
	GeometryGraph *graph;  // used to find non-node vertices
	vector<LinearRing*> *rings;
	Envelope *totalEnv;
	Quadtree *qt;
	Coordinate nestedPt;
	void buildQuadtree();
};

/**
 * This class tests that the interior of an area Geometry (Polygon or MultiPolygon)
 * is connected.  The Geometry is invalid if the interior is disconnected (as can happen
 * if one or more holes either form a chain touching the shell at two places,
 * or if one or more holes form a ring around a portion of the interior)
 */
class ConnectedInteriorTester {
public:
	ConnectedInteriorTester(GeometryGraph *newGeomGraph);
	~ConnectedInteriorTester();
	Coordinate& getCoordinate();
	bool isInteriorsConnected();
private:
	GeometryFactory *geometryFactory;
	CGAlgorithms *cga;
	GeometryGraph *geomGraph;
	// save a coordinate for any disconnected interior found
	// the coordinate will be somewhere on the ring surrounding the disconnected interior
	Coordinate disconnectedRingcoord;
	void setAllEdgesInResult(PlanarGraph *graph);
	vector<EdgeRing*>* buildEdgeRings(vector<EdgeEnd*> *dirEdges);
	/**
	* Mark all the edges for the edgeRings corresponding to the shells
	* of the input polygons.  Note only ONE ring gets marked for each shell.
	*/
	void visitShellInteriors(Geometry *g, PlanarGraph *graph);
	void visitInteriorRing(LineString *ring, PlanarGraph *graph);
	/**
	* Check if any shell ring has an unvisited edge.
	* A shell ring is a ring which is not a hole and which has the interior
	* of the parent area on the RHS.
	* (Note that there may be non-hole rings with the interior on the LHS,
	* since the interior of holes will also be polygonized into CW rings
	* by the linkAllDirectedEdges() step)
	*
	* @return true if there is an unvisited edge in a non-hole ring
	*/
	bool hasUnvisitedShellEdge(vector<EdgeRing*> *edgeRings);
protected:
	void visitLinkedDirectedEdges(DirectedEdge *start);
};

class IsValidOp: public GeometryGraphOperation {
public:
	/**
	* Find a point from the list of testCoords
	* that is NOT a node in the edge for the list of searchCoords
	*
	* @return the point found, or <code>null</code> if none found
	*/
	static Coordinate& 
		findPtNotNode(CoordinateList *testCoords,LinearRing *searchRing, GeometryGraph *graph);
	IsValidOp(Geometry *g);
	bool isValid();
	TopologyValidationError* getValidationError();
private:
	bool isChecked;
	TopologyValidationError* validErr;
	void checkValid(Geometry *g);
	void checkValid(LineString *g);
	void checkValid(Polygon *g);
	void checkValid(MultiPolygon *g);
	void checkValid(GeometryCollection *gc);
	void checkConsistentArea();
	void checkNoSelfIntersectingRings(GeometryGraph *graph);
	/**
	* check that a ring does not self-intersect, except at its endpoints.
	* Algorithm is to count the number of times each node along edge occurs.
	* If any occur more than once, that must be a self-intersection.
	*/
	void checkSelfIntersectingRing(EdgeIntersectionList *eiList);
	void checkNoRepeatedPoint(Geometry *g);
	/**
	* Test that each hole is inside the polygon shell.
	* This routine assumes that the holes have previously been tested
	* to ensure that all vertices lie on the shell or inside it.
	* A simple test of a single point in the hole can be used,
	* provide the point is chosen such that it does not lie on the
	* boundary of the shell.
	*/
	void checkHolesInShell(Polygon *p);
	void OLDcheckHolesInShell(Polygon *p);
	/**
	* Tests that no hole is nested inside another hole.
	* This routine assumes that the holes are disjoint.
	* To ensure this, holes have previously been tested
	* to ensure that:
	* <ul>
	* <li>they do not partially overlap
	* (checked by <code>checkRelateConsistency</code>)
	* <li>they are not identical
	* (checked by <code>checkRelateConsistency</code>)
	* <li>they do not touch at a vertex
	* (checked by <code>????</code>)
	* </ul>
	*/
	void checkHolesNotNested(Polygon *p);
	void SLOWcheckHolesNotNested(Polygon *p);
	/**
	* Test that no element polygon is wholly in the interior of another element polygon.
	* TODO: It handles the case that one polygon is nested inside a hole of another.
	* <p>
	* Preconditions:
	* <ul>
	* <li>shells do not partially overlap
	* <li>shells do not touch along an edge
	* <li>no duplicate rings exist
	* </ul>
	* This routine relies on the fact that while polygon shells may touch at one or
	* more vertices, they cannot touch at ALL vertices.
	*/
	void checkShellsNotNested(MultiPolygon *mp);
	/**
	* Check if a shell is incorrectly nested within a polygon.  This is the case
	* if the shell is inside the polygon shell, but not inside a polygon hole.
	* (If the shell is inside a polygon hole, the nesting is valid.)
	* <p>
	* The algorithm used relies on the fact that the rings must be properly contained.
	* E.g. they cannot partially overlap (this has been previously checked by
	* <code>checkRelateConsistency</code>
	*/
	void checkShellNotNested(LinearRing *shell,Polygon *p);
	/**
	* This routine checks to see if a shell is properly contained in a hole.
	*/
	void checkShellInsideHole(LinearRing *shell,LinearRing *hole);
	void checkConnectedInteriors();
};

#endif
