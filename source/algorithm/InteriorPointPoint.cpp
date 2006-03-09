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

#include <typeinfo>
#include <cassert>

//#include <geos/geosAlgorithm.h>
//#include <geos/platform.h>

#include <geos/algorithm/InteriorPointPoint.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/*public*/
InteriorPointPoint::InteriorPointPoint(const Geometry *g)
{
	minDistance=DoubleInfinity;
	if ( ! g->getCentroid(centroid) ) {
		hasInterior=false;
	} else {
		hasInterior=true;
		add(g);
	}
}

/*private*/
void
InteriorPointPoint::add(const Geometry *geom)
{
	const Point *po = dynamic_cast<const Point*>(geom);
	if ( po ) {
		add(po->getCoordinate());
		return;
	}

	const GeometryCollection *gc = dynamic_cast<const GeometryCollection*>(geom);
	if ( gc )
	{
		for(unsigned int i=0, n=gc->getNumGeometries(); i<n; i++) {
			add(gc->getGeometryN(i));
		}
	}
}

/*private*/
void
InteriorPointPoint::add(const Coordinate *point)
{
	assert ( point ); // we wouldn't been called if this was an empty geom
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
 * Revision 1.13  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.12  2006/03/01 18:36:57  strk
 * Geometry::createPointFromInternalCoord dropped (it's a duplication of GeometryFactory::createPointFromInternalCoord).
 * Fixed bugs in InteriorPoint* and getCentroid() inserted by previous commits.
 *
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

