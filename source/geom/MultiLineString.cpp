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
 * Revision 1.19  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateList
 * Changed createMultiPoint(CoordinateList) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.18  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.17  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
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


#include <geos/geom.h>
#include <geos/operation.h>

namespace geos {

//MultiLineString::MultiLineString(){}

// @deprecated Use GeometryFactory instead
MultiLineString::MultiLineString(vector<Geometry *> *lineStrings, PrecisionModel* precisionModel, int SRID): GeometryCollection(lineStrings, new GeometryFactory(precisionModel, SRID,CoordinateListFactory::internalFactory)) { }

/**
* Constructs a <code>MultiLineString</code>.
*
* @param  newLines
*	the <code>LineStrings</code>s for this
*	<code>MultiLineString</code>, or <code>null</code>
*	or an empty array to create the empty geometry.
*	Elements may be empty <code>LineString</code>s,
*	but not <code>null</code>s.
*
*	Constructed object will take ownership of
*	the vector and its elements.
*/
MultiLineString::MultiLineString(vector<Geometry *> *newLines, const GeometryFactory *factory): GeometryCollection(newLines,factory){}

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
	GeometryGraph gg(0, toInternalGeometry(this));
	//GeometryGraph *g=new GeometryGraph(0,toInternalGeometry(this));
	CoordinateList *pts=gg.getBoundaryPoints();
	//delete g;
	Geometry *ret = getFactory()->createMultiPoint(*pts);
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

