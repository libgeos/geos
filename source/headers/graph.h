#ifndef GEOS_GRAPH_H
#define GEOS_GRAPH_H

#include <string>
#include <vector>
#include <map>
#include "geom.h"
#include "graphindex.h"
#include "geosAlgorithm.h"
#include "platform.h"

using namespace std;

class EdgeSetIntersector;
class SegmentIntersector;
class MonotoneChainEdge;


class Position {
public:
	enum {
		ON,
		LEFT,
		RIGHT
	};
	static int opposite(int position);
};

class TopologyLocation {
public:
	TopologyLocation();
	~TopologyLocation();
	TopologyLocation(vector<int>* newLocation);
	TopologyLocation(int on, int left, int right);
	TopologyLocation(int on);
	TopologyLocation(const TopologyLocation &gl);
	int get(int posIndex);
	bool isNull();
	bool isAnyNull();
	bool isEqualOnSide(TopologyLocation &le, int locIndex);
	bool isArea();
	bool isLine();
	void flip();
	void setAllLocations(int locValue);
	void setAllLocationsIfNull(int locValue);
	void setLocation(int locIndex, int locValue);
	void setLocation(int locValue);
	vector<int>* getLocations();
	void setLocations(int on, int left, int right);
	void setLocations(TopologyLocation &gl);
	bool allPositionsEqual(int loc);
	void merge(TopologyLocation* gl);
	string toString();
protected:
	vector<int>* location;
private:
	void init(int size);
};

class Label {
public:
	static Label* toLineLabel(Label* label);
	Label(int onLoc);
	Label(int geomIndex, int onLoc);
	Label(int onLoc, int leftLoc, int rightLoc);
	Label(const Label &l);
	Label();
	~Label();
	Label(int geomIndex,int onLoc,int leftLoc,int rightLoc);
	Label(int geomIndex,TopologyLocation* gl);
	void flip();
	int getLocation(int geomIndex, int posIndex);
	int getLocation(int geomIndex);
	void setLocation(int geomIndex, int posIndex, int location);
	void setLocation(int geomIndex, int location);
	void setAllLocations(int geomIndex, int location);
	void setAllLocationsIfNull(int geomIndex, int location);
	void setAllLocationsIfNull(int location);
	void merge(Label* lbl);
	int getGeometryCount();
	bool isNull(int geomIndex);
	bool isAnyNull(int geomIndex);
	bool isArea();
	bool isArea(int geomIndex);
	bool isLine(int geomIndex);
	bool isEqualOnSide(Label* lbl, int side);
	bool allPositionsEqual(int geomIndex, int loc);
	void toLine(int geomIndex);
	string toString();
protected:
	TopologyLocation* elt[2];
private:
	void setGeometryLocation(int geomIndex, TopologyLocation* tl);
};

class Depth {
public:
	static int depthAtLocation(int location);
	Depth();
	int getDepth(int geomIndex,int posIndex);
	void setDepth(int geomIndex,int posIndex,int depthValue);
	int getLocation(int geomIndex,int posIndex);
	void add(int geomIndex,int posIndex,int location);
	bool isNull();
	bool isNull(int geomIndex);
	bool isNull(int geomIndex,int posIndex);
	int getDelta(int geomIndex);
	void normalize();
	void add(Label* lbl);
	string toString();
private:
	enum {
		DEPTHNULL=-1 //Replaces NULL
	};
//	static const int DEPTHNULL=-1; //Replaces NULL
	int depth[2][3];
};

//class Coordinate;
class Quadrant {
public:
	static int quadrant(double dx, double dy);
	static int quadrant(Coordinate p0, Coordinate p1);
	static bool isOpposite(int quad1, int quad2);
	static int commonHalfPlane(int quad1, int quad2);
	static bool isInHalfPlane(int quad, int halfPlane);
	static bool isNorthern(int quad);
};

