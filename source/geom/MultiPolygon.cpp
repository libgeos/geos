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
 * Revision 1.17  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.16  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateList argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.15  2004/06/15 20:34:52  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.14  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateList.
 *
 * Revision 1.13  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.12  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.10  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more i
 * calls to new getCoordinatesRO() when applicable.
 *
 **********************************************************************/


#include "../headers/geom.h"

namespace geos {

//MultiPolygon::MultiPolygon(){}
MultiPolygon::MultiPolygon(const vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID): GeometryCollection(polygons, new GeometryFactory(precisionModel, SRID,CoordinateListFactory::internalFactory)){}

MultiPolygon::MultiPolygon(const vector<Geometry *> *polygons, const GeometryFactory *newFactory): GeometryCollection(polygons,newFactory){}

MultiPolygon::~MultiPolygon(){}

int MultiPolygon::getDimension() const {
	return 2;
}

int MultiPolygon::getBoundaryDimension() const {
	return 1;
}

string MultiPolygon::getGeometryType() const {
	return "MultiPolygon";
}

bool MultiPolygon::isSimple() const {
	return true;
}

Geometry* MultiPolygon::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	vector<Geometry *>* allRings=new vector<Geometry *>();
	for (unsigned int i = 0; i < geometries->size(); i++) {
		Polygon *pg=(Polygon *) (*geometries)[i];
		Geometry *g=pg->getBoundary();
		GeometryCollection* rings=(GeometryCollection*)g;
		for (int j = 0; j < rings->getNumGeometries(); j++) {
			allRings->push_back(new LineString(*(LineString*)rings->getGeometryN(j)));
		}
		delete g;
	}
//LineString[] allRingsArray = new LineString[allRings.size()];
	Geometry *ret=getFactory()->createMultiLineString(allRings);
	for (int i=0; i<allRings->size(); i++)
		delete (*allRings)[i];
	delete allRings;
	return ret;
}

bool
MultiPolygon::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other, tolerance);
}
GeometryTypeId
MultiPolygon::getGeometryTypeId() const {
	return GEOS_MULTIPOLYGON;
}
}

