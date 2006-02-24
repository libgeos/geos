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
 **********************************************************************/

#include <sstream>
#include <geos/geom.h>
#include <geos/util.h>

namespace geos {

const double PrecisionModel::maximumPreciseValue=9007199254740992.0;



/**
* Rounds an numeric value to the PrecisionModel grid.
*/
double PrecisionModel::makePrecise(double val) const {
	if (modelType==FLOATING_SINGLE) {
		float floatSingleVal = (float) val;
		return (double) floatSingleVal;
	}
	if (modelType == FIXED) {
		//double d=val*scale;
		//double me=((d >= 0.0) ? floor(d+0.5)/scale : - floor(-d+0.5)/scale);
//		double ret = nearbyint(val*scale)/scale;
//		double ret = util::rint_vc(val*scale)/scale;
		double ret = util::sym_round(val*scale)/scale;
		return ret;
	}
	// modelType == FLOATING - no rounding necessary
	return val;
}

/**
* Rounds given Coordinate to the PrecisionModel grid.
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
PrecisionModel::PrecisionModel(): modelType(FLOATING), scale(1.0)
{
	//modelType=FLOATING;
	//scale=1.0;
}

/**
* Creates a <code>PrecisionModel</code> that specifies
* an explicit precision model type.
* If the model type is FIXED the scale factor will default to 1.
*
* @param modelType the type of the precision model
*/
PrecisionModel::PrecisionModel(Type nModelType): modelType(nModelType)
{
	//modelType=nModelType;
	if (modelType==FIXED){
		setScale(1.0);
	}
}


/*
 * Creates a <code>PrecisionModel</code> that specifies Fixed precision.
 * Fixed-precision coordinates are represented as precise internal coordinates,
 * which are rounded to the grid defined by the scale factor.
 *
 * @param  scale    amount by which to multiply a coordinate after subtracting
 *      the offset, to obtain a precise coordinate
 * @param  offsetX  not used.
 * @param  offsetY  not used.
 *
 * @deprecated offsets are no longer supported,
 * since internal representation is rounded floating point
 */
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY): modelType(FIXED)
	//throw(IllegalArgumentException *)
{
	//modelType = FIXED;
	setScale(newScale);
}

/*
 *  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
 *  Fixed-precision coordinates are represented as precise internal coordinates,
 *  which are rounded to the grid defined by the scale factor.
 *
 *@param  scale    amount by which to multiply a coordinate after subtracting
 *      the offset, to obtain a precise coordinate
 */
PrecisionModel::PrecisionModel(double newScale): modelType(FIXED)
	//throw (IllegalArgumentException *)
{
	//modelType=FIXED;
	setScale(newScale);
}


PrecisionModel::PrecisionModel(const PrecisionModel &pm):
	modelType(pm.modelType), scale(pm.scale)
{
	//modelType = pm.modelType;
	//scale = pm.scale;
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
PrecisionModel::Type PrecisionModel::getType() const {
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
	if ( newScale == 0 ) throw  IllegalArgumentException("PrecisionModel scale cannot be 0"); 
	scale=fabs(newScale);
}

/*
 * Returns the x-offset used to obtain a precise coordinate.
 *
 * @return the amount by which to subtract the x-coordinate before
 *         multiplying by the scale
 * @deprecated Offsets are no longer used
 */
double PrecisionModel::getOffsetX() const {
	return 0;
}

/*
 * Returns the y-offset used to obtain a precise coordinate.
 *
 * @return the amount by which to subtract the y-coordinate before
 *         multiplying by the scale
 * @deprecated Offsets are no longer used
 */
double PrecisionModel::getOffsetY() const {
	return 0;
}

/*
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

/*
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

/*
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

/*
 * Sets <code>external</code> to the external representation of
 * <code>internal</code>.
 *
 *@param  internal  the original coordinate
 *@param  external  the coordinate whose values will be changed to the
 *      external representation of <code>internal</code>
 * @deprecated no longer needed, since internal representation is same
 * as external representation
 */
void PrecisionModel::toExternal(const Coordinate& internal, Coordinate* external) const {
	external->x = internal.x;
	external->y = internal.y;
}
  
string PrecisionModel::toString() const {
	ostringstream s;
  	if (modelType == FLOATING) {
  		s<<"Floating";
  	} else if (modelType == FLOATING_SINGLE) {
  		s<<"Floating-Single";
  	} else if (modelType == FIXED) {
		s<<"Fixed (Scale="<<getScale()<<")";
	} else {
		s<<"UNKNOWN";
	}
	return s.str();
}

bool operator==(const PrecisionModel& a, const PrecisionModel& b) {
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

/**********************************************************************
 * $Log$
 * Revision 1.33  2006/02/24 16:20:14  strk
 * Added Mateusz implementation of round() in a new math.cpp file
 * named sym_round(). Changed use of rint_vc to sym_round in PrecisionModel.
 * Moved rint_vc to math.cpp (geos::util namespace), to be renamed
 * to something more meaningful
 *
 * Revision 1.32  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.31  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.30  2005/09/26 12:23:51  strk
 * Initializzation lists in PrecisionModel ctors
 *
 * Revision 1.29  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 **********************************************************************/

