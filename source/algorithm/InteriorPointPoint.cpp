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
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {

InteriorPointPoint::InteriorPointPoint(const Geometry *g) {
	interiorPoint=NULL;
	minDistance=DoubleInfinity;
	auto_ptr<Point> p(g->getCentroid());
	centroid=p->getCoordinate();
	add(g);
}

InteriorPointPoint::~InteriorPointPoint() {
}

/**
* Tests the point(s) defined by a Geometry for the best inside point.
* If a Geometry is not of dimension 0 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointPoint::add(const Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) {
		add(geom->getCoordinate());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

void InteriorPointPoint::add(const Coordinate *point) {
	double dist=point->distance(*centroid);
	if (dist<minDistance) {
		delete interiorPoint;
		interiorPoint=new Coordinate(*point);
		minDistance=dist;
	}
}

Coordinate* InteriorPointPoint::getInteriorPoint() const {
	return interiorPoint;
}
}

