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


#ifndef GEOS_GEOMGRAPH_H
#define GEOS_GEOMGRAPH_H

#include <geos/geom.h>
#include <geos/geomgraphindex.h>
#include <geos/geosAlgorithm.h>
#include <geos/indexQuadtree.h>
#include <geos/platform.h>
#include <geos/noding.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility> // std::pair
#include <vector>

namespace geos {

/** \brief
 * Contains classes that implement topology graphs.
 * 
 * The Java Topology Suite (JTS) is a Java API that implements a core
 * set of spatial data operations using an explicit precision model
 * and robust geometric algorithms. JTS is int ended to be used in the
 * development of applications that support the validation, cleaning,
 * integration and querying of spatial datasets.
 *
 * JTS attempts to implement the OpenGIS Simple Features Specification (SFS)
 * as accurately as possible.  In some cases the SFS is unclear or omits a
 * specification; in this case JTS attempts to choose a reasonable and
 * consistent alternative.  Differences from and elaborations of the SFS
 * are documented in this specification.
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
namespace geomgraph { // geos.geomgraph

// forward decls

namespace index {
	class MonotoneChainEdge;
	class EdgeSetIntersector;
	class SegmentIntersector;
} // geos.geomgraph.index

class DirectedEdge;
class Edge;
class EdgeIntersectionList;
class EdgeRing;
class GeometryGraph;
class Node;


class Position {
public:
	enum {
		/*
		 * An indicator that a Location is <i>on</i>
		 * a GraphComponent
		 */
		ON=0,

		/*
		 * An indicator that a Location is to the
		 * <i>left</i> of a GraphComponent
		 */  
		LEFT,

		/*
		 * An indicator that a Location is to the
		 * <i>right</i> of a GraphComponent
		 */  
		RIGHT
	};

	/**
	 * Returns LEFT if the position is RIGHT, RIGHT if
	 * the position is LEFT, or the position otherwise.
	 */
	static int opposite(int position);
};

class TopologyLocation {
public:
	TopologyLocation();
	~TopologyLocation();
	TopologyLocation(const std::vector<int> &newLocation);

	/**
	 * Constructs a TopologyLocation specifying how points on,
	 * to the left of, and to the right of some GraphComponent
	 * relate to some Geometry. Possible values for the
	 * parameters are Location.NULL, Location.EXTERIOR, Location.BOUNDARY, 
	 * and Location.INTERIOR.
	 * @see Location
	 */
	TopologyLocation(int on, int left, int right);
	TopologyLocation(int on);
	TopologyLocation(const TopologyLocation &gl);
	int get(unsigned int posIndex) const;
	bool isNull() const;
	bool isAnyNull() const;
	bool isEqualOnSide(const TopologyLocation &le, int locIndex) const;
	bool isArea() const;
	bool isLine() const;
	void flip();
	void setAllLocations(int locValue);
	void setAllLocationsIfNull(int locValue);
	void setLocation(unsigned int locIndex, int locValue);
	void setLocation(int locValue);
	const std::vector<int> &getLocations() const;
	void setLocations(int on, int left, int right);
	bool allPositionsEqual(int loc) const;
	void merge(const TopologyLocation &gl);
	std::string toString() const;
private:
	std::vector<int> location;
};

class Label {
public:
	static Label* toLineLabel(const Label& label);
	Label(int onLoc);
	Label(int geomIndex, int onLoc);
	Label(int onLoc, int leftLoc, int rightLoc);
	Label(const Label &l);
	Label();
	virtual ~Label();
	Label(int geomIndex,int onLoc,int leftLoc,int rightLoc);
	void flip();
	int getLocation(int geomIndex, int posIndex) const;
	int getLocation(int geomIndex) const;
	void setLocation(int geomIndex, int posIndex, int location);
	void setLocation(int geomIndex, int location);
	void setAllLocations(int geomIndex, int location);
	void setAllLocationsIfNull(int geomIndex, int location);
	void setAllLocationsIfNull(int location);
	void merge(const Label &lbl);
	int getGeometryCount() const;
	bool isNull(int geomIndex) const;
	bool isAnyNull(int geomIndex) const;
	bool isArea() const;
	bool isArea(int geomIndex) const;
	bool isLine(int geomIndex) const;
	bool isEqualOnSide(const Label &lbl, int side) const;
	bool allPositionsEqual(int geomIndex, int loc) const;
	void toLine(int geomIndex);
	std::string toString() const;
protected:
	TopologyLocation elt[2];
};

