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
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geomgraph.h>
#include <geos/util.h>
#include <typeinfo>
#include <math.h>
#include <stdio.h>

namespace geos {

//CGAlgorithms* EdgeEnd::cga=new RobustCGAlgorithms();

EdgeEnd::EdgeEnd() {
	this->edge=NULL;
	label=NULL;
	node=NULL;
	dx=dy=0.0;
	quadrant=0;
}

EdgeEnd::~EdgeEnd() {
//	delete edge;
	delete label;
//	delete node;  
}

EdgeEnd::EdgeEnd(Edge* newEdge){
	this->edge=newEdge;
	label=NULL;
	node=NULL;
	dx=dy=0.0;
	quadrant=0;
}

EdgeEnd::EdgeEnd(Edge* newEdge,Coordinate& newP0, Coordinate& newP1){
	this->edge=newEdge;
	node=NULL;
	dx=dy=0.0;
	quadrant=0;
	init(newP0,newP1);
	label=NULL;
}

EdgeEnd::EdgeEnd(Edge* newEdge, Coordinate& newP0, Coordinate& newP1, Label* newLabel){
	this->edge=newEdge;
	node=NULL;
	dx=dy=0.0;
	quadrant=0;
	init(newP0,newP1);
	label=newLabel;
}

void EdgeEnd::init(const Coordinate& newP0, const Coordinate& newP1){
	p0=newP0;
	p1=newP1;
	dx=p1.x-p0.x;
	dy=p1.y-p0.y;
	quadrant=Quadrant::quadrant(dx,dy);
	Assert::isTrue(!(dx == 0 && dy == 0), "EdgeEnd with identical endpoints found");
}

Edge* EdgeEnd::getEdge() {return edge;}
Label* EdgeEnd::getLabel() {return label;}
Coordinate& EdgeEnd::getCoordinate() {return p0;}
Coordinate& EdgeEnd::getDirectedCoordinate() {return p1;}
int EdgeEnd::getQuadrant() {return quadrant;}
double EdgeEnd::getDx() {return dx;}
double EdgeEnd::getDy() {return dy;}
void EdgeEnd::setNode(Node* newNode) {node=newNode;}
Node* EdgeEnd::getNode() {return node;}

int EdgeEnd::compareTo(EdgeEnd* e) {
	return compareDirection(e);
}

int EdgeEnd::compareDirection(EdgeEnd* e) {
	if (dx == e->dx && dy == e->dy)
		return 0;
	// if the rays are in different quadrants, determining the ordering is trivial
	if (quadrant>e->quadrant) return 1;
	if (quadrant<e->quadrant) return -1;
	// vectors are in the same quadrant - check relative orientation of direction vectors
	// this is > e if it is CCW of e
	return CGAlgorithms::computeOrientation(e->p0,e->p1,p1);
}

void EdgeEnd::computeLabel(){
	// subclasses should override this if they are using labels
}

string EdgeEnd::print() {
	char buffer[255];
	sprintf(buffer,"%i:%g",quadrant,atan2(dy,dx));
	string angleStr(buffer);
//	string className=typeid(*this).name();
	string className="EdgeEnd";
	string out="  "+className+": "+p0.toString()+" - "+p1.toString()+" ";
	out.append(angleStr);
	out+="   " + label->toString();
	return out;
}

}

