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
 * Revision 1.17  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.16  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateList argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.15  2004/05/19 13:18:24  strk
 * made CoordinateList::toString() a const member function
 *
 * Revision 1.14  2004/05/17 21:09:26  ybychkov
 * toString() performance enhancement
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.12  2003/10/15 11:23:00  strk
 * Formalized const nature of toVector() method and of first 
 * argument to static removeRepeatedPoints().
 *
 * Revision 1.11  2003/10/15 10:17:36  strk
 * Made setPoints() get a const vector<Coordinate>.
 *
 **********************************************************************/


#include <geos/geom.h>
#include <stdio.h>

namespace geos {

BasicCoordinateList::BasicCoordinateList(const CoordinateList *c){
	vect=new vector<Coordinate>();
	int size=c->getSize();
	for(int i=0; i<size; i++) {
		vect->push_back(c->getAt(i));
	}
}

BasicCoordinateList::BasicCoordinateList() {
	vect=new vector<Coordinate>();
}

BasicCoordinateList::BasicCoordinateList(int n) {
	vect=new vector<Coordinate>(n);
//	vect->reserve(n);
//	vect->resize(n);
}

BasicCoordinateList::BasicCoordinateList(const Coordinate& c) {
	vect=new vector<Coordinate>(1,c);
}

BasicCoordinateList::BasicCoordinateList(const BasicCoordinateList &c) {
	vect=new vector<Coordinate>(*(c.vect));
}

void BasicCoordinateList::setPoints(const vector<Coordinate> &v) {
	//vect->swap(v);
	delete vect;
	vect=new vector<Coordinate>(v);
}

vector<Coordinate>* BasicCoordinateList::toVector() const {
	return new vector<Coordinate>(vect->begin(),vect->end());
}

bool BasicCoordinateList::isEmpty() const {
	return vect->empty();
}

void BasicCoordinateList::add(const Coordinate& c){
	vect->push_back(c);
}

int BasicCoordinateList::getSize() const {
	return (int) vect->size();
}

const Coordinate& BasicCoordinateList::getAt(int pos) const {
//	if (pos>=0 && pos<=vect->size()-1) 
		return (*vect)[pos];
//	else
//		throw "BasicCoordinateList exception: can't retrieve element\n";
}

void BasicCoordinateList::setAt(const Coordinate& c, int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=c;
//	else
//		throw "BasicCoordinateList exception: can't change element\n";
}
void BasicCoordinateList::deleteAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		vect->erase(vect->begin()+pos);
//	else
//		throw "BasicCoordinateList exception: can't remove element\n";
}

string BasicCoordinateList::toString() const {
	string result("");
	if (getSize()>0) {
		char buffer[100];
		for (unsigned int i=0; i<vect->size(); i++) {
			Coordinate& c=(*vect)[i];
			sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
			result.append(buffer);
		}
		result.append("");
	}
	return result;
}

BasicCoordinateList::~BasicCoordinateList() {
	delete vect;
}
}
