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
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geomgraph.h>

namespace geos {

Label* Label::toLineLabel(const Label* label) {
	Label *lineLabel=new Label(Location::UNDEF);
	for (int i=0; i<2; i++) {
		lineLabel->setLocation(i,label->getLocation(i));
	}
	return lineLabel;
}

/**
 * Construct a Label with a single location for both Geometries.
 * Initialize the locations to Null
 */
Label::Label(int onLoc){
	elt[0]=new TopologyLocation(onLoc);
	elt[1]=new TopologyLocation(onLoc);
}

/**
 * Construct a Label with a single location for both Geometries.
 * Initialize the location for the Geometry index.
 */
Label::Label(int geomIndex,int onLoc) {
	elt[0]=new TopologyLocation(Location::UNDEF);
	elt[1]=new TopologyLocation(Location::UNDEF);
	elt[geomIndex]->setLocation(onLoc);
}

/**
 * Construct a Label with On, Left and Right locations for both Geometries.
 * Initialize the locations for both Geometries to the given values.
 */
Label::Label(int onLoc,int leftLoc,int rightLoc) {
	elt[0]=new TopologyLocation(onLoc,leftLoc,rightLoc);
	elt[1]=new TopologyLocation(onLoc,leftLoc,rightLoc);
}

Label::Label() {
	elt[0]=new TopologyLocation(Location::UNDEF);
	elt[1]=new TopologyLocation(Location::UNDEF);
}

Label::Label(const Label *l) {
	elt[0]=new TopologyLocation(l->elt[0]);
	elt[1]=new TopologyLocation(l->elt[1]);
}

Label::~Label() {
	delete elt[0];
	delete elt[1];
}

/**
 * Construct a Label with On, Left and Right locations for both Geometries.
 * Initialize the locations for the given Geometry index.
 */
Label::Label(int geomIndex,int onLoc,int leftLoc,int rightLoc){
	elt[0]=new TopologyLocation(Location::UNDEF,Location::UNDEF,Location::UNDEF);
	elt[1]=new TopologyLocation(Location::UNDEF,Location::UNDEF,Location::UNDEF);
	elt[geomIndex]->setLocations(onLoc,leftLoc,rightLoc);
}

/**
 * Construct a Label with the same values as the argument for the
 * given Geometry index.
 */
Label::Label(int geomIndex,const TopologyLocation* gl){
	elt[0]=new TopologyLocation(gl->getLocations());
	elt[1]=new TopologyLocation(gl->getLocations());
	elt[geomIndex]->setLocations(*gl);
}

void Label::flip(){
	elt[0]->flip();
	elt[1]->flip();
}

int Label::getLocation(int geomIndex,int posIndex) const {
	return elt[geomIndex]->get(posIndex);
}

int Label::getLocation(int geomIndex) const {
	return elt[geomIndex]->get(Position::ON);
}

void Label::setLocation(int geomIndex,int posIndex,int location) {
	elt[geomIndex]->setLocation(posIndex,location);
}

void Label::setLocation(int geomIndex,int location){
	elt[geomIndex]->setLocation(Position::ON,location);
}

void Label::setAllLocations(int geomIndex,int location){
	elt[geomIndex]->setAllLocations(location);
}

void Label::setAllLocationsIfNull(int geomIndex,int location){
	elt[geomIndex]->setAllLocationsIfNull(location);
}

void Label::setAllLocationsIfNull(int location){
	setAllLocationsIfNull(0,location);
	setAllLocationsIfNull(1,location);
}

/**
 * Merge this label with another one.
 * Merging updates any null attributes of this label with the attributes from lbl
 */
void Label::merge(const Label* lbl) {
	for (int i=0; i<2; i++) {
		if (elt[i]==NULL && lbl->elt[i]!=NULL) {
			elt[i]=new TopologyLocation(lbl->elt[i]);
		} else {
			elt[i]->merge(lbl->elt[i]);
		}
	}
}

void Label::setGeometryLocation(int geomIndex, const TopologyLocation* tl){
	if (tl==NULL) return;
	elt[geomIndex]->setLocations(*tl);
}

int Label::getGeometryCount() const {
	int count = 0;
	if (!elt[0]->isNull()) count++;
    if (!elt[1]->isNull()) count++;
	return count;
}

bool Label::isNull(int geomIndex) const {
	return elt[geomIndex]->isNull();
}

bool Label::isAnyNull(int geomIndex) const {
	return elt[geomIndex]->isAnyNull();
}

bool Label::isArea() const {
	return elt[0]->isArea() || elt[1]->isArea();
}

bool Label::isArea(int geomIndex) const {
	return elt[geomIndex]->isArea();
}

bool Label::isLine(int geomIndex) const {
	return elt[geomIndex]->isLine();
}

bool Label::isEqualOnSide(Label* lbl, int side) const {
	return 
		elt[0]->isEqualOnSide(*(lbl->elt[0]), side)
		&& elt[1]->isEqualOnSide(*(lbl->elt[1]), side);
}

bool Label::allPositionsEqual(int geomIndex, int loc) const {
	return elt[geomIndex]->allPositionsEqual(loc);
}

/**
 * Converts one GeometryLocation to a Line location
 */
void Label::toLine(int geomIndex){
	if (elt[geomIndex]->isArea()) {
		TopologyLocation *tl=elt[geomIndex];
		elt[geomIndex]=new TopologyLocation((*(elt[geomIndex]->getLocations()))[0]);
		delete tl;
	}
}

string Label::toString() const {
	string buf="";
	if (elt[0]!=NULL) {
		buf.append("a:");
		buf.append(elt[0]->toString());
	}
	if (elt[1]!=NULL) {
		buf.append(" b:");
		buf.append(elt[1]->toString());
	}
	return buf;
}

}

