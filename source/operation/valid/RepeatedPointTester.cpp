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
 * Revision 1.11  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.10  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/16 17:33:20  strk
 * dropped useless string() cast
 *
 **********************************************************************/


#include <geos/opValid.h>
#include <stdio.h>
#include <typeinfo>

namespace geos {

Coordinate& RepeatedPointTester::getCoordinate(){
	return repeatedCoord;
}

bool RepeatedPointTester::hasRepeatedPoint(const Geometry *g){
	if (g->isEmpty()) return false;
	if (typeid(*g)==typeid(Point)) return false;
	else if (typeid(*g)==typeid(MultiPoint)) return false;
	// LineString also handles LinearRings
	else if (typeid(*g)==typeid(LineString)) return hasRepeatedPoint(((LineString*)g)->getCoordinates());
	else if (typeid(*g)==typeid(Polygon)) return hasRepeatedPoint((Polygon*)g);
	else if (typeid(*g)==typeid(MultiPolygon)) return hasRepeatedPoint((MultiPolygon*)g);
	else if (typeid(*g)==typeid(MultiLineString)) return hasRepeatedPoint((MultiLineString*)g);
	else if (typeid(*g)==typeid(GeometryCollection)) return hasRepeatedPoint((GeometryCollection*)g);
	else  throw new UnsupportedOperationException(typeid(*g).name());
}

bool RepeatedPointTester::hasRepeatedPoint(const CoordinateSequence *coord){
	for(int i=1; i<coord->getSize(); i++) {
		if (coord->getAt(i - 1)==coord->getAt(i)) {
			repeatedCoord=coord->getAt(i);
			return true;
		}
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const Polygon *p){
	if (hasRepeatedPoint(p->getExteriorRing()->getCoordinates())) return true;
	for(int i=0; i<p->getNumInteriorRing(); i++) {
		if (hasRepeatedPoint(p->getInteriorRingN(i)->getCoordinates())) return true;
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const GeometryCollection *gc){
	for(int i = 0; i<gc->getNumGeometries(); i++) {
		const Geometry *g=gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) return true;
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const MultiPolygon *gc){
	for(int i = 0; i<gc->getNumGeometries(); i++) {
		const Geometry *g=gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) return true;
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const MultiLineString *gc){
	for(int i = 0; i<gc->getNumGeometries(); i++) {
		const Geometry *g=gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) return true;
	}
	return false;
}
}

