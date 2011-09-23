/**********************************************************************
 * $Id: CentroidPoint.cpp 3179 2011-02-03 19:59:23Z strk $
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
  if ( const Point *p = dynamic_cast<const Point*>(geom) )
  {
		add(p->getCoordinate());
  }
  else if ( const GeometryCollection *gc =
            dynamic_cast<const GeometryCollection*>(geom) )
  {
    for(std::size_t i=0, n=gc->getNumGeometries(); i<n; ++i) {
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
 * Revision 1.12  2006/04/07 09:54:29  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.11  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.10  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

