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
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/operation.h"

namespace geos {

MultiPoint::MultiPoint(){}
MultiPoint::MultiPoint(vector<Geometry *> *points,PrecisionModel* pm, int SRID):
GeometryCollection(points, pm, SRID){}
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
	return new GeometryCollection(NULL, precisionModel, SRID);
}

bool MultiPoint::isSimple() const {
	auto_ptr<IsSimpleOp> iso(new IsSimpleOp());
	return iso->isSimple(this);
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

