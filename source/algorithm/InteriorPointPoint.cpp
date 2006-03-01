/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>
#include <cassert>

namespace geos {
namespace algorithm { // geos.algorithm

/*public*/
InteriorPointPoint::InteriorPointPoint(const Geometry *g)
{
	minDistance=DoubleInfinity;
	if ( ! g->getCentroid(centroid) ) {
		hasInterior=false;
	}
	add(g);
}

/*private*/
void
InteriorPointPoint::add(const Geometry *geom)
{
	if (typeid(*geom)==typeid(Point)) {
		add(geom->getCoordinate());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

/*private*/
void
InteriorPointPoint::add(const Coordinate *point)
{
	if ( ! point ) return;
	if ( ! hasInterior ) return;

	double dist=point->distance(centroid);
	if (dist<minDistance) {
		interiorPoint=*point;
		minDistance=dist;
	}
}

/*public*/
bool
InteriorPointPoint::getInteriorPoint(Coordinate& ret) const
{
	if ( ! hasInterior ) return false;
	ret=interiorPoint;
	return true;
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/01 17:16:31  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.10  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