//class IntersectionMatrix;
class GraphComponent {
public:
	GraphComponent();
	GraphComponent(Label* newLabel);
	~GraphComponent();
	Label* getLabel();
	virtual void setLabel(Label* newLabel);
	virtual void setInResult(bool isInResult);
	virtual bool isInResult();
	virtual void setCovered(bool isCovered);
	virtual bool isCovered();
	virtual bool isCoveredSet();
	virtual bool isVisited();
	virtual void setVisited(bool isVisited);
	virtual Coordinate& getCoordinate()=0;
	virtual bool isIsolated()=0;
	virtual void updateIM(IntersectionMatrix *im);
protected:
	Label* label;
	virtual void computeIM(IntersectionMatrix *im)=0;
private:
	bool isInResultVar;
	bool isCoveredVar;
	bool isCoveredSetVar;
	bool isVisitedVar;
};

class Node;
class EdgeIntersectionList;
class Edge: public GraphComponent{
public:
	static void updateIM(Label *lbl,IntersectionMatrix *im);
	CoordinateList* pts;
	EdgeIntersectionList *eiList;
	Edge();
	Edge(CoordinateList* newPts, Label *newLabel);
	Edge(CoordinateList* newPts);
	~Edge();
	virtual int getNumPoints();
	virtual void setName(string newName);
	virtual CoordinateList* getCoordinates();
	virtual Coordinate& getCoordinate(int i);
	virtual Coordinate& getCoordinate();
	virtual Depth *getDepth();
	virtual int getDepthDelta();
	virtual void setDepthDelta(int newDepthDelta);
	virtual int getMaximumSegmentIndex();
	virtual EdgeIntersectionList* getEdgeIntersectionList();
	virtual MonotoneChainEdge* getMonotoneChainEdge();
	virtual bool isClosed();
	virtual bool isCollapsed();
	virtual Edge* getCollapsedEdge();
	virtual void setIsolated(bool newIsIsolated);
	virtual bool isIsolated();
	virtual void addIntersections(LineIntersector *li,int segmentIndex,int geomIndex);
	virtual void addIntersection(LineIntersector *li,int segmentIndex,int geomIndex,int intIndex);
	virtual void computeIM(IntersectionMatrix *im);
	virtual bool isPointwiseEqual(Edge *e);
	virtual string print();
	virtual string printReverse();
	virtual bool equals(Edge* e);
private:
	string name;
	MonotoneChainEdge *mce;
	bool isIsolatedVar;
	Depth *depth;
	int depthDelta;   // the change in area depth from the R to L side of this edge
};

class EdgeEnd {
public:
	EdgeEnd();
	~EdgeEnd();
	EdgeEnd(Edge* newEdge, Coordinate& newP0, Coordinate& newP1);
	EdgeEnd(Edge* newEdge, Coordinate& newP0, Coordinate& newP1, Label* newLabel);
	virtual Edge* getEdge();
	virtual Label* getLabel();
	virtual Coordinate& getCoordinate();
	virtual Coordinate& getDirectedCoordinate();
	virtual int getQuadrant();
	virtual double getDx();
	virtual double getDy();
	virtual void setNode(Node* newNode);
	virtual Node* getNode();
	virtual int compareTo(EdgeEnd *e);
	virtual int compareDirection(EdgeEnd *e);
	virtual void computeLabel();
	virtual string print();
protected:
	static CGAlgorithms *cga;
	Edge* edge;// the parent edge of this edge end
	Label* label;
	EdgeEnd(Edge* newEdge);
	virtual void init(Coordinate& newP0, Coordinate& newP1);
private:
	Node* node;          // the node this edge end originates at
	Coordinate p0,p1;  // points of initial line segment
	double dx, dy;      // the direction vector for this edge from its starting point
	int quadrant;
};

struct EdgeEndLT {
	bool operator()(EdgeEnd *s1, EdgeEnd *s2) const {
		return s1->compareTo(s2)<0;
	}
};

class GeometryGraph;
class EdgeEndStar {
public:
	EdgeEndStar();
	~EdgeEndStar();
	virtual void insert(EdgeEnd *e){};
	virtual Coordinate& getCoordinate();
	virtual int getDegree();
	virtual vector<EdgeEnd*>::iterator getIterator();
	virtual vector<EdgeEnd*>* getEdges();
	virtual EdgeEnd* getNextCW(EdgeEnd *ee);
	virtual void computeLabelling(vector<GeometryGraph*> *geom);
	virtual int getLocation(int geomIndex,Coordinate& p,vector<GeometryGraph*> *geom);
	virtual bool isAreaLabelsConsistent();
	virtual void propagateSideLabels(int geomIndex);
	virtual int findIndex(EdgeEnd *eSearch);
	virtual string print();
protected:
	map<EdgeEnd*,void*,EdgeEndLT> *edgeMap;
	vector<EdgeEnd*> *edgeList;
	virtual void insertEdgeEnd(EdgeEnd *e,void* obj);
private:
	int ptInAreaLocation[2];
	virtual void computeEdgeEndLabels();
	virtual bool checkAreaLabelsConsistent(int geomIndex);
};

