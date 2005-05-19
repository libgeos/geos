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
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_H
#define GEOS_GEOMGRAPH_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <geos/geom.h>
#include <geos/geomgraphindex.h>
#include <geos/geosAlgorithm.h>
#include <geos/platform.h>

using namespace std;

namespace geos {

class EdgeSetIntersector;
class SegmentIntersector;
class MonotoneChainEdge;


class Position {
public:
	enum {
		/* An indicator that a Location is <i>on</i> a GraphComponent */
		ON,
		/* An indicator that a Location is to the <i>left</i> of a GraphComponent */  
		LEFT,
		/* An indicator that a Location is to the <i>right</i> of a GraphComponent */  
		RIGHT
	};
	/**
	* Returns LEFT if the position is RIGHT, RIGHT if the position is LEFT, or the position
	* otherwise.
	*/
	static int opposite(int position);
};

class TopologyLocation {
public:
	TopologyLocation();
	virtual ~TopologyLocation();
	TopologyLocation(const vector<int>* newLocation);
	/**
	* Constructs a TopologyLocation specifying how points on, to the left of, and to the
	* right of some GraphComponent relate to some Geometry. Possible values for the
	* parameters are Location.NULL, Location.EXTERIOR, Location.BOUNDARY, 
	* and Location.INTERIOR.
	* @see Location
	*/
	TopologyLocation(int on, int left, int right);
	TopologyLocation(int on);
	TopologyLocation(const TopologyLocation *gl);
	int get(int posIndex) const;
	bool isNull() const;
	bool isAnyNull() const;
	bool isEqualOnSide(const TopologyLocation &le, int locIndex) const;
	bool isArea() const;
	bool isLine() const;
	void flip();
	void setAllLocations(int locValue);
	void setAllLocationsIfNull(int locValue);
	void setLocation(int locIndex, int locValue);
	void setLocation(int locValue);
	const vector<int>* getLocations() const;
	void setLocations(int on, int left, int right);
	void setLocations(const TopologyLocation &gl);
	bool allPositionsEqual(int loc) const;
	void merge(const TopologyLocation* gl);
	string toString() const;
protected:
	vector<int>* location;
private:
	void init(int size);
};

class Label {
public:
	static Label* toLineLabel(const Label* label);
	Label(int onLoc);
	Label(int geomIndex, int onLoc);
	Label(int onLoc, int leftLoc, int rightLoc);
	Label(const Label *l);
	Label();
	virtual ~Label();
	Label(int geomIndex,int onLoc,int leftLoc,int rightLoc);
	Label(int geomIndex,const TopologyLocation* gl);
	void flip();
	int getLocation(int geomIndex, int posIndex) const;
	int getLocation(int geomIndex) const;
	void setLocation(int geomIndex, int posIndex, int location);
	void setLocation(int geomIndex, int location);
	void setAllLocations(int geomIndex, int location);
	void setAllLocationsIfNull(int geomIndex, int location);
	void setAllLocationsIfNull(int location);
	void merge(const Label* lbl);
	int getGeometryCount() const;
	bool isNull(int geomIndex) const;
	bool isAnyNull(int geomIndex) const;
	bool isArea() const;
	bool isArea(int geomIndex) const;
	bool isLine(int geomIndex) const;
	bool isEqualOnSide(Label* lbl, int side) const;
	bool allPositionsEqual(int geomIndex, int loc) const;
	void toLine(int geomIndex);
	string toString() const;
protected:
	TopologyLocation* elt[2];
private:
	void setGeometryLocation(int geomIndex, const TopologyLocation* tl);
};

class Depth {
public:
	static int depthAtLocation(int location);
	Depth();
	virtual ~Depth();
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

/*
 * Utility functions for working with quadrants, which are numbered as follows:
 * <pre>
 * 1 | 0
 * --+--
 * 2 | 3
 * <pre>
 *
 */
class Quadrant {
public:
	/**
	* Returns the quadrant of a directed line segment (specified as x and y
	* displacements, which cannot both be 0).
	*/
	static int quadrant(double dx, double dy);
	/**
	* Returns the quadrant of a directed line segment from p0 to p1.
	*/
	static int quadrant(const Coordinate& p0, const Coordinate& p1);
	/**
	* Returns true if the quadrants are 1 and 3, or 2 and 4
	*/
	static bool isOpposite(int quad1, int quad2);
	/* 
	* Returns the right-hand quadrant of the halfplane defined by the two quadrants,
	* or -1 if the quadrants are opposite, or the quadrant if they are identical.
	*/
	static int commonHalfPlane(int quad1, int quad2);
	/**
	* Returns whether the given quadrant lies within the given halfplane (specified
	* by its right-hand quadrant).
	*/
	static bool isInHalfPlane(int quad, int halfPlane);
	/**
	* Returns true if the given quadrant is 0 or 1.
	*/
	static bool isNorthern(int quad);
};

//class IntersectionMatrix;
class GraphComponent {
public:
	GraphComponent();
	GraphComponent(Label* newLabel); // newLabel is deleted by destructor
	virtual ~GraphComponent();
	Label* getLabel();
	virtual void setLabel(Label* newLabel);
	virtual void setInResult(bool isInResult);
	virtual bool isInResult();
	virtual void setCovered(bool isCovered);
	virtual bool isCovered();
	virtual bool isCoveredSet();
	virtual bool isVisited();
	virtual void setVisited(bool isVisited);
	//virtual Coordinate& getCoordinate()=0; // strk removed
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
	CoordinateSequence* pts;
	EdgeIntersectionList *eiList;
	Edge();
	Edge(CoordinateSequence* newPts, Label *newLabel);
	Edge(CoordinateSequence* newPts);
	virtual ~Edge();
	virtual int getNumPoints();
	virtual void setName(string newName);
	virtual const CoordinateSequence* getCoordinates() const;
	virtual const Coordinate& getCoordinate(int i);
	virtual const Coordinate& getCoordinate(); 
	virtual Depth *getDepth();
	/**
	* The depthDelta is the change in depth as an edge is crossed from R to L
	* @return the change in depth as the edge is crossed from R to L
	*/
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
	virtual Envelope* getEnvelope();
private:
	string name;
	MonotoneChainEdge *mce;
	Envelope *env;
	bool isIsolatedVar;
	Depth *depth;
	int depthDelta;   // the change in area depth from the R to L side of this edge
	int npts;
};

class EdgeEnd {
friend class Unload;
public:
	EdgeEnd();
	virtual ~EdgeEnd();
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
//	static CGAlgorithms *cga;
	Edge* edge;// the parent edge of this edge end
	Label* label;
	EdgeEnd(Edge* newEdge);
	virtual void init(const Coordinate& newP0, const Coordinate& newP1);
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
	virtual ~EdgeEndStar();
	virtual void insert(EdgeEnd *e);
	virtual Coordinate& getCoordinate();
	virtual int getDegree();
	virtual vector<EdgeEnd*>::iterator getIterator();
	virtual vector<EdgeEnd*>* getEdges();
	virtual EdgeEnd* getNextCW(EdgeEnd *ee);
	virtual void computeLabelling(vector<GeometryGraph*> *geom); // throw(TopologyException *);
	virtual int getLocation(int geomIndex,Coordinate& p,vector<GeometryGraph*> *geom);
	virtual bool isAreaLabelsConsistent();
	virtual void propagateSideLabels(int geomIndex); // throw(TopologyException *);
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
	void computeLabelling(vector<GeometryGraph*> *geom); // throw(TopologyException *);
	void mergeSymLabels();
	void updateLabelling(Label *nodeLabel);
	void linkResultDirectedEdges(); // throw(TopologyException *);
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
	Node(Coordinate& newCoord, EdgeEndStar* newEdges);
	virtual ~Node();
	virtual const Coordinate& getCoordinate() const;
	virtual EdgeEndStar* getEdges();
	virtual bool isIsolated();
	virtual void add(EdgeEnd *e);
	virtual void mergeLabel(const Node* n);
	virtual void mergeLabel(const Label* label2);
	virtual void setLabel(int argIndex, int onLocation);
	virtual void setLabelBoundary(int argIndex);
	virtual int computeMergedLocation(const Label* label2, int eltIndex);
	virtual string print();
	virtual const vector<double> &getZ() const;
	virtual void addZ(double);

protected:
	Coordinate coord;
	EdgeEndStar* edges;
	virtual void computeIM(IntersectionMatrix *im) {};

private:
	vector<double>zvals;
	double ztot;

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
	EdgeIntersection(const Coordinate& newCoord, int newSegmentIndex, double newDist);
	virtual ~EdgeIntersection();
	int compare(int newSegmentIndex, double newDist);
	bool isEndPoint(int maxSegmentIndex);
	string print();
	int compareTo(void* obj);
};

class EdgeIntersectionList{
public:
	vector<EdgeIntersection*> *list;
	Edge *edge;
	EdgeIntersectionList(Edge *edge);
	~EdgeIntersectionList();
	EdgeIntersection* add(const Coordinate& coord, int segmentIndex, double dist);
	vector<EdgeIntersection*>::iterator iterator();
	bool isEmpty();
	bool findInsertionPoint(int segmentIndex,double dist,vector<EdgeIntersection*>::iterator *insertIt);
	bool isIntersection(const Coordinate& pt);
	void addEndpoints();
	void addSplitEdges(vector<Edge*> *edgeList);
	Edge *createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1);
	string print();
};

class EdgeList {
public:
	EdgeList();
	virtual ~EdgeList();
	void add(Edge *e);
	void addAll(vector<Edge*> *edgeColl);
	vector<Edge*>* getEdges();
	Edge* findEqualEdge(Edge* e);
	Edge* get(int i);
	int findEdgeIndex(Edge *e);
	string print();
private:
	vector<Edge*> *edges;
	/**
	* An index of the edges, for fast lookup.
	*
	* a Quadtree is used, because this index needs to be dynamic
	* (e.g. allow insertions after queries).
	* An alternative would be to use an ordered set based on the values
	* of the edge coordinates
	*
	*/
	SpatialIndex* index;
};

struct CoordLT {
	bool operator()(Coordinate *s1, Coordinate *s2) const {
		return s1->compareTo(*s2)<0;
	}
	bool operator()(const Coordinate *s1, const Coordinate *s2) const {
		return s1->compareTo(*s2)<0;
	}
};

class NodeMap{
public:
	map<Coordinate*,Node*,CoordLT>nodeMap;
	NodeFactory *nodeFact;
	// newNodeFact will be deleted by ~NodeMap
	NodeMap(NodeFactory *newNodeFact);
	virtual ~NodeMap();
	Node* addNode(const Coordinate& coord);
	Node* addNode(Node *n);
	void add(EdgeEnd *e);
	Node *find(const Coordinate& coord) const;
	map<Coordinate*,Node*,CoordLT>::const_iterator iterator() const;
	map<Coordinate*,Node*,CoordLT>::iterator iterator();
	//Collection values(); //Doesn't work yet. Use iterator.
	//vector instead of Collection
	vector<Node*>* getBoundaryNodes(int geomIndex) const; //returns new obj
	string print() const;
};

class EdgeRing;

class DirectedEdge: public EdgeEnd{
public:
	static int depthFactor(int currLocation, int nextLocation);
	DirectedEdge();	
	//virtual ~DirectedEdge();	
	DirectedEdge(Edge *newEdge, bool newIsForward);
	inline Edge* getEdge();
	inline void setInResult(bool newIsInResult);
	inline bool isInResult();
	inline bool isVisited();
	inline void setVisited(bool newIsVisited);
	inline void setEdgeRing(EdgeRing *newEdgeRing);
	inline EdgeRing* getEdgeRing();
	inline void setMinEdgeRing(EdgeRing *newMinEdgeRing);
	inline EdgeRing* getMinEdgeRing();
	inline int getDepth(int position);
	void setDepth(int position, int newDepth);
	int getDepthDelta();
	void setVisitedEdge(bool newIsVisited);

