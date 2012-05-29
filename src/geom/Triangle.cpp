/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Triangle.h>
#include <geos/geom/Coordinate.h>

namespace geos {
namespace geom { // geos::geom


void
Triangle::inCentre(Coordinate& result)
{
	// the lengths of the sides, labelled by their opposite vertex
	double len0 = p1.distance(p2);
	double len1 = p0.distance(p2);
	double len2 = p0.distance(p1);
	double circum = len0 + len1 + len2;
	double inCentreX = (len0 * p0.x + len1 * p1.x +len2 * p2.x)  / circum;
	double inCentreY = (len0 * p0.y + len1 * p1.y +len2 * p2.y)  / circum;

	result = Coordinate(inCentreX, inCentreY);
}

} // namespace geos::geom
} // namespace geos
