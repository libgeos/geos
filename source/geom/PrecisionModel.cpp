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
 * Revision 1.18  2004/04/14 09:38:10  strk
 * PrecisionModel(double newScale) missed to set the scale
 *
 * Revision 1.17  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.16  2003/11/12 17:15:05  strk
 * made sure PrecisionModel scale is never 0
 *
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

map<string,PrecisionModel::Type*>* PrecisionModel::Type::nameToTypeMap=new map<string,PrecisionModel::Type*>();
PrecisionModel::Type* PrecisionModel::FIXED=new PrecisionModel::Type("FIXED");
PrecisionModel::Type* PrecisionModel::FLOATING=new PrecisionModel::Type("FLOATING");
PrecisionModel::Type* PrecisionModel::FLOATING_SINGLE=new PrecisionModel::Type("FLOATING SINGLE");
PrecisionModel::Type::Type(string newName) { 
	name=newName;
	nameToTypeMap->insert(pair<string,PrecisionModel::Type*>(name, this));
}

void* PrecisionModel::Type::readResolve() {
	return nameToTypeMap->find(name)->second;
}    

string PrecisionModel::Type::toString() {
	return name;
}

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
	if (modelType==FLOATING_SINGLE) {
		float floatSingleVal = (float) val;
		return (double) floatSingleVal;
	}
	if (modelType == FIXED) {
		double v=val*scale;
		if (val>=0.0) 
			v=floor(v+0.5);
		else 
			v=-floor(-v+0.5);
		return v/scale;
	}
	// modelType == FLOATING - no rounding necessary
	return val;
}

/**
* Rounds a Coordinate to the PrecisionModel grid.
*/
void PrecisionModel::makePrecise(Coordinate *coord) const {
    // optimization for full precision
	if (modelType==FLOATING) return;
	coord->x=makePrecise(coord->x);
	coord->y=makePrecise(coord->y);
}


	/**
	* Creates a <code>PrecisionModel</code> with a default precision
	* of FLOATING.
	*/
PrecisionModel::PrecisionModel(){
	modelType=FLOATING;
	scale=1.0;
}

/**
* Creates a <code>PrecisionModel</code> that specifies
* an explicit precision model type.
* If the model type is FIXED the scale factor will default to 1.
*
* @param modelType the type of the precision model
*/
PrecisionModel::PrecisionModel(Type* nModelType){
	modelType=nModelType;
	if (modelType==FIXED){
		setScale(1.0);
	}
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
* @deprecated offsets are no longer supported, since internal representation is rounded floating point
*/
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY)
	//throw(IllegalArgumentException *)
{
	modelType = FIXED;
	setScale(newScale);
}

/**
*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
*  Fixed-precision coordinates are represented as precise internal coordinates,
*  which are rounded to the grid defined by the scale factor.
*
*@param  scale    amount by which to multiply a coordinate after subtracting
*      the offset, to obtain a precise coordinate
*/
PrecisionModel::PrecisionModel(double newScale)
	//throw (IllegalArgumentException *)
{
	modelType=FIXED;
	setScale(newScale);
}


PrecisionModel::PrecisionModel(const PrecisionModel &pm)
{
	modelType = pm.modelType;
	scale = pm.scale;
}

/**
* Tests whether the precision model supports floating point
* @return <code>true</code> if the precision model supports floating point
*/
bool PrecisionModel::isFloating() const {
	return (modelType == FLOATING || modelType == FLOATING_SINGLE);
}

/**
* Returns the maximum number of significant digits provided by this
* precision model.
* Intended for use by routines which need to print out precise values.
*
* @return the maximum number of decimal places provided by this precision model
*/
int PrecisionModel::getMaximumSignificantDigits() const {
	int maxSigDigits = 16;
	if (modelType == FLOATING) {
		maxSigDigits = 16;
	} else if (modelType == FLOATING_SINGLE) {
		maxSigDigits = 6;
	} else if (modelType == FIXED) {
		maxSigDigits = 1 + (int)ceil((double)log(getScale())/(double)log(10.0));
	}
	return maxSigDigits;
}

/**
* Gets the type of this PrecisionModel
* @return the type of this PrecisionModel
*/
PrecisionModel::Type* PrecisionModel::getType(){
	return modelType;
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
	if ( newScale == 0 ) throw new IllegalArgumentException("PrecisionModel scale cannot be 0"); 
	scale=fabs(newScale);
}

