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
 * Revision 1.14  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.13  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_OPRELATE_H
#define GEOS_OPRELATE_H

#include <memory>
#include <string>
#include <vector>
#include "platform.h"
#include "operation.h"
#include "geomgraph.h"
#include "geosAlgorithm.h"

namespace geos {

class RelateNode: public Node {
public:
	RelateNode(Coordinate& coord,EdgeEndStar *edges);
	virtual ~RelateNode();
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
	virtual ~EdgeEndBundle();
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
	virtual ~EdgeEndBundleStar();
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
	virtual ~RelateNodeGraph();
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
friend class Unload;
public:
	RelateComputer();
	virtual ~RelateComputer();
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
	void labelIsolatedEdge(Edge *e,int targetIndex, const Geometry *target);
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
	static IntersectionMatrix* relate(const Geometry *a,const Geometry *b);
	RelateOp(const Geometry *g0, const Geometry *g1);
	virtual ~RelateOp();
	IntersectionMatrix* getIntersectionMatrix();
private:
	RelateComputer *relateComp;
	static vector<const Geometry*>* toList(const Geometry *geom);
	static vector<const Geometry*>* addToList(const Geometry *geom, vector<const Geometry*>* geomList);
	static bool isBaseGeometryCollection(const Geometry* geom);
	static IntersectionMatrix* relateGC(vector<const Geometry*> *a,vector<const Geometry*> *b);
};
}

#endif