	/**
	 * Each Edge gives rise to a pair of symmetric DirectedEdges,
	 * in opposite directions.
	 * @return the DirectedEdge for the same Edge but in the
	 *         opposite direction
	 */
	inline DirectedEdge* getSym();

	inline bool isForward();
	inline void setSym(DirectedEdge *de);
	inline DirectedEdge* getNext();
	inline void setNext(DirectedEdge *newNext);
	inline DirectedEdge* getNextMin();
	inline void setNextMin(DirectedEdge *newNextMin);
	bool isLineEdge();
	bool isInteriorAreaEdge();
	void setEdgeDepths(int position, int newDepth);
	void OLDsetEdgeDepths(int position, int newDepth);
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

// INLINES
Edge* DirectedEdge::getEdge() { return edge; }
void DirectedEdge::setInResult(bool v) { isInResultVar=v; }
bool DirectedEdge::isInResult() { return isInResultVar; }
bool DirectedEdge::isVisited() { return isVisitedVar; }
void DirectedEdge::setVisited(bool v) { isVisitedVar=v; }
void DirectedEdge::setEdgeRing(EdgeRing *er) { edgeRing=er; }
EdgeRing* DirectedEdge::getEdgeRing() { return edgeRing; }
void DirectedEdge::setMinEdgeRing(EdgeRing *mer) { minEdgeRing=mer; }
EdgeRing* DirectedEdge::getMinEdgeRing() { return minEdgeRing; }
int DirectedEdge::getDepth(int position){ return depth[position]; }
DirectedEdge* DirectedEdge::getSym() { return sym; }
bool DirectedEdge::isForward() { return isForwardVar; }
void DirectedEdge::setSym(DirectedEdge *de){ sym=de; }
DirectedEdge* DirectedEdge::getNext() { return next; }
void DirectedEdge::setNext(DirectedEdge *newNext) { next=newNext; }
DirectedEdge* DirectedEdge::getNextMin() { return nextMin; }
void DirectedEdge::setNextMin(DirectedEdge *nm) { nextMin=nm; }

class EdgeRing {
public:

