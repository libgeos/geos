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
#include <typeinfo>

namespace geos {
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

ConnectedElementLocationFilter::ConnectedElementLocationFilter(vector<GeometryLocation*> *newLocations): locations(newLocations) {
	//locations=newLocations;
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

}
