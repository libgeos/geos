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
 * Revision 1.15  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.14  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <typeinfo>

namespace geos {

/**
* locate is the main location function.  It handles both single-element
* and multi-element Geometries.  The algorithm for multi-element Geometries
* is more complex, since it has to take into account the boundaryDetermination rule
*/
int SimplePointInAreaLocator::locate(const Coordinate& p, const Geometry *geom){
	if (geom->isEmpty()) return Location::EXTERIOR;
	if (containsPoint(p,geom))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

bool SimplePointInAreaLocator::containsPoint(const Coordinate& p,const Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) {
		return containsPointInPolygon(p,(Polygon*)geom);
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
			   (typeid(*geom)==typeid(MultiPoint)) ||
			   (typeid(*geom)==typeid(MultiLineString)) ||
			   (typeid(*geom)==typeid(MultiPolygon))) {
		GeometryCollectionIterator geomi((GeometryCollection*)geom);
		while (geomi.hasNext()) {
			const Geometry *g2=geomi.next();
			if (g2!=geom)
				if (containsPoint(p,g2))
					return true;
		}
	}
	return false;
}

bool SimplePointInAreaLocator::containsPointInPolygon(const Coordinate& p,const Polygon *poly) {
	if (poly->isEmpty()) return false;
	const LineString *shell=poly->getExteriorRing();
	const CoordinateList *cl;
	cl = shell->getCoordinatesRO();
	if (!CGAlgorithms::isPointInRing(p,cl)) {
		return false;
	}

	// now test if the point lies in or on the holes
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*)poly->getInteriorRingN(i);
		cl = hole->getCoordinatesRO();
		if (CGAlgorithms::isPointInRing(p,cl)) {
			return false;
		}
	}
	return true;
}
}

