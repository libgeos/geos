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
 **********************************************************************/

#include <geos/noding.h>

namespace geos {

/**
 * This function copies given CoordinateSequence
 */
SegmentString::SegmentString(const CoordinateSequence *newPts, const void* newContext)
{
	eiList=new SegmentNodeList(this);
	isIsolatedVar=false;
	pts=newPts;
	context=newContext;
}

SegmentString::~SegmentString() {
	delete eiList;
	//delete pts;
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

CoordinateSequence*
SegmentString::getCoordinates() const
{
	return pts->clone();
}

const CoordinateSequence*
SegmentString::getCoordinatesRO() const
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
	const Coordinate &intPt=li->getIntersection(intIndex);
	double dist=li->getEdgeDistance(geomIndex, intIndex);
	addIntersection((Coordinate&)intPt, segmentIndex, dist);
	//delete intPt;
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
SegmentString::addIntersection(Coordinate& intPt,
	int segmentIndex, double dist)
{
	int normalizedSegmentIndex = segmentIndex;
	//Debug.println("edge intpt: " + intPt + " dist: " + dist);
	// normalize the intersection point location
	int nextSegIndex = normalizedSegmentIndex + 1;
	if (nextSegIndex < pts->getSize()) {
		const Coordinate &nextPt = pts->getAt(nextSegIndex);
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
	//SegmentNode *ei=
	eiList->add(&intPt, normalizedSegmentIndex, dist);
	//ei.print(System.out);

}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.10  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.9  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.6  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.5  2004/05/27 10:27:03  strk
 * Memory leaks fixed.
 *
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

