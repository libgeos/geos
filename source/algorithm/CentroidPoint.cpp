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

#include <geos/algorithm/CentroidPoint.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/Point.h>

#include <typeinfo>

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
 * Revision 1.11  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.10  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

