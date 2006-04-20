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
#include <iostream>
#include <iomanip>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

// Define to make -ffloat-store be effective for this class
//#define STORE_INTERMEDIATE_COMPUTATION_VALUES

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

        HCoordinate hc1p1(p1);

#if GEOS_DEBUG
	cerr << "HCoordinate(p1): "
	     << hc1p1 << endl;
#endif

        HCoordinate hc1p2(p2);

#if GEOS_DEBUG
	cerr << "HCoordinate(p2): "
	     << hc1p2 << endl;
#endif

        HCoordinate l1(hc1p1, hc1p2);

#if GEOS_DEBUG
	cerr << "L1 - HCoordinate(HCp1, HCp2): "
	     << l1 << endl;
#endif

        HCoordinate hc2q1(q1);

#if GEOS_DEBUG
	cerr << "HCoordinate(q1): "
	     << hc2q1 << endl;
#endif

        HCoordinate hc2q2(q2);

#if GEOS_DEBUG
	cerr << "HCoordinate(q2): "
	     << hc2q2 << endl;
#endif

        HCoordinate l2(hc2q1, hc2q2);

#if GEOS_DEBUG
	cerr << "L2 - HCoordinate(HCq1, HCq2): "
	     << l2 << endl;
#endif

        HCoordinate intHCoord(l1, l2);

#if GEOS_DEBUG
	cerr << "HCoordinate(L1, L2): "
	     << intHCoord << endl;
#endif

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

std::ostream& operator<< (std::ostream& o, const HCoordinate& c)
{
	return o << "(" << c.x << ", "
	         << c.y << ") [w: " << c.w << "]";
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.22  2006/04/20 11:11:57  strk
 * source/algorithm/HCoordinate.cpp: added compile time define to force storage of intermediate computation values to variables (in order to make the -ffloat-store gcc switch effective). Disabled by default.
 *
 * Revision 1.21  2006/04/14 09:02:16  strk
 * Hadded output operator and debugging prints for HCoordinate.
 *
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