class Depth {
public:
	static int depthAtLocation(int location);
	Depth();
	virtual ~Depth();
	int getDepth(int geomIndex,int posIndex) const;
	void setDepth(int geomIndex,int posIndex,int depthValue);
	int getLocation(int geomIndex,int posIndex) const;
	void add(int geomIndex,int posIndex,int location);
	bool isNull() const;
	bool isNull(int geomIndex) const;
	bool isNull(int geomIndex, int posIndex) const;
	int getDelta(int geomIndex) const;
	void normalize();
	void add(const Label& lbl);
	std::string toString() const;
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
	 * Returns the quadrant of a directed line segment
	 * (specified as x and y displacements, which cannot both be 0).
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
	 * Returns the right-hand quadrant of the halfplane defined by
	 * the two quadrants,
	 * or -1 if the quadrants are opposite, or the quadrant if they
	 * are identical.
	 */
	static int commonHalfPlane(int quad1, int quad2);

	/**
	 * Returns whether the given quadrant lies within the given halfplane
	 * (specified by its right-hand quadrant).
	 */
	static bool isInHalfPlane(int quad, int halfPlane);

	/**
	 * Returns true if the given quadrant is 0 or 1.
	 */
	static bool isNorthern(int quad);
};

class GraphComponent {
public:
	GraphComponent();

	/*
	 * GraphComponent takes ownership of the given Label.
	 * newLabel is deleted by destructor.
	 */
	GraphComponent(Label* newLabel); 
	virtual ~GraphComponent();
	Label* getLabel();
	virtual void setLabel(Label* newLabel);
	virtual void setInResult(bool isInResult) { isInResultVar=isInResult; }
	virtual bool isInResult() const { return isInResultVar; }
	virtual void setCovered(bool isCovered);
	virtual bool isCovered() const { return isCoveredVar; }
	virtual bool isCoveredSet() const { return isCoveredSetVar; }
	virtual bool isVisited() const { return isVisitedVar; }
	virtual void setVisited(bool isVisited) { isVisitedVar = isVisited; }
	virtual bool isIsolated() const=0;
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

class EdgeEnd {
friend class Unload;
public:
	EdgeEnd();
	virtual ~EdgeEnd();
	EdgeEnd(Edge* newEdge, const Coordinate& newP0,
		const Coordinate& newP1);
	EdgeEnd(Edge* newEdge, const Coordinate& newP0,
		const Coordinate& newP1, Label* newLabel);
	virtual Edge* getEdge();
	virtual Label* getLabel();
	virtual Coordinate& getCoordinate();
	virtual Coordinate& getDirectedCoordinate();
	virtual int getQuadrant();
	virtual double getDx();
	virtual double getDy();
	virtual void setNode(Node* newNode);
	virtual Node* getNode();
	virtual int compareTo(const EdgeEnd *e) const;
	virtual int compareDirection(const EdgeEnd *e) const;
	virtual void computeLabel();
	virtual std::string print();
protected:
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
	bool operator()(const EdgeEnd *s1, const EdgeEnd *s2) const {
		return s1->compareTo(s2)<0;
	}
};

/**
 * A EdgeEndStar is an ordered list of EdgeEnds around a node.
 * They are maintained in CCW order (starting with the positive x-axis)
 * around the node for efficient lookup and topology building.
 *
 * @version 1.4
 */
class EdgeEndStar {
public:

	typedef std::set<EdgeEnd *, EdgeEndLT> container;

	typedef container::iterator iterator;
	typedef container::reverse_iterator reverse_iterator;

	EdgeEndStar();

	virtual ~EdgeEndStar() {};

	virtual void insert(EdgeEnd *e)=0;

	virtual Coordinate& getCoordinate();

	virtual unsigned int getDegree() { return edgeMap.size(); }

	virtual iterator begin() { return edgeMap.begin(); }
	virtual iterator end() { return edgeMap.end(); }

	virtual reverse_iterator rbegin() { return edgeMap.rbegin(); }
	virtual reverse_iterator rend() { return edgeMap.rend(); }

	virtual container &getEdges() { return edgeMap; }

	virtual EdgeEnd* getNextCW(EdgeEnd *ee);

