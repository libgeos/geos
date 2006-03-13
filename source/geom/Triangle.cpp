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
 **********************************************************************/

//#include <geos/geom.h>
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
 * Revision 1.6  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.5  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.4  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
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

