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
 * Revision 1.2  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.33  2003/11/12 15:43:38  strk
 * Added some more throw specifications
 *
 * Revision 1.32  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.31  2003/10/20 13:53:03  strk
 * LinearRing handled as a LineString in 
 * GeometryGraph::add(const Geometry *) - more explicit exception 
 * thrown for unknown geometries
 *
 * Revision 1.30  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 * Revision 1.29  2003/10/15 11:24:28  strk
 * Use getCoordinatesRO() introduced.
 *
 **********************************************************************/


#include "../headers/geomgraph.h"
#include <typeinfo>
#include "../headers/util.h"

namespace geos {

/**
* This method implements the Boundary Determination Rule
* for determining whether
* a component (node or edge) that appears multiple times in elements
* of a MultiGeometry is in the boundary or the interior of the Geometry
* <br>
* The SFS uses the "Mod-2 Rule", which this function implements
* <br>
* An alternative (and possibly more intuitive) rule would be
* the "At Most One Rule":
*    isInBoundary = (componentCount == 1)
*/
bool GeometryGraph::isInBoundary(int boundaryCount){
	// the "Mod-2 Rule"
	return boundaryCount%2==1;
}

int GeometryGraph::determineBoundary(int boundaryCount){
	return isInBoundary(boundaryCount)?Location::BOUNDARY : Location::INTERIOR;
}

GeometryGraph::GeometryGraph():PlanarGraph(){
	lineEdgeMap=new map<const LineString*,Edge*,LineStringLT>();
	useBoundaryDeterminationRule=false;
	boundaryNodes=NULL;
	parentGeom=NULL;
	hasTooFewPointsVar=false;
}

GeometryGraph::~GeometryGraph(){
//	map<LineString*,Edge*,LineStringLT>::iterator it=lineEdgeMap->begin();
//	for (;it!=lineEdgeMap->end();it++) {
//		Edge *e=it->second;
//		delete e;
//	}
	delete lineEdgeMap;
}

GeometryGraph::GeometryGraph(int newArgIndex, const Geometry *newParentGeom):PlanarGraph() {
	hasTooFewPointsVar=false;
	boundaryNodes=NULL;
	lineEdgeMap=new map<const LineString*,Edge*,LineStringLT>();
	useBoundaryDeterminationRule=false;
	argIndex=newArgIndex;
	parentGeom=newParentGeom;
	if (parentGeom!=NULL) {
//		precisionModel=parentGeom->getPrecisionModel();
//		SRID=parentGeom->getSRID();
		add(parentGeom);
	}
}

EdgeSetIntersector* GeometryGraph::createEdgeSetIntersector() {
	// various options for computing intersections, from slowest to fastest

	//private EdgeSetIntersector esi = new SimpleEdgeSetIntersector();
	//private EdgeSetIntersector esi = new MonotoneChainIntersector();
	//private EdgeSetIntersector esi = new NonReversingChainIntersector();
	//private EdgeSetIntersector esi = new SimpleSweepLineIntersector();
	//private EdgeSetIntersector esi = new MCSweepLineIntersector();

	//return new SimpleEdgeSetIntersector();
	return new SimpleMCSweepLineIntersector();
}

//NO LONGER USED
/**
* This constructor is used by clients that wish to add Edges explicitly,
* rather than adding a Geometry.  (An example is BufferOp).
*/
//GeometryGraph::GeometryGraph(int newArgIndex, const PrecisionModel *newPrecisionModel, int newSRID):PlanarGraph(){
//	boundaryNodes=NULL;
//	lineEdgeMap=new map<const LineString*,Edge*,LineStringLT>();
//	useBoundaryDeterminationRule=false;
//	argIndex=newArgIndex;
//	parentGeom=NULL;
//	precisionModel=newPrecisionModel;
//	SRID=newSRID;
//	hasTooFewPointsVar=false;
//}
//
//const PrecisionModel* GeometryGraph::getPrecisionModel(){
//	return precisionModel;
//}
//
//int GeometryGraph::getSRID() {
//	return SRID;
//}

const Geometry* GeometryGraph::getGeometry() {
	return parentGeom;
}

vector<Node*>* GeometryGraph::getBoundaryNodes() {
	if (boundaryNodes==NULL)
		boundaryNodes=nodes->getBoundaryNodes(argIndex);
	return boundaryNodes;
}

CoordinateList* GeometryGraph::getBoundaryPoints() {
	vector<Node*> *coll=getBoundaryNodes();
	CoordinateList *pts=CoordinateListFactory::internalFactory->createCoordinateList((int)coll->size());
	int i=0;
	for (vector<Node*>::iterator it=coll->begin();it<coll->end();it++) {
		Node *node=*it;
		pts->setAt(node->getCoordinate(),i++);
	}
	delete coll;
	return pts;
}

Edge* GeometryGraph::findEdge(const LineString *line){
	return lineEdgeMap->find(line)->second;
}

void GeometryGraph::computeSplitEdges(vector<Edge*> *edgelist) {
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		e->eiList->addSplitEdges(edgelist);
	}
}

