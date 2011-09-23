/**********************************************************************
 * $Id: Quadrant.cpp 2394 2009-04-20 09:37:55Z strk $
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
 * Last port: geomgraph/Quadrant.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#include <sstream>

#include <geos/geomgraph/Quadrant.h>
#include <geos/util/IllegalArgumentException.h>

#include <geos/geom/Coordinate.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/* public static */
int
Quadrant::quadrant(double dx, double dy)
{
	if (dx == 0.0 && dy == 0.0) {
		ostringstream s;
		s<<"Cannot compute the quadrant for point ";
		s<<"("<<dx<<","<<dy<<")"<<endl;
		throw util::IllegalArgumentException(s.str());
	}
	if (dx >= 0) {
		if (dy >= 0)
			return NE;
		else
			return SE;
	} else {
		if (dy >= 0)
			return NW;
		else
			return SW;
	}
}

/* public static */
int
Quadrant::quadrant(const Coordinate& p0, const Coordinate& p1)
{
	if (p1.x == p0.x && p1.y == p0.y)
	{
		throw util::IllegalArgumentException("Cannot compute the quadrant for two identical points " + p0.toString());
	}

	if (p1.x >= p0.x) {
		if (p1.y >= p0.y)
			return NE;
		else
			return SE;
	}
	else {
		if (p1.y >= p0.y)
			return NW;
		else
			return SW;
	}
}

/* public static */
bool
Quadrant::isOpposite(int quad1, int quad2)
{
	if (quad1==quad2) return false;
	int diff=(quad1-quad2+4)%4;
	// if quadrants are not adjacent, they are opposite
	if (diff==2) return true;
	return false;
}

/* public static */
int
Quadrant::commonHalfPlane(int quad1, int quad2)
{
	// if quadrants are the same they do not determine a unique
	// common halfplane.
	// Simply return one of the two possibilities
	if (quad1==quad2) return quad1;
	int diff=(quad1-quad2+4)%4;
	// if quadrants are not adjacent, they do not share a common halfplane
	if (diff==2) return -1;
	//
	int min=(quad1<quad2)? quad1:quad2;
	int max=(quad1>quad2)? quad1:quad2;
	// for this one case, the righthand plane is NOT the minimum index;
	if (min==0 && max==3) return 3;
	// in general, the halfplane index is the minimum of the two
	// adjacent quadrants
	return min;
}

/* public static */
bool
Quadrant::isInHalfPlane(int quad, int halfPlane)
{
	if (halfPlane==SE) {
		return quad==SE || quad==SW;
	}
	return quad==halfPlane || quad==halfPlane+1;
}

/* public static */
bool
Quadrant::isNorthern(int quad)
{
	return quad==NE || quad==NW;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.9  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.8  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.7  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.4  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.3  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