/**
* Returns the x-offset used to obtain a precise coordinate.
*
* @return the amount by which to subtract the x-coordinate before
*         multiplying by the scale
* @deprecated Offsets are no longer used
*/
double PrecisionModel::getOffsetX() const {
	return 0;
}

/**
* Returns the y-offset used to obtain a precise coordinate.
*
* @return the amount by which to subtract the y-coordinate before
*         multiplying by the scale
* @deprecated Offsets are no longer used
*/
double PrecisionModel::getOffsetY() const {
	return 0;
}

/**
*  Sets <code>internal</code> to the precise representation of <code>external</code>.
*
* @param external the original coordinate
* @param internal the coordinate whose values will be changed to the
*                 precise representation of <code>external</code>
* @deprecated use makePrecise instead
*/
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

/**
*  Returns the precise representation of <code>external</code>.
*
*@param  external  the original coordinate
*@return           the coordinate whose values will be changed to the precise
*      representation of <code>external</code>
* @deprecated use makePrecise instead
*/
Coordinate* PrecisionModel::toInternal(const Coordinate& external) const {
	Coordinate* internal=new Coordinate(external);
	makePrecise(internal);
	return internal;
}

/**
*  Returns the external representation of <code>internal</code>.
*
*@param  internal  the original coordinate
*@return           the coordinate whose values will be changed to the
*      external representation of <code>internal</code>
* @deprecated no longer needed, since internal representation is same as external representation
*/
Coordinate* PrecisionModel::toExternal(const Coordinate& internal) const {
	Coordinate* external=new Coordinate(internal);
	return external;
}

/**
*  Sets <code>external</code> to the external representation of <code>internal</code>
*  .
*
*@param  internal  the original coordinate
*@param  external  the coordinate whose values will be changed to the
*      external representation of <code>internal</code>
* @deprecated no longer needed, since internal representation is same as external representation
*/
void PrecisionModel::toExternal(const Coordinate& internal, Coordinate* external) const {
	external->x = internal.x;
	external->y = internal.y;
}
  
string PrecisionModel::toString() const {
	string result("");
	char buffer[255];
	result="UNKNOWN";
  	if (modelType == FLOATING) {
  		result = "Floating";
  	} else if (modelType == FLOATING_SINGLE) {
  		result = "Floating-Single";
  	} else if (modelType == FIXED) {
		sprintf(buffer,"Fixed (Scale=%g)",getScale());
		result="";
		result.append(buffer);
		result.append("");
    }
	return result;
}

PrecisionModel::~PrecisionModel(){}

bool operator==(const PrecisionModel a, const PrecisionModel b) {
	return a.isFloating() == b.isFloating() &&
			a.getScale() == b.getScale();
}

/**
*  Compares this {@link PrecisionModel} object with the specified object for order.
* A PrecisionModel is greater than another if it provides greater precision.
* The comparison is based on the value returned by the
* {@link getMaximumSignificantDigits) method.
* This comparison is not strictly accurate when comparing floating precision models
* to fixed models; however, it is correct when both models are either floating or fixed.
*
*@param  o  the <code>PrecisionModel</code> with which this <code>PrecisionModel</code>
*      is being compared
*@return    a negative integer, zero, or a positive integer as this <code>PrecisionModel</code>
*      is less than, equal to, or greater than the specified <code>PrecisionModel</code>
*/
int PrecisionModel::compareTo(const PrecisionModel *other) const {
	int sigDigits=getMaximumSignificantDigits();
	int otherSigDigits=other->getMaximumSignificantDigits();
	return sigDigits<otherSigDigits? -1: (sigDigits==otherSigDigits? 0:1);
	//if (modelType==FLOATING && other->modelType==FLOATING) return 0;
	//if (modelType==FLOATING && other->modelType!=FLOATING) return 1;
	//if (modelType!=FLOATING && other->modelType==FLOATING) return -1;
	//if (modelType==FIXED && other->modelType==FIXED) {
	//	if (scale>other->scale)
	//		return 1;
	//	else if (scale<other->scale)
	//		return -1;
	//	else
	//		return 0;
	//}
	//Assert::shouldNeverReachHere("Unknown Precision Model type encountered");
	//return 0;
}
}

