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
 **********************************************************************/

#include <geos/geomgraph.h>
//#include "../headers/util.h"

#ifndef DEBUG
#define DEBUG 0
#endif
#ifndef COMPUTE_Z
#define COMPUTE_Z 0
#endif

namespace geos {

Node::Node(Coordinate& newCoord, EdgeEndStar* newEdges): GraphComponent(new Label(0,Location::UNDEF)) {
	coord=newCoord;
#if COMPUTE_Z
	z=0;
	validz=0;
#endif
	edges=newEdges;
}

Node::~Node(){
	delete edges;
}

const Coordinate& Node::getCoordinate() const {
	return coord;
}

EdgeEndStar* Node::getEdges() {
	return edges;
}

bool Node::isIsolated() {
	return (label->getGeometryCount()==1);
}

void Node::add(EdgeEnd *e) {
#if DEBUG
	cerr<<"Node::add("<<e->print()<<")"<<endl;
#endif
	// Assert: start pt of e is equal to node point
	if (edges==NULL)
		edges=new EdgeEndStar();
	edges->insert(e);
	e->setNode(this);
#if COMPUTE_Z
	Coordinate &ec = e->getCoordinate();
	if ( ec.z != DoubleNotANumber )
	{
		z+=ec.z;
		validz++;
	}
	coord.z=z/validz;
#if DEBUG
	cerr<<" z=="<<z<<endl;
	cerr<<" validz=="<<validz<<endl;
	cerr<<" coord.z=="<<coord.z<<endl;
#endif // DEBUG
#endif
}

void Node::mergeLabel(const Node* n) {
	mergeLabel(n->label);
}

void Node::mergeLabel(const Label* label2) {
	for (int i=0; i<2; i++) {
		int loc=computeMergedLocation(label2, i);
		int thisLoc=label->getLocation(i);
		if (thisLoc==Location::UNDEF) label->setLocation(i,loc);
	}
}

void Node::setLabel(int argIndex, int onLocation) {
	if (label==NULL) {
		label=new Label(argIndex, onLocation);
	} else
		label->setLocation(argIndex, onLocation);
}

void Node::setLabelBoundary(int argIndex) {
	int loc=Location::UNDEF;
	if (label!=NULL)
		loc=label->getLocation(argIndex);
	// flip the loc
	int newLoc;
	switch (loc){
		case Location::BOUNDARY: newLoc=Location::INTERIOR; break;
		case Location::INTERIOR: newLoc=Location::BOUNDARY; break;
		default: newLoc=Location::BOUNDARY;  break;
	}
	label->setLocation(argIndex, newLoc);
}

int Node::computeMergedLocation(const Label* label2, int eltIndex){
	int loc=Location::UNDEF;
	loc=label->getLocation(eltIndex);
	if (!label2->isNull(eltIndex)) {
		int nLoc=label2->getLocation(eltIndex);
		if (loc!=Location::BOUNDARY) loc=nLoc;
	}
	return loc;
}

string Node::print(){
	string out="node "+coord.toString()+" lbl: "+label->toString();
	return out;
}

}

/**********************************************************************
 * $Log$
 * Revision 1.3  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
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