void
GeometryGraph::add(const Geometry *g)
	//throw (UnsupportedOperationException *)
{
	if (g->isEmpty()) return;
	// check if this Geometry should obey the Boundary Determination Rule
	// all collections except MultiPolygons obey the rule
	if ((typeid(*g)==typeid(GeometryCollection)) ||
			   (typeid(*g)==typeid(MultiPoint)) ||
			   (typeid(*g)==typeid(MultiLineString)) &&
			   !(typeid(*g)==typeid(MultiPolygon)))
			useBoundaryDeterminationRule=true;
	if (typeid(*g)==typeid(Polygon))
		addPolygon((Polygon*) g);
	else if (typeid(*g)==typeid(LineString))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(LinearRing))
		addLineString((LineString*) g);
	else if (typeid(*g)==typeid(Point))
		addPoint((Point*) g);
	else if (typeid(*g)==typeid(MultiPoint))
		addCollection((MultiPoint*) g);
	else if (typeid(*g)==typeid(MultiLineString))
		addCollection((MultiLineString*) g);
	else if (typeid(*g)==typeid(MultiPolygon))
		addCollection((MultiPolygon*) g);
	else if (typeid(*g)==typeid(GeometryCollection))
		addCollection((GeometryCollection*) g);
	else {
		string out=typeid(*g).name();
		throw new UnsupportedOperationException("GeometryGraph::add(Geometry *): unknown geometry type: "+out);
	}
}

void GeometryGraph::addCollection(const GeometryCollection *gc) {
	for (int i=0;i<gc->getNumGeometries();i++) {
		const Geometry *g=gc->getGeometryN(i);
		add(g);
	}
}

/**
* Add a Point to the graph.
*/
void GeometryGraph::addPoint(const Point *p){
	const Coordinate& coord=*(p->getCoordinate());
	insertPoint(argIndex,coord,Location::INTERIOR);
}

/**
* The left and right topological location arguments assume that the ring is oriented CW.
* If the ring is in the opposite orientation,
* the left and right locations must be interchanged.
*/
void GeometryGraph::addPolygonRing(const LinearRing *lr, int cwLeft, int cwRight) {
	CoordinateList *lrcl;
	lrcl = lr->getCoordinates();
	CoordinateList* coord=CoordinateList::removeRepeatedPoints(lrcl);
	delete lrcl; // strk 2003-10-07
	if (coord->getSize()<4) {
		hasTooFewPointsVar=true;
		invalidPoint=coord->getAt(0); // its now a Coordinate
		delete coord;
		return;
	}
	int left=cwLeft;
	int right=cwRight;
	if (cga->isCCW(coord)) {
		left=cwRight;
		right=cwLeft;
	}

	Edge *e=new Edge(coord,new Label(argIndex,Location::BOUNDARY,left,right));
	(*lineEdgeMap)[lr]=e;
	insertEdge(e);
	insertPoint(argIndex,coord->getAt(0), Location::BOUNDARY);
#if 0
	CoordinateList *ncr=CoordinateListFactory::internalFactory->createCoordinateList();
	for(int i=0;i<coord->getSize();i++) {
		ncr->add(coord->getAt(i));
	}
	Edge *e=new Edge(ncr,new Label(argIndex,Location::BOUNDARY,left,right));
//	Edge *e=new Edge(coord,new Label(argIndex,Location::BOUNDARY,left,right));
	(*lineEdgeMap)[lr]=e;
	insertEdge(e);
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex,coord->getAt(0), Location::BOUNDARY);
#endif
}

void GeometryGraph::addPolygon(const Polygon *p){
	addPolygonRing((LinearRing*) p->getExteriorRing(),Location::EXTERIOR,Location::INTERIOR);
	for (int i=0;i<p->getNumInteriorRing();i++) {
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CW)
		addPolygonRing((LinearRing*) p->getInteriorRingN(i),Location::INTERIOR,Location::EXTERIOR);
	}
}

void GeometryGraph::addLineString(const LineString *line){
	CoordinateList* coord=CoordinateList::removeRepeatedPoints(line->getCoordinatesRO());
	if(coord->getSize()<2) {
		hasTooFewPointsVar=true;
		invalidPoint=coord->getAt(0);
		delete coord;
		return;
	}
	// add the edge for the LineString
	// line edges do not have locations for their left and right sides
	CoordinateList *ncr=CoordinateListFactory::internalFactory->createCoordinateList();
	for(int i=0;i<coord->getSize();i++) {
		ncr->add(coord->getAt(i));
	}
	Edge *e=new Edge(ncr,new Label(argIndex,Location::INTERIOR));
//	Edge *e=new Edge(coord,new Label(argIndex,Location::INTERIOR));
	(*lineEdgeMap)[line]=e;
	insertEdge(e);
	/**
	* Add the boundary points of the LineString, if any.
	* Even if the LineString is closed, add both points as if they were endpoints.
	* This allows for the case that the node already exists and is a boundary point.
	*/
	Assert::isTrue(coord->getSize()>= 2,"found LineString with single point");
	insertBoundaryPoint(argIndex,coord->getAt(0));
	insertBoundaryPoint(argIndex,coord->getAt(coord->getSize()-1));
	delete coord;
}

