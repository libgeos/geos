/**********************************************************************
 * $Id: Triangle.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.6  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 **********************************************************************/