	virtual void computeLabelling(std::vector<GeometryGraph*> *geom);
		// throw(TopologyException *);

	virtual int getLocation(int geomIndex,
		const Coordinate& p,
		std::vector<GeometryGraph*> *geom); 

	virtual bool isAreaLabelsConsistent();

	virtual void propagateSideLabels(int geomIndex);
		// throw(TopologyException *);

	//virtual int findIndex(EdgeEnd *eSearch);
	virtual iterator find(EdgeEnd *eSearch) {
		return edgeMap.find(eSearch);
	}

	virtual std::string print();

protected:

	EdgeEndStar::container edgeMap;

	/**
	 * Insert an EdgeEnd into the map.
	 */
	virtual void insertEdgeEnd(EdgeEnd *e) { edgeMap.insert(e); }

private:

	int ptInAreaLocation[2];

	virtual void computeEdgeEndLabels();

	virtual bool checkAreaLabelsConsistent(int geomIndex);

};

/**
 * \brief
 * A DirectedEdgeStar is an ordered list of <b>outgoing</b> DirectedEdges around a node.
 *
 * It supports labelling the edges as well as linking the edges to form both
 * MaximalEdgeRings and MinimalEdgeRings.
 *
 * Last port: geomgraph/DirectedEdgeStar.java rev. 1.4 (JTS-1.7)
 */
class DirectedEdgeStar: public EdgeEndStar {

public:

	DirectedEdgeStar()
		:
		EdgeEndStar(),
		resultAreaEdgeList(0)
	{}

	~DirectedEdgeStar() {
		delete resultAreaEdgeList;
	}

	/// Insert a directed edge in the list
	void insert(EdgeEnd *ee);

	Label &getLabel() { return label; }

	int getOutgoingDegree();

	int getOutgoingDegree(EdgeRing *er);

	DirectedEdge* getRightmostEdge();

	/** \brief
	 * Compute the labelling for all dirEdges in this star, as well
	 * as the overall labelling
	 */
	void computeLabelling(std::vector<GeometryGraph*> *geom); // throw(TopologyException *);

	/** \brief
	 * For each dirEdge in the star,
	 * merge the label from the sym dirEdge into the label
	 */
	void mergeSymLabels();

	/// Update incomplete dirEdge labels from the labelling for the node
	void updateLabelling(Label *nodeLabel);


	/**
	 * Traverse the star of DirectedEdges, linking the included edges together.
	 * To link two dirEdges, the <next> pointer for an incoming dirEdge
	 * is set to the next outgoing edge.
	 * 
	 * DirEdges are only linked if:
	 * 
	 * - they belong to an area (i.e. they have sides)
	 * - they are marked as being in the result
	 * 
	 * Edges are linked in CCW order (the order they are stored).
	 * This means that rings have their face on the Right
	 * (in other words,
	 * the topological location of the face is given by the RHS label of the DirectedEdge)
	 * 
	 * PRECONDITION: No pair of dirEdges are both marked as being in the result
	 */
	void linkResultDirectedEdges(); // throw(TopologyException *);

	void linkMinimalDirectedEdges(EdgeRing *er);

	void linkAllDirectedEdges();

	/** \brief
	 * Traverse the star of edges, maintaing the current location in the result
	 * area at this node (if any).
	 *
	 * If any L edges are found in the interior of the result, mark them as covered.
	 */
	void findCoveredLineEdges();

	/** \brief
	 * Compute the DirectedEdge depths for a subsequence of the edge array.
	 *
	 * @return the last depth assigned (from the R side of the last edge visited)
	 */
	void computeDepths(DirectedEdge *de);

	std::string print();

private:

	/**
	 * A list of all outgoing edges in the result, in CCW order
	 */
	std::vector<DirectedEdge*> *resultAreaEdgeList;

	Label label;

	/// \brief
	/// Returned vector is onwed by DirectedEdgeStar object, but
	/// lazily created
	std::vector<DirectedEdge*>* getResultAreaEdges();

	/// States for linResultDirectedEdges
	enum {
		SCANNING_FOR_INCOMING=1,
		LINKING_TO_OUTGOING
	};

