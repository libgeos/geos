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
 * Revision 1.16  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.15  2004/05/07 13:04:57  strk
 * leak removed in MultiLineString::getBoundary()
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
 * Revision 1.12  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/operation.h"

namespace geos {

//MultiLineString::MultiLineString(){}
/**
*  Constructs a <code>MultiLineString</code>.
*
*@param  lineStrings     the <code>LineString</code>s for this <code>MultiLineString</code>
*      , or <code>null</code> or an empty array to create the empty geometry.
*      Elements may be empty <code>LineString</code>s, but not <code>null</code>
*      s.
*@param  precisionModel  the specification of the grid of allowable points
*      for this <code>MultiLineString</code>
*@param  SRID            the ID of the Spatial Reference System used by this
*      <code>MultiLineString</code>
* @deprecated Use GeometryFactory instead
*/
MultiLineString::MultiLineString(const vector<Geometry *> *lineStrings, PrecisionModel* precisionModel, int SRID):
	GeometryCollection(lineStrings, new GeometryFactory(precisionModel, SRID,CoordinateListFactory::internalFactory)){}

/**
* @param lineStrings
*            the <code>LineString</code>s for this <code>MultiLineString</code>,
*            or <code>null</code> or an empty array to create the empty
*            geometry. Elements may be empty <code>LineString</code>s,
*            but not <code>null</code>s.
*/
MultiLineString::MultiLineString(const vector<Geometry *> *lineStrings, const GeometryFactory *newFactory): 
	GeometryCollection(lineStrings,newFactory){}

MultiLineString::~MultiLineString(){}

int MultiLineString::getDimension() const {
	return 1;
}

int MultiLineString::getBoundaryDimension() const {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

string MultiLineString::getGeometryType() const {
	return "MultiLineString";
}

bool MultiLineString::isClosed() const {
	if (isEmpty()) {
		return false;
	}
	for (unsigned int i = 0; i < geometries->size(); i++) {
		if (!((LineString *)(*geometries)[i])->isClosed()) {
			return false;
		}
	}
	return true;
}

bool MultiLineString::isSimple() const {
	auto_ptr<IsSimpleOp> iso(new IsSimpleOp());
	return iso->isSimple((MultiLineString*) toInternalGeometry(this));
}

Geometry* MultiLineString::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	GeometryGraph *g=new GeometryGraph(0,toInternalGeometry(this));
	CoordinateList *pts=g->getBoundaryPoints();
	delete g;
	Geometry *ret = (Geometry *)getFactory()->createMultiPoint(pts);
	delete pts;
	return ret;
}

bool
MultiLineString::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other, tolerance);
}
GeometryTypeId
MultiLineString::getGeometryTypeId() const {
	return GEOS_MULTILINESTRING;
}
}

