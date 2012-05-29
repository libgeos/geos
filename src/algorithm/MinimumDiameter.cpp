/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * 
 * TODO:
 * 	- avoid heap allocation for LineSegment and Coordinate
 *
 **********************************************************************/

#include <geos/algorithm/MinimumDiameter.h>
#include <geos/algorithm/ConvexHull.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>

#include <typeinfo>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/**
 * Computes the minimum diameter of a Geometry.
 * The minimum diameter is defined to be the
 * width of the smallest band that
 * contains the geometry,
 * where a band is a strip of the plane defined
 * by two parallel lines.
 * This can be thought of as the smallest hole that the geometry can be
 * moved through, with a single rotation.
 * 
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
MinimumDiameter::MinimumDiameter(const Geometry* newInputGeom)
{
	minBaseSeg=new LineSegment();
	minWidthPt=NULL;
	minPtIndex=0;
	minWidth=0.0;
	inputGeom=newInputGeom;
	isConvex=false;
}

/**
 * Compute a minimum diameter for a giver Geometry,
 * with a hint if
 * the Geometry is convex
 * (e.g. a convex Polygon or LinearRing,
 * or a two-point LineString, or a Point).
 *
 * @param geom a Geometry which is convex
 * @param isConvex <code>true</code> if the input geometry is convex
 */
MinimumDiameter::MinimumDiameter(const Geometry* newInputGeom, const bool newIsConvex)
{
	minBaseSeg=new LineSegment();
	minWidthPt=NULL;
	minWidth=0.0;
	inputGeom=newInputGeom;
	isConvex=newIsConvex;
}

MinimumDiameter::~MinimumDiameter()
{
	delete minBaseSeg;
	delete minWidthPt;
}

/**
 * Gets the length of the minimum diameter of the input Geometry
 *
 * @return the length of the minimum diameter
 */
double
MinimumDiameter::getLength()
{
	computeMinimumDiameter();
	return minWidth;
}

/**
 * Gets the {@link Coordinate} forming one end of the minimum diameter
 *
 * @return a coordinate forming one end of the minimum diameter
 */
Coordinate*
MinimumDiameter::getWidthCoordinate()
{
	computeMinimumDiameter();
	return minWidthPt;
}

/**
 * Gets the segment forming the base of the minimum diameter
 *
 * @return the segment forming the base of the minimum diameter
 */
LineString*
MinimumDiameter::getSupportingSegment() {
	computeMinimumDiameter();
	const GeometryFactory *fact = inputGeom->getFactory();
	CoordinateSequence* cl=fact->getCoordinateSequenceFactory()->create(NULL);
	cl->add(minBaseSeg->p0);
	cl->add(minBaseSeg->p1);
	return fact->createLineString(cl);
}

/**
 * Gets a LineString which is a minimum diameter
 *
 * @return a LineString which is a minimum diameter
 */
LineString*
MinimumDiameter::getDiameter()
{
	computeMinimumDiameter();
	// return empty linestring if no minimum width calculated
	if (minWidthPt==NULL)
		return inputGeom->getFactory()->createLineString(NULL);

	Coordinate basePt;
	minBaseSeg->project(*minWidthPt, basePt);

	CoordinateSequence* cl=inputGeom->getFactory()->getCoordinateSequenceFactory()->create(NULL);
	cl->add(basePt);
	cl->add(*minWidthPt);
	return inputGeom->getFactory()->createLineString(cl);
}

/* private */
void
MinimumDiameter::computeMinimumDiameter()
{
	// check if computation is cached
	if (minWidthPt!=NULL)
		return;
	if (isConvex)
		computeWidthConvex(inputGeom);
	else {
		ConvexHull ch(inputGeom);
		Geometry* convexGeom=ch.getConvexHull();
		computeWidthConvex(convexGeom);
		delete convexGeom;
	}
}

/* private */
void
MinimumDiameter::computeWidthConvex(const Geometry *geom)
{
	//System.out.println("Input = " + geom);
	CoordinateSequence* pts=NULL;
	if (typeid(*geom)==typeid(Polygon))
	{
		const Polygon* p = dynamic_cast<const Polygon*>(geom);
		pts=p->getExteriorRing()->getCoordinates();
	}
	else
	{
		pts=geom->getCoordinates();
	}

	// special cases for lines or points or degenerate rings
	switch(pts->getSize())
	{
		case 0:
			minWidth=0.0;
			minWidthPt=NULL;
			minBaseSeg=NULL;
			break;
		case 1:
			minWidth = 0.0;
			minWidthPt=new Coordinate(pts->getAt(0));
			minBaseSeg->p0=pts->getAt(0);
			minBaseSeg->p1=pts->getAt(0);
			break;
		case 2:
		case 3:
			minWidth = 0.0;
			minWidthPt=new Coordinate(pts->getAt(0));
			minBaseSeg->p0=pts->getAt(0);
			minBaseSeg->p1=pts->getAt(1);
			break;
		default:
			computeConvexRingMinDiameter(pts);
	}
	delete pts; 
}

/**
 * Compute the width information for a ring of {@link Coordinate}s.
 * Leaves the width information in the instance variables.
 *
 * @param pts
 * @return
 */
void
MinimumDiameter::computeConvexRingMinDiameter(const CoordinateSequence* pts)
{
	minWidth=DoubleMax;
	unsigned int currMaxIndex=1;
	LineSegment seg;

	// compute the max distance for all segments in the ring, and pick the minimum
	const std::size_t npts=pts->getSize();
	for (std::size_t i=1; i<npts; ++i) {
		seg.p0=pts->getAt(i-1);
		seg.p1=pts->getAt(i);
		currMaxIndex=findMaxPerpDistance(pts, &seg, currMaxIndex);
	}
}

unsigned int
MinimumDiameter::findMaxPerpDistance(const CoordinateSequence *pts,
		LineSegment* seg, unsigned int startIndex)
{
	double maxPerpDistance=seg->distancePerpendicular(pts->getAt(startIndex));
	double nextPerpDistance = maxPerpDistance;
	unsigned int maxIndex = startIndex;
	unsigned int nextIndex = maxIndex;
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

unsigned int
MinimumDiameter::getNextIndex(const CoordinateSequence *pts,
	unsigned int index)
{
	if (++index >= pts->getSize()) index = 0;
	return index;
}

} // namespace geos.algorithm
} // namespace geos