	// CGAlgorithms argument obsoleted, unused.
	EdgeRing(DirectedEdge *newStart, const GeometryFactory *newGeometryFactory, CGAlgorithms *newCga=NULL);

	virtual ~EdgeRing();
	bool isIsolated();
	bool isHole();
	const Coordinate& getCoordinate(int i);
	LinearRing* getLinearRing();
	Label* getLabel();
	bool isShell();
	EdgeRing *getShell();
	void setShell(EdgeRing *newShell);
	void addHole(EdgeRing *edgeRing);
	Polygon* toPolygon(const GeometryFactory* geometryFactory);
	void computeRing();
	virtual DirectedEdge* getNext(DirectedEdge *de)=0;
	virtual void setEdgeRing(DirectedEdge *de, EdgeRing *er)=0;
	vector<DirectedEdge*>* getEdges();
	int getMaxNodeDegree();
	void setInResult();
	bool containsPoint(Coordinate& p);
protected:
	DirectedEdge *startDe; // the directed edge which starts the list of edges for this EdgeRing
	const GeometryFactory *geometryFactory;
	//CGAlgorithms *cga;
	void computePoints(DirectedEdge *newStart);
	void mergeLabel(Label *deLabel);
	void mergeLabel(Label *deLabel, int geomIndex);
	void addPoints(Edge *edge, bool isForward, bool isFirstEdge);
	vector<EdgeRing*>* holes; // a list of EdgeRings which are holes in this EdgeRing
private:
	int maxNodeDegree;
	vector<DirectedEdge*>* edges; // the DirectedEdges making up this EdgeRing
	CoordinateSequence* pts;
	Label* label; // label stores the locations of each geometry on the face surrounded by this ring
	LinearRing *ring;  // the ring created for this EdgeRing
	bool isHoleVar;
	EdgeRing *shell;   // if non-null, the ring is a hole and this EdgeRing is its containing shell
	void computeMaxNodeDegree();
};

class PlanarGraph {
public:
	static CGAlgorithms *cga;
//	static LineIntersector *li;
	static void linkResultDirectedEdges(vector<Node*>* allNodes); // throw(TopologyException *);
	// nodeFact will be deleted by ~NodeMap
	PlanarGraph(NodeFactory *nodeFact);
	PlanarGraph();
	virtual ~PlanarGraph();
	virtual vector<Edge*>::iterator getEdgeIterator();
	virtual vector<EdgeEnd*>* getEdgeEnds();
	virtual bool isBoundaryNode(int geomIndex,Coordinate& coord);
	virtual void add(EdgeEnd *e);
	virtual map<Coordinate*,Node*,CoordLT>::iterator getNodeIterator();
	virtual vector<Node*>* getNodes();
	virtual Node* addNode(Node *node);
	virtual Node* addNode(const Coordinate& coord);
	virtual Node* find(Coordinate& coord);
	virtual void addEdges(vector<Edge*>* edgesToAdd);
	virtual void linkResultDirectedEdges();
	virtual void linkAllDirectedEdges();
	virtual EdgeEnd* findEdgeEnd(Edge *e);
	virtual Edge* findEdge(const Coordinate& p0,const Coordinate& p1);
	virtual Edge* findEdgeInSameDirection(const Coordinate& p0,const Coordinate& p1);
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
	bool matchInSameDirection(const Coordinate& p0, const Coordinate& p1, const Coordinate& ep0, const Coordinate& ep1);
};

struct LineStringLT {
	bool operator()(const LineString *ls1, const LineString *ls2) const {
		return ls1->compareTo(ls2)<0;
	}
};

class GeometryGraph: public PlanarGraph {
public:
	static bool isInBoundary(int boundaryCount);
	static int determineBoundary(int boundaryCount);
	GeometryGraph();
	virtual ~GeometryGraph();
	GeometryGraph(int newArgIndex, const Geometry *newParentGeom);
	const Geometry* getGeometry();
	vector<Node*>* getBoundaryNodes();
	CoordinateSequence* getBoundaryPoints();
	Edge* findEdge(const LineString *line);
	void computeSplitEdges(vector<Edge*> *edgelist);
	void addEdge(Edge *e);
	void addPoint(Coordinate& pt);
	SegmentIntersector* computeSelfNodes(LineIntersector *li,
		bool computeRingSelfNodes);
	SegmentIntersector* computeEdgeIntersections(GeometryGraph *g,
		LineIntersector *li,bool includeProper);
	vector<Edge*> *getEdges();
	bool hasTooFewPoints();
	const Coordinate& getInvalidPoint(); 

private:

