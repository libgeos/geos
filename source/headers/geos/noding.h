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
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.12  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.11  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.10  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.9  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.8  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.7  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.6  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.5  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.4  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.3  2004/04/14 09:30:48  strk
 * Private iterated noding funx now use int* instead of vector to know
 * when it's time to stop.
 *
 * Revision 1.2  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#ifndef GEOS_NODING_H
#define GEOS_NODING_H

#include <string>
#include <vector>
#include <set>
#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>

using namespace std;

namespace geos {

/*
 * Represents an intersection point between two {@link SegmentString}s.
 *
 */
class SegmentNode {
public:
	Coordinate *coord;   // the point of intersection
	int segmentIndex;   // the index of the containing line segment in the parent edge
	double dist;        // the edge distance of this point along the containing line segment
	SegmentNode(Coordinate *newCoord, int nSegmentIndex, double newDist);
	virtual ~SegmentNode();
	/**
	* @return -1 this EdgeIntersection is located before the argument location
	* @return 0 this EdgeIntersection is at the argument location
	* @return 1 this EdgeIntersection is located after the argument location
	*/
	int compare(int cSegmentIndex,double cDist);
	bool isEndPoint(int maxSegmentIndex);
	int compareTo(void* obj);
	string print();
};

struct SegmentNodeLT {
	bool operator()(SegmentNode *s1, SegmentNode *s2) const {
		return s1->compareTo(s2)<0;
	}
};

class SegmentString;
/*
 * A list of the {@link SegmentNode}s present along a
 * noded {@link SegmentString}.
 *
 */
class SegmentNodeList {
private:
	set<SegmentNode*,SegmentNodeLT> *nodes;
	const SegmentString *edge;  // the parent edge
	vector<SegmentNode*> *sortedNodes;

	// This vector is here to keep track of created splitEdges
	vector<SegmentString*> splitEdges;

	// This vector is here to keep track of created Coordinates
	vector<CoordinateSequence*> splitCoordLists;

	void checkSplitEdgesCorrectness(vector<SegmentString*> *splitEdges);
	/**
	* Create a new "split edge" with the section of points between
	* (and including) the two intersections.
	* The label for the new edge is the same as the label for the parent edge.
	*/
	SegmentString* createSplitEdge(SegmentNode *ei0, SegmentNode *ei1);

public:

	SegmentNodeList(const SegmentString *newEdge);

	virtual ~SegmentNodeList();

	/**
	* Adds an intersection into the list, if it isn't already there.
	* The input segmentIndex and dist are expected to be normalized.
	* @return the SegmentIntersection found or added
	*/
	SegmentNode* add(Coordinate *intPt, int segmentIndex, double dist);

	/**
	* returns the set of SegmentNodes
	*/
	//replaces iterator()
	set<SegmentNode*,SegmentNodeLT>* getNodes() { return nodes; }

	/**
	* Adds entries for the first and last points of the edge to the list
	*/
	void addEndpoints();

	/**
	* Creates new edges for all the edges that the intersections in this
	* list split the parent edge into.
	* Adds the edges to the input list (this is so a single list
	* can be used to accumulate all split edges for a Geometry).
	*/
	void addSplitEdges(vector<SegmentString*> *edgeList);

