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
 * Revision 1.9  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opDistance.h>
#include <typeinfo>

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/**
* Returns a list containing a Coordinate from each Polygon, LineString, and Point
* found inside the specified geometry. Thus, if the specified geometry is
* not a GeometryCollection, an empty list will be returned.
*/
vector<const Coordinate*>* ConnectedElementPointFilter::getCoordinates(const Geometry *geom) {
	vector<const Coordinate*> *points=new vector<const Coordinate*>();
	ConnectedElementPointFilter *c=new ConnectedElementPointFilter(points);
	geom->apply_ro(c);
	delete c;
	return points;
}

void ConnectedElementPointFilter::filter_ro(const Geometry *geom) {
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(Polygon)))
			pts->push_back(geom->getCoordinate());
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos

