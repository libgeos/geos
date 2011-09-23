/**********************************************************************
 * $Id: Octant.cpp 2131 2008-07-15 22:04:51Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/Octant.java rev. 1.2 (JTS-1.7)
 *
 **********************************************************************/

#include <cmath>
#include <sstream>

#include <geos/util/IllegalArgumentException.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*public static*/
int
Octant::octant(double dx, double dy)
{
	if (dx == 0.0 && dy == 0.0)
	{
		std::ostringstream s;
      		s<<"Cannot compute the octant for point ( "<<dx<<", "<<dy<<" )";
      		throw util::IllegalArgumentException(s.str());
	}

    double adx = std::fabs(dx);
    double ady = std::fabs(dy);

    if (dx >= 0) {
      if (dy >= 0) {
        if (adx >= ady)
          return 0;
        else
          return 1;
      }
      else { // dy < 0
        if (adx >= ady)
          return 7;
        else
          return 6;
      }
    }
    else { // dx < 0
      if (dy >= 0) {
        if (adx >= ady)
          return 3;
        else
          return 2;
      }
      else { // dy < 0
        if (adx >= ady)
          return 4;
        else
          return 5;
      }
    }
 
}

/*public static*/
int
Octant::octant(const Coordinate& p0, const Coordinate& p1)
{
	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;

	if (dx == 0.0 && dy == 0.0)
	{
		std::ostringstream s;
      		s<<"Cannot compute the octant for "<<"two identical points "<<p0.toString();
      		throw util::IllegalArgumentException(s.str());
	}

	return octant(dx, dy);
}

} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/20 11:42:28  strk
 * Added missing <cmath> include
 *
 * Revision 1.5  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.4  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.3  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.2  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
