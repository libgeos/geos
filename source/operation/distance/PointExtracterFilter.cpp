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

vector<const Geometry*>* PointExtracterFilter::getPoints(const Geometry *geom) {
	vector<const Geometry*> *points=new vector<const Geometry*>();
	PointExtracterFilter *p=new PointExtracterFilter(points);
	geom->apply_ro(p);
	delete p;
	return points;
}

PointExtracterFilter::PointExtracterFilter(vector<const Geometry*> *newPts){
	pts=newPts;
}

void PointExtracterFilter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) pts->push_back(geom);
}
}