	int computeDepths(EdgeEndStar::iterator startIt,
		EdgeEndStar::iterator endIt, int startDepth);
};

class Node: public GraphComponent {
using GraphComponent::setLabel;

public:
	Node(const Coordinate& newCoord, EdgeEndStar* newEdges);
	virtual ~Node();
	virtual const Coordinate& getCoordinate() const;
	virtual EdgeEndStar* getEdges();
	virtual bool isIsolated() const;
	virtual void add(EdgeEnd *e);
	virtual void mergeLabel(const Node* n);
	virtual void mergeLabel(const Label* label2);
	virtual void setLabel(int argIndex, int onLocation);
	virtual void setLabelBoundary(int argIndex);
	virtual int computeMergedLocation(const Label* label2, int eltIndex);
	virtual std::string print();
	virtual const std::vector<double> &getZ() const;
	virtual void addZ(double);
	virtual bool isIncidentEdgeInResult() const;

protected:
	Coordinate coord;
	EdgeEndStar* edges;
	virtual void computeIM(IntersectionMatrix *im) {};

private:
	std::vector<double>zvals;
	double ztot;

};

class NodeFactory {
public:
	virtual Node* createNode(const Coordinate &coord) const;
	static const NodeFactory &instance();
protected:
	NodeFactory() {};
};

class EdgeIntersection {
public:
	Coordinate coord;
	int segmentIndex;
	double dist;
	EdgeIntersection(const Coordinate& newCoord, int newSegmentIndex, double newDist);
	virtual ~EdgeIntersection();
	int compare(int newSegmentIndex, double newDist) const;
	bool isEndPoint(int maxSegmentIndex);
	std::string print() const;
	int compareTo(const EdgeIntersection *) const;
};

struct EdgeIntersectionLessThen {
	bool operator()(const EdgeIntersection *ei1,
		const EdgeIntersection *ei2) const
	{
		if ( ei1->segmentIndex<ei2->segmentIndex ||
			( ei1->segmentIndex==ei2->segmentIndex &&
		     	ei1->dist<ei2->dist ) ) return true;
		return false;
	}
};

/**
 * A list of edge intersections along an Edge.
 * Implements splitting an edge with intersections
 * into multiple resultant edges.
 */
class EdgeIntersectionList{
public:
	typedef std::set<EdgeIntersection *, EdgeIntersectionLessThen> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;

private:
	container nodeMap;

public:

	Edge *edge;
	EdgeIntersectionList(Edge *edge);
	~EdgeIntersectionList();

	/*
	 * Adds an intersection into the list, if it isn't already there.
	 * The input segmentIndex and dist are expected to be normalized.
	 * @return the EdgeIntersection found or added
	 */
	EdgeIntersection* add(const Coordinate& coord,
		int segmentIndex, double dist);

	iterator begin() { return nodeMap.begin(); }
	iterator end() { return nodeMap.end(); }
	const_iterator begin() const { return nodeMap.begin(); }
	const_iterator end() const { return nodeMap.end(); }

	bool isEmpty() const;
	bool isIntersection(const Coordinate& pt) const;

	/*
	 * Adds entries for the first and last points of the edge to the list
	 */
	void addEndpoints();

	/**
	 * Creates new edges for all the edges that the intersections in this
	 * list split the parent edge into.
	 * Adds the edges to the input list (this is so a single list
	 * can be used to accumulate all split edges for a Geometry).
	 *
	 * @param edgeList a list of EdgeIntersections
	 */
	void addSplitEdges(std::vector<Edge*> *edgeList);

	Edge *createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1);
	std::string print() const;

};

class EdgeList {

private:

	std::vector<Edge*> edges;

	/**
	 * An index of the edges, for fast lookup.
	 *
	 * a Quadtree is used, because this index needs to be dynamic
	 * (e.g. allow insertions after queries).
	 * An alternative would be to use an ordered set based on the values
	 * of the edge coordinates
	 *
	 */
	geos::index::SpatialIndex* index;

public:
	EdgeList()
		:
		edges(),
		index(new geos::index::quadtree::Quadtree())
	{}

	virtual ~EdgeList() { delete index; }

	/**
	 * Insert an edge unless it is already in the list
	 */
	void add(Edge *e);

	void addAll(const std::vector<Edge*> &edgeColl);

	std::vector<Edge*> &getEdges() { return edges; }

	Edge* findEqualEdge(Edge* e);

	Edge* get(int i);

	int findEdgeIndex(Edge *e);

	std::string print();

};

