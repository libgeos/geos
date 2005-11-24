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
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {

CentroidArea::CentroidArea():
	areasum2(0)
{
}

CentroidArea::~CentroidArea()
{
}

/**
 * Adds the area defined by a Geometry to the centroid total.
 * If the geometry has no area it does not contribute to the centroid.
 *
 * @param geom the geometry to add
 */
void
CentroidArea::add(const Geometry *geom)
{
	if(const Polygon *poly=dynamic_cast<const Polygon*>(geom)) {
		setBasePoint(poly->getExteriorRing()->getCoordinateN(0));
		add(poly);
	}
	else if(const GeometryCollection *gc=dynamic_cast<const GeometryCollection*>(geom)) 
	{
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

/**
 * Adds the area defined by an array of
 * coordinates.  The array must be a ring;
 * i.e. end with the same coordinate as it starts with.
 * @param ring an array of {@link Coordinate}s
 */
void
CentroidArea::add(const CoordinateSequence *ring)
{
	setBasePoint(ring->getAt(0));
	addShell(ring);
}

Coordinate*
CentroidArea::getCentroid() const
{
	Coordinate *cent = new Coordinate();
	cent->x = cg3.x/3.0/areasum2;
	cent->y = cg3.y/3.0/areasum2;
	return cent;
}

void
CentroidArea::setBasePoint(const Coordinate &newbasePt)
{
	basePt=newbasePt;
}

void
CentroidArea::add(const Polygon *poly)
{
	addShell(poly->getExteriorRing()->getCoordinatesRO());
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		addHole(poly->getInteriorRingN(i)->getCoordinatesRO());
	}
}

void
CentroidArea::addShell(const CoordinateSequence *pts)
{
	bool isPositiveArea=!CGAlgorithms::isCCW(pts);
	unsigned int n=pts->getSize()-1;
	for(unsigned int i=0; i<n; ++i)
	{
		addTriangle(basePt, pts->getAt(i), pts->getAt(i+1), isPositiveArea);
	}
}

void
CentroidArea::addHole(const CoordinateSequence *pts)
{
	bool isPositiveArea=CGAlgorithms::isCCW(pts);
	unsigned int n=pts->getSize()-1;
	for(unsigned int i=0; i<n; ++i)
	{
		addTriangle(basePt, pts->getAt(i), pts->getAt(i+1), isPositiveArea);
	}
}

void
CentroidArea::addTriangle(const Coordinate &p0, const Coordinate &p1,
		const Coordinate &p2, bool isPositiveArea)
{
	double sign=(isPositiveArea)?1.0:-1.0;
	centroid3(p0,p1,p2,triangleCent3);
	double area2res=area2(p0,p1,p2);
	cg3.x+=sign*area2res*triangleCent3.x;
	cg3.y+=sign*area2res*triangleCent3.y;
	areasum2+=sign*area2res;
}

/**
 * Returns three times the centroid of the triangle p1-p2-p3.
 * The factor of 3 is
 * left in to permit division to be avoided until later.
 */
void
CentroidArea::centroid3(const Coordinate &p1, const Coordinate &p2,
		const Coordinate &p3, Coordinate &c)
{
	c.x=p1.x+p2.x+p3.x;
	c.y=p1.y+p2.y+p3.y;
}

/**
 * Returns twice the signed area of the triangle p1-p2-p3,
 * positive if a,b,c are oriented ccw, and negative if cw.
 */
double
CentroidArea::area2(const Coordinate &p1, const Coordinate &p2, const Coordinate &p3)
{
	return (p2.x-p1.x)*(p3.y-p1.y)-(p3.x-p1.x)*(p2.y-p1.y);
}

} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.16  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.15  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.14  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.13  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.12  2004/05/17 07:42:54  strk
 * CentroidArea::add(const Geometry *geom) uses dynamic_cast
 *
 * Revision 1.11  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.9  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 * Revision 1.8  2003/10/13 15:39:03  strk
 * Fixed some leak or fault flips (forced copy of a single coordinate)
 *
 * Revision 1.7  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 **********************************************************************/


