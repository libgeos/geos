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
 * Revision 1.7  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {

InteriorPointLine::InteriorPointLine(Geometry *g) {
	interiorPoint=NULL;
	minDistance=DoubleInfinity;
	Point *p=g->getCentroid();
	centroid=p->getCoordinate();
	addInterior(g);
	if (interiorPoint==NULL)
		addEndpoints(g);
	delete p;
}

InteriorPointLine::~InteriorPointLine() {
}

/**
* Tests the interior vertices (if any)
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointLine::addInterior(const Geometry *geom){
	if (typeid(*geom)==typeid(LineString)) {
		addInterior(((LineString *)geom)->getCoordinatesRO());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			addInterior(gc->getGeometryN(i));
		}
	}
}

void InteriorPointLine::addInterior(const CoordinateList *pts) {
	for(int i=1;i<pts->getSize()-1;i++) {
		add(&(pts->getAt(i)));
	}
}

/**
* Tests the endpoint vertices
* defined by a linear Geometry for the best inside point.
* If a Geometry is not of dimension 1 it is not tested.
* @param geom the geometry to add
*/
void InteriorPointLine::addEndpoints(const Geometry *geom) {
	if (typeid(*geom)==typeid(LineString)) {
		addEndpoints(((LineString*)geom)->getCoordinatesRO());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			addEndpoints(gc->getGeometryN(i));
		}
	}
}

void InteriorPointLine::addEndpoints(const CoordinateList *pts){
	add(&(pts->getAt(0)));
	add(&(pts->getAt(pts->getSize()-1)));
}

void InteriorPointLine::add(const Coordinate *point) {
	double dist=point->distance(*centroid);
	if (dist<minDistance) {
		delete interiorPoint;
		interiorPoint=new Coordinate(*point);
		minDistance=dist;
	}
}

Coordinate* InteriorPointLine::getInteriorPoint() const {
	return interiorPoint;
}
}

