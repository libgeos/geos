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

vector<const Geometry*>* PolygonExtracterFilter::getPolygons(const Geometry *geom) {
	vector<const Geometry*> *cmp=new vector<const Geometry*>();
	PolygonExtracterFilter *p=new PolygonExtracterFilter(cmp);
	geom->apply_ro(p);
	delete p;
	return cmp;
}

PolygonExtracterFilter::PolygonExtracterFilter(vector<const Geometry*> *newComps){
	comps=newComps;
}

void PolygonExtracterFilter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) comps->push_back(geom);
}
}

