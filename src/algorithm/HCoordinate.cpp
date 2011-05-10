/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011      Sandro Santilli <strk@keybit.net>
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
 * Last port: algorithm/HCoordinate.java r386 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/algorithm/HCoordinate.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/geom/Coordinate.h>
#include <geos/platform.h>

#include <memory>
#include <cmath>
#include <limits>
#include <iostream>
#include <iomanip>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

// Define to make -ffloat-store be effective for this class
#define STORE_INTERMEDIATE_COMPUTATION_VALUES 1

using namespace std;
using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/*public static*/
void
HCoordinate::intersection(const Coordinate &p1, const Coordinate &p2,
	const Coordinate &q1, const Coordinate &q2, Coordinate &ret)
{

#if GEOS_DEBUG
	cerr << __FUNCTION__ << ":" << endl
	     << setprecision(20)
	     << " p1: " << p1 << endl
	     << " p2: " << p2 << endl
	     << " q1: " << q1 << endl
	     << " q2: " << q2 << endl;
#endif

#if 0 // NOTE: unrolled computation is intentionally turned off
      //       as it makes the algorithm less stable numerically
      //       In particular the first case in TestBufferMitredJoin.xml
      //       would fail with unrolled computation on and no input
      //       simplification in operation/buffer/OffsetCurveBuilder.cpp

	// unrolled computation

	double px = p1.y - p2.y;
	double py = p2.x - p1.x;
	double pw = p1.x * p2.y - p2.x * p1.y;

	double qx = q1.y - q2.y;
	double qy = q2.x - q1.x;
	double qw = q1.x * q2.y - q2.x * q1.y;

	double x = py * qw - qy * pw;
	double y = qx * pw - px * qw;
	double w = px * qy - qx * py;

	double xInt = x/w;
	double yInt = y/w;

	if ( (!FINITE(xInt)) || (!FINITE(yInt)) )
	{
		throw NotRepresentableException();
	}

	ret = Coordinate(xInt, yInt);
#else

	HCoordinate hc1p1(p1);
	HCoordinate hc1p2(p2);
	HCoordinate l1(hc1p1, hc1p2);

	HCoordinate hc2q1(q1);
	HCoordinate hc2q2(q2);
	HCoordinate l2(hc2q1, hc2q2);

	HCoordinate intHCoord(l1, l2);
	intHCoord.getCoordinate(ret);
#endif

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
HCoordinate::HCoordinate(const Coordinate& p1, const Coordinate& p2)
	:
	// optimization when it is known that w = 1
	x ( p1.y - p2.y ),
	y ( p2.x - p1.x ),
	w ( p1.x * p2.y - p2.x * p1.y )
{
}

/*public*/
HCoordinate::HCoordinate(const Coordinate& p1, const Coordinate& p2,
			 const Coordinate& q1, const Coordinate& q2)
{
	// unrolled computation
	double px = p1.y - p2.y;
	double py = p2.x - p1.x;
	double pw = p1.x * p2.y - p2.x * p1.y;

	double qx = q1.y - q2.y;
	double qy = q2.x - q1.x;
	double qw = q1.x * q2.y - q2.x * q1.y;

	x = py * qw - qy * pw;
	y = qx * pw - px * qw;
	w = px * qy - qx * py;
}

/*public*/
#ifndef STORE_INTERMEDIATE_COMPUTATION_VALUES

HCoordinate::HCoordinate(const HCoordinate &p1, const HCoordinate &p2)
	:
	x( p1.y*p2.w - p2.y*p1.w ),
	y( p2.x*p1.w - p1.x*p2.w ),
	w( p1.x*p2.y - p2.x*p1.y )
{
}

#else // def STORE_INTERMEDIATE_COMPUTATION_VALUES

HCoordinate::HCoordinate(const HCoordinate &p1, const HCoordinate &p2)
{
        double xf1 = p1.y*p2.w;
        double xf2 = p2.y*p1.w;
        x = xf1 - xf2;

        double yf1 = p2.x*p1.w;
        double yf2 = p1.x*p2.w;
        y = yf1 - yf2;

        double wf1 = p1.x*p2.y;
        double wf2 = p2.x*p1.y;
        w = wf1 - wf2;

#if GEOS_DEBUG
        cerr
             << " xf1: " << xf1 << endl
             << " xf2: " << xf2 << endl
             << " yf1: " << yf1 << endl
             << " yf2: " << yf2 << endl
             << " wf1: " << wf1 << endl
             << " wf2: " << wf2 << endl
             << "   x: " << x << endl
             << "   y: " << y << endl
             << "   w: " << w << endl;
#endif // def GEOS_DEBUG
}
#endif // def STORE_INTERMEDIATE_COMPUTATION_VALUES

/*public*/
double
HCoordinate::getX() const
{
	double a = x/w;

	//if (std::fabs(a) > std::numeric_limits<double>::max())
	if ( !FINITE(a) )
	{
		throw NotRepresentableException();
	}
	return a;
}

/*public*/
double
HCoordinate::getY() const
{
	double a = y/w;

	//if (std::fabs(a) > std::numeric_limits<double>::max())
	if ( !FINITE(a) )
	{
		throw  NotRepresentableException();
	}
	return a;
}

/*public*/
void
HCoordinate::getCoordinate(Coordinate &ret) const
{
	ret = Coordinate(static_cast<double>(getX()), static_cast<double>(getY()));
}

std::ostream& operator<< (std::ostream& o, const HCoordinate& c)
{
	return o << "(" << c.x << ", "
	         << c.y << ") [w: " << c.w << "]";
}

} // namespace geos.algorithm
} // namespace geos

