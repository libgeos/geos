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
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 * Revision 1.15  2003/10/15 15:47:43  strk
 * Adapted to new getCoordinatesRO() interface
 *
 **********************************************************************/


#include "../headers/geosAlgorithm.h"
#include <typeinfo>
#include "../headers/graph.h"

namespace geos {

PointLocator::PointLocator() {
	cga=new RobustCGAlgorithms();
}

PointLocator::~PointLocator() {
	delete cga;
}

/**
* Convenience method to test a point for intersection with
* a Geometry
* @param p the coordinate to test
* @param geom the Geometry to test
* @return <code>true</code> if the point is in the interior or boundary of the Geometry
*/
bool PointLocator::intersects(const Coordinate& p,const Geometry *geom) {
	return locate(p,geom)!=Location::EXTERIOR;
}


/**
* Computes the topological relationship ({@link Location}) of a single point
* to a Geometry.
* It handles both single-element
* and multi-element Geometries.
* The algorithm for multi-part Geometries
* takes into account the boundaryDetermination rule.
*
* @return the {@link Location} of the point relative to the input Geometry
*/
int PointLocator::locate(const Coordinate& p, const Geometry *geom) {
	if (geom->isEmpty()) return Location::EXTERIOR;
	if (typeid(*geom)==typeid(LineString)) {
		return locate(p,(LineString*) geom);
	}
	if (typeid(*geom)==typeid(LinearRing)) {
		return locate(p,(LinearRing*) geom);
	} else if (typeid(*geom)==typeid(Polygon)) {
		return locate(p,(Polygon*) geom);
	}

	isIn=false;
	numBoundaries=0;
	computeLocation(p,geom);
	if (GeometryGraph::isInBoundary(numBoundaries)) return Location::BOUNDARY;
	if (numBoundaries>0 || isIn) return Location::INTERIOR;
	return Location::EXTERIOR;
}

void PointLocator::computeLocation(const Coordinate& p, const Geometry *geom) {
	if (typeid(*geom)==typeid(LineString)) {
		updateLocationInfo(locate(p,(LineString*) geom));
	}
	if (typeid(*geom)==typeid(LinearRing)) {
		updateLocationInfo(locate(p,(LinearRing*) geom));
	} else if (typeid(*geom)==typeid(Polygon)) {
		updateLocationInfo(locate(p,(Polygon*) geom));
	} else if (typeid(*geom)==typeid(MultiLineString)) {
		MultiLineString *ml=(MultiLineString*) geom;
		for(int i=0;i<ml->getNumGeometries();i++) {
			LineString *l=(LineString*) ml->getGeometryN(i);
			updateLocationInfo(locate(p,l));
		}
	} else if (typeid(*geom)==typeid(MultiPolygon)) {
		MultiPolygon *mpoly=(MultiPolygon*) geom;
		for(int i=0;i<mpoly->getNumGeometries();i++) {
			Polygon *poly=(Polygon*) mpoly->getGeometryN(i);
			updateLocationInfo(locate(p,poly));
		}
	} else if (typeid(*geom)==typeid(GeometryCollection)) {
		GeometryCollectionIterator geomi((GeometryCollection*) geom);
		while (geomi.hasNext()) {
			const Geometry *g2=geomi.next();
//			if (! g2->equals(geom))
			if (g2!=geom)
				computeLocation(p,g2);
		}
	}
}

void PointLocator::updateLocationInfo(int loc) {
	if (loc==Location::INTERIOR) isIn=true;
	if (loc==Location::BOUNDARY) numBoundaries++;
}

int PointLocator::locate(const Coordinate& p, const LineString *l) {
	const CoordinateList* pt=l->getCoordinatesRO();
	if (! l->isClosed()) {
		if ((p==pt->getAt(0)) || (p==pt->getAt(pt->getSize()-1))) {
			return Location::BOUNDARY;
		}
	}
	if (cga->isOnLine(p,pt))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

int PointLocator::locate(const Coordinate& p, const LinearRing *ring) {
	const CoordinateList *cl = ring->getCoordinatesRO();
	if (cga->isOnLine(p,cl)) {
		return Location::BOUNDARY;
	}
	if (cga->isPointInRing(p,cl))
	{
		return Location::INTERIOR;
	}
	return Location::EXTERIOR;
}

int PointLocator::locate(const Coordinate& p,const Polygon *poly) {
	if (poly->isEmpty()) return Location::EXTERIOR;

	const LinearRing *shell=(LinearRing*) poly->getExteriorRing();

	int shellLoc=locate(p,shell);
	if (shellLoc==Location::EXTERIOR) return Location::EXTERIOR;
	if (shellLoc==Location::BOUNDARY) return Location::BOUNDARY;
	// now test if the point lies in or on the holes
	for(int i=0;i<poly->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*) poly->getInteriorRingN(i);
		int holeLoc=locate(p,hole);
		if (holeLoc==Location::INTERIOR) return Location::EXTERIOR;
		if (holeLoc==Location::BOUNDARY) return Location::BOUNDARY;
	}
	return Location::INTERIOR;
}

}

