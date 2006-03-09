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

#include <typeinfo>

//#include <geos/geosAlgorithm.h>
//#include <geos/platform.h>
#include <geos/algorithm/CentroidPoint.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/Point.h>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm


void
CentroidPoint::add(const Geometry *geom)
{
	if (typeid(*geom)==typeid(Point)) {
		add(geom->getCoordinate());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

void
CentroidPoint::add(const Coordinate *pt)
{
	ptCount+=1;
	centSum.x += pt->x;
	centSum.y += pt->y;
}

Coordinate*
CentroidPoint::getCentroid() const
{
	return new Coordinate(centSum.x/ptCount, centSum.y/ptCount);
}

bool
CentroidPoint::getCentroid(Coordinate& ret) const
{
	if ( ptCount == 0.0 ) return false;
	ret=Coordinate(centSum.x/ptCount, centSum.y/ptCount);
	return true;
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.9  2006/03/01 17:16:31  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

