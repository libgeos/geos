#ifndef GEOS_GRAPH_INDEX_H
#define GEOS_GRAPH_INDEX_H

#include "graph.h"
#include "geom.h"
#include <vector>
#include "geosAlgorithm.h"
#include "platform.h"

class Edge;
class Node;
class CoordinateList;

class SegmentIntersector{
public:
	static bool isAdjacentSegments(int i1,int i2);
	// testing only
	int numTests;
	SegmentIntersector();
	SegmentIntersector(LineIntersector *newLi,bool newIncludeProper,bool newRecordIsolated);
	void setBoundaryNodes(vector<Node*> *bdyNodes0,vector<Node*> *bdyNodes1);
	Coordinate getProperIntersectionPoint();
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
	EdgeSetIntersector() {};
	/**
	* Computes all self-intersections between edges in a set of edges
	*/
	virtual void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si)=0;
	/**
	* Computes all mutual intersections between two sets of edges
	*/
	virtual void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si)=0;
protected:
	vector<Edge*> edgesZero;
	vector<Edge*> edgesOne;
};

class SweepLineSegment{
public:
	SweepLineSegment(Edge *newEdge,int newPtIndex,int newGeomIndex);
	double getMinX();
	double getMaxX();
	void computeIntersections(SweepLineSegment *ss,SegmentIntersector *si);
protected:
	Edge *edge;
//	CoordinateList pts;
	CoordinateList* pts;
	int ptIndex;
	int geomIndex;
};

class SweepLineEvent{
public:
	enum {
		INSERT=1,
		DELETE
	};
	SweepLineEvent(int newGeomIndex,double x,SweepLineEvent *newInsertEvent,void *newObj);
	bool isInsert();
	bool isDelete();
	SweepLineEvent* getInsertEvent();
	int getDeleteEventIndex();
	void setDeleteEventIndex(int newDeleteEventIndex);
	void* getObject();
	int compareTo(void *o);
	int compareTo(SweepLineEvent *sle);
	string print();
protected:
	void* obj;
private:
	int geomIndex;    // used for red-blue intersection detection
	double xValue;
	int eventType;
	SweepLineEvent *insertEvent; // null if this is an INSERT event
	int deleteEventIndex;
};

class MonotoneChainIndexer{
public:
//	public static int[] toIntArray(List list); //Not needed
	MonotoneChainIndexer(){};
	vector<int> getChainStartIndices(CoordinateList& pts);
private:
	int findChainEnd(CoordinateList& pts,int start);
};

class MonotoneChainEdge{
public:
	MonotoneChainEdge();
	MonotoneChainEdge(Edge *newE);
	CoordinateList* getCoordinates();
	vector<int> getStartIndexes();
	double getMinX(int chainIndex);
	double getMaxX(int chainIndex);
	void computeIntersects(MonotoneChainEdge *mce,SegmentIntersector *si);
	void computeIntersectsForChain(int chainIndex0,MonotoneChainEdge *mce,int chainIndex1,SegmentIntersector *si);
protected:
	Edge *e;
//	CoordinateList pts; // cache a reference to the coord array, for efficiency
	CoordinateList* pts; // cache a reference to the coord array, for efficiency
	// the lists of start/end indexes of the monotone chains.
	// Includes the end point of the edge as a sentinel
	vector<int> startIndex;
	// these envelopes are created once and reused
	Envelope *env1;
	Envelope *env2;
private:
	void computeIntersectsForChain(int start0,int end0,MonotoneChainEdge *mce,
									int start1,int end1,SegmentIntersector *ei);
};

class MonotoneChain{
public:
	MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex,int newGeomIndex);
	void computeIntersections(MonotoneChain *mc,SegmentIntersector *si);
	int getGeomIndex();
protected:
	MonotoneChainEdge *mce;
	int chainIndex;
	int geomIndex;
};

class SimpleMCSweepLineIntersector: public EdgeSetIntersector {
public:
	SimpleMCSweepLineIntersector();
	void computeIntersections(vector<Edge*> *edges,SegmentIntersector *si);
	void computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si);
protected:
	vector<SweepLineEvent*> events;
	// statistics information
	int nOverlaps;
private:
	void add(vector<Edge*> *edges,int geomIndex);
	void add(Edge *edge,int geomIndex);
	void prepareEvents();
	void computeIntersections(SegmentIntersector *si,bool doMutualOnly);
	void processOverlaps(int start,int end,MonotoneChain *mc0,SegmentIntersector *si,bool doMutualOnly);
};

bool sleLessThen(SweepLineEvent *first,SweepLineEvent *second);

#endif
