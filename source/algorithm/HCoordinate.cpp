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


#include <geos/geosAlgorithm.h>
#include <geos/platform.h>

namespace geos {

/**
* Computes the (approximate) intersection point between two line segments
* using homogeneous coordinates.
* <p>
* Note that this algorithm is
* not numerically stable; i.e. it can produce intersection points which
* lie outside the envelope of the line segments themselves.  In order
* to increase the precision of the calculation input points should be normalized
* before passing them to this routine.
*/
Coordinate* HCoordinate::intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2) {
	auto_ptr<HCoordinate> intHCoord(new HCoordinate(HCoordinate(HCoordinate(p1),HCoordinate(p2)),HCoordinate(HCoordinate(q1),HCoordinate(q2))));
	return intHCoord->getCoordinate();
}

HCoordinate::HCoordinate(){
	x = 0.0;
	y = 0.0;
	w = 1.0;
}

HCoordinate::HCoordinate(double _x, double _y, double _w) {
	x = _x;
	y = _y;
	w = _w;
}

HCoordinate::HCoordinate(Coordinate& p) {
	x = p.x;
	y = p.y;
	w = 1.0;
}

HCoordinate::HCoordinate(HCoordinate p1, HCoordinate p2) {
	x = p1.y*p2.w - p2.y*p1.w;
	y = p2.x*p1.w - p1.x*p2.w;
	w = p1.x*p2.y - p2.x*p1.y;
}

double HCoordinate::getX() {
	double a = x/w;
	if ((a==DoubleNotANumber)||(a==DoubleInfinity)||(a==DoubleNegInfinity)) {
		throw new NotRepresentableException();
	}
	return a;
}

double HCoordinate::getY() {
	double a = y/w;
	if ((a==DoubleNotANumber)||(a==DoubleInfinity)||(a==DoubleNegInfinity)) {
		throw new NotRepresentableException();
	}
	return a;
}

Coordinate* HCoordinate::getCoordinate() {
	return new Coordinate(getX(),getY());
}
}

