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
 * Revision 1.15  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.14  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.12  2003/10/11 03:23:22  strk
 * fixed spurious typos
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

LinearRing::LinearRing(const LinearRing &lr): LineString(lr) {}

/**
*  Constructs a <code>LinearRing</code> with the given points.
*
*@param  points          points forming a closed and simple linestring, or
*      <code>null</code> or an empty array to create the empty geometry.
*      This array must not contain <code>null</code> elements.
*
*@param  precisionModel  the specification of the grid of allowable points
*      for this <code>LinearRing</code>
*@param  SRID            the ID of the Spatial Reference System used by this
*      <code>LinearRing</code>
* @deprecated Use GeometryFactory instead
*/
LinearRing::LinearRing(const CoordinateList* points, const PrecisionModel* pm,
	int SRID): LineString(points, new GeometryFactory(pm,SRID,CoordinateListFactory::internalFactory)) {
	validateConstruction();	
}

/**
*  Constructs a <code>LinearRing</code> with the given points.
*
*@param  points          points forming a closed and simple linestring, or
*      <code>null</code> or an empty array to create the empty geometry.
*      This array must not contain <code>null</code> elements.
*
*/
LinearRing::LinearRing(const CoordinateList* points, const GeometryFactory *newFactory): LineString(points,newFactory) {
	validateConstruction();	
}


void LinearRing::validateConstruction() {
	if (!LineString::isEmpty() && !LineString::isClosed()) {
		throw new IllegalArgumentException("points must form a closed linestring");
    }
	if (!points->isEmpty() && (points->getSize()>=1 && points->getSize()<=3)) {
		throw new IllegalArgumentException("Number of points must be 0 or >3");
	}
}


		
// superclass LineString will delete internal CoordinateList
LinearRing::~LinearRing(){
}

bool LinearRing::isSimple() const {
	return true;
}
string LinearRing::getGeometryType() const {
	return "LinearRing";
}
bool LinearRing::isClosed() const {
	return true;
}

void LinearRing::setPoints(CoordinateList* cl){
	vector<Coordinate> *v=cl->toVector();
	points->setPoints(*(v));
	delete v;
}
}

