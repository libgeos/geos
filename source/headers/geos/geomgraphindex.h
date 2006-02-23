/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
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
namespace geomgraph { // geos.geomgraph

class Node;
class Edge;

namespace index { // geos.geomgraph.index


class SegmentIntersector{

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

	algorithm::LineIntersector *li;

	bool includeProper;

	bool recordIsolated;

	//bool isSelfIntersection;

	//bool intersectionFound;

	int numIntersections;

	/// Elements are externally owned
	std::vector<std::vector<Node*>*> bdyNodes;

	bool isTrivialIntersection(Edge *e0,int segIndex0,Edge *e1, int segIndex1);

	bool isBoundaryPoint(algorithm::LineIntersector *li,
			std::vector<std::vector<Node*>*>& tstBdyNodes);

	bool isBoundaryPoint(algorithm::LineIntersector *li,
			std::vector<Node*> *tstBdyNodes);

public:

	static bool isAdjacentSegments(int i1,int i2);

	// testing only
	int numTests;

	//SegmentIntersector();

	virtual ~SegmentIntersector() {}

	SegmentIntersector(algorithm::LineIntersector *newLi,
			bool newIncludeProper, bool newRecordIsolated)
		:
		hasIntersectionVar(false),
		hasProper(false),
		hasProperInterior(false),
		li(newLi),
		includeProper(newIncludeProper),
		recordIsolated(newRecordIsolated),
		numIntersections(0),
		bdyNodes(2),
		numTests(0)
	{}

	/// \brief
	/// Parameters are externally owned.
	/// Make sure they live for the whole lifetime of this object
	void setBoundaryNodes(std::vector<Node*> *bdyNodes0,
			std::vector<Node*> *bdyNodes1);

	Coordinate& getProperIntersectionPoint();

	bool hasIntersection();

	bool hasProperIntersection();

	bool hasProperInteriorIntersection();

	void addIntersections(Edge *e0,int segIndex0,Edge *e1,int segIndex1);

};

/* 
 * This is derived from a Java interface.
 */
class EdgeSetIntersector {
public:
	/**
	 * Computes all self-intersections between edges in a set of edges,
	 * allowing client to choose whether self-intersections are computed.
	 *
	 * @param edges a list of edges to test for intersections
	 * @param si the SegmentIntersector to use
	 * @param testAllSegments true if self-intersections are to be tested as well
	 */
	virtual void computeIntersections(std::vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments)=0;

	/**
	 * Computes all mutual intersections between two sets of edges
	 */
	virtual void computeIntersections(std::vector<Edge*> *edges0,std::vector<Edge*> *edges1,SegmentIntersector *si)=0;
	virtual ~EdgeSetIntersector(){};
};


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

class SweepLineEventLessThen;

class SweepLineEvent{
friend class SweepLineEventLessThen;
public:
	enum {
		INSERT=1,
		DELETE
	};
	SweepLineEvent(void* newEdgeSet,double x,SweepLineEvent *newInsertEvent,SweepLineEventOBJ *newObj);
	virtual ~SweepLineEvent();
	bool isInsert() { return insertEvent==NULL; }
	bool isDelete() { return insertEvent!=NULL; }
	SweepLineEvent* getInsertEvent() { return insertEvent; }
	int getDeleteEventIndex() { return deleteEventIndex; }
	void setDeleteEventIndex(int newDeleteEventIndex) {
		deleteEventIndex=newDeleteEventIndex;
	}
	SweepLineEventOBJ* getObject() const { return obj; }
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

class SweepLineEventLessThen {
public:
	bool operator()(const SweepLineEvent *f, const SweepLineEvent *s) const
	{
		if (f->xValue<s->xValue) return true;
		if (f->xValue>s->xValue) return false;
		if (f->eventType<s->eventType) return true;
		return false;
	}
};

class MonotoneChainIndexer{
public:
//	public static int[] toIntArray(List list); //Not needed
	MonotoneChainIndexer(){};
	void getChainStartIndices(const CoordinateSequence*, std::vector<int>&);
private:
	int findChainEnd(const CoordinateSequence* pts,int start);
};

class MonotoneChainEdge{
public:
	//MonotoneChainEdge();
	~MonotoneChainEdge();
	MonotoneChainEdge(Edge *newE);
	const CoordinateSequence* getCoordinates();
	std::vector<int>& getStartIndexes();
	double getMinX(int chainIndex);
	double getMaxX(int chainIndex);

