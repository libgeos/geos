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
 * Revision 1.6  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/05/14 14:47:39  strk
 * Added LinearRing support
 *
 * Revision 1.2  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.1  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opDistance.h>
#include <vector>

using namespace std;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/**
* Returns a list containing a point from each Polygon, LineString, and Point
* found inside the specified geometry. Thus, if the specified geometry is
* not a GeometryCollection, an empty list will be returned. The elements of the list 
* are {@link com.vividsolutions.jts.operation.distance.GeometryLocation}s.
*/  
vector<GeometryLocation*>* ConnectedElementLocationFilter::getLocations(const Geometry *geom) {
	vector<GeometryLocation*> *loc=new vector<GeometryLocation*>();
	ConnectedElementLocationFilter *c = new ConnectedElementLocationFilter(loc);
	geom->apply_ro(c);
	delete c;
	return loc;
}

void ConnectedElementLocationFilter::filter_ro(const Geometry *geom){
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
