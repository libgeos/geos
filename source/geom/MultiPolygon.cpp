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
MultiPolygon::MultiPolygon(vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID):
	GeometryCollection(polygons, new GeometryFactory(precisionModel, SRID,CoordinateListFactory::internalFactory)){}

MultiPolygon::MultiPolygon(vector<Geometry *> *polygons, GeometryFactory *newFactory): 
	GeometryCollection(polygons,newFactory){}

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
}

