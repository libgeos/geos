#ifndef GEOS_OPVALID_H
#define GEOS_OPVALID_H

#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "opRelate.h"
#include "indexSweepline.h"

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


class IsValidOp {
public:
	static Coordinate& 
		findPtNotNode(CoordinateList *innerRingPts,LinearRing *searchRing, GeometryGraph *graph);
};
//class RelateNode: public Node {
//public:
//	RelateNode(Coordinate& coord,EdgeEndStar *edges);
//	void updateIMFromEdges(IntersectionMatrix *im);
//protected:
//	void computeIM(IntersectionMatrix *im);
//};
//
//class EdgeEndBuilder {
//public:
//	EdgeEndBuilder();
//	vector<EdgeEnd*> *computeEdgeEnds(vector<Edge*> *edges);
//	void computeEdgeEnds(Edge *edge,vector<EdgeEnd*> *l);
//protected:
//	void createEdgeEndForPrev(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiPrev);
//	void createEdgeEndForNext(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiNext);
//};
//
//class EdgeEndBundle: public EdgeEnd {
//public:
//	EdgeEndBundle(EdgeEnd *e);
//	~EdgeEndBundle();
//	Label *getLabel();
////Iterator iterator() //Not needed
//	vector<EdgeEnd*>* getEdgeEnds();
//	void insert(EdgeEnd *e);
//	void computeLabel() ; 
//	void updateIM(IntersectionMatrix *im);
//	string print();
//protected:
//	vector<EdgeEnd*> *edgeEnds;
//	void computeLabelOn(int geomIndex);
//	void computeLabelSides(int geomIndex);
//	void computeLabelSide(int geomIndex,int side);
//};
//
//class EdgeEndBundleStar: public EdgeEndStar {
//public:
//	EdgeEndBundleStar();
//	void insert(EdgeEnd *e);
//	void updateIM(IntersectionMatrix *im);
//};
//
//class RelateNodeFactory: public NodeFactory {
//public:
//	Node* createNode(Coordinate coord);
//};
//
//class RelateNodeGraph {
//public:
//	RelateNodeGraph();
////	Iterator getNodeIterator();
//	map<Coordinate,Node*,CoordLT>* getNodeMap();
//	void build(GeometryGraph *geomGraph);
//	void computeIntersectionNodes(GeometryGraph *geomGraph,int argIndex);
//	void copyNodesAndLabels(GeometryGraph *geomGraph,int argIndex);
//	void insertEdgeEnds(vector<EdgeEnd*> *ee);
//private:
//	NodeMap *nodes;
//};
//
//class RelateComputer {
//public:
//	static const LineIntersector* li;
//	RelateComputer();
//	~RelateComputer();
//	RelateComputer(vector<GeometryGraph*> *newArg);
//	Coordinate& getInvalidPoint();
//	bool isNodeConsistentArea();
//	bool hasDuplicateRings();
//	IntersectionMatrix* computeIM();
//private:
//	static const PointLocator* ptLocator;
//	vector<GeometryGraph*> *arg;  // the arg(s) of the operation
//	NodeMap *nodes;
//	// this intersection matrix will hold the results compute for the relate
//	IntersectionMatrix *im;
//	vector<Edge*> *isolatedEdges;
//	// the intersection point found (if any)
//	Coordinate invalidPoint;
//	void insertEdgeEnds(vector<EdgeEnd*> *ee);
//	void computeProperIntersectionIM(SegmentIntersector *intersector,IntersectionMatrix *imX);
//	void copyNodesAndLabels(int argIndex);
//	void computeIntersectionNodes(int argIndex);
//	void labelIntersectionNodes(int argIndex);
//	void computeDisjointIM(IntersectionMatrix *imX);
//	bool isNodeEdgeAreaLabelsConsistent();
//	void labelNodeEdges();
//	void updateIM(IntersectionMatrix *imX);
//	void labelIsolatedEdges(int thisIndex,int targetIndex);
//	void labelIsolatedEdge(Edge *e,int targetIndex,Geometry *target);
//	void labelIsolatedNodes();
//	void labelIsolatedNode(Node *n,int targetIndex);
//};
//
///**
// * Note that RelateOp does not need to build a complete graph structure to compute
// * the IntersectionMatrix.  The relationship between the geometries can
// * be computed by simply examining the labelling of edges incident on each node.
// */
//class RelateOp: public GeometryGraphOperation {
//public:
//	static IntersectionMatrix* relate(Geometry *a,Geometry *b);
//	RelateOp(Geometry *g0,Geometry *g1);
//	~RelateOp();
//	IntersectionMatrix* getIntersectionMatrix();
//private:
//	RelateComputer *relateComp;
//};
//

#endif
