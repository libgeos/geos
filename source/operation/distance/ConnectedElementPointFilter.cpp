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
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<const Coordinate*>* ConnectedElementPointFilter::getCoordinates(const Geometry *geom) {
	vector<const Coordinate*> *points=new vector<const Coordinate*>();
	ConnectedElementPointFilter *c=new ConnectedElementPointFilter(points);
	geom->apply_ro(c);
	delete c;
	return points;
}

ConnectedElementPointFilter::ConnectedElementPointFilter(vector<const Coordinate*> *newPts){
	pts=newPts;
}

void ConnectedElementPointFilter::filter_ro(const Geometry *geom) {
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(Polygon)))
			pts->push_back(geom->getCoordinate());
}
}

