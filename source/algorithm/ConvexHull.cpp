/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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
 * Last port: algorightm/ConvexHull.java rev. 1.26
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <geos/util.h>
#include <typeinfo>

namespace geos {

/**
 * Create a new convex hull construction for the input {@link Geometry}.
 */
ConvexHull::ConvexHull(const Geometry *newGeometry):
	geometry(newGeometry)
{
}

ConvexHull::~ConvexHull()
{
}


/**
 * Returns a {@link Geometry} that represents the convex hull of the input
 * geometry.
 * The geometry will contain the minimal number of points needed to
 * represent the convex hull.  In particular, no more than two consecutive
 * points will be collinear.
 *
 * @return if the convex hull contains 3 or more points, a Polygon;
 * 2 points, a LineString;
 * 1 point, a Point;
 * 0 points, an empty GeometryCollection.
 */
Geometry*
ConvexHull::getConvexHull()
{
	factory=geometry->getFactory();
	UniqueCoordinateArrayFilter filter;
	geometry->apply_ro(&filter);

	if (filter.getCoordinates()->getSize()==0)
	{
		Geometry *g=factory->createGeometryCollection(NULL);
		return g;
	}

	// Get a reference to filtered coordinates
	const CoordinateSequence *fpts = filter.getCoordinates();

	if (fpts->getSize()==1)
	{
		Geometry *g=factory->createPoint(fpts->clone());
		return g;
	}

	if (fpts->getSize()==2)
	{
		Geometry *g=factory->createLineString(fpts->clone());
		return g;
	}

	// sort points for Graham scan.
	CoordinateSequence *cH;
	if (fpts->getSize()>10)
	{
		//Probably should be somewhere between 50 and 100?
		CoordinateSequence *rpts=reduce(fpts);
		// Use Graham scan to find convex hull.
		cH=grahamScan(preSort(rpts));
		delete rpts; 
	} else {
		// Use Graham scan to find convex hull.
		CoordinateSequence *pts=fpts->clone();
		cH=grahamScan(preSort(pts));
		delete pts;
	}

	// Convert array to linear ring.
	Geometry *g=lineOrPolygon(cH);
	delete cH;
	return g;
}

/**
 * Uses a heuristic to reduce the number of points scanned
 * to compute the hull.
 * The heuristic is to find a quadrilateral guaranteed to
 * be in (or on) the hull, and eliminate all points inside it).
 *
 * @param pts
 * @return a newly allocate CoordinateSequence
 */
CoordinateSequence*
ConvexHull::reduce(const CoordinateSequence *pts)
{
	BigQuad bigQuad;
	makeBigQuad(pts, bigQuad);

	// Build a linear ring defining a big poly.
	CoordinateSequence *bigPoly=factory->getCoordinateSequenceFactory()->create(NULL);

	bigPoly->add(bigQuad.westmost);
	if (CoordinateSequence::indexOf(&(bigQuad.northmost),bigPoly)==-1) {
		bigPoly->add(bigQuad.northmost);
	}
	if (CoordinateSequence::indexOf(&(bigQuad.eastmost),bigPoly)==-1) {
		bigPoly->add(bigQuad.eastmost);
	}
	if (CoordinateSequence::indexOf(&(bigQuad.southmost),bigPoly)==-1) {
		bigPoly->add(bigQuad.southmost);
	}
	if (bigPoly->getSize()<3) {
		delete bigPoly;
		return pts->clone();
	}
	bigPoly->add(bigQuad.westmost);
	LinearRing *bQ=factory->createLinearRing(*bigPoly);
	// load an array with all points not in the big poly
	// and the defining points.

//!!!Note to self: this might not work properly because of sorting.

	CoordinateSequence *cl=bigPoly;
	unsigned int npts=pts->getSize();
	for(unsigned int i=0; i<npts; ++i)
	{
		if (pointLocator.locate(pts->getAt(i),bQ)==Location::EXTERIOR) {
			cl->add(pts->getAt(i));
		}
	}
	// Return this array as the reduced problem.
	delete bQ;
	return cl;
}

/*
 * Returns a pointer to input, modifying input
 */
CoordinateSequence*
ConvexHull::preSort(CoordinateSequence *pts)
{
	Coordinate t;

	// find the lowest point in the set. If two or more points have
	// the same minimum y coordinate choose the one with the minimu x.
	// This focal point is put in array location pts[0].
	unsigned int npts=pts->getSize();
	for(unsigned int i=1; i<npts; ++i)
	{
		const Coordinate &p0=pts->getAt(0); // this will change
		const Coordinate &pi=pts->getAt(i);
		if ( (pi.y<p0.y) || ((pi.y==p0.y) && (pi.x<p0.x)) )
		{
			t=p0;
			pts->setAt(pi, 0);
			pts->setAt( t, i);
		}
	}
	// sort the points radially around the focal point.
	radialSort(pts);
	return pts;
}

/*
 * returns a newly allocated CoordinateSequence object
 */
CoordinateSequence*
ConvexHull::grahamScan(const CoordinateSequence *c)
{
	const Coordinate *p;

	vector<Coordinate> *ps=new vector<Coordinate>();
	ps->push_back(c->getAt(0));
	ps->push_back(c->getAt(1));
	ps->push_back(c->getAt(2));

	p=&(c->getAt(2));
	unsigned int npts=c->getSize();
	for(unsigned int i=3; i<npts; ++i)
	{
		p=&(ps->back());
		ps->pop_back();
		while (CGAlgorithms::computeOrientation(ps->back(), *p, c->getAt(i)) > 0)
		{
			p=&(ps->back());
			ps->pop_back();
		}
		ps->push_back(*p);
		ps->push_back(c->getAt(i));
	}
	ps->push_back(c->getAt(0));

	return factory->getCoordinateSequenceFactory()->create(ps);
}

void
ConvexHull::radialSort(CoordinateSequence *p)
{
	// A selection sort routine, assumes the pivot point is
	// the first point (i.e., p[0]).

	const Coordinate &p0=p->getAt(0); // the pivot point

	Coordinate t;
	unsigned int npts=p->getSize();
	for(unsigned int i=1; i<npts-1; ++i)
	{
		unsigned int min=i;

		for(unsigned int j=i+1; j<npts; ++j)
		{
			const Coordinate &pj=p->getAt(j);

			if ( polarCompare(p0, pj, p->getAt(min)) < 0 )
			{
				min=j;
			}
		}

		/*
		 * Swap point[i] and point[min]
		 * We can skip this if they have
		 * the same value
		 */
		if ( i != min )
		{
			t=p->getAt(i);
			p->setAt(p->getAt(min), i);
			p->setAt(t, min);
		}
	}
}

int
ConvexHull::polarCompare(const Coordinate &o, const Coordinate &p, const Coordinate &q)
{
	// Given two points p and q compare them with respect to their radial
	// ordering about point o. -1, 0 or 1 depending on whether p is less than,
	// equal to or greater than q. First checks radial ordering then if both
	// points lie on the same line, check distance to o.
	double dxp=p.x-o.x;
	double dyp=p.y-o.y;
	double dxq=q.x-o.x;
	double dyq=q.y-o.y;
	double alph=atan2(dxp, dyp);
	double beta=atan2(dxq, dyq);
	if (alph<beta) {
		return -1;
	}
	if (alph>beta) {
		return 1;
	}
	double op=dxp*dxp+dyp*dyp;
	double oq=dxq*dxq+dyq*dyq;
	if (op<oq) {
		return -1;
	}
	if (op>oq) {
		return 1;
	}
	return 0;
}

/**
 * @return  whether the three coordinates are collinear and c2 lies between
 *          c1 and c3 inclusive
 */
bool
ConvexHull::isBetween(const Coordinate &c1, const Coordinate &c2, const Coordinate &c3)
{
	if (CGAlgorithms::computeOrientation(c1, c2, c3)!=0) {
		return false;
	}
	if (c1.x!=c3.x) {
		if (c1.x<=c2.x && c2.x<=c3.x) {
			return true;
		}
		if (c3.x<=c2.x && c2.x<=c1.x) {
			return true;
		}
	}
	if (c1.y!=c3.y) {
		if (c1.y<=c2.y && c2.y<=c3.y) {
			return true;
		}
		if (c3.y<=c2.y && c2.y<=c1.y) {
			return true;
		}
	}
	return false;
}

void
ConvexHull::makeBigQuad(const CoordinateSequence *pts, BigQuad &bigQuad)
{
	bigQuad.northmost=bigQuad.southmost=
		bigQuad.westmost=bigQuad.eastmost=pts->getAt(0);

	unsigned int npts=pts->getSize();
	for (unsigned int i=1; i<npts; ++i)
	{
		const Coordinate &pi=pts->getAt(i);

		if (pi.x<bigQuad.westmost.x)
			bigQuad.westmost=pi;

		if (pi.x>bigQuad.eastmost.x)
			bigQuad.eastmost=pi;

		if (pi.y<bigQuad.southmost.y)
			bigQuad.southmost=pi;

		if (pi.y>bigQuad.northmost.y)
			bigQuad.northmost=pi;
	}
}

/**
 * @param  vertices  the vertices of a linear ring, which may or may not be
 *      flattened (i.e. vertices collinear)
 * @return a 2-vertex <code>LineString</code> if the vertices are
 *      collinear;otherwise, a <code>Polygon</code> with unnecessary
 *      (collinear) vertices removed
 */
Geometry*
ConvexHull::lineOrPolygon(const CoordinateSequence *newCoordinates)
{
	CoordinateSequence *coordinates=cleanRing(newCoordinates);
	if (coordinates->getSize()==3) { // shouldn't this be 2 ??
		CoordinateSequence *cl1=factory->getCoordinateSequenceFactory()->create(NULL);
		cl1->add(coordinates->getAt(0));
		cl1->add(coordinates->getAt(1));
		delete coordinates;
		LineString *ret =factory->createLineString(cl1);
		return ret;
	}
	LinearRing *linearRing=geometry->getFactory()->createLinearRing(coordinates);
	return factory->createPolygon(linearRing,NULL);
}

/**
 * @param  vertices  the vertices of a linear ring, which may or may not be
 *      flattened (i.e. vertices collinear)
 * @return a newly allocated CoordinateSequence
 */
CoordinateSequence*
ConvexHull::cleanRing(const CoordinateSequence *original)
{
	Assert::equals(original->getAt(0),original->getAt(original->getSize()-1));

	unsigned int npts=original->getSize();

	vector<Coordinate> *newPts=new vector<Coordinate>;
	newPts->reserve(npts);

	const Coordinate *previousDistinctCoordinate=NULL;
	for(unsigned int i=0; i<npts-1; ++i)
	{
		const Coordinate &currentCoordinate=original->getAt(i);
		const Coordinate &nextCoordinate=original->getAt(i+1);

		// skip repeated points (shouldn't this have been already done elsewhere?)
		if (currentCoordinate==nextCoordinate) continue;

		// skip collinear point
		if (previousDistinctCoordinate!=NULL && 
			isBetween(*previousDistinctCoordinate, currentCoordinate, nextCoordinate))
		{
			continue;
		}

		newPts->push_back(currentCoordinate);

		previousDistinctCoordinate=&currentCoordinate;
	}

	newPts->push_back(original->getAt(npts-1));

	CoordinateSequence *cleanedRing=factory->getCoordinateSequenceFactory()->create(newPts);
	return cleanedRing;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2005/12/14 02:32:41  strk
 * New entry
 *
 * Revision 1.14  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.13  2005/11/21 16:03:20  strk
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
 * Revision 1.12  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.11  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.9  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.8  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.6  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 *
 **********************************************************************/

