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
 * Revision 1.6  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.5  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.4  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.3  2004/05/27 10:26:50  strk
 * set (useless?) recordIsolated member in constructor
 *
 * Revision 1.2  2004/05/03 12:09:22  strk
 * newline added at end of file
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include <geos/noding.h>

namespace geos {
bool nodingSegmentIntersector::isAdjacentSegments(int i1, int i2){
	return abs(i1-i2)==1;
}



nodingSegmentIntersector::nodingSegmentIntersector(LineIntersector *newLi) {
	li=newLi;
	hasIntersectionVar=false;
	hasProperVar = false;
	hasProperInteriorVar = false;
	hasInteriorVar = false;
	properIntersectionPoint=NULL;
	numIntersections = 0;
	numInteriorIntersections = 0;
	numProperIntersections = 0;
	numTests = 0;
	recordIsolated = false;
}

LineIntersector* nodingSegmentIntersector::getLineIntersector() {
	return li;
}

/**
* @return the proper intersection point, or <code>null</code> if none was found
*/
Coordinate* nodingSegmentIntersector::getProperIntersectionPoint() {
	return properIntersectionPoint;
}

bool nodingSegmentIntersector::hasIntersection() { 
	return hasIntersectionVar;
}
/**
* A proper intersection is an intersection which is interior to at least two
* line segments.  Note that a proper intersection is not necessarily
* in the interior of the entire Geometry, since another edge may have
* an endpoint equal to the intersection, which according to SFS semantics
* can result in the point being on the Boundary of the Geometry.
*/
bool nodingSegmentIntersector::hasProperIntersection() {
	return hasProperVar;
}
/**
* A proper interior intersection is a proper intersection which is <b>not</b>
* contained in the set of boundary nodes set for this nodingSegmentIntersector.
*/
bool nodingSegmentIntersector::hasProperInteriorIntersection() { 
	return hasProperInteriorVar;
}
/**
* An interior intersection is an intersection which is
* in the interior of some segment.
*/
bool nodingSegmentIntersector::hasInteriorIntersection() {
	return hasInteriorVar;
}

/**
* A trivial intersection is an apparent self-intersection which in fact
* is simply the point shared by adjacent line segments.
* Note that closed edges require a special check for the point shared by the beginning
* and end segments.
*/
bool nodingSegmentIntersector::isTrivialIntersection(SegmentString *e0, int segIndex0, SegmentString *e1, int segIndex1){
	if (e0 == e1) {
		if (li->getIntersectionNum() == 1) {
			if (isAdjacentSegments(segIndex0, segIndex1))
				return true;
			if (e0->isClosed()) {
				int maxSegIndex = e0->size() - 1;
				if ((segIndex0 == 0 && segIndex1 == maxSegIndex) ||
					(segIndex1 == 0 && segIndex0 == maxSegIndex)) {
						return true;
				}
			}
		}
	}
	return false;
}

/**
* This method is called by clients
* of the {@link nodingSegmentIntersector} class to process
* intersections for two segments of the {@link SegmentStrings} being intersected.
* Note that some clients (such as {@link MonotoneChain}s) may optimize away
* this call for segment pairs which they have determined do not intersect
* (e.g. by an disjoint envelope test).
*/
void nodingSegmentIntersector::processIntersections(SegmentString *e0, int segIndex0,SegmentString *e1, int segIndex1){
	if (e0==e1 && segIndex0==segIndex1) return;
	numTests++;
	Coordinate p00=e0->getCoordinate(segIndex0);
	Coordinate p01=e0->getCoordinate(segIndex0+1);
	Coordinate p10=e1->getCoordinate(segIndex1);
	Coordinate p11=e1->getCoordinate(segIndex1+1);
	li->computeIntersection(p00, p01, p10, p11);
	//if (li.hasIntersection() && li.isProper()) Debug.println(li);
	if (li->hasIntersection()) {
		if (recordIsolated) {
			e0->setIsolated(false);
			e1->setIsolated(false);
		}
		//intersectionFound = true;
		numIntersections++;
		if (li->isInteriorIntersection()) {
			numInteriorIntersections++;
			hasInteriorVar = true;
			//System.out.println(li);
		}
		// if the segments are adjacent they have at least one trivial intersection,
		// the shared endpoint.  Don't bother adding it if it is the
		// only intersection.
		if (!isTrivialIntersection(e0, segIndex0, e1, segIndex1)) {
			hasIntersectionVar = true;
			//Debug.println(li);
			e0->addIntersections(li, segIndex0, 0);
			e1->addIntersections(li, segIndex1, 1);
			if (li->isProper()) {
				numProperIntersections++;
				//properIntersectionPoint = (Coordinate) li.getIntersection(0).clone();
				hasProperVar = true;
				hasProperInteriorVar = true;
			}
		}
	}
}
}
