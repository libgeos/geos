#ifndef GEOS_OPRELATE_H
#define GEOS_OPRELATE_H

#include <string>
#include <vector>
#include "platform.h"
#include "operation.h"
#include "graph.h"
#include "geosAlgorithm.h"

class RelateNode: public Node {
public:
	RelateNode(Coordinate& coord,EdgeEndStar *edges);
	void updateIMFromEdges(IntersectionMatrix *im);
protected:
	void computeIM(IntersectionMatrix *im);
};

class EdgeEndBuilder {
public:
	EdgeEndBuilder();
	vector<EdgeEnd*> *computeEdgeEnds(vector<Edge*> *edges);
	void computeEdgeEnds(Edge *edge,vector<EdgeEnd*> *l);
protected:
	void createEdgeEndForPrev(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiPrev);
	void createEdgeEndForNext(Edge *edge,vector<EdgeEnd*> *l,EdgeIntersection *eiCurr,EdgeIntersection *eiNext);
};

class EdgeEndBundle: public EdgeEnd {
public:
	EdgeEndBundle(EdgeEnd *e);
	~EdgeEndBundle();
	Label *getLabel();
//Iterator iterator() //Not needed
	vector<EdgeEnd*>* getEdgeEnds();
	void insert(EdgeEnd *e);
	void computeLabel() ; 
	void updateIM(IntersectionMatrix *im);
	string print();
protected:
	vector<EdgeEnd*> *edgeEnds;
	void computeLabelOn(int geomIndex);
	void computeLabelSides(int geomIndex);
	void computeLabelSide(int geomIndex,int side);
};

class EdgeEndBundleStar: public EdgeEndStar {
public:
	EdgeEndBundleStar();
	void insert(EdgeEnd *e);
	void updateIM(IntersectionMatrix *im);
};

class RelateNodeFactory: public NodeFactory {
public:
	Node* createNode(Coordinate coord);
};

class RelateNodeGraph {
public:
	RelateNodeGraph();
//	Iterator getNodeIterator();
	map<Coordinate,Node*,CoordLT>* getNodeMap();
	void build(GeometryGraph *geomGraph);
	void computeIntersectionNodes(GeometryGraph *geomGraph,int argIndex);
	void copyNodesAndLabels(GeometryGraph *geomGraph,int argIndex);
	void insertEdgeEnds(vector<EdgeEnd*> *ee);
private:
	NodeMap *nodes;
};

/**
 * Note that RelateComputer does not need to build a complete graph structure to compute
 * the IntersectionMatrix.  The relationship between the geometries can
 * be computed by simply examining the labelling of edges incident on each node.
 * <p>
 * RelateComputer does not currently support arbitrary GeometryCollections.
 * This is because GeometryCollections can contain overlapping Polygons.
 * In order to correct compute relate on overlapping Polygons, they
 * would first need to be noded and merged (if not explicitly, at least
 * implicitly).
 */
class RelateComputer {
public:
	RelateComputer();
	~RelateComputer();
	RelateComputer(vector<GeometryGraph*> *newArg);
	IntersectionMatrix* computeIM();
private:
	static const LineIntersector* li;
	static const PointLocator* ptLocator;
	vector<GeometryGraph*> *arg;  // the arg(s) of the operation
	NodeMap *nodes;
	// this intersection matrix will hold the results compute for the relate
	IntersectionMatrix *im;
	vector<Edge*> *isolatedEdges;
	// the intersection point found (if any)
	Coordinate invalidPoint;
	void insertEdgeEnds(vector<EdgeEnd*> *ee);
	void computeProperIntersectionIM(SegmentIntersector *intersector,IntersectionMatrix *imX);
	void copyNodesAndLabels(int argIndex);
	void computeIntersectionNodes(int argIndex);
	void labelIntersectionNodes(int argIndex);
	void computeDisjointIM(IntersectionMatrix *imX);
	bool isNodeEdgeAreaLabelsConsistent();
	void labelNodeEdges();
	void updateIM(IntersectionMatrix *imX);
	void labelIsolatedEdges(int thisIndex,int targetIndex);
	void labelIsolatedEdge(Edge *e,int targetIndex,Geometry *target);
	void labelIsolatedNodes();
	void labelIsolatedNode(Node *n,int targetIndex);
};

/**
* Implements the relate() operation on {@link Geometry}s.
* <p>
* </b>
* WARNING: The current implementation of this class will compute a result for
* GeometryCollections.  However, the semantics of this operation are
* not well-defined and the value returned may not represent
* an appropriate notion of relate.
* </b>
*/
class RelateOp: public GeometryGraphOperation {
public:
	static IntersectionMatrix* relate(Geometry *a,Geometry *b);
	RelateOp(Geometry *g0,Geometry *g1);
	~RelateOp();
	IntersectionMatrix* getIntersectionMatrix();
private:
	RelateComputer *relateComp;
	static vector<Geometry*>* toList(Geometry *geom);
	static vector<Geometry*>* addToList(Geometry *geom, vector<Geometry*>* geomList);
	static bool isBaseGeometryCollection(Geometry* geom);
	static IntersectionMatrix* relateGC(vector<Geometry*> *a,vector<Geometry*> *b);
};


#endif

