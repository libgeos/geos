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
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.7  2003/10/29 10:38:26  strk
 * Added support for LinearRing types (treated as LineString)
 *
 * Revision 1.6  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/


#include "../headers/geosAlgorithm.h"
#include "../headers/platform.h"
#include <typeinfo>

namespace geos {

CentroidLine::CentroidLine() {
	totalLength=0.0;
	centSum=new Coordinate();
}

CentroidLine::~CentroidLine() {
	delete centSum;
}

/**
* Adds the linestring(s) defined by a Geometry to the centroid total.
* If the geometry is not linear it does not contribute to the centroid
* @param geom the geometry to add
*/
void CentroidLine::add(const Geometry *geom) {
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) {
		add(((LineString*)geom)->getCoordinatesRO());
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
void CentroidLine::add(const CoordinateList *pts) {
	for(int i=0;i<pts->getSize()-1;i++) {
		double segmentLen=pts->getAt(i).distance(pts->getAt(i + 1));
		totalLength+=segmentLen;
		double midx=(pts->getAt(i).x+pts->getAt(i+1).x)/2;
		centSum->x+=segmentLen*midx;
		double midy=(pts->getAt(i).y+pts->getAt(i+1).y)/2;
		centSum->y+=segmentLen*midy;
	}
}

Coordinate* CentroidLine::getCentroid() const {
	return new Coordinate(centSum->x/totalLength,centSum->y/totalLength);
}
}