class DirectedEdge;
class EdgeRing;
class DirectedEdgeStar: public EdgeEndStar {
public:
	DirectedEdgeStar();
	~DirectedEdgeStar();
	void insert(EdgeEnd *ee);
	Label *getLabel();
	int getOutgoingDegree();
	int getOutgoingDegree(EdgeRing *er);
	DirectedEdge* getRightmostEdge();
	void computeLabelling(vector<GeometryGraph*> *geom);
	void mergeSymLabels();
	void updateLabelling(Label *nodeLabel);
	void linkResultDirectedEdges();
	void linkMinimalDirectedEdges(EdgeRing *er);
	void linkAllDirectedEdges();
	void findCoveredLineEdges();
	void computeDepths(DirectedEdge *de);
	string print();
private:
	/**
	 * A list of all outgoing edges in the result, in CCW order
	 */
	vector<DirectedEdge*> *resultAreaEdgeList;
	Label *label;
	vector<DirectedEdge*>* getResultAreaEdges();
	enum {
		SCANNING_FOR_INCOMING=1,
		LINKING_TO_OUTGOING
	};
//	static const int SCANNING_FOR_INCOMING=1;
//	static const int LINKING_TO_OUTGOING=2;
	int computeDepths(int startIndex, int endIndex, int startDepth);
};

class Node: public GraphComponent {
public:
	Node(Coordinate newCoord, EdgeEndStar* newEdges);
	~Node();
	virtual Coordinate& getCoordinate();
	virtual EdgeEndStar* getEdges();
	virtual bool isIsolated();
	virtual void add(EdgeEnd *e);
	virtual void mergeLabel(Node* n);
	virtual void mergeLabel(Label* label2);
	virtual void setLabel(int argIndex, int onLocation);
	virtual void setLabelBoundary(int argIndex);
	virtual int computeMergedLocation(Label* label2, int eltIndex);
	virtual string print();
protected:
	Coordinate coord;
	EdgeEndStar* edges;
	virtual void computeIM(IntersectionMatrix *im) {};
};

class NodeFactory {
public:
	virtual Node* createNode(Coordinate coord);
};

class EdgeIntersection {
public:
	Coordinate coord;
	int segmentIndex;
	double dist;
	EdgeIntersection(Coordinate newCoord, int newSegmentIndex, double newDist);
	int compare(int newSegmentIndex, double newDist);
	bool isEndPoint(int maxSegmentIndex);
	string print();
};

class EdgeIntersectionList{
public:
	vector<EdgeIntersection*> *list;
	Edge *edge;
	EdgeIntersectionList(Edge *edge);
	~EdgeIntersectionList();
	EdgeIntersection* add(Coordinate coord, int segmentIndex, double dist);
	vector<EdgeIntersection*>::iterator iterator();
	bool isEmpty();
	bool findInsertionPoint(int segmentIndex,double dist,vector<EdgeIntersection*>::iterator *insertIt);
	bool isIntersection(Coordinate pt);
	void addEndpoints();
	void addSplitEdges(vector<Edge*> *edgeList);
	Edge *createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1);
	string print();
};

class EdgeList: public vector<Edge*> {
public:
	EdgeList();
	void insert(Edge *e);
	int findEdgeIndex(Edge *e);
	string print();
};

struct CoordLT {
	bool operator()(Coordinate s1, Coordinate s2) const {
		return s1.compareTo(s2)<0;
	}
};

