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
 * Revision 1.15  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.14  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 * Revision 1.13  2003/10/09 10:10:05  strk
 * Throw an exception if scale is 0. Added Log entry.
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/util.h"
#include "stdio.h"

namespace geos {

const double maximumPreciseValue=9007199254740992.0;

//double PrecisionModel::makePrecise(double val){
//	//return rint(val);
//	return ((val >= 0.0) ? floor(val+0.5) : - floor(-val+0.5));
//    /*
//     * Other options:
//     * - Math.floor(a + 0.5d);
//     * - Math.floor(a);
//     * -  (val >= 0.0) ? Math.floor(val) : - Math.floor(-val);
//     */
//}

/**
* Rounds an numeric value to the PrecisionModel grid.
*/
double PrecisionModel::makePrecise(double val) const {
	double v=val*scale;
	if (val>=0.0) 
		v=floor(v+0.5);
	else 
		v=-floor(-v+0.5);
	return v/scale;
}

/**
* Rounds a Coordinate to the PrecisionModel grid.
*/
void PrecisionModel::makePrecise(Coordinate *coord) const {
	if (modelType==FLOATING) return;
	coord->x=makePrecise(coord->x);
	coord->y=makePrecise(coord->y);
}


PrecisionModel::PrecisionModel(){
	modelType=FLOATING;
	scale=0.0;
	offsetX=0.0;
	offsetY=0.0;
}

/**
*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
*  Fixed-precision coordinates are represented as precise internal coordinates,
*  which are rounded to the grid defined by the scale factor.
*
*@param  scale    amount by which to multiply a coordinate after subtracting
*      the offset, to obtain a precise coordinate
*@param  offsetX  not used.
*@param  offsetY  not used.
*
* @deprecated
*/
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY) {
	if ( newScale == 0 ) throw new IllegalArgumentException("PrecisionModel scale cannot be 0"); 
	modelType = FIXED;
	setScale(newScale);
	offsetX = newOffsetX;
	offsetY = newOffsetY;
}

/**
*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
*  Fixed-precision coordinates are represented as precise internal coordinates,
*  which are rounded to the grid defined by the scale factor.
*
*@param  scale    amount by which to multiply a coordinate after subtracting
*      the offset, to obtain a precise coordinate
*/
PrecisionModel::PrecisionModel(double newScale) {
	modelType=FIXED;
	setScale(scale);
	offsetX=0;
	offsetY=0;
}


PrecisionModel::PrecisionModel(const PrecisionModel &pm) {
	modelType = pm.modelType;
	scale = pm.scale;
	offsetX = pm.offsetX;
	offsetY = pm.offsetY;
}

bool PrecisionModel::isFloating() const {
	return modelType == FLOATING;
}

/**
*  Returns the multiplying factor used to obtain a precise coordinate.
* This method is private because PrecisionModel is intended to
* be an immutable (value) type.
*
*@return    the amount by which to multiply a coordinate after subtracting
*      the offset
*/
double PrecisionModel::getScale() const {
	return scale;
}

/**
*  Sets the multiplying factor used to obtain a precise coordinate.
* This method is private because PrecisionModel is intended to
* be an immutable (value) type.
*
*/
void PrecisionModel::setScale(double newScale) {
	scale=fabs(newScale);
}

double PrecisionModel::getOffsetX() const {
	return offsetX;
}

double PrecisionModel::getOffsetY() const {
	return offsetY;
}

void PrecisionModel::toInternal (const Coordinate& external, Coordinate* internal) const {
	if (isFloating()) {
		internal->x = external.x;
		internal->y = external.y;
	} else {
		internal->x=makePrecise(external.x);
		internal->y=makePrecise(external.y);
	}
	internal->z = external.z;
}

Coordinate* PrecisionModel::toInternal(const Coordinate& external) const {
	Coordinate* internal=new Coordinate();
	toInternal(external, internal);
	return internal;
}

Coordinate* PrecisionModel::toExternal(const Coordinate& internal) const {
	Coordinate* external=new Coordinate();
	toExternal(internal, external);
	return external;
}

void PrecisionModel::toExternal(const Coordinate& internal, Coordinate* external) const {
	external->x = internal.x;
	external->y = internal.y;
}
  
string PrecisionModel::toString() const {
	string result("");
	char buffer[255];
    if (isFloating()) {
		result="Floating";
	}
    else {
		sprintf(buffer,"Fixed (Scale=%g, Offset X=%g, Offset Y=%g)",scale,offsetX,offsetY);
		result.append(buffer);
		result.append("");
    }
	return result;
}

PrecisionModel::~PrecisionModel(){}

bool operator==(const PrecisionModel a, const PrecisionModel b) {
	return a.isFloating() == b.isFloating() &&
			a.getOffsetX() == b.getOffsetX() &&
			a.getOffsetY() == b.getOffsetY() &&
			a.getScale() == b.getScale();
}

/**
*  Compares this {@link PrecisionModel} object with the specified object for order.
* A PrecisionModel is greater than another if it provides greater precision.
*
*@param  o  the <code>PrecisionModel</code> with which this <code>PrecisionModel</code>
*      is being compared
*@return    a negative integer, zero, or a positive integer as this <code>PrecisionModel</code>
*      is less than, equal to, or greater than the specified <code>PrecisionModel</code>
*/
int PrecisionModel::compareTo(const PrecisionModel *other) const {
	if (modelType==FLOATING && other->modelType==FLOATING) return 0;
	if (modelType==FLOATING && other->modelType!=FLOATING) return 1;
	if (modelType!=FLOATING && other->modelType==FLOATING) return -1;
	if (modelType==FIXED && other->modelType==FIXED) {
		if (scale>other->scale)
			return 1;
		else if (scale<other->scale)
			return -1;
		else
			return 0;
	}
	Assert::shouldNeverReachHere("Unknown Precision Model type encountered");
	return 0;
}
}

