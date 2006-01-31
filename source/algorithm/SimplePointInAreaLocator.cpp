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
 * Revision 1.17  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.16  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
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
	const CoordinateSequence *cl;
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

