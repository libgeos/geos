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
 * Revision 1.2  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.1  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

Triangle::Triangle(const Coordinate& nP0,const Coordinate& nP1,const Coordinate& nP2) {
	p0=nP0;
	p1=nP1;
	p2=nP2;
}

/**
* The inCentre of a triangle is the point which is equidistant
* from the sides of the triangle.  This is also the point at which the bisectors
* of the angles meet.
*
* @return the point which is the inCentre of the triangle
*/
Coordinate* Triangle::inCentre() {
	// the lengths of the sides, labelled by their opposite vertex
	double len0 = p1.distance(p2);
	double len1 = p0.distance(p2);
	double len2 = p0.distance(p1);
	double circum = len0 + len1 + len2;
	double inCentreX = (len0 * p0.x + len1 * p1.x +len2 * p2.x)  / circum;
	double inCentreY = (len0 * p0.y + len1 * p1.y +len2 * p2.y)  / circum;
	return new Coordinate(inCentreX, inCentreY);
}
}