/**
* Add an Edge computed externally.  The label on the Edge is assumed
* to be correct.
*/
void GeometryGraph::addEdge(Edge *e) {
	insertEdge(e);
	const CoordinateList* coord=e->getCoordinates();
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex,coord->getAt(0),Location::BOUNDARY);
	insertPoint(argIndex,coord->getAt(coord->getSize()-1),Location::BOUNDARY);
}

/**
* Add a point computed externally.  The point is assumed to be a
* Point Geometry part, which has a location of INTERIOR.
*/
void GeometryGraph::addPoint(Coordinate& pt) {
	insertPoint(argIndex,pt,Location::INTERIOR);
}

/**
* Compute self-nodes, taking advantage of the Geometry type to
* minimize the number of intersection tests.  (E.g. rings are
* not tested for self-intersection, since they are assumed to be valid).
* @param li the LineIntersector to use
* @param computeRingSelfNodes if <false>, intersection checks are optimized to not test rings for self-intersection
* @return the SegmentIntersector used, containing information about the intersections found
*/
SegmentIntersector* GeometryGraph::computeSelfNodes(LineIntersector *li, bool computeRingSelfNodes){
	SegmentIntersector *si=new SegmentIntersector(li,true,false);
	//EdgeSetIntersector esi = new MCQuadIntersector();
    auto_ptr<EdgeSetIntersector> esi(createEdgeSetIntersector());
	// optimized test for Polygons and Rings
	if (parentGeom==NULL) {
		esi->computeIntersections(edges,si,true);
	} else if (!computeRingSelfNodes & (typeid(*parentGeom)==typeid(LinearRing)||typeid(*parentGeom)==typeid(Polygon)||typeid(*parentGeom)==typeid(MultiPolygon))) {
			esi->computeIntersections(edges, si, false);
	} else {
		esi->computeIntersections(edges,si,true);
	}
	//System.out.println("SegmentIntersector # tests = " + si.numTests);
	addSelfIntersectionNodes(argIndex);
	return si;
}

SegmentIntersector* GeometryGraph::computeEdgeIntersections(GeometryGraph *g,
													LineIntersector *li,
													bool includeProper){
	SegmentIntersector *si=new SegmentIntersector(li,includeProper,true);
	si->setBoundaryNodes(getBoundaryNodes(),g->getBoundaryNodes());
	auto_ptr<EdgeSetIntersector> esi(createEdgeSetIntersector());
	esi->computeIntersections(edges,g->edges,si);
	return si;
}

void GeometryGraph::insertPoint(int argIndex,const Coordinate& coord, int onLocation){
	Node *n=nodes->addNode(coord);
	Label *lbl=n->getLabel();
	if (lbl==NULL) {
		n->setLabel(argIndex,onLocation);
	} else
		lbl->setLocation(argIndex,onLocation);
}

/**
* Adds points using the mod-2 rule of SFS.  This is used to add the boundary
* points of dim-1 geometries (Curves/MultiCurves).  According to the SFS,
* an endpoint of a Curve is on the boundary
* iff if it is in the boundaries of an odd number of Geometries
*/
void GeometryGraph::insertBoundaryPoint(int argIndex,const Coordinate& coord){
	Node *n=nodes->addNode(coord);
	Label *lbl=n->getLabel();
	// the new point to insert is on a boundary
	int boundaryCount=1;
	// determine the current location for the point (if any)
	int loc=Location::UNDEF;
	if (lbl!=NULL) loc=lbl->getLocation(argIndex,Position::ON);
	if (loc==Location::BOUNDARY) boundaryCount++;
	// determine the boundary status of the point according to the Boundary Determination Rule
	int newLoc=determineBoundary(boundaryCount);
	lbl->setLocation(argIndex,newLoc);
}

void GeometryGraph::addSelfIntersectionNodes(int argIndex){
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int eLoc=e->getLabel()->getLocation(argIndex);
		vector<EdgeIntersection*> *eil=e->eiList->list;
		for (vector<EdgeIntersection*>::iterator eiIt=eil->begin();eiIt<eil->end();eiIt++) {
			EdgeIntersection *ei=*eiIt;
			addSelfIntersectionNode(argIndex,ei->coord,eLoc);
		}
	}
}

/**
* Add a node for a self-intersection.
* If the node is a potential boundary node (e.g. came from an edge which
* is a boundary) then insert it as a potential boundary node.
* Otherwise, just add it as a regular node.
*/
void GeometryGraph::addSelfIntersectionNode(int argIndex,Coordinate& coord,int loc){
	// if this node is already a boundary node, don't change it
	if (isBoundaryNode(argIndex,coord)) return;
	if (loc==Location::BOUNDARY && useBoundaryDeterminationRule)
		insertBoundaryPoint(argIndex,coord);
	else
		insertPoint(argIndex,coord,loc);
}

vector<Edge*> *GeometryGraph::getEdges() {
	return edges;
}

bool GeometryGraph::hasTooFewPoints() {
	return hasTooFewPointsVar;
}

const Coordinate& GeometryGraph::getInvalidPoint() {
	return invalidPoint;
}

}