class NodeMap{
public:

	typedef std::map<Coordinate*,Node*,CoordinateLessThen> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef std::pair<Coordinate*,Node*> pair;

	container nodeMap;
	const NodeFactory &nodeFact;
	// newNodeFact will be deleted by ~NodeMap
	NodeMap(const NodeFactory &newNodeFact);
	virtual ~NodeMap();
	Node* addNode(const Coordinate& coord);
	Node* addNode(Node *n);
	void add(EdgeEnd *e);
	Node *find(const Coordinate& coord) const;
	const_iterator begin() const { return nodeMap.begin(); }
	iterator begin() { return nodeMap.begin(); }
	//Collection values(); //Doesn't work yet. Use iterator.
	//vector instead of Collection

	void getBoundaryNodes(int geomIndex,
		std::vector<Node*>&bdyNodes) const;

	std::string print() const;
};


class DirectedEdge: public EdgeEnd{
public:
	static int depthFactor(int currLocation, int nextLocation);
	//DirectedEdge();	
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
	std::string print();
	std::string printEdge();
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

	EdgeRing(DirectedEdge *newStart, const GeometryFactory *newGeometryFactory);

	virtual ~EdgeRing();
	bool isIsolated();
	bool isHole();

	/*
	 * Return a pointer to the LinearRing owned by
	 * this object. Make a copy if you need it beyond
	 * this objects's lifetime.
	 */
	LinearRing* getLinearRing();

	Label* getLabel();
	bool isShell();
	EdgeRing *getShell();
	void setShell(EdgeRing *newShell);
	void addHole(EdgeRing *edgeRing);

	/*
	 * Return a Polygon copying coordinates from this
	 * EdgeRing and its holes.
	 */
	Polygon* toPolygon(const GeometryFactory* geometryFactory);

	void computeRing();
	virtual DirectedEdge* getNext(DirectedEdge *de)=0;
	virtual void setEdgeRing(DirectedEdge *de, EdgeRing *er)=0;
	std::vector<DirectedEdge*>* getEdges();
	int getMaxNodeDegree();
	void setInResult();
	bool containsPoint(const Coordinate& p);
protected:
	DirectedEdge *startDe; // the directed edge which starts the list of edges for this EdgeRing
	const GeometryFactory *geometryFactory;
	void computePoints(DirectedEdge *newStart);
	void mergeLabel(Label *deLabel);
	void mergeLabel(Label *deLabel, int geomIndex);
	void addPoints(Edge *edge, bool isForward, bool isFirstEdge);
	std::vector<EdgeRing*> holes; // a list of EdgeRings which are holes in this EdgeRing
private:
	int maxNodeDegree;
	std::vector<DirectedEdge*> edges; // the DirectedEdges making up this EdgeRing
	CoordinateSequence* pts;
	Label* label; // label stores the locations of each geometry on the face surrounded by this ring
	LinearRing *ring;  // the ring created for this EdgeRing
	bool isHoleVar;
	EdgeRing *shell;   // if non-null, the ring is a hole and this EdgeRing is its containing shell
	void computeMaxNodeDegree();
};

/**
 * \brief
 * Represents a directed graph which is embeddable in a planar surface.
 * 
 * The computation of the IntersectionMatrix relies on the use of a structure
 * called a "topology graph".  The topology graph contains nodes and edges
 * corresponding to the nodes and line segments of a Geometry. Each
 * node and edge in the graph is labeled with its topological location
 * relative to the source geometry.
 * 
 * Note that there is no requirement that points of self-intersection
 * be a vertex.
 * Thus to obtain a correct topology graph, Geometry objects must be
 * self-noded before constructing their graphs.
 *
 * Two fundamental operations are supported by topology graphs:
 * 
 *  - Computing the intersections between all the edges and nodes of
 *    a single graph
 *  - Computing the intersections between the edges and nodes of two
 *    different graphs
 * 
 */
class PlanarGraph {
public:
	//static algorithm::CGAlgorithms *cga;

	static void linkResultDirectedEdges(std::vector<Node*>* allNodes);
			// throw(TopologyException);

	PlanarGraph(const NodeFactory &nodeFact);

	PlanarGraph();

	virtual ~PlanarGraph();

	virtual std::vector<Edge*>::iterator getEdgeIterator();

	virtual std::vector<EdgeEnd*>* getEdgeEnds();

