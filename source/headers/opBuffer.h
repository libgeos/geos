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
 * Revision 1.7  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.5  2003/11/06 18:46:34  strk
 * Added throw specification for BufferOp's ::buildSubgraphs() 
 * and ::computeBuffer()
 *
 **********************************************************************/


#ifndef GEOS_OPBUFFER_H
#define GEOS_OPBUFFER_H

#include <memory>
#include "platform.h"
#include "operation.h"
#include "opOverlay.h"
#include "geom.h"
#include <vector>

namespace geos {
/**
 * LineFilter implements a filter that removes small loops from the line created
 * by BufferLineBuilder
 *
 * @version 1.3
 */
class LoopFilter {
private:
	static const CoordinateList* arrayTypeCoordinate;
	int maxPointsInLoop;           // maximum number of points in a loop
	double maxLoopExtent;    // the maximum X and Y extents of a loop
	CoordinateList* newPts;
	void addPoint(Coordinate p);
	int checkForLoop(CoordinateList *pts,int startIndex);
public:
	LoopFilter();
	~LoopFilter();
	CoordinateList* filter(CoordinateList *inputPts);
};

/**
 * A RightmostEdgeFinder find the DirectedEdge in a list which has the highest coordinate,
 * and which is oriented L to R at that point. (I.e. the right side is on the RHS of the edge.)
 */
class RightmostEdgeFinder {
private:
	CGAlgorithms* cga;
	int minIndex;
	Coordinate minCoord;
	DirectedEdge *minDe;
	DirectedEdge *orientedDe;
	void findRightmostEdgeAtNode();
	void findRightmostEdgeAtVertex();
	void checkForRightmostCoordinate(DirectedEdge *de);
	int getRightmostSide(DirectedEdge *de, int index);
	int getRightmostSideOfSegment(DirectedEdge *de, int i);
	/**
	* A RightmostEdgeFinder finds the DirectedEdge with the rightmost coordinate.
	* The DirectedEdge returned is guranteed to have the R of the world on its RHS.
	*/
public:
	RightmostEdgeFinder(CGAlgorithms *newCga);
	DirectedEdge* getEdge();
	Coordinate& getCoordinate();
	void findEdge(vector<DirectedEdge*>* dirEdgeList);
};

/**
 *A BufferSubgraph is a connected subset of the graph of DirectedEdges and Nodes
 *in a BufferGraph.  Its edges will generate either
 *<ul>
 *<li> a single polygon in the complete buffer, with zero or more holes, or
 *<li> one or more connected holes
 *</ul>
 *
 */
class BufferSubgraph {
private:
	RightmostEdgeFinder *finder;
	vector<DirectedEdge*> *dirEdgeList;
	vector<Node*> *nodes;
	Coordinate rightMostCoord;
	void add(Node *node);
	void clearVisitedEdges();
	void computeNodeDepth(Node *n,DirectedEdge *startEdge);
public:
	BufferSubgraph(CGAlgorithms *newCga);
	~BufferSubgraph();
	vector<DirectedEdge*>* getDirectedEdges();
	vector<Node*>* getNodes();
	Coordinate& getRightmostCoordinate();
	void create(Node *node);
	void computeDepth(int outsideDepth);
	void findResultEdges();
	int compareTo(void* o);
	int compareTo(BufferSubgraph *bsg);
};

class BufferLineBuilder {
public:
	static const int DEFAULT_QUADRANT_SEGMENTS=6;    // controls point density in fillets
	BufferLineBuilder(CGAlgorithms *newCga,LineIntersector *newLi,const PrecisionModel *newPM);
	~BufferLineBuilder();
	BufferLineBuilder(CGAlgorithms *newCga,LineIntersector *newLi,const PrecisionModel *newPM,int quadrantSegments);
	vector<CoordinateList*>* getLineBuffer(CoordinateList *inputPts, double distance);
	vector<CoordinateList*>* getRingBuffer(CoordinateList *inputPts, int side, double distance);
private: 
	static double minSegmentLength;
	static bool useMinSegmentLength;
	static double facetAngle(double angleInc, double totalAngle);
	static double angleBetween(Coordinate& pa, Coordinate& p, Coordinate& pb);
	CGAlgorithms *cga;
	LineIntersector *li;
	LoopFilter *loopFilter;
	double angleInc;
	CoordinateList *ptList;
	double distance;
	const PrecisionModel *precisionModel;
	vector<CoordinateList*> *lineList;
	void init(double newDistance);
	CoordinateList* getCoordinates();
	void computeLineBuffer(CoordinateList *inputPts);
	void computeRingBuffer(CoordinateList *inputPts, int side);
	void addPt(const Coordinate &pt);
	void closePts();
	Coordinate s0, s1, s2;
	LineSegment *seg0;
	LineSegment *seg1;
	LineSegment *offset0;
	LineSegment *offset1;
	int side;
	void initSideSegments(const Coordinate &ns1, const Coordinate &ns2, int nside);
	void addNextSegment(const Coordinate &p, bool addStartPoint);
	void addLastSegment();
	void computeOffsetSegment(LineSegment *seg, int side, double distance, LineSegment *offset);
	void addLineEndCap(const Coordinate &p0, const Coordinate &p1);
	void addFillet(const Coordinate &p, const Coordinate &p0, const Coordinate &p1, int direction, double distance);
	void addFillet(const Coordinate &p, double startAngle, double endAngle, int direction, double distance);
	void addCircle(const Coordinate &p, double distance);
};


/**
*BufferEdgeBuilder creates all the "rough" edges in the buffer for a Geometry.
*Rough edges need to be noded together and polygonized to form the final buffer polygon.
 */
class BufferEdgeBuilder {
private:
	CGAlgorithms *cga;
	double distance;
	//BufferMultiLineBuilder lineBuilder;
	BufferLineBuilder *lineBuilder;
	vector<Edge*> *edgeList;
	void addEdges(vector<CoordinateList*> *lineList, int leftLoc, int rightLoc);
	void addEdge(const CoordinateList *coord, int leftLoc, int rightLoc);
	void add(const Geometry *g);
	void addCollection(const GeometryCollection *gc);
	void addPoint(const Point *p);
	void addLineString(const LineString *line);
	void addPolygon(const Polygon *p);
	void addPolygonRing(const LinearRing *lr, double distance, int side, int cwLeftLoc, int cwRightLoc);
public:
	BufferEdgeBuilder(CGAlgorithms *newCga,LineIntersector *li,double newDistance, const PrecisionModel *precisionModel,int quadrantSegments);
	~BufferEdgeBuilder();
	vector<Edge*>* getEdges(const Geometry *geom);
};

class BufferOp: public GeometryGraphOperation {
public:
	static Geometry* bufferOp(const Geometry *g, double distance);
	static Geometry* bufferOp(const Geometry *g, double distance, int quadrantSegments);
	BufferOp(const Geometry *g0);
	~BufferOp();
	Geometry* getResultGeometry(double distance);
	Geometry* getResultGeometry(double distance,int quadrantSegments);
private:
	static int depthDelta(Label *label);
	GeometryFactory *geomFact;
	Geometry *resultGeom;
	PlanarGraph *graph;
	EdgeList *edgeList;
	void computeBuffer(double distance, int quadrantSegments)
		throw(TopologyException *);
	vector<Edge*>* nodeEdges(vector<Edge*> *edges);
	void checkDimensionalCollapse(Label *labelToMerge,Label *existingLabel);
	void replaceCollapsedEdges();
	vector<BufferSubgraph*>* createSubgraphs();
	void buildSubgraphs(vector<BufferSubgraph*> *subgraphList,
		PolygonBuilder *polyBuilder) throw(TopologyException *);
	Geometry* computeGeometry(vector<Polygon*> *resultPolyList);
	Geometry* toLineStrings(EdgeList *edges);
protected:
	void insertEdge(Edge *e);
};
}
#endif
