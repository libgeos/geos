/**********************************************************************
 * $Id: ConnectedElementLocationFilter.cpp 2553 2009-06-06 00:55:55Z strk $
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
 *
 * Last port: operation/distance/ConnectedElementLocationFilter.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/distance/ConnectedElementLocationFilter.h>
#include <geos/operation/distance/GeometryLocation.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>

#include <vector>
#include <typeinfo>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/*public*/
vector<GeometryLocation*>*
ConnectedElementLocationFilter::getLocations(const Geometry *geom)
{
	vector<GeometryLocation*> *loc=new vector<GeometryLocation*>();
	ConnectedElementLocationFilter c(loc);
	geom->apply_ro(&c);
	return loc;
}

void
ConnectedElementLocationFilter::filter_ro(const Geometry *geom)
{
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(LinearRing)) ||
		(typeid(*geom)==typeid(Polygon)))
	{
		locations->push_back(new GeometryLocation(geom, 0, *(geom->getCoordinate())));
	}
}

void ConnectedElementLocationFilter::filter_rw(Geometry *geom){
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(LinearRing)) ||
		(typeid(*geom)==typeid(Polygon)))
			locations->push_back(new GeometryLocation(geom, 0, *(geom->getCoordinate())));
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 * Revision 1.6  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 **********************************************************************/