	virtual bool isBoundaryNode(int geomIndex, const Coordinate& coord);

	virtual void add(EdgeEnd *e);

	virtual NodeMap::iterator getNodeIterator();

	virtual std::vector<Node*>* getNodes();

	virtual Node* addNode(Node *node);

	virtual Node* addNode(const Coordinate& coord);

	virtual Node* find(Coordinate& coord);

	virtual void addEdges(const std::vector<Edge*> &edgesToAdd);

	virtual void linkResultDirectedEdges();

	virtual void linkAllDirectedEdges();

	virtual EdgeEnd* findEdgeEnd(Edge *e);

	virtual Edge* findEdge(const Coordinate& p0,const Coordinate& p1);

	virtual Edge* findEdgeInSameDirection(const Coordinate& p0,const Coordinate& p1);

	virtual std::string printEdges();

	virtual NodeMap* getNodeMap();

protected:

	std::vector<Edge*> *edges;

	NodeMap *nodes;

	std::vector<EdgeEnd*> *edgeEndList;

	virtual void insertEdge(Edge *e);

private:

	bool matchInSameDirection(const Coordinate& p0, const Coordinate& p1,
			const Coordinate& ep0, const Coordinate& ep1);
};

class GeometryGraph: public PlanarGraph {
using PlanarGraph::add;
using PlanarGraph::findEdge;

private:

	const Geometry* parentGeom;

	/**
	 * The lineEdgeMap is a map of the linestring components of the
	 * parentGeometry to the edges which are derived from them.
	 * This is used to efficiently perform findEdge queries
	 */
	std::map<const LineString*,Edge*,LineStringLT> lineEdgeMap;

	/**
	 * If this flag is true, the Boundary Determination Rule will
	 * used when deciding whether nodes are in the boundary or not
	 */
	bool useBoundaryDeterminationRule;

	/**
	 * the index of this geometry as an argument to a spatial function
	 * (used for labelling)
	 */
	int argIndex;

	/// Cache for fast responses to getBoundaryPoints
	std::auto_ptr< CoordinateSequence > boundaryPoints;

	std::auto_ptr< std::vector<Node*> > boundaryNodes;

	bool hasTooFewPointsVar;

	Coordinate invalidPoint; 

	std::vector<index::SegmentIntersector*> newSegmentIntersectors;

	/// Allocates a new EdgeSetIntersector. Remember to delete it!
	index::EdgeSetIntersector* createEdgeSetIntersector();

	void add(const Geometry *g); // throw(UnsupportedOperationException);

	void addCollection(const GeometryCollection *gc);

	void addPoint(const Point *p);

	void addPolygonRing(const LinearRing *lr,int cwLeft,int cwRight);

	void addPolygon(const Polygon *p);

	void addLineString(const LineString *line);

	void insertPoint(int argIndex, const Coordinate& coord,int onLocation);

	void insertBoundaryPoint(int argIndex, const Coordinate& coord);

	void addSelfIntersectionNodes(int argIndex);

	void addSelfIntersectionNode(int argIndex,
		const Coordinate& coord, int loc);

public:

	static bool isInBoundary(int boundaryCount);

	static int determineBoundary(int boundaryCount);

	GeometryGraph()
		:
		PlanarGraph(),
		parentGeom(NULL),
		useBoundaryDeterminationRule(false),
		argIndex(-1),
		hasTooFewPointsVar(false)
	{}

	GeometryGraph(int newArgIndex, const Geometry *newParentGeom)
		:
		PlanarGraph(),
		parentGeom(newParentGeom),
		useBoundaryDeterminationRule(false),
		argIndex(newArgIndex),
		hasTooFewPointsVar(false)
	{
		if (parentGeom!=NULL) add(parentGeom);
	}

	virtual ~GeometryGraph() {}


	const Geometry* getGeometry() { return parentGeom; }

	/// Returned object is owned by this GeometryGraph
	std::vector<Node*>* getBoundaryNodes();

	void getBoundaryNodes(std::vector<Node*>&bdyNodes) {
		nodes->getBoundaryNodes(argIndex, bdyNodes);
	}

	/// Returned object is owned by this GeometryGraph
	CoordinateSequence* getBoundaryPoints();

	Edge* findEdge(const LineString *line);

	void computeSplitEdges(std::vector<Edge*> *edgelist);

