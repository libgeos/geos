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
 * Last port: algorithm/HCoordinate.java rev. 1.17 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/algorithm/HCoordinate.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/geom/Coordinate.h>
#include <geos/platform.h>

#include <memory>
#include <cmath> // for finite()

using namespace std;
using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/*public static*/
void
HCoordinate::intersection(const Coordinate &p1, const Coordinate &p2,
	const Coordinate &q1, const Coordinate &q2, Coordinate &ret)
{
        HCoordinate hc1p1(p1);
        HCoordinate hc1p2(p2);
        HCoordinate l1(hc1p1, hc1p2);

        HCoordinate hc2q1(q1);
        HCoordinate hc2q2(q2);
        HCoordinate l2(hc2q1, hc2q2);

        HCoordinate intHCoord(l1, l2);

        intHCoord.getCoordinate(ret);

}

/*public*/
HCoordinate::HCoordinate()
	:
	x(0.0),
	y(0.0),
	w(1.0)
{
}

/*public*/
HCoordinate::HCoordinate(double _x, double _y, double _w)
	:
	x(_x),
	y(_y),
	w(_w)
{
}

/*public*/
HCoordinate::HCoordinate(const Coordinate& p)
	:
	x(p.x),
	y(p.y),
	w(1.0)
{
}

/*public*/
HCoordinate::HCoordinate(const HCoordinate &p1, const HCoordinate &p2)
	:
	x( p1.y*p2.w - p2.y*p1.w ),
	y( p2.x*p1.w - p1.x*p2.w ),
	w( p1.x*p2.y - p2.x*p1.y )
{
}

/*public*/
double
HCoordinate::getX() const
{
	double a = x/w;

	// finite() also checks for NaN
	if ( ! finite(a) )
	{
		throw  NotRepresentableException();
	}
	return a;
}

/*public*/
double
HCoordinate::getY() const
{
	double a = y/w;

	// finite() also checks for NaN
	if ( ! finite(a) )
	{
		throw  NotRepresentableException();
	}
	return a;
}

/*public*/
void
HCoordinate::getCoordinate(Coordinate &ret) const
{
	ret=Coordinate(getX(), getY());
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2006/04/04 11:37:00  strk
 * Port information + initialization lists in ctors
 *
 * Revision 1.19  2006/04/04 11:28:12  strk
 * NotRepresentable condition detected using finite() from <cmath>
 * rather then using FINITE() macro. Made ::intersection() body
 * more readable.
 *
 * Revision 1.18  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.17  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

