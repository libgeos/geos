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
 * Revision 1.15  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

bool CoordinateList::hasRepeatedPoints() const {
	int size=(int) getSize();
	for(int i=1; i<size; i++) {
		if (getAt(i-1)==getAt(i)) {
			return true;
		}
	}
	return false;
}

/**
* Returns either the given coordinate array if its length is greater than the
* given amount, or an empty coordinate array.
*/
CoordinateList* atLeastNCoordinatesOrNothing(int n,CoordinateList *c) {
	return c->getSize()>=n?c:CoordinateListFactory::internalFactory->createCoordinateList();
}      


bool CoordinateList::hasRepeatedPoints(const CoordinateList *cl) {
	int size=(int) cl->getSize();
	for(int i=1;i<size; i++) {
		if (cl->getAt(i-1)==cl->getAt(i)) {
			return true;
		}
	}
	return false;
}

const Coordinate* CoordinateList::minCoordinate() const {
	const Coordinate* minCoord=NULL;
	int size=(int) getSize();
	for(int i=0; i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(getAt(i))>0) {
			minCoord=&getAt(i);
		}
	}
	return minCoord;
}

const Coordinate*
CoordinateList::minCoordinate(CoordinateList *cl)
{
	const Coordinate* minCoord=NULL;
	int size=(int) cl->getSize();
	for(int i=0;i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(cl->getAt(i))>0) {
			minCoord=&(cl->getAt(i));
		}
	}
	return minCoord;
}

int
CoordinateList::indexOf(const Coordinate *coordinate, const CoordinateList *cl)
{
	for (int i=0; i<cl->getSize(); i++) {
		if ((*coordinate)==cl->getAt(i)) {
			return i;
		}
	}
	return -1;
}

void
CoordinateList::scroll(CoordinateList* cl,const Coordinate* firstCoordinate)
{
	int ind=indexOf(firstCoordinate,cl);
	if (ind<0) return;
	int length=cl->getSize();
	vector<Coordinate> v(length);
	for (int i=ind; i<length; i++) {
		v[i-ind]=cl->getAt(i);
	}
	for (int j=0; j<ind; j++) {
		v[length-ind+j]=cl->getAt(j);
	}
	cl->setPoints(v);
}

void CoordinateList::reverse(CoordinateList *cl){
	int last=cl->getSize()-1;
	int mid=last/2;
	for(int i=0;i<=mid;i++) {
		const Coordinate& tmp=cl->getAt(i);
		cl->setAt(cl->getAt(last-i),i);
		cl->setAt(tmp,last-i);
	}
}

bool CoordinateList::equals(CoordinateList *cl1,CoordinateList *cl2){
	if (cl1==cl2) return true;
	if (cl1==NULL||cl2==NULL) return false;
	if (cl1->getSize()!=cl2->getSize()) return false;
	for (int i = 0; i<cl1->getSize(); i++) {
		if (!(cl1->getAt(i)==cl2->getAt(i))) return false;
	}
	return true;
}

void CoordinateList::add(vector<Coordinate>* vc,bool allowRepeated) {
	for(int i=0;i<(int)vc->size();i++) {
		add((*vc)[i],allowRepeated);
	}
}

void CoordinateList::add(const Coordinate& c,bool allowRepeated) {
	if (!allowRepeated) {
		if (getSize()>=1) {
			const Coordinate& last=getAt(getSize()-1);
			if (last.equals2D(c)) return;
		}
	}
	add(c);
}

/**
* This function allocates space for a CoordinateList object
* being a copy of the input once with consecutive equal points
* removed.
**/
CoordinateList* CoordinateList::removeRepeatedPoints(const CoordinateList *cl){
	CoordinateList* ret=CoordinateListFactory::internalFactory->createCoordinateList();
	vector<Coordinate> *v=cl->toVector();
	ret->add(v,false);
	delete v;
	return ret;
#if 0 // what is all this ?? --strk;
	v=ret->toVector();
	cl->setPoints(*(v));
	delete v;
	delete ret;
//	return ret;
	return cl;
#endif
}
}