	void addEdge(Edge *e);

	void addPoint(Coordinate& pt);

	/**
	 * \brief
	 * Compute self-nodes, taking advantage of the Geometry type to
	 * minimize the number of intersection tests.  (E.g. rings are
	 * not tested for self-intersection, since they are assumed to be valid).
	 *
	 * @param li the LineIntersector to use
	 *
	 * @param computeRingSelfNodes if <false>, intersection checks are
	 *	optimized to not test rings for self-intersection
	 *
	 * @return the SegmentIntersector used, containing information about
	 *	the intersections found
	 */
	index::SegmentIntersector* computeSelfNodes(algorithm::LineIntersector *li,
		bool computeRingSelfNodes);

	// Quick inline calling the function above, the above should probably
	// be deprecated.
	index::SegmentIntersector* computeSelfNodes(algorithm::LineIntersector& li,
		bool computeRingSelfNodes)
	{
		return computeSelfNodes(&li, computeRingSelfNodes);
	}

	index::SegmentIntersector* computeEdgeIntersections(GeometryGraph *g,
		algorithm::LineIntersector *li,bool includeProper);

	std::vector<Edge*> *getEdges();

	bool hasTooFewPoints();

	const Coordinate& getInvalidPoint(); 

};


/**
 * Validates that a collection of SegmentStrings is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 * @version 1.4
 */
class EdgeNodingValidator {

private:
	std::vector<noding::SegmentString*>& toSegmentStrings(std::vector<Edge*> *edges);
	noding::NodingValidator nv;
	noding::SegmentString::NonConstVect segStr;
	//std::vector<SegmentString*> segStr;
	std::vector<CoordinateSequence*>newCoordSeq;

public:

	EdgeNodingValidator(std::vector<Edge*> *edges)
		:
		nv(toSegmentStrings(edges))
	{}

	~EdgeNodingValidator();

	void checkValid() { nv.checkValid(); }
};

class Edge: public GraphComponent{
using GraphComponent::updateIM;

private:

	std::string name;

	/// Lazily-created, owned by Edge.
	index::MonotoneChainEdge *mce;

	/// Lazily-created, owned by Edge.
	Envelope *env;

	bool isIsolatedVar;

	Depth depth;

	int depthDelta;   // the change in area depth from the R to L side of this edge

	void testInvariant() const {
		assert(pts);
		assert(pts->size() > 1);
	}
public:

	static void updateIM(Label *lbl,IntersectionMatrix *im);

	/// Externally-set, owned by Edge. FIXME: refuse ownership
	CoordinateSequence* pts;

	EdgeIntersectionList eiList;

	//Edge();

	Edge(CoordinateSequence* newPts, Label *newLabel);

	Edge(CoordinateSequence* newPts);

	virtual ~Edge();

	virtual int getNumPoints() const {
		return pts->getSize();
	}

	virtual void setName(const std::string &newName) {
		name=newName;
	}

	virtual const CoordinateSequence* getCoordinates() const {
		testInvariant();
		return pts;
	}

	virtual const Coordinate& getCoordinate(int i) const {
		testInvariant();
		return pts->getAt(i);
	}

	virtual const Coordinate& getCoordinate() const {
		testInvariant();
		return pts->getAt(0);
	}


	virtual Depth &getDepth() { 
		testInvariant();
		return depth;
	}

	/** \brief
	 * The depthDelta is the change in depth as an edge is crossed from R to L
	 *
	 * @return the change in depth as the edge is crossed from R to L
	 */
	virtual int getDepthDelta() const {
		testInvariant();
		return depthDelta;
	}

	virtual void setDepthDelta(int newDepthDelta) {
		depthDelta=newDepthDelta;
		testInvariant();
	}

	virtual int getMaximumSegmentIndex() const {
		testInvariant();
		return getNumPoints()-1;
	}

	virtual EdgeIntersectionList& getEdgeIntersectionList() {
		testInvariant();
		return eiList;
	}

	/// \brief
	/// Return this Edge's index::MonotoneChainEdge,
	/// ownership is retained by this object.
	///
	virtual index::MonotoneChainEdge* getMonotoneChainEdge();

	virtual bool isClosed() const {
		testInvariant();
		return pts->getAt(0)==pts->getAt(getNumPoints()-1);
	}