	void computeIntersects(const MonotoneChainEdge &mce,
		SegmentIntersector &si);

	void computeIntersectsForChain(int chainIndex0,
		const MonotoneChainEdge &mce, int chainIndex1,
		SegmentIntersector &si);

protected:
	Edge *e;
	const CoordinateSequence* pts; // cache a reference to the coord array, for efficiency
	// the lists of start/end indexes of the monotone chains.
	// Includes the end point of the edge as a sentinel
	std::vector<int> startIndex;
	// these envelopes are created once and reused
	Envelope env1;
	Envelope env2;
private:
	void computeIntersectsForChain(int start0, int end0,
		const MonotoneChainEdge &mce,
		int start1, int end1,
		SegmentIntersector &ei);
};

/**
 * Last port: geomgraph/index/MonotoneChain.java rev. 1.3 (JTS-1.7)
 */
class MonotoneChain: public SweepLineEventOBJ {
private:
	MonotoneChainEdge *mce;
	int chainIndex;

public:

	MonotoneChain(MonotoneChainEdge *newMce, int newChainIndex):
		mce(newMce),
		chainIndex(newChainIndex)
	{}

	~MonotoneChain() {}

	void computeIntersections(MonotoneChain *mc,SegmentIntersector *si) {
		mce->computeIntersectsForChain(chainIndex,*(mc->mce),mc->chainIndex,*si);
	}
};

/*
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
	void computeIntersections(std::vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(std::vector<Edge*> *edges0,std::vector<Edge*> *edges1,SegmentIntersector *si);
protected:
	std::vector<SweepLineEvent*> events;
	// statistics information
	int nOverlaps;
private:
	void add(std::vector<Edge*> *edges);
	void add(std::vector<Edge*> *edges,void* edgeSet);
	void add(Edge *edge,void* edgeSet);
	void prepareEvents();
	void computeIntersections(SegmentIntersector *si);
	void processOverlaps(int start,int end,SweepLineEvent *ev0,SegmentIntersector *si);
};

class SimpleEdgeSetIntersector: public EdgeSetIntersector {
public:
	SimpleEdgeSetIntersector();
	void computeIntersections(std::vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(std::vector<Edge*> *edges0,std::vector<Edge*> *edges1,SegmentIntersector *si);
private:
	int nOverlaps;
	void computeIntersects(Edge *e0,Edge *e1,SegmentIntersector *si);
};

/*
 * Finds all intersections in one or two sets of edges,
 * using a simple x-axis sweepline algorithm.
 * While still O(n^2) in the worst case, this algorithm
 * drastically improves the average-case time.
 */
class SimpleSweepLineIntersector: public EdgeSetIntersector {
public:
	SimpleSweepLineIntersector();
	virtual ~SimpleSweepLineIntersector();
	void computeIntersections(std::vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments);
	void computeIntersections(std::vector<Edge*> *edges0,std::vector<Edge*> *edges1,SegmentIntersector *si);
private:
	void add(std::vector<Edge*> *edges);
	std::vector<SweepLineEvent*> events;
	// statistics information
	int nOverlaps;
	void add(std::vector<Edge*> *edges,void* edgeSet);
	void add(Edge *edge,void* edgeSet);
	void prepareEvents();
	void computeIntersections(SegmentIntersector *si);
	void processOverlaps(int start,int end,SweepLineEvent *ev0,SegmentIntersector *si);
};

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/02/23 11:54:20  strk
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
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2005/11/15 10:04:37  strk
 *
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.6  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.5  2005/11/03 19:51:28  strk
 * Indentation changes, small vector memory allocation optimization.
 *
 * Revision 1.4  2005/10/27 14:05:20  strk
 * Added a SweepLineEventLessThen functor to be used by sort algorithm.
 *
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