class NodeMap{
public:
	map<Coordinate,Node*,CoordLT>* nodeMap;
	NodeFactory *nodeFact;
	NodeMap(NodeFactory *newNodeFact);
	~NodeMap();
	Node* addNode(Coordinate coord);
	Node* addNode(Node *n);
	void add(EdgeEnd *e);
	Node *find(Coordinate coord);
	map<Coordinate,Node*,CoordLT>::iterator iterator();
	//Collection values(); //Doesn't work yet. Use iterator.
	//vector instead of Collection
	vector<Node*>* getBoundaryNodes(int geomIndex);
	string print();
};

class EdgeRing;

class DirectedEdge: public EdgeEnd{
public:
	static int depthFactor(int currLocation, int nextLocation);
	DirectedEdge();	
	~DirectedEdge();	
	DirectedEdge(Edge *newEdge, bool newIsForward);
	Edge* getEdge();
	void setInResult(bool newIsInResult);
	bool isInResult();
	bool isVisited();
	void setVisited(bool newIsVisited);
	void setEdgeRing(EdgeRing *newEdgeRing);
	EdgeRing* getEdgeRing();
	void setMinEdgeRing(EdgeRing *newMinEdgeRing);
	EdgeRing* getMinEdgeRing();
	int getDepth(int position);
	void setDepth(int position, int newDepth);
	void setVisitedEdge(bool newIsVisited);
	DirectedEdge* getSym();
	bool isForward();
	void setSym(DirectedEdge *de);
	DirectedEdge* getNext();
	void setNext(DirectedEdge *newNext);
	DirectedEdge* getNextMin();
	void setNextMin(DirectedEdge *newNextMin);
	bool isLineEdge();
	bool isInteriorAreaEdge();
	void setEdgeDepths(int position, int newDepth);
	string print();
	string printEdge();
protected:
	bool isForwardVar;
private:
	bool isInResultVar;
	bool isVisitedVar;
	DirectedEdge *sym; // the symmetric edge
	DirectedEdge *next;  // the next edge in the edge ring for the polygon containing this edge
	DirectedEdge *nextMin;  // the next edge in the MinimalEdgeRing that contains this edge
	EdgeRing *edgeRing;  // the EdgeRing that this edge is part of
	EdgeRing *minEdgeRing;  // the MinimalEdgeRing that this edge is part of
	/**
	 * The depth of each side (position) of this edge.
	 * The 0 element of the array is never used.
	*/
	int depth[3];
	void computeDirectedLabel();
};

class EdgeRing{
public:
	EdgeRing(DirectedEdge *newStart, GeometryFactory *newGeometryFactory, CGAlgorithms *newCga);
	~EdgeRing();
	bool isIsolated();
	bool isHole();
	Coordinate getCoordinate(int i);
	LinearRing* getLinearRing();
	Label* getLabel();
	bool isShell();
	EdgeRing *getShell();
	void setShell(EdgeRing *newShell);
	void addHole(EdgeRing *edgeRing);
	Polygon* toPolygon(GeometryFactory* geometryFactory);
	void computeRing();
	virtual DirectedEdge* getNext(DirectedEdge *de)=0;
	virtual void setEdgeRing(DirectedEdge *de, EdgeRing *er)=0;
	vector<DirectedEdge*>* getEdges();
	int getMaxNodeDegree();
	void setInResult();
	bool containsPoint(Coordinate p);
protected:
	DirectedEdge *startDe; // the directed edge which starts the list of edges for this EdgeRing
	GeometryFactory *geometryFactory;
	CGAlgorithms *cga;
	void computePoints(DirectedEdge *newStart);
	void mergeLabel(Label *deLabel);
	void mergeLabel(Label *deLabel, int geomIndex);
	void addPoints(Edge *edge, bool isForward, bool isFirstEdge);
private:
	int maxNodeDegree;
	vector<DirectedEdge*>* edges; // the DirectedEdges making up this EdgeRing
	CoordinateList* pts;
	Label* label; // label stores the locations of each geometry on the face surrounded by this ring
	LinearRing *ring;  // the ring created for this EdgeRing
	bool isHoleVar;
	EdgeRing *shell;   // if non-null, the ring is a hole and this EdgeRing is its containing shell
	vector<EdgeRing*>* holes; // a list of EdgeRings which are holes in this EdgeRing
	void computeMaxNodeDegree();
};

