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
 * Represents a homogeneous coordinate in a 2-D coordinate space.
 * HCoordinate are used as a clean way
 * of computing intersections between line segments.
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>

namespace geos {

/*
 * Computes the (approximate) intersection point between two line segments
 * using homogeneous coordinates.
 * 
 * Note that this algorithm is
 * not numerically stable; i.e. it can produce intersection points which
 * lie outside the envelope of the line segments themselves.  In order
 * to increase the precision of the calculation input points should be
 * normalized before passing them to this routine.
 */
void
HCoordinate::intersection(const Coordinate &p1, const Coordinate &p2,
	const Coordinate &q1, const Coordinate &q2, Coordinate &ret)
{
#if 1
	auto_ptr<HCoordinate> intHCoord(new HCoordinate(
		HCoordinate(HCoordinate(p1),HCoordinate(p2)),
		HCoordinate(HCoordinate(q1),HCoordinate(q2))
	));
#else
	HCoordinate intHCoord(
		HCoordinate(HCoordinate(p1),HCoordinate(p2)),
		HCoordinate(HCoordinate(q1),HCoordinate(q2))
	);
#endif

	intHCoord->getCoordinate(ret);
}

HCoordinate::HCoordinate()
{
	x = 0.0;
	y = 0.0;
	w = 1.0;
}

HCoordinate::HCoordinate(double _x, double _y, double _w)
{
	x = _x;
	y = _y;
	w = _w;
}

HCoordinate::HCoordinate(const Coordinate& p)
{
	x = p.x;
	y = p.y;
	w = 1.0;
}

HCoordinate::HCoordinate(const HCoordinate &p1, const HCoordinate &p2)
{
	x = p1.y*p2.w - p2.y*p1.w;
	y = p2.x*p1.w - p1.x*p2.w;
	w = p1.x*p2.y - p2.x*p1.y;
}

double
HCoordinate::getX() const
{
	double a = x/w;
	if (!FINITE(a))
	{
		throw  NotRepresentableException();
	}
	return a;
}

double
HCoordinate::getY() const
{
	double a = y/w;
	if (!FINITE(a))
	{
		throw  NotRepresentableException();
	}
	return a;
}

void
HCoordinate::getCoordinate(Coordinate &ret) const
{
	ret=Coordinate(getX(),getY());
	//return new Coordinate(getX(),getY());
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.14  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
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
 * Revision 1.12  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.11  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.10  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

