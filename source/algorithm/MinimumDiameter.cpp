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
 * Revision 1.7  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.6  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2004/05/05 13:08:01  strk
 * Leaks fixed, explicit allocations/deallocations reduced.
 *
 * Revision 1.4  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.3  2004/04/20 12:47:57  strk
 * MinimumDiameter leaks plugged.
 *
 * Revision 1.2  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.1  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <stdio.h>
#include <typeinfo>

namespace geos {
/**
* Computes the minimum diameter of a {@link Geometry}.
* The minimum diameter is defined to be the
* width of the smallest band that
* contains the geometry,
* where a band is a strip of the plane defined
* by two parallel lines.
* This can be thought of as the smallest hole that the geometry can be
* moved through, with a single rotation.
* <p>
* The first step in the algorithm is computing the convex hull of the Geometry.
* If the input Geometry is known to be convex, a hint can be supplied to
* avoid this computation.
*
* @see ConvexHull
*
* @version 1.4
*/
/**
* Compute a minimum diameter for a giver {@link Geometry}.
*
* @param geom a Geometry
*/
MinimumDiameter::MinimumDiameter(const Geometry* newInputGeom){
	minBaseSeg=new LineSegment();
	minWidthPt=NULL;
	minWidth=0.0;
	inputGeom=newInputGeom;
	isConvex=false;
}

/**
* Compute a minimum diameter for a giver {@link Geometry},
* with a hint if
* the Geometry is convex
* (e.g. a convex Polygon or LinearRing,
* or a two-point LineString, or a Point).
*
* @param geom a Geometry which is convex
* @param isConvex <code>true</code> if the input geometry is convex
*/
MinimumDiameter::MinimumDiameter(const Geometry* newInputGeom, const bool newIsConvex) {
	minBaseSeg=new LineSegment();
	minWidthPt=NULL;
	minWidth=0.0;
	inputGeom=newInputGeom;
	isConvex=newIsConvex;
}

MinimumDiameter::~MinimumDiameter() {
	delete minBaseSeg;
	delete minWidthPt;
}

/**
* Gets the length of the minimum diameter of the input Geometry
*
* @return the length of the minimum diameter
*/
double MinimumDiameter::getLength(){
	computeMinimumDiameter();
	return minWidth;
}

/**
* Gets the {@link Coordinate} forming one end of the minimum diameter
*
* @return a coordinate forming one end of the minimum diameter
*/
Coordinate* MinimumDiameter::getWidthCoordinate() {
	computeMinimumDiameter();
	return minWidthPt;
}

/**
* Gets the segment forming the base of the minimum diameter
*
* @return the segment forming the base of the minimum diameter
*/
LineString* MinimumDiameter::getSupportingSegment() {
	computeMinimumDiameter();
	const GeometryFactory *fact = inputGeom->getFactory();
	CoordinateSequence* cl=fact->getCoordinateSequenceFactory()->create(NULL);
	cl->add(minBaseSeg->p0);
	cl->add(minBaseSeg->p1);
	return fact->createLineString(cl);
}

/**
* Gets a {@link LineString} which is a minimum diameter
*
* @return a {@link LineString} which is a minimum diameter
*/
LineString* MinimumDiameter::getDiameter(){
	computeMinimumDiameter();
	// return empty linestring if no minimum width calculated
	if (minWidthPt==NULL)
		return inputGeom->getFactory()->createLineString(NULL);
	Coordinate* basePt=minBaseSeg->project(*minWidthPt);
	CoordinateSequence* cl=inputGeom->getFactory()->getCoordinateSequenceFactory()->create(NULL);
	cl->add(*basePt);
	cl->add(*minWidthPt);
	delete basePt;
	return inputGeom->getFactory()->createLineString(cl);
}

/* private */
void
MinimumDiameter::computeMinimumDiameter(){
	// check if computation is cached
	if (minWidthPt!=NULL)
		return;
	if (isConvex)
		computeWidthConvex(inputGeom);
	else {
		ConvexHull *ch = new ConvexHull(inputGeom);
		Geometry* convexGeom=ch->getConvexHull();
		computeWidthConvex(convexGeom);
		delete convexGeom;
		delete ch;
	}
}

/* private */
void
MinimumDiameter::computeWidthConvex(const Geometry *geom) {
	//System.out.println("Input = " + geom);
	CoordinateSequence* pts=NULL;
	if (typeid(*geom)==typeid(Polygon))
		pts=((Polygon*)geom)->getExteriorRing()->getCoordinates();
	else
		pts=geom->getCoordinates();

	// special cases for lines or points or degenerate rings
	if (pts->getSize()==0) {
		minWidth=0.0;
		minWidthPt=NULL;
		minBaseSeg=NULL;
	} else if (pts->getSize()==1) {
		minWidth = 0.0;
		minWidthPt=new Coordinate(pts->getAt(0));
		minBaseSeg->p0=pts->getAt(0);
		minBaseSeg->p1=pts->getAt(0);
	} else if (pts->getSize()==2 || pts->getSize()==3) {
		minWidth = 0.0;
		minWidthPt=new Coordinate(pts->getAt(0));
		minBaseSeg->p0=pts->getAt(0);
		minBaseSeg->p1=pts->getAt(1);
	} else
		computeConvexRingMinDiameter(pts);
	delete pts; 
}

/**
* Compute the width information for a ring of {@link Coordinate}s.
* Leaves the width information in the instance variables.
*
* @param pts
* @return
*/
void MinimumDiameter::computeConvexRingMinDiameter(const CoordinateSequence* pts){
	minWidth=DoubleInfinity;
	int currMaxIndex=1;
	LineSegment* seg=new LineSegment();
	// compute the max distance for all segments in the ring, and pick the minimum
	for (int i = 0; i < pts->getSize()-1; i++) {
		seg->p0=pts->getAt(i);
		seg->p1=pts->getAt(i + 1);
		currMaxIndex=findMaxPerpDistance(pts, seg, currMaxIndex);
	}
	delete seg;
}

int
MinimumDiameter::findMaxPerpDistance(const CoordinateSequence *pts, LineSegment* seg, int startIndex)
{
	double maxPerpDistance=seg->distancePerpendicular(pts->getAt(startIndex));
	double nextPerpDistance = maxPerpDistance;
	int maxIndex = startIndex;
	int nextIndex = maxIndex;
	while (nextPerpDistance >= maxPerpDistance) {
		maxPerpDistance = nextPerpDistance;
		maxIndex=nextIndex;
		nextIndex=getNextIndex(pts, maxIndex);
		nextPerpDistance = seg->distancePerpendicular(pts->getAt(nextIndex));
	}
	// found maximum width for this segment - update global min dist if appropriate
	if (maxPerpDistance < minWidth) {
		minPtIndex = maxIndex;
		minWidth = maxPerpDistance;
		delete minWidthPt; 
		minWidthPt = new Coordinate(pts->getAt(minPtIndex));
		delete minBaseSeg;
		minBaseSeg = new LineSegment(*seg);
//      System.out.println(minBaseSeg);
//      System.out.println(minWidth);
	}
	return maxIndex;
}

int MinimumDiameter::getNextIndex(const CoordinateSequence *pts, int index) {
	index++;
	if (index >= pts->getSize()) index = 0;
	return index;
}
}
