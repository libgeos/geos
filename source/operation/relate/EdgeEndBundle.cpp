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
 * Revision 1.11  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.10  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opRelate.h>
#include <stdio.h>

namespace geos {

EdgeEndBundle::EdgeEndBundle(EdgeEnd *e):
	EdgeEnd(e->getEdge(),e->getCoordinate(),
		e->getDirectedCoordinate(),
		new Label(*(e->getLabel())))
{
	edgeEnds=new vector<EdgeEnd*>();
	insert(e);
}

EdgeEndBundle::~EdgeEndBundle(){
	for(int i=0;i<(int)edgeEnds->size();i++) {
		delete (*edgeEnds)[i];
	}
	delete edgeEnds;
}

Label* EdgeEndBundle::getLabel(){
	return label;
}

//Not needed
//public Iterator iterator() { return edgeEnds.iterator(); }

vector<EdgeEnd*>* EdgeEndBundle::getEdgeEnds() {
	return edgeEnds;
}

void EdgeEndBundle::insert(EdgeEnd *e){
	// Assert: start point is the same
	// Assert: direction is the same
	edgeEnds->push_back(e);
}
  

/**
* This computes the overall edge label for the set of
* edges in this EdgeStubBundle.  It essentially merges
* the ON and side labels for each edge.  These labels must be compatible
*/
void EdgeEndBundle::computeLabel() {
	// create the label.  If any of the edges belong to areas,
	// the label must be an area label
	bool isArea=false;
	for(vector<EdgeEnd*>::iterator it=edgeEnds->begin();it<edgeEnds->end();it++) {
		EdgeEnd *e=*it;
		if (e->getLabel()->isArea()) isArea=true;
	}
	if (isArea) {
		delete label;
		label=new Label(Location::UNDEF,Location::UNDEF,Location::UNDEF);
	} else {
		delete label;
		label=new Label(Location::UNDEF);
	}
	// compute the On label, and the side labels if present
	for(int i=0;i<2;i++) {
		computeLabelOn(i);
		if (isArea)
			computeLabelSides(i);
	}
}


/**
* Compute the overall ON location for the list of EdgeStubs.
* (This is essentially equivalent to computing the self-overlay of a single Geometry)
* edgeStubs can be either on the boundary (eg Polygon edge)
* OR in the interior (e.g. segment of a LineString)
* of their parent Geometry.
* In addition, GeometryCollections use the mod-2 rule to determine
* whether a segment is on the boundary or not.
* Finally, in GeometryCollections it can still occur that an edge is both
* on the boundary and in the interior (e.g. a LineString segment lying on
* top of a Polygon edge.) In this case as usual the Boundary is given precendence.
* <br>
* These observations result in the following rules for computing the ON location:
* <ul>
* <li> if there are an odd number of Bdy edges, the attribute is Bdy
* <li> if there are an even number >= 2 of Bdy edges, the attribute is Int
* <li> if there are any Int edges, the attribute is Int
* <li> otherwise, the attribute is NULL.
* </ul>
*/
void EdgeEndBundle::computeLabelOn(int geomIndex) {
	// compute the ON location value
	int boundaryCount=0;
	bool foundInterior=false;

	for(vector<EdgeEnd*>::iterator it=edgeEnds->begin();it<edgeEnds->end();it++) {
		EdgeEnd *e=*it;
		int loc=e->getLabel()->getLocation(geomIndex);
		if (loc==Location::BOUNDARY) boundaryCount++;
		if (loc==Location::INTERIOR) foundInterior=true;
	}
	int loc=Location::UNDEF;
	if (foundInterior) loc=Location::INTERIOR;
	if (boundaryCount>0) {
		loc=GeometryGraph::determineBoundary(boundaryCount);
	}
	label->setLocation(geomIndex,loc);
}


/**
* Compute the labelling for each side
*/
void EdgeEndBundle::computeLabelSides(int geomIndex) {
	computeLabelSide(geomIndex,Position::LEFT);
	computeLabelSide(geomIndex,Position::RIGHT);
}

/**
* To compute the summary label for a side, the algorithm is:
*   FOR all edges
*     IF any edge's location is INTERIOR for the side, side location = INTERIOR
*     ELSE IF there is at least one EXTERIOR attribute, side location = EXTERIOR
*     ELSE  side location = NULL
*  <br>
*  Note that it is possible for two sides to have apparently contradictory information
*  i.e. one edge side may indicate that it is in the interior of a geometry, while
*  another edge side may indicate the exterior of the same geometry.  This is
*  not an incompatibility - GeometryCollections may contain two Polygons that touch
*  along an edge.  This is the reason for Interior-primacy rule above - it
*  results in the summary label having the Geometry interior on <b>both</b> sides.
*/
void EdgeEndBundle::computeLabelSide(int geomIndex, int side) {
	for(vector<EdgeEnd*>::iterator it=edgeEnds->begin();it<edgeEnds->end();it++) {
		EdgeEnd *e=*it;
		if (e->getLabel()->isArea()) {
			int loc=e->getLabel()->getLocation(geomIndex,side);
			if (loc==Location::INTERIOR) {
				label->setLocation(geomIndex,side,Location::INTERIOR);
				return;
			} else if (loc==Location::EXTERIOR) {
				label->setLocation(geomIndex,side,Location::EXTERIOR);
			}
		}
	}
}

/**
* Update the IM with the contribution for the computed label for the EdgeStubs.
*/
void EdgeEndBundle::updateIM(IntersectionMatrix *im) {
	Edge::updateIM(label,im);
}

string EdgeEndBundle::print() {
	string out="EdgeEndBundle--> Label: "+label->toString()+"\n";
	for(vector<EdgeEnd*>::iterator it=edgeEnds->begin();it<edgeEnds->end();it++) {
		EdgeEnd *e=*it;
		out+=e->print();
		out+="\n";
	}
	return out;
}

}

