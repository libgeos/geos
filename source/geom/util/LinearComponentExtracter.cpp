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
 * Revision 1.4  2004/05/14 13:42:46  strk
 * DistanceOp bug removed, cascading errors fixed.
 *
 * Revision 1.3  2004/05/14 09:20:47  strk
 * Mem leaks fixed
 *
 * Revision 1.2  2004/05/03 20:49:20  strk
 * Some more leaks fixed
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/geomUtil.h"
#include <typeinfo>

namespace geos {
/**
* Returns the linear components from a single geometry.
* If more than one geometry is to be processed, it is more
* efficient to create a single {@link LinearComponentExtracterFilter} instance
* and pass it to multiple geometries.
*/
vector<Geometry*>* LinearComponentExtracter::getLines(const Geometry *geom){
	vector<Geometry*> *ret=new vector<Geometry*>();
	LinearComponentExtracter lce(ret);
	geom->apply_ro(&lce);
	return ret;
}

/**
* Constructs a LinearComponentExtracterFilter with a list in which to store LineStrings found.
*/
LinearComponentExtracter::LinearComponentExtracter(vector<Geometry*> *newComps){
	comps=newComps;
}

void LinearComponentExtracter::filter_rw(Geometry *geom) {
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) comps->push_back(geom);
}

void LinearComponentExtracter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) comps->push_back((Geometry *)geom);
}
}