class PlanarGraph {
public:
	static CGAlgorithms *cga;
	static LineIntersector *li;
	static void linkResultDirectedEdges(vector<Node*>* allNodes);
	PlanarGraph(NodeFactory *nodeFact);
	PlanarGraph();
	~PlanarGraph();
	virtual vector<Edge*>::iterator getEdgeIterator();
	virtual vector<EdgeEnd*>* getEdgeEnds();
	virtual bool isBoundaryNode(int geomIndex,Coordinate coord);
	virtual void add(EdgeEnd *e);
	virtual map<Coordinate,Node*,CoordLT>::iterator getNodeIterator();
//	Wouldn't work. Use iterator
//	Collection getNodes() { return nodes.values(); }
	virtual Node* addNode(Node *node);
	virtual Node* addNode(Coordinate coord);
	virtual Node* find(Coordinate coord);
	virtual void addEdges(vector<Edge*>* edgesToAdd);
	virtual void linkResultDirectedEdges();
	virtual void linkAllDirectedEdges();
	virtual EdgeEnd* findEdgeEnd(Edge *e);
	virtual Edge* findEdge(Coordinate p0,Coordinate p1);
	virtual Edge* findEdgeInSameDirection(Coordinate p0,Coordinate p1);
	virtual string printEdges();
	virtual NodeMap* getNodeMap();
	//Not used 
	//string debugPrint();
	//string debugPrintln();
protected:
	vector<Edge*> *edges;
	NodeMap *nodes;
	vector<EdgeEnd*> *edgeEndList;
	virtual void insertEdge(Edge *e);
private:
	bool matchInSameDirection(Coordinate p0,Coordinate p1,Coordinate ep0,Coordinate ep1);
};

struct LineStringLT {
	bool operator()(LineString *ls1, LineString *ls2) const {
		return ls1->compareTo(ls2)<0;
	}
};

class GeometryGraph: public PlanarGraph {
public:
	static bool isInBoundary(int boundaryCount);
	static int determineBoundary(int boundaryCount);
	GeometryGraph();
	~GeometryGraph();
	GeometryGraph(int newArgIndex, Geometry *newParentGeom);
	GeometryGraph(int newArgIndex, PrecisionModel *newPrecisionModel, int newSRID);
	PrecisionModel* getPrecisionModel();
	int getSRID();
	Geometry* getGeometry();
	vector<Node*>* getBoundaryNodes();
	CoordinateList* getBoundaryPoints();
	Edge* findEdge(LineString *line);
	void computeSplitEdges(vector<Edge*> *edgelist);
	void addEdge(Edge *e);
	void addPoint(Coordinate& pt);
	SegmentIntersector* computeSelfNodes(LineIntersector *li);
	SegmentIntersector* computeEdgeIntersections(GeometryGraph *g,LineIntersector *li,bool includeProper);
	vector<Edge*> *getEdges();
private:
	Geometry *parentGeom;
	// the precision model of the Geometry represented by this graph
	PrecisionModel *precisionModel;
	int SRID;
	/**
	* The lineEdgeMap is a map of the linestring components of the
	* parentGeometry to the edges which are derived from them.
	* This is used to efficiently perform findEdge queries
	*/
	map<LineString*,Edge*,LineStringLT>* lineEdgeMap;
	/**
	* If this flag is true, the Boundary Determination Rule will used when deciding
	* whether nodes are in the boundary or not
	*/
	bool useBoundaryDeterminationRule;
	int argIndex;  // the index of this geometry as an argument to a spatial function (used for labelling)
	vector<Node*>* boundaryNodes;
	EdgeSetIntersector* createEdgeSetIntersector();
	void add(Geometry *g);
	void addCollection(GeometryCollection *gc);
	void addPoint(Point *p);
	void addPolygonRing(LinearRing *lr,int cwLeft,int cwRight);
	void addPolygon(Polygon *p);
	void addLineString(LineString *line);
	void insertPoint(int argIndex,Coordinate coord,int onLocation);
	void insertBoundaryPoint(int argIndex,Coordinate coord);
	void addSelfIntersectionNodes(int argIndex);
	void addSelfIntersectionNode(int argIndex,Coordinate coord,int loc);
};
//Operators
bool operator==(Edge a,Edge b);

#endif
