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
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.2  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.20  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_INDEX_H
#define GEOS_GEOMGRAPH_INDEX_H

#include <memory>
#include <geos/geomgraph.h>
#include <geos/geom.h>
#include <vector>
#include <geos/geosAlgorithm.h>
#include <geos/platform.h>

namespace geos {

class Edge;
class Node;
class CoordinateSequence;

class SegmentIntersector{
public:
	static bool isAdjacentSegments(int i1,int i2);
	// testing only
	int numTests;
	SegmentIntersector();
	virtual ~SegmentIntersector();
	SegmentIntersector(LineIntersector *newLi,bool newIncludeProper,bool newRecordIsolated);
	void setBoundaryNodes(vector<Node*> *bdyNodes0,vector<Node*> *bdyNodes1);
	Coordinate& getProperIntersectionPoint();
	bool hasIntersection();
	bool hasProperIntersection();
	bool hasProperInteriorIntersection();
	void addIntersections(Edge *e0,int segIndex0,Edge *e1,int segIndex1);
private:
	/**
	 * These variables keep track of what types of intersections were
	 * found during ALL edges that have been intersected.
	 */
	bool hasIntersectionVar;
	bool hasProper;
	bool hasProperInterior;
	// the proper intersection point found
	Coordinate properIntersectionPoint;
	LineIntersector *li;
	bool includeProper;
	bool recordIsolated;
	bool isSelfIntersection;
	//bool intersectionFound;
	int numIntersections;
	vector<vector<Node*>*> *bdyNodes;
	bool isTrivialIntersection(Edge *e0,int segIndex0,Edge *e1, int segIndex1);
	bool isBoundaryPoint(LineIntersector *li,vector<vector<Node*>*> *tstBdyNodes);
	bool isBoundaryPoint(LineIntersector *li,vector<Node*> *tstBdyNodes);
};

class EdgeSetIntersector{
public:
	/**
	* Computes all self-intersections between edges in a set of edges,
	* allowing client to choose whether self-intersections are computed.
	*
	* @param edges a list of edges to test for intersections
	* @param si the SegmentIntersector to use
	* @param testAllSegments true if self-intersections are to be tested as well
	*/
	virtual void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments)=0;
	/**
	* Computes all mutual intersections between two sets of edges
	*/
	virtual void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si)=0;
	virtual ~EdgeSetIntersector(){};
protected:
//	vector<Edge*>* edgesZero;
//	vector<Edge*>* edgesOne;
};
//
// This is here so that SweepLineEvent constructor
// can use it as argument type. 
// Both  SweepLineSegment and MonotoneChain will
// inherit from it.
class SweepLineEventOBJ {
public:
	virtual ~SweepLineEventOBJ(){};
};


class SweepLineSegment: public SweepLineEventOBJ {
public:
	SweepLineSegment(Edge *newEdge,int newPtIndex);
	~SweepLineSegment();
	double getMinX();
	double getMaxX();
	void computeIntersections(SweepLineSegment *ss,SegmentIntersector *si);
protected:
	Edge *edge;
	const CoordinateSequence* pts;
	int ptIndex;
};

class SweepLineEvent{
public:
	enum {
		INSERT=1,
		DELETE
	};
	SweepLineEvent(void* newEdgeSet,double x,SweepLineEvent *newInsertEvent,SweepLineEventOBJ *newObj);
	virtual ~SweepLineEvent();
	bool isInsert();
	bool isDelete();
	SweepLineEvent* getInsertEvent();
	int getDeleteEventIndex();
	void setDeleteEventIndex(int newDeleteEventIndex);
	SweepLineEventOBJ* getObject() const;
	int compareTo(SweepLineEvent *sle);
	string print();
	void* edgeSet;    // used for red-blue intersection detection
protected:
	SweepLineEventOBJ* obj;
private:
	double xValue;
	int eventType;
	SweepLineEvent *insertEvent; // null if this is an INSERT event
	int deleteEventIndex;
};

class MonotoneChainIndexer{
public:
//	public static int[] toIntArray(List list); //Not needed
	MonotoneChainIndexer(){};
	vector<int>* getChainStartIndices(const CoordinateSequence* pts);
private:
	int findChainEnd(const CoordinateSequence* pts,int start);
};

class MonotoneChainEdge{
public:
	MonotoneChainEdge();
	~MonotoneChainEdge();
	MonotoneChainEdge(Edge *newE);
	const CoordinateSequence* getCoordinates();
	vector<int>* getStartIndexes();
	double getMinX(int chainIndex);
	double getMaxX(int chainIndex);
	void computeIntersects(MonotoneChainEdge *mce,SegmentIntersector *si);
	void computeIntersectsForChain(int chainIndex0,MonotoneChainEdge *mce,int chainIndex1,SegmentIntersector *si);
protected:
	Edge *e;
	const CoordinateSequence* pts; // cache a reference to the coord array, for efficiency
	// the lists of start/end indexes of the monotone chains.
	// Includes the end point of the edge as a sentinel
	vector<int>* startIndex;
	// these envelopes are created once and reused
	Envelope *env1;
	Envelope *env2;
private:
	void computeIntersectsForChain(int start0,int end0,MonotoneChainEdge *mce,
									int start1,int end1,SegmentIntersector *ei);
};

class MonotoneChain: public SweepLineEventOBJ {
public:
	MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex);
	~MonotoneChain();
	void computeIntersections(MonotoneChain *mc,SegmentIntersector *si);
protected:
	MonotoneChainEdge *mce;
	int chainIndex;
};

/**
 * Finds all intersections in one or two sets of edges,
 * using an x-axis sweepline algorithm in conjunction with Monotone Chains.
 * While still O(n^2) in the worst case, this algorithm
 * drastically improves the average-case time.
 * The use of MonotoneChains as the items in the index
 * seems to offer an improvement in performance over a sweep-line alone.
 */
class SimpleMCSweepLineIntersector: public EdgeSetIntersector {
public:
	SimpleMCSweepLineIntersector();
	virtual ~SimpleMCSweepLineIntersector();
	void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si);
protected:
	vector<SweepLineEvent*>* events;
	// statistics information
	int nOverlaps;
private:
	void add(vector<Edge*> *edges);
	void add(vector<Edge*> *edges,void* edgeSet);
	void add(Edge *edge,void* edgeSet);
	void prepareEvents();
	void computeIntersections(SegmentIntersector *si);
	void processOverlaps(int start,int end,SweepLineEvent *ev0,SegmentIntersector *si);
};

class SimpleEdgeSetIntersector: public EdgeSetIntersector {
public:
	SimpleEdgeSetIntersector();
	void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si);
private:
	int nOverlaps;
	void computeIntersects(Edge *e0,Edge *e1,SegmentIntersector *si);
};

/**
 * Finds all intersections in one or two sets of edges,
 * using a simple x-axis sweepline algorithm.
 * While still O(n^2) in the worst case, this algorithm
 * drastically improves the average-case time.
 */
class SimpleSweepLineIntersector: public EdgeSetIntersector {
public:
	SimpleSweepLineIntersector();
	virtual ~SimpleSweepLineIntersector();
	void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si);
private:
	void add(vector<Edge*> *edges);
	vector<SweepLineEvent*>* events;
	// statistics information
	int nOverlaps;
	void add(vector<Edge*> *edges,void* edgeSet);
	void add(Edge *edge,void* edgeSet);
	void prepareEvents();
	void computeIntersections(SegmentIntersector *si);
	void processOverlaps(int start,int end,SweepLineEvent *ev0,SegmentIntersector *si);
};

bool sleLessThen(SweepLineEvent *first,SweepLineEvent *second);
}
#endif