	const Geometry *parentGeom;

	/*
	 * The lineEdgeMap is a map of the linestring components of the
	 * parentGeometry to the edges which are derived from them.
	 * This is used to efficiently perform findEdge queries
	 */
	map<const LineString*,Edge*,LineStringLT>* lineEdgeMap;

	/*
	 * If this flag is true, the Boundary Determination Rule will
	 * used when deciding whether nodes are in the boundary or not
	 */
	bool useBoundaryDeterminationRule;

	/*
	 * the index of this geometry as an argument to a spatial function
	 * (used for labelling)
	 */
	int argIndex;

	vector<Node*>* boundaryNodes;

	bool hasTooFewPointsVar;

	Coordinate invalidPoint; 

	EdgeSetIntersector* createEdgeSetIntersector();

	void add(const Geometry *g); // throw(UnsupportedOperationException *);
	void addCollection(const GeometryCollection *gc);
	void addPoint(const Point *p);
	void addPolygonRing(const LinearRing *lr,int cwLeft,int cwRight);
	void addPolygon(const Polygon *p);
	void addLineString(const LineString *line);

	void insertPoint(int argIndex, const Coordinate& coord,int onLocation);
	void insertBoundaryPoint(int argIndex, const Coordinate& coord);

	void addSelfIntersectionNodes(int argIndex);
	void addSelfIntersectionNode(int argIndex,Coordinate& coord,int loc);
};

class SegmentString;
class NodingValidator;

/*
 * Validates that a collection of SegmentStrings is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 * @version 1.4
 */
class EdgeNodingValidator {
private:
	static vector<SegmentString*>* toSegmentStrings(vector<Edge*> *edges);
	NodingValidator *nv;
public:
	EdgeNodingValidator(vector<Edge*> *edges);
	virtual ~EdgeNodingValidator();
	void checkValid();
};

//Operators
bool operator==(const Edge &a, const Edge &b);

} // namespace geos

#endif // ifndef GEOS_GEOMGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.10  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.9  2005/02/22 16:24:18  strk
 * cached number of points in Edge
 *
 * Revision 1.8  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.7  2004/11/20 15:46:45  strk
 * Added composing Z management functions and elements for class Node
 *
 * Revision 1.6  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.5  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.4  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.3  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.6  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.5  2004/05/26 09:50:05  strk
 * Added comments about OverlayNodeFactory() ownership in NodeMap and PlanarGraph constuctors
 *
 * Revision 1.4  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.3  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.2  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.27  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.26  2003/11/12 15:43:38  strk
 * Added some more throw specifications
 *
 * Revision 1.25  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.24  2003/11/06 18:45:05  strk
 * Added throw specification for DirectEdgeStar::linkResultDirectedEdges()
 *
 **********************************************************************/

