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
 * Revision 1.4  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.3  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.2  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include "../headers/noding.h"

namespace geos {

SegmentString::SegmentString(const CoordinateList *newPts, const void* newContext)
{
	eiList=new SegmentNodeList(this);
	isIsolatedVar=false;
	pts=newPts;
	context=newContext;
}

SegmentString::~SegmentString() {
	delete eiList;
}

const void*
SegmentString::getContext() const
{
	return context;
}

SegmentNodeList*
SegmentString::getIntersectionList() const
{
	return eiList;
}

int
SegmentString::size() const 
{
	return pts->getSize();
}

const Coordinate&
SegmentString::getCoordinate(int i) const
{
	return pts->getAt(i);
}

const CoordinateList*
SegmentString::getCoordinates() const
{
	return pts;
}

void SegmentString::setIsolated(bool isIsolated)  {
	isIsolatedVar=isIsolated;
}

bool
SegmentString::isIsolated() const
{
	return isIsolatedVar;
}


bool
SegmentString::isClosed() const
{
	return pts->getAt(0)==pts->getAt(pts->getSize()-1);
}

/**
* Adds EdgeIntersections for one or both
* intersections found for a segment of an edge to the edge intersection list.
*/
void
SegmentString::addIntersections(LineIntersector *li, int segmentIndex, int geomIndex)
{
	for (int i=0; i<li->getIntersectionNum(); i++) {
		addIntersection(li,segmentIndex, geomIndex, i);
	}
}

/**
* Add an SegmentNode for intersection intIndex.
* An intersection that falls exactly on a vertex
* of the SegmentString is normalized
* to use the higher of the two possible segmentIndexes
*/
void
SegmentString::addIntersection(LineIntersector *li, int segmentIndex, int geomIndex, int intIndex)
{
	Coordinate* intPt=new Coordinate(li->getIntersection(intIndex));
	double dist=li->getEdgeDistance(geomIndex, intIndex);
	addIntersection(*intPt, segmentIndex, dist);
}

void SegmentString::OLDaddIntersection(LineIntersector *li, int segmentIndex, int geomIndex, int intIndex){
	Coordinate *intPt=new Coordinate(li->getIntersection(intIndex));
	int normalizedSegmentIndex = segmentIndex;
	double dist=li->getEdgeDistance(geomIndex, intIndex);
	//Debug.println("edge intpt: " + intPt + " dist: " + dist);
	// normalize the intersection point location
	int nextSegIndex = normalizedSegmentIndex + 1;
	if (nextSegIndex < pts->getSize()) {
		Coordinate nextPt=pts->getAt(nextSegIndex);
		//Debug.println("next pt: " + nextPt);
		// Normalize segment index if intPt falls on vertex
		// The check for point equality is 2D only - Z values are ignored
		if (intPt->equals2D(nextPt)) {
			//Debug.println("normalized distance");
			normalizedSegmentIndex = nextSegIndex;
			dist = 0.0;
		}
	}
	/**
	* Add the intersection point to edge intersection list.
	*/
	SegmentNode *ei=eiList->add(intPt,normalizedSegmentIndex,dist);
	//ei.print(System.out);
}
/**
* Add an EdgeIntersection for intersection intIndex.
* An intersection that falls exactly on a vertex of the edge is normalized
* to use the higher of the two possible segmentIndexes
*/
void
SegmentString::addIntersection(Coordinate& intPt, int segmentIndex)
{
	double dist=LineIntersector::computeEdgeDistance(intPt,pts->getAt(segmentIndex),pts->getAt(segmentIndex + 1));
	addIntersection(intPt, segmentIndex, dist);
}

void
SegmentString::addIntersection(Coordinate& intPt, int segmentIndex, double dist)
{
	int normalizedSegmentIndex = segmentIndex;
	//Debug.println("edge intpt: " + intPt + " dist: " + dist);
	// normalize the intersection point location
	int nextSegIndex = normalizedSegmentIndex + 1;
	if (nextSegIndex < pts->getSize()) {
		Coordinate nextPt = pts->getAt(nextSegIndex);
		//Debug.println("next pt: " + nextPt);

		// Normalize segment index if intPt falls on vertex
		// The check for point equality is 2D only - Z values are ignored
		if (intPt.equals2D(nextPt)) {
			//Debug.println("normalized distance");
			normalizedSegmentIndex = nextSegIndex;
			dist = 0.0;
		}
	}
	/**
	* Add the intersection point to edge intersection list.
	*/
	SegmentNode *ei=eiList->add(&intPt, normalizedSegmentIndex, dist);
	//ei.print(System.out);

}

}

