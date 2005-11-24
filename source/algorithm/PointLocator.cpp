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
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <typeinfo>
#include <geos/geomgraph.h>

namespace geos {

PointLocator::PointLocator() {
}

PointLocator::~PointLocator() {
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
 * @return the Location of the point relative to the input Geometry
 */
int
PointLocator::locate(const Coordinate& p, const Geometry *geom)
{
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

void
PointLocator::computeLocation(const Coordinate& p, const Geometry *geom)
{
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

void
PointLocator::updateLocationInfo(int loc)
{
	if (loc==Location::INTERIOR) isIn=true;
	if (loc==Location::BOUNDARY) numBoundaries++;
}

int
PointLocator::locate(const Coordinate& p, const LineString *l)
{
	const CoordinateSequence* pt=l->getCoordinatesRO();
	if (! l->isClosed()) {
		if ((p==pt->getAt(0)) || (p==pt->getAt(pt->getSize()-1))) {
			return Location::BOUNDARY;
		}
	}
	if (CGAlgorithms::isOnLine(p,pt))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

int
PointLocator::locate(const Coordinate& p, const LinearRing *ring)
{
	const CoordinateSequence *cl = ring->getCoordinatesRO();
	if (CGAlgorithms::isOnLine(p,cl)) {
		return Location::BOUNDARY;
	}
	if (CGAlgorithms::isPointInRing(p,cl))
	{
		return Location::INTERIOR;
	}
	return Location::EXTERIOR;
}

int
PointLocator::locate(const Coordinate& p,const Polygon *poly)
{
	if (poly->isEmpty()) return Location::EXTERIOR;

	const LinearRing *shell=(LinearRing *)poly->getExteriorRing();

	int shellLoc=locate(p, shell);
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.24  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.23  2005/06/24 11:09:42  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.22  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.21  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.20  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.19  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.18  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
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

