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
 * Revision 1.3  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geomgraph.h>
#include <cmath>

namespace geos {

/**
 * Computes the factor for the change in depth when moving from one location to another.
 * E.g. if crossing from the INTERIOR to the EXTERIOR the depth decreases, so the factor is -1
 */
int DirectedEdge::depthFactor(int currLocation, int nextLocation){
	if (currLocation==Location::EXTERIOR && nextLocation==Location::INTERIOR)
		return 1;
	else if (currLocation==Location::INTERIOR && nextLocation==Location::EXTERIOR)
		return -1;
	return 0;
}

DirectedEdge::DirectedEdge(): EdgeEnd() {
	isInResultVar=false;
	isVisitedVar=false;
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;

	sym=NULL;
	next=NULL;
	nextMin=NULL;

	edgeRing=NULL;
	minEdgeRing=NULL;
}

DirectedEdge::~DirectedEdge() {
//aaaa
}

DirectedEdge::DirectedEdge(Edge *newEdge, bool newIsForward): EdgeEnd(newEdge){
	isInResultVar=false;
	isVisitedVar=false;
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;

	sym=NULL;
	next=NULL;
	nextMin=NULL;

	edgeRing=NULL;
	minEdgeRing=NULL;

	isForwardVar=newIsForward;
	if (isForwardVar) {
		init(edge->getCoordinate(0), edge->getCoordinate(1));
	} else {
		int n=edge->getNumPoints()-1;
		init(edge->getCoordinate(n), edge->getCoordinate(n-1));
	}
	computeDirectedLabel();
}

Edge* DirectedEdge::getEdge() {
	return edge;
}

void DirectedEdge::setInResult(bool newIsInResult) {
	isInResultVar=newIsInResult;
}

bool DirectedEdge::isInResult() {
	return isInResultVar;
}

bool DirectedEdge::isVisited() {
	return isVisitedVar;
}

void DirectedEdge::setVisited(bool newIsVisited) {
	isVisitedVar=newIsVisited;
}

void DirectedEdge::setEdgeRing(EdgeRing *newEdgeRing) {
	edgeRing=newEdgeRing;
}

EdgeRing* DirectedEdge::getEdgeRing() {
	return edgeRing;
}

void DirectedEdge::setMinEdgeRing(EdgeRing *newMinEdgeRing) {
	minEdgeRing=newMinEdgeRing;
}

EdgeRing* DirectedEdge::getMinEdgeRing() {
	return minEdgeRing;
}

int DirectedEdge::getDepth(int position){
	return depth[position];
}

void DirectedEdge::setDepth(int position, int newDepth) {
	if (depth[position]!=-999) {
		//if (depth[position]!=newDepth) {
		//	cout << "Debug: " << this <<endl;;
		//}
		if (depth[position]!=newDepth)
			throw new TopologyException("assigned depths do not match", &getCoordinate());
			//Assert.isTrue(depth[position] == depthVal, "assigned depths do not match at " + getCoordinate());
	}
	depth[position]=newDepth;
}

int DirectedEdge::getDepthDelta(){
	int depthDelta=edge->getDepthDelta();
	if (!isForwardVar) depthDelta=-depthDelta;
	return depthDelta;
}

/**
 * setVisitedEdge marks both DirectedEdges attached to a given Edge.
 * This is used for edges corresponding to lines, which will only
 * appear oriented in a single direction in the result.
 */
void DirectedEdge::setVisitedEdge(bool newIsVisited){
	setVisited(newIsVisited);
	sym->setVisited(newIsVisited);
}

/**
 * Each Edge gives rise to a pair of symmetric DirectedEdges, in opposite
 * directions.
 * @return the DirectedEdge for the same Edge but in the opposite direction
 */
DirectedEdge* DirectedEdge::getSym() {
	return sym;
}

bool DirectedEdge::isForward() {
	return isForwardVar;
}

void DirectedEdge::setSym(DirectedEdge *de){
	sym=de;
}

DirectedEdge* DirectedEdge::getNext() {
	return next;
}

void DirectedEdge::setNext(DirectedEdge *newNext) {
	next=newNext;
}

DirectedEdge* DirectedEdge::getNextMin() {
	return nextMin;
}

void DirectedEdge::setNextMin(DirectedEdge *newNextMin) {
	nextMin=newNextMin;
}

/**
 * This edge is a line edge if
 * <ul>
 * <li> at least one of the labels is a line label
 * <li> any labels which are not line labels have all Locations = EXTERIOR
 * </ul>
 */
bool DirectedEdge::isLineEdge() {
	bool isLine=label->isLine(0) || label->isLine(1);
	bool isExteriorIfArea0=!label->isArea(0) || label->allPositionsEqual(0,Location::EXTERIOR);
	bool isExteriorIfArea1=!label->isArea(1) || label->allPositionsEqual(1,Location::EXTERIOR);
	return isLine && isExteriorIfArea0 && isExteriorIfArea1;
}

/**
 * This is an interior Area edge if
 * <ul>
 * <li> its label is an Area label for both Geometries
 * <li> and for each Geometry both sides are in the interior.
 * </ul>
 *
 * @return true if this is an interior Area edge
 */
bool DirectedEdge::isInteriorAreaEdge(){
	bool isInteriorAreaEdge=true;
	for (int i=0; i<2; i++) {
		if (!(label->isArea(i)
			&& label->getLocation(i,Position::LEFT )==Location::INTERIOR
			&& label->getLocation(i,Position::RIGHT)==Location::INTERIOR)) {
				isInteriorAreaEdge=false;
		}
	}
	return isInteriorAreaEdge;
}

/**
 * Compute the label in the appropriate orientation for this DirEdge
 */
void DirectedEdge::computeDirectedLabel(){
	delete label;
	label=new Label(edge->getLabel());
	if (!isForwardVar)
		label->flip();
}

/**
 * Set both edge depths.  One depth for a given side is provided.  The other is
 * computed depending on the Location transition and the depthDelta of the edge.
 */
void DirectedEdge::setEdgeDepths(int position, int newDepth){
	// get the depth transition delta from R to L for this directed Edge
	int depthDelta=getEdge()->getDepthDelta();
	if (!isForwardVar) depthDelta=-depthDelta;
	// if moving from L to R instead of R to L must change sign of delta
	int directionFactor=1;
	if (position==Position::LEFT)
		directionFactor=-1;
	int oppositePos=Position::opposite(position);
	int delta=depthDelta*directionFactor;
	//TESTINGint delta = depthDelta * DirectedEdge.depthFactor(loc, oppositeLoc);
	int oppositeDepth=newDepth+delta;
	setDepth(position,newDepth);
	setDepth(oppositePos,oppositeDepth);
}

/**
 * Set both edge depths.  One depth for a given side is provided.  The other is
 * computed depending on the Location transition and the depthDelta of the edge.
 */
void DirectedEdge::OLDsetEdgeDepths(int position, int newDepth){
	int depthDelta=getEdge()->getDepthDelta();
	int loc=label->getLocation(0, position);
	int oppositePos=Position::opposite(position);
	int oppositeLoc=label->getLocation(0, oppositePos);
	int delta=abs(depthDelta)*DirectedEdge::depthFactor(loc,oppositeLoc);
	int oppositeDepth=newDepth + delta;
	setDepth(position, newDepth);
	setDepth(oppositePos,oppositeDepth);
}

string DirectedEdge::print(){
	string out=EdgeEnd::print();
	out+=" ";
	out+=depth[Position::LEFT];
	out+="/";
	out+=depth[Position::RIGHT];
	out+=" (";
	out+=getDepthDelta();
	out+=")";
	if (isInResultVar) out+=" inResult";
	return out;
}

string DirectedEdge::printEdge(){
	string out=print();
	out+=" ";
	if (isForwardVar)
		out+=edge->print();
	else
		out+=edge->printReverse();
	return out;
}

}

