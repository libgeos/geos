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
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.22  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.21  2003/11/06 17:59:03  strk
 * Memory leaks fixed in ::containsPoint()
 *
 * Revision 1.20  2003/10/20 14:02:14  strk
 * more explicit exception thrown on null Directed Edge detection
 *
 * Revision 1.19  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 * $Log$
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.22  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geomgraph.h"
#include "../headers/util.h"

namespace geos {

EdgeRing::EdgeRing(DirectedEdge *newStart,GeometryFactory *newGeometryFactory,CGAlgorithms *newCga) {
	label=new Label(Location::UNDEF);
	maxNodeDegree=-1;
	geometryFactory=newGeometryFactory;
	cga=newCga;
	edges=new vector<DirectedEdge*>();
	pts=CoordinateListFactory::internalFactory->createCoordinateList();
	ring=NULL;
	shell=NULL;
	holes=new vector<EdgeRing*>();
//	Commented out to fix different polymorphism in C++ (from Java)
//	computePoints(newStart);
//	computeRing();
}

EdgeRing::~EdgeRing(){
	delete edges;
	delete pts;
	delete label;
	delete ring;
//	delete shell;
	for(int i=0;i<(int)holes->size();i++) {
		delete (*holes)[i];
	}
	delete holes;
}

bool EdgeRing::isIsolated(){
	return (label->getGeometryCount()==1);
}

bool EdgeRing::isHole(){
	return isHoleVar;
}

const Coordinate& EdgeRing::getCoordinate(int i) {
	return pts->getAt(i);
}

LinearRing* EdgeRing::getLinearRing() {
	return new LinearRing(*ring);
//	return ring;
}

Label* EdgeRing::getLabel() {
	return label;
}

bool EdgeRing::isShell(){
	return shell==NULL;
}

EdgeRing* EdgeRing::getShell() {
	return shell;
}

void EdgeRing::setShell(EdgeRing *newShell) {
	shell=newShell;
	if (shell!=NULL) shell->addHole(this);
}

void EdgeRing::addHole(EdgeRing *edgeRing) {
	holes->push_back(edgeRing);
}

Polygon* EdgeRing::toPolygon(GeometryFactory* geometryFactory){
	vector<Geometry *> *holeLR=new vector<Geometry *>();
	for (unsigned int i=0;i<holes->size();i++) {
        holeLR->push_back((*holes)[i]->getLinearRing());
//        holeLR->push_back((*holes)[i]->ring);
	}
	return geometryFactory->createPolygon(getLinearRing(),holeLR);
}

void EdgeRing::computeRing() {
	if (ring!=NULL) return;   // don't compute more than once
	ring=geometryFactory->createLinearRing(pts);
	CoordinateList *cl = ring->getCoordinates();
	isHoleVar=cga->isCCW(cl);
	delete cl;
}

  /**
 * Returns the list of DirectedEdges that make up this EdgeRing
 */
vector<DirectedEdge*>* EdgeRing::getEdges() {
	return edges;
}

void EdgeRing::computePoints(DirectedEdge *newStart){
	startDe=newStart;
	DirectedEdge *de=newStart;
	bool isFirstEdge=true;
	do {
		Assert::isTrue(de!=NULL,"EdgeRing::computePoints: found null Directed Edge");
		if (de->getEdgeRing()==this)
			throw new TopologyException("Directed Edge visited twice during ring-building at ",&(de->getCoordinate()));
		edges->push_back(de);
		Label *deLabel=de->getLabel();
		Assert::isTrue(deLabel->isArea());
		mergeLabel(deLabel);
		addPoints(de->getEdge(),de->isForward(),isFirstEdge);
		isFirstEdge=false;
		setEdgeRing(de,this);
		de=getNext(de);
	} while (de!=startDe);
}

int EdgeRing::getMaxNodeDegree(){
	if (maxNodeDegree<0) computeMaxNodeDegree();
	return maxNodeDegree;
}

void EdgeRing::computeMaxNodeDegree() {
	maxNodeDegree=0;
	DirectedEdge *de=startDe;
	do {
		Node *node=de->getNode();
		int degree=((DirectedEdgeStar*) node->getEdges())->getOutgoingDegree(this);
		if (degree>maxNodeDegree) maxNodeDegree=degree;
		de=getNext(de);
	} while (de!=startDe);
	maxNodeDegree *= 2;
}

void EdgeRing::setInResult(){
	DirectedEdge *de=startDe;
	do {
		(de->getEdge())->setInResult(true);
		de=de->getNext();
	} while (de!=startDe);
}

void EdgeRing::mergeLabel(Label *deLabel){
	mergeLabel(deLabel, 0);
	mergeLabel(deLabel, 1);
}

/**
 * Merge the RHS label from a DirectedEdge into the label for this EdgeRing.
 * The DirectedEdge label may be null.  This is acceptable - it results
 * from a node which is NOT an intersection node between the Geometries
 * (e.g. the end node of a LinearRing).  In this case the DirectedEdge label
 * does not contribute any information to the overall labelling, and is simply skipped.
 */
void EdgeRing::mergeLabel(Label *deLabel, int geomIndex){
	int loc=deLabel->getLocation(geomIndex,Position::RIGHT);
	// no information to be had from this label
	if (loc==Location::UNDEF) return;
	// if there is no current RHS value, set it
	if (label->getLocation(geomIndex)==Location::UNDEF) {
		label->setLocation(geomIndex,loc);
		return;
	}
}

void EdgeRing::addPoints(Edge *edge, bool isForward, bool isFirstEdge){
	const CoordinateList* edgePts=edge->getCoordinates();
	if (isForward) {
		int startIndex=1;
		if (isFirstEdge) startIndex=0;
		for (int i=startIndex; i<edgePts->getSize(); i++) {
			pts->add(edgePts->getAt(i));
		}
	} else { // is backward
		int startIndex=edgePts->getSize()-2;
		if (isFirstEdge) startIndex=edgePts->getSize()-1;
		for (int i=startIndex;i>=0;i--) {
			pts->add(edgePts->getAt(i));
		}
	}
}

/**
 * This method will cause the ring to be computed.
 * It will also check any holes, if they have been assigned.
 */
bool EdgeRing::containsPoint(Coordinate& p){
	//LinearRing *lrShell=getLinearRing();
	//Envelope* env=lrShell->getEnvelopeInternal();
	Envelope* env=ring->getEnvelopeInternal();
	if (!env->contains(p))
	{
		delete env;
		return false;
	}
	delete env;

//External Dependency
//	if (!cga.isPointInPolygon(p, shell.getCoordinates()) ) return false;
	for (vector<EdgeRing*>::iterator i=holes->begin();i<holes->end();i++) {
		EdgeRing *hole=*i;
		if (hole->containsPoint(p))
		{
			return false;
		}
	}
	return true;
}
}

