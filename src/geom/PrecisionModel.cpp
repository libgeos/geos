/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/PrecisionModel.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/math.h>
#include <geos/util.h>

#include <sstream>
#include <string>
#include <cmath>
#include <iostream>
#include <iomanip> 

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

namespace geos {
namespace geom { // geos::geom

/**
*  The maximum precise value representable in a double. Since IEE754
*  double-precision numbers allow 53 bits of mantissa, the value is equal to
*  2^53 - 1.  This provides <i>almost</i> 16 decimal digits of precision.
*/
const double PrecisionModel::maximumPreciseValue = 9007199254740992.0;

/*public*/
double
PrecisionModel::makePrecise(double val) const
{
#if GEOS_DEBUG
    std::cerr << "PrecisionModel[" << this << "]::makePrecise called" << std::endl;
#endif

    if(modelType == FLOATING_SINGLE) {
        float floatSingleVal = static_cast<float>(val);
        return static_cast<double>(floatSingleVal);
    }
    if(modelType == FIXED) {
        //-- make arithmetic robust by using integral value if available
        if (gridSize > 1) {
//double v2 = util::round(val / gridSize) * gridSize;
//std::cout << std::setprecision(16) << "GS[" << gridSize << "] " << val << " -> "  << v2 << std::endl;
            return util::round(val / gridSize) * gridSize;
        }
        //-- since grid size is <= 1, scale must be >= 1 OR 0
        //-- if scale == 0, this is a no-op (should never happen)
        else if (scale != 0.0) {
//double v2 = util::round(val * scale) / scale;
//std::cout << std::setprecision(16) << "SC[" << scale << "] " << val << " -> " << "SC " << v2 << std::endl;
            return util::round(val * scale) / scale;
        }
    }
    // modelType == FLOATING - no rounding necessary
    return val;
}

/*public*/
PrecisionModel::PrecisionModel()
    :
    modelType(FLOATING),
    scale(0.0),
    gridSize(0.0)
{
#if GEOS_DEBUG
    std::cerr << "PrecisionModel[" << this << "] ctor()" << std::endl;
#endif
    //modelType=FLOATING;
    //scale=1.0;
}

/*public*/
PrecisionModel::PrecisionModel(Type nModelType)
    :
    modelType(nModelType),
    scale(1.0),
    gridSize(1.0)
{
#if GEOS_DEBUG
    std::cerr << "PrecisionModel[" << this << "] ctor(Type)" << std::endl;
#endif
    //modelType=nModelType;
    //if (modelType==FIXED) setScale(1.0);
    //else setScale(666); // arbitrary number for invariant testing
}


/*public (deprecated) */
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY)
    :
    modelType(FIXED)
{
    ::geos::ignore_unused_variable_warning(newOffsetX);
    ::geos::ignore_unused_variable_warning(newOffsetY);

#if GEOS_DEBUG
    std::cerr << "PrecisionModel[" << this << "] ctor(scale,offsets)" << std::endl;
#endif

    //modelType = FIXED;
    setScale(newScale);
}

/*public*/
PrecisionModel::PrecisionModel(double newScale)
    :
    modelType(FIXED)
{
#if GEOS_DEBUG
    std::cerr << "PrecisionModel[" << this << "] ctor(scale)" << std::endl;
#endif
    setScale(newScale);
}

/*public*/
bool
PrecisionModel::isFloating() const
{
    return (modelType == FLOATING || modelType == FLOATING_SINGLE);
}

/*public*/
int
PrecisionModel::getMaximumSignificantDigits() const
{
    int maxSigDigits = 16;
    if(modelType == FLOATING) {
        maxSigDigits = 16;
    }
    else if(modelType == FLOATING_SINGLE) {
        maxSigDigits = 6;
    }
    else if(modelType == FIXED) {

        double dgtsd = std::log(getScale()) / std::log(double(10.0));
        const int dgts = static_cast<int>(
                             dgtsd > 0 ? std::ceil(dgtsd)
                             : std::floor(dgtsd)
                         );
        maxSigDigits = dgts;
    }
    return maxSigDigits;
}

//-- this value is not critical, since most common usage should be VERY close to integral
const double GRIDSIZE_INTEGER_TOLERANCE = 1e-5;

/*private*/
void
PrecisionModel::setScale(double newScale)
{
    //-- should never happen, but make this a no-op in case
    if (newScale == 0) {
        scale = 0.0;
        gridSize = 0.0;
    }
    /**
    * A negative scale indicates the grid size is being set.
    * The scale is set as well, as the reciprocal.
    * NOTE: may not need to support negative grid size now due to robust arithmetic
    */
    if (newScale < 0) {
        scale = 1.0 / std::fabs(newScale);
    }
    else {
        scale = newScale;
    }
    //-- snap nearly integral scale or gridsize to exact integer
    //-- this handles the most common case of fractional powers of ten
    if (scale < 1) {
        gridSize = snapToInt(1.0 / scale, GRIDSIZE_INTEGER_TOLERANCE);
    }
    else {
        scale = snapToInt( scale, GRIDSIZE_INTEGER_TOLERANCE);
        gridSize = 1.0 / scale;
    }
}

/*private*/ 
double
PrecisionModel::snapToInt(double val, double tolerance) {
    double valInt = std::round(val);
    if (std::abs(val - valInt) < tolerance) {
        return valInt;
    }
    return val;
}

/*public*/
double
PrecisionModel::getOffsetX() const
{
    return 0;
}

/*public*/
double
PrecisionModel::getOffsetY() const
{
    return 0;
}


std::string
PrecisionModel::toString() const
{
    std::ostringstream s;
    if(modelType == FLOATING) {
        s << "Floating";
    }
    else if(modelType == FLOATING_SINGLE) {
        s << "Floating-Single";
    }
    else if(modelType == FIXED) {
        s << "Fixed (Scale=" << getScale()
          << " OffsetX=" << getOffsetX()
          << " OffsetY=" << getOffsetY()
          << ")";
    }
    else {
        s << "UNKNOWN";
    }
    return s.str();
}

bool
operator==(const PrecisionModel& a, const PrecisionModel& b)
{
    return a.isFloating() == b.isFloating() &&
           a.getScale() == b.getScale();
}

/*public*/
int
PrecisionModel::compareTo(const PrecisionModel* other) const
{
    int sigDigits = getMaximumSignificantDigits();
    int otherSigDigits = other->getMaximumSignificantDigits();
    return sigDigits < otherSigDigits ? -1 : (sigDigits == otherSigDigits ? 0 : 1);
}

} // namespace geos::geom
} // namespace geos
