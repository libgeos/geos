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
 * Revision 1.18  2004/07/01 14:12:44  strk
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.17  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.16  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateList.
 *
 * Revision 1.15  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.14  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.13  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/operation.h"

namespace geos {

//MultiPoint::MultiPoint(){}

// @deprecated Use GeometryFactory instead
MultiPoint::MultiPoint(vector<Geometry *> *points,PrecisionModel* pm, int SRID): GeometryCollection(points, new GeometryFactory(pm, SRID,CoordinateListFactory::internalFactory)){}

/**
* Constructs a <code>MultiPoint</code>.
*
* @param  newPoints
*	the <code>Point</code>s for this <code>MultiPoint</code>,
*	or <code>null</code> or an empty array to create the empty
* 	geometry.
*	Elements may be empty <code>Point</code>s,
*	but not <code>null</code>s.
*
*	Constructed object will take ownership of
*	the vector and its elements.
*/
MultiPoint::MultiPoint(vector<Geometry *> *newPoints, const GeometryFactory *factory): GeometryCollection(newPoints,factory){}

/**
* Constructs a <code>MultiPoint</code>.
*
* @param  fromPoints
*	the <code>Point</code>s for this <code>MultiPoint</code>,
*	or an empty array to create the empty geometry.
*	Elements may be empty <code>Point</code>s,
*	but not <code>null</code>s.
*
*	Constructed object will copy 
*	the vector and its elements.
*/
MultiPoint::MultiPoint(const vector<Geometry *> &fromPoints, const GeometryFactory *factory): GeometryCollection(fromPoints,factory){}

MultiPoint::~MultiPoint(){}

int MultiPoint::getDimension() const {
	return 0;
}

int MultiPoint::getBoundaryDimension() const {
	return Dimension::False;
}

string MultiPoint::getGeometryType() const {
	return "MultiPoint";
}

Geometry* MultiPoint::getBoundary() const {
	return getFactory()->createGeometryCollection(NULL);
}

bool MultiPoint::isSimple() const {
	auto_ptr<IsSimpleOp> iso(new IsSimpleOp());
	return iso->isSimple((MultiPoint*) toInternalGeometry(this));
}

bool MultiPoint::isValid() const {
	return true;
}

bool
MultiPoint::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other,tolerance);
  }

const Coordinate* MultiPoint::getCoordinate(int n) const {
	return ((Point *)(*geometries)[n])->getCoordinate();
}
GeometryTypeId
MultiPoint::getGeometryTypeId() const {
	return GEOS_MULTIPOINT;
}
}

