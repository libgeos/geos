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
/**
*  Constructs a <code>MultiPoint</code>.
*
*@param  points          the <code>Point</code>s for this <code>MultiPoint</code>
*      , or <code>null</code> or an empty array to create the empty geometry.
*      Elements may be empty <code>Point</code>s, but not <code>null</code>s.
*@param  precisionModel  the specification of the grid of allowable points
*      for this <code>MultiPoint</code>
*@param  SRID            the ID of the Spatial Reference System used by this
*      <code>MultiPoint</code>
* @deprecated Use GeometryFactory instead
*/
MultiPoint::MultiPoint(vector<Geometry *> *points,PrecisionModel* pm, int SRID):
	GeometryCollection(points, new GeometryFactory(pm, SRID,CoordinateListFactory::internalFactory)){}

/**
*@param  points          the <code>Point</code>s for this <code>MultiPoint</code>
*      , or <code>null</code> or an empty array to create the empty geometry.
*      Elements may be empty <code>Point</code>s, but not <code>null</code>s.
*/
MultiPoint::MultiPoint(vector<Geometry *> *points, GeometryFactory *newFactory): 
	GeometryCollection(points,newFactory){}
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
}

