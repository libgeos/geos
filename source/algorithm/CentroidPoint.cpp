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
 * Revision 1.7  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {

CentroidPoint::CentroidPoint() {
	ptCount=0;
	centSum=new Coordinate();
}

CentroidPoint::~CentroidPoint() {
	delete centSum;
}

/**
* Adds the point(s) defined by a Geometry to the centroid total.
* If the geometry is not of dimension 0 it does not contribute to the centroid.
* @param geom the geometry to add
*/
void CentroidPoint::add(const Geometry *geom) {
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

/**
* Adds the length defined by an array of coordinates.
* @param pts an array of {@link Coordinate}s
*/
void CentroidPoint::add(const Coordinate *pt) {
	ptCount+=1;
	centSum->x+=pt->x;
	centSum->y+=pt->y;
}

Coordinate* CentroidPoint::getCentroid() const {
	return new Coordinate(centSum->x/ptCount,centSum->y/ptCount);
}
}