	/** \brief
	 * An Edge is collapsed if it is an Area edge and it consists of
	 * two segments which are equal and opposite (eg a zero-width V).
	 */
	virtual bool isCollapsed() const;

	virtual Edge* getCollapsedEdge();

	virtual void setIsolated(bool newIsIsolated) {
		isIsolatedVar=newIsIsolated;
		testInvariant();
	}

	virtual bool isIsolated() const {
		testInvariant();
		return isIsolatedVar;
	}

	/** \brief
	 * Adds EdgeIntersections for one or both
	 * intersections found for a segment of an edge to the edge intersection list.
	 */
	virtual void addIntersections(algorithm::LineIntersector *li, int segmentIndex,
		int geomIndex);

	/// Add an EdgeIntersection for intersection intIndex.
	//
	/// An intersection that falls exactly on a vertex of the edge is normalized
	/// to use the higher of the two possible segmentIndexes
	///
	virtual void addIntersection(algorithm::LineIntersector *li, int segmentIndex,
		int geomIndex, int intIndex);

	/// Update the IM with the contribution for this component.
	//
	/// A component only contributes if it has a labelling for both parent geometries
	///
	virtual void computeIM(IntersectionMatrix *im) {
		updateIM(label, im);
		testInvariant();
	}

	/// return true if the coordinate sequences of the Edges are identical
	virtual bool isPointwiseEqual(const Edge *e) const;

	virtual std::string print() const;

	virtual std::string printReverse() const;

	/**
	 * equals is defined to be:
	 * 
	 * e1 equals e2
	 * <b>iff</b>
	 * the coordinates of e1 are the same or the reverse of the coordinates in e2
	 */
	virtual bool equals(const Edge& e) const;

	virtual bool equals(const Edge* e) const {
		assert(e);
		return equals(*e);
	}

	virtual Envelope* getEnvelope();
};


//Operators
inline bool operator==(const Edge &a, const Edge &b) {
	return a.equals(b);
}

} // namespace geos.geomgraph
} // namespace geos

#endif // ifndef GEOS_GEOMGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.39  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.38  2006/03/02 14:34:30  strk
 * GeometryGraphOperation::li made a non-static member, and not more a pointer
 *
 * Revision 1.37  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.36  2006/02/27 11:53:17  strk
 * DirectedEdgeStar made more safe trough assert(), use of standard iterator and
 * dynamic casts substituting static ones.
 *
 * Revision 1.35  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.34  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.33  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.32  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.31  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.30  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.29  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.28  2006/02/04 00:54:57  strk
 * - Doxygen dox updated
 * - LineStringLT struct moved from geomgraph.h to geom.h
 * - New planarSubgraph class
 * - Fixed ruby Makefiles to avoid running tests when disabled
 * - Renamed TESTS variable to XMLTESTS to not confuse 'make check' rule
 *
 * Revision 1.27  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.26  2005/12/07 19:18:23  strk
 * Changed PlanarGraph::addEdges and EdgeList::addAll to take
 * a const vector by reference rather then a non-const vector by
 * pointer.
 * Optimized polygon vector allocations in OverlayOp::computeOverlay.
 *
 * Revision 1.25  2005/11/29 15:45:39  strk
 * Fixed signedness of TopologyLocation methods, cleanups.
 *
 * Revision 1.24  2005/11/29 14:39:46  strk
 * Removed number of points cache in Edge, replaced with local caches.
 *
 * Revision 1.23  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.22  2005/11/24 23:43:13  strk
 * Yes another fix, sorry. Missing const-correctness.
 *
 * Revision 1.21  2005/11/24 23:24:38  strk
 * Fixed equals() function [ optimized in previous commit, but unchecked ]
 *
 * Revision 1.20  2005/11/21 16:03:20  strk
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
 * Revision 1.19  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.18  2005/11/16 22:21:45  strk
 * enforced const-correctness and use of initializer lists.
 *
 * Revision 1.17  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.16  2005/11/15 18:30:59  strk
 * Removed dead code
 *
 * Revision 1.15  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.14  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.13  2005/11/09 13:44:28  strk
 * Cleanups in Node and NodeMap.
 * Optimization of EdgeIntersectionLessThen.
 *
 * Revision 1.12  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.11  2005/06/25 10:20:39  strk
 * OverlayOp speedup (JTS port)
 *
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

