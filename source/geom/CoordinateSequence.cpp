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
 * Revision 1.2  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/


#include <geos/geom.h>
#include <stdio.h>

namespace geos {

bool CoordinateSequence::hasRepeatedPoints() const {
	int size=getSize();
	for(int i=1; i<size; i++) {
		if (getAt(i-1)==getAt(i)) {
			return true;
		}
	}
	return false;
}

/*
 * Returns either the given coordinate array if its length is greater than the
 * given amount, or an empty coordinate array.
 */
CoordinateSequence *
CoordinateSequence::atLeastNCoordinatesOrNothing(int n, CoordinateSequence *c)
{
	return c->getSize()>=n?c:DefaultCoordinateSequenceFactory::instance()->create(NULL);
}      


bool
CoordinateSequence::hasRepeatedPoints(const CoordinateSequence *cl) {
	int size=(int) cl->getSize();
	for(int i=1;i<size; i++) {
		if (cl->getAt(i-1)==cl->getAt(i)) {
			return true;
		}
	}
	return false;
}

const Coordinate* CoordinateSequence::minCoordinate() const {
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
CoordinateSequence::minCoordinate(CoordinateSequence *cl)
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
CoordinateSequence::indexOf(const Coordinate *coordinate, const CoordinateSequence *cl)
{
	for (int i=0; i<cl->getSize(); i++) {
		if ((*coordinate)==cl->getAt(i)) {
			return i;
		}
	}
	return -1;
}

void
CoordinateSequence::scroll(CoordinateSequence* cl,const Coordinate* firstCoordinate)
{
	int i, j=0;
	int ind=indexOf(firstCoordinate,cl);
	if (ind<1) return; // not found or already first
	int length=cl->getSize();
	vector<Coordinate> v(length);
	for (i=ind; i<length; i++) {
		v[j++]=cl->getAt(i);
	}
	for (i=0; i<ind; i++) {
		v[j++]=cl->getAt(i);
	}
	cl->setPoints(v);
}

void CoordinateSequence::reverse(CoordinateSequence *cl){
	int last=cl->getSize()-1;
	int mid=last/2;
	for(int i=0;i<=mid;i++) {
		const Coordinate& tmp=cl->getAt(i);
		cl->setAt(cl->getAt(last-i),i);
		cl->setAt(tmp,last-i);
	}
}

bool CoordinateSequence::equals(CoordinateSequence *cl1,CoordinateSequence *cl2){
	if (cl1==cl2) return true;
	if (cl1==NULL||cl2==NULL) return false;
	if (cl1->getSize()!=cl2->getSize()) return false;
	for (int i = 0; i<cl1->getSize(); i++) {
		if (!(cl1->getAt(i)==cl2->getAt(i))) return false;
	}
	return true;
}

/** Add an array of coordinates
* @param vc The coordinates
* @param allowRepeated if set to false, repeated coordinates are collapsed
* @return true (as by general collection contract)
*/
void CoordinateSequence::add(const vector<Coordinate>* vc,bool allowRepeated) {
	for(int i=0;i<(int)vc->size();i++) {
		add((*vc)[i],allowRepeated);
	}
}

/** Add a coordinate
* @param c The coordinate to add
* @param allowRepeated if set to false, repeated coordinates are collapsed
* @return true (as by general collection contract)
*/
void CoordinateSequence::add(const Coordinate& c,bool allowRepeated) {
	if (!allowRepeated) {
		if (getSize()>=1) {
			const Coordinate& last=getAt(getSize()-1);
			if (last.equals2D(c)) return;
		}
	}
	add(c);
}

/** Add an array of coordinates
* @param cl The coordinates
* @param allowRepeated if set to false, repeated coordinates are collapsed
* @param direction if false, the array is added in reverse order
* @return true (as by general collection contract)
*/
void CoordinateSequence::add(CoordinateSequence *cl,bool allowRepeated,bool direction){
	if (direction) {
		for (int i = 0; i < cl->getSize(); i++) {
			add(cl->getAt(i), allowRepeated);
		}
	} else {
		for (int j =cl->getSize()-1;j>=0;j--) {
			add(cl->getAt(j), allowRepeated);
		}
	}
}


/**
* This function allocates space for a CoordinateSequence object
* being a copy of the input once with consecutive equal points
* removed.
**/
CoordinateSequence* CoordinateSequence::removeRepeatedPoints(const CoordinateSequence *cl){
	CoordinateSequence* ret=DefaultCoordinateSequenceFactory::instance()->create(NULL);
	const vector<Coordinate> *v=cl->toVector();
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