	string print();
};



/*
 * Contains a list of consecutive line segments which can be used to node the segments.
 * The line segments are represented by an array of {@link Coordinate}s.
 *
 */
class SegmentString {
private:
	SegmentNodeList *eiList;
	const CoordinateSequence *pts;
	const void* context;
	bool isIsolatedVar;
public:
	/**
	 * This function copies given CoordinateSequence
	 */
	SegmentString(const CoordinateSequence *newPts, const void* newContext);
	virtual ~SegmentString();
	const void* getContext() const;
	SegmentNodeList* getIntersectionList() const;
	int size() const;
	const Coordinate& getCoordinate(int i) const;
	CoordinateSequence* getCoordinates() const;
	const CoordinateSequence* getCoordinatesRO() const;
	void setIsolated(bool isIsolated);
	bool isIsolated() const;
	bool isClosed() const;
	/**
	* Adds EdgeIntersections for one or both
	* intersections found for a segment of an edge to the edge intersection list.
	*/
	void addIntersections(LineIntersector *li,int segmentIndex, int geomIndex);
	/**
	* Add an SegmentNode for intersection intIndex.
	* An intersection that falls exactly on a vertex
	* of the SegmentString is normalized
	* to use the higher of the two possible segmentIndexes
	*/
	void addIntersection(LineIntersector *li, int segmentIndex, int geomIndex, int intIndex);
	void OLDaddIntersection(LineIntersector *li, int segmentIndex, int geomIndex, int intIndex);
	/**
	* Add an EdgeIntersection for intersection intIndex.
	* An intersection that falls exactly on a vertex of the edge is normalized
	* to use the higher of the two possible segmentIndexes
	*/
	void addIntersection(Coordinate& intPt, int segmentIndex);
	void addIntersection(Coordinate& intPt, int segmentIndex, double dist);
};

/*
 * Computes the intersections between two line segments in {@link SegmentString}s
 * and adds them to each string.
 * The {@link nodingSegmentIntersector} is passed to a {@link Noder}.
 * The {@link addIntersections} method is called whenever the {@link Noder}
 * detects that two SegmentStrings <i>might</i> intersect.
 * This class is an example of the <i>Strategy</i> pattern.
 *
 */
class nodingSegmentIntersector {
private:
	/**
	* These variables keep track of what types of intersections were
	* found during ALL edges that have been intersected.
	*/
	bool hasIntersectionVar;
	bool hasProperVar;
	bool hasProperInteriorVar;
	bool hasInteriorVar;
	// the proper intersection point found
	Coordinate *properIntersectionPoint;
	LineIntersector *li;
	bool recordIsolated;
	bool isSelfIntersectionVar;
	/**
	* A trivial intersection is an apparent self-intersection which in fact
	* is simply the point shared by adjacent line segments.
	* Note that closed edges require a special check for the point shared by the beginning
	* and end segments.
	*/
	bool isTrivialIntersection(SegmentString *e0, int segIndex0, SegmentString *e1, int segIndex1);
public:
	static bool isAdjacentSegments(int i1, int i2);
	int numIntersections;
	int numInteriorIntersections;
	int numProperIntersections;
	// testing only
	int numTests;
	nodingSegmentIntersector(LineIntersector *newLi);
	LineIntersector* getLineIntersector();
	/**
	* @return the proper intersection point, or <code>null</code> if none was found
	*/
	Coordinate* getProperIntersectionPoint();
	bool hasIntersection();
	/**
	* A proper intersection is an intersection which is interior to at least two
	* line segments.  Note that a proper intersection is not necessarily
	* in the interior of the entire Geometry, since another edge may have
	* an endpoint equal to the intersection, which according to SFS semantics
	* can result in the point being on the Boundary of the Geometry.
	*/
	bool hasProperIntersection();
	/**
	* A proper interior intersection is a proper intersection which is <b>not</b>
	* contained in the set of boundary nodes set for this nodingSegmentIntersector.
	*/
	bool hasProperInteriorIntersection();
	/**
	* An interior intersection is an intersection which is
	* in the interior of some segment.
	*/
	bool hasInteriorIntersection();
	/**
	* This method is called by clients
	* of the {@link nodingSegmentIntersector} class to process
	* intersections for two segments of the {@link SegmentStrings} being intersected.
	* Note that some clients (such as {@link MonotoneChain}s) may optimize away
	* this call for segment pairs which they have determined do not intersect
	* (e.g. by an disjoint envelope test).
	*/
	void processIntersections(SegmentString *e0, int segIndex0,SegmentString *e1, int segIndex1);
};

/*
 * Computes all intersections between segments in a set of {@link SegmentString}s.
 * Intersections found are represented as {@link SegmentNode}s and add to the
 * {@link SegmentString}s in which they occur.
 *
 */
class Noder {
public:
	static vector<SegmentString*>* getNodedEdges(vector<SegmentString*>* segStrings);
	static void getNodedEdges(vector<SegmentString*>* segStrings,vector<SegmentString*>* resultEdgelist);
	nodingSegmentIntersector *segInt;
public:
	Noder(){};
	virtual void setSegmentIntersector(nodingSegmentIntersector *newSegInt);
	virtual vector<SegmentString*>* node(vector<SegmentString*>* segStrings)=0;
};

/*
 * Nodes a set of {@link SegmentString}s by
 * performing a brute-force comparison of every segment to every other one.
 * This has n^2 performance, so is too slow for use on large numbers
 * of segments.
 *
 */
class SimpleNoder: public Noder {
public:
	SimpleNoder(){};
	virtual vector<SegmentString*>* node(vector<SegmentString*> *inputSegStrings);
private:
	virtual void computeIntersects(SegmentString *e0, SegmentString *e1);
};

/*
 * Validates that a collection of {@link SegmentString}s is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 */
class NodingValidator {
public:
	NodingValidator(vector<SegmentString*> *newSegStrings);
	virtual ~NodingValidator();
	void checkValid();
private:
	LineIntersector *li;
	vector<SegmentString*> *segStrings;
	void checkProperIntersections();
	void checkProperIntersections(SegmentString *ss0, SegmentString *ss1);
	void checkProperIntersections(SegmentString *e0, int segIndex0, SegmentString *e1, int segIndex1);
	/**
	*@return true if there is an intersection point which is not an endpoint of the segment p0-p1
	*/
	bool hasInteriorIntersection(LineIntersector *aLi, Coordinate& p0, Coordinate& p1);
	void checkNoInteriorPointsSame();
	void checkNoInteriorPointsSame(const Coordinate& testPt,vector<SegmentString*> *segStrings);
};


/*
 * Nodes a set of {@link SegmentStrings} using a index based
 * on {@link indexMonotoneChain}s and a {@link SpatialIndex}.
 * The {@link SpatialIndex} used should be something that supports
 * envelope (range) queries efficiently (such as a {@link Quadtree}
 * or {@link STRtree}.
 *
 */
class MCQuadtreeNoder: public Noder {
public:
	MCQuadtreeNoder();
	virtual ~MCQuadtreeNoder();
	vector<SegmentString*>* node(vector<SegmentString*> *inputSegStrings);
	class SegmentOverlapAction: public MonotoneChainOverlapAction {
		private:
			nodingSegmentIntersector *si;
		public:
			SegmentOverlapAction(nodingSegmentIntersector *newSi);
			void overlap(indexMonotoneChain *mc1, int start1, indexMonotoneChain *mc2, int start2);
	};
private:
	vector<indexMonotoneChain*> *chains;
	SpatialIndex *index;
	int idCounter;
	// statistics
	int nOverlaps;
	void intersectChains();
	void add(SegmentString *segStr);
};

/*
 * Nodes a set of SegmentStrings completely.
 * The set of segmentStrings is fully noded;
 * i.e. noding is repeated until no further
 * intersections are detected.
 * <p>
 * Iterated noding using a FLOATING precision model is not guaranteed to converge,
 * due to roundoff error.   This problem is detected and an exception is thrown.
 * Clients can choose to rerun the noding using a lower precision model.
 *
 */
class IteratedNoder {
private:
	/**
	* Node the input segment strings once
	* and create the split edges between the nodes
	*/
	vector<SegmentString*>* node(vector<SegmentString*> *segStrings, int *numInteriorIntersections);
	const PrecisionModel *pm;
	LineIntersector *li;
public:
	IteratedNoder(const PrecisionModel *newPm);
	virtual ~IteratedNoder();
	/**
	* Fully nodes a list of {@link SegmentStrings}, i.e. peforms noding iteratively
	* until no intersections are found between segments.
	* Maintains labelling of edges correctly through
	* the noding.
	*
	* @param segStrings a collection of SegmentStrings to be noded
	* @return a collection of the noded SegmentStrings
	* @throws TopologyException if the iterated noding fails to converge.
	*/
	vector<SegmentString*>* node(vector<SegmentString*> *segStrings); // throw(GEOSException *);
};

}
#endif

