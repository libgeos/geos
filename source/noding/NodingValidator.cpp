/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include "../headers/noding.h"

namespace geos {

NodingValidator::NodingValidator(vector<SegmentString*> *newSegStrings){
	segStrings=newSegStrings;
	li=new RobustLineIntersector();
}

NodingValidator::~NodingValidator(){
	delete li;
}

void NodingValidator::checkValid(){
	checkNoInteriorPointsSame();
	checkProperIntersections();
}


void NodingValidator::checkProperIntersections() {
	for (int i=0; i<(int)segStrings->size();i++) {
		SegmentString *ss0=(*segStrings)[i];
		for (int j=0; j<(int)segStrings->size();j++) {
			SegmentString *ss1=(*segStrings)[j];
			checkProperIntersections(ss0, ss1);
		}
	}
}

void NodingValidator::checkProperIntersections(SegmentString *ss0, SegmentString *ss1){
	const CoordinateList *pts0=ss0->getCoordinates();
	const CoordinateList *pts1=ss1->getCoordinates();
	for (int i0=0;i0<pts0->getSize()-1; i0++) {
		for (int i1=0;i1<pts1->getSize()-1; i1++) {
			checkProperIntersections(ss0, i0, ss1, i1);
		}
	}
}

void NodingValidator::checkProperIntersections(SegmentString *e0, int segIndex0, SegmentString *e1, int segIndex1){
	if (e0 == e1 && segIndex0 == segIndex1) return;
	//numTests++;
	Coordinate p00=e0->getCoordinates()->getAt(segIndex0);
	Coordinate p01=e0->getCoordinates()->getAt(segIndex0+1);
	Coordinate p10=e1->getCoordinates()->getAt(segIndex1);
	Coordinate p11=e1->getCoordinates()->getAt(segIndex1+1);
	li->computeIntersection(p00, p01, p10, p11);
	if (li->hasIntersection()) {
		if (   li->isProper()
			|| hasInteriorIntersection(li, p00, p01)
			|| hasInteriorIntersection(li, p00, p01)) {
				throw new GEOSException("found non-noded intersection at "+ p00.toString() + "-" + p01.toString()+ " and "+ p10.toString() + "-" + p11.toString());
		}
	}
}

/**
*@return true if there is an intersection point which is not an endpoint of the segment p0-p1
*/
bool NodingValidator::hasInteriorIntersection(LineIntersector *aLi, Coordinate& p0, Coordinate& p1){
	for (int i = 0; i < aLi->getIntersectionNum(); i++) {
		Coordinate intPt=aLi->getIntersection(i);
		if (!(intPt==p0 || intPt==p1))
			return true;
	}
	return false;
}

void NodingValidator::checkNoInteriorPointsSame() {
	for (int i=0; i<(int)segStrings->size();i++) {
		SegmentString *ss0=(*segStrings)[i];
		const CoordinateList *pts=ss0->getCoordinates();
		checkNoInteriorPointsSame(pts->getAt(0), segStrings);
		checkNoInteriorPointsSame(pts->getAt(pts->getSize()-1), segStrings);
	}
}

void NodingValidator::checkNoInteriorPointsSame(const Coordinate& testPt,vector<SegmentString*> *aSegStrings) {
	for (int i=0; i<(int)segStrings->size();i++) {
			SegmentString *ss0=(*segStrings)[i];
			const CoordinateList *pts=ss0->getCoordinates();
			for (int j=1;j<pts->getSize()-1; j++) {
				if (pts->getAt(j)==testPt)
					throw new GEOSException("found bad noding at pt " + testPt.toString());
			}
	}
}

}
