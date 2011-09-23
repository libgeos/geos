/**********************************************************************
 * $Id: PrecisionModel.cpp 3292 2011-04-22 13:04:20Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
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

#ifndef GEOS_INLINE
# include <geos/geom/PrecisionModel.inl>
#endif

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

const double PrecisionModel::maximumPreciseValue=9007199254740992.0;

/*public*/
double
PrecisionModel::makePrecise(double val) const
{
#if GEOS_DEBUG
    cerr<<"PrecisionModel["<<this<<"]::makePrecise called"<<endl;
#endif

    if (modelType==FLOATING_SINGLE) {
        float floatSingleVal = static_cast<float>(val);
        return static_cast<double>(floatSingleVal);
    }
    if (modelType == FIXED) {
        // Use whatever happens to be the default rounding method
        const double ret = util::round(val*scale)/scale;
        return ret;
    }
    // modelType == FLOATING - no rounding necessary
    return val;
}

/*public*/
void
PrecisionModel::makePrecise(Coordinate& coord) const
{
	// optimization for full precision 
	if (modelType==FLOATING) return;

	coord.x=makePrecise(coord.x);
	coord.y=makePrecise(coord.y);
}


/*public*/
PrecisionModel::PrecisionModel()
	:
	modelType(FLOATING),
	scale(0.0)
{
#if GEOS_DEBUG
	cerr<<"PrecisionModel["<<this<<"] ctor()"<<endl;
#endif
	//modelType=FLOATING;
	//scale=1.0;
}

/*public*/
PrecisionModel::PrecisionModel(Type nModelType)
	:
	modelType(nModelType),
	scale(1.0)
{
#if GEOS_DEBUG
	cerr<<"PrecisionModel["<<this<<"] ctor(Type)"<<endl;
#endif
	//modelType=nModelType;
	//if (modelType==FIXED) setScale(1.0);
	//else setScale(666); // arbitrary number for invariant testing
}


/*public (deprecated) */
PrecisionModel::PrecisionModel(double newScale, double newOffsetX, double newOffsetY)
		//throw(IllegalArgumentException *)
	:
	modelType(FIXED)
{
    ::geos::ignore_unused_variable_warning(newOffsetX);
    ::geos::ignore_unused_variable_warning(newOffsetY);

#if GEOS_DEBUG
	cerr<<"PrecisionModel["<<this<<"] ctor(scale,offsets)"<<endl;
#endif

	//modelType = FIXED;
	setScale(newScale);
}

/*public*/
PrecisionModel::PrecisionModel(double newScale)
		//throw (IllegalArgumentException *)
	:
	modelType(FIXED)
{
#if GEOS_DEBUG
	cerr<<"PrecisionModel["<<this<<"] ctor(scale)"<<endl;
#endif
	setScale(newScale);
}


PrecisionModel::PrecisionModel(const PrecisionModel &pm)
	:
	modelType(pm.modelType),
	scale(pm.scale)
{
#if GEOS_DEBUG
	cerr<<"PrecisionModel["<<this<<"] ctor(pm["<< &pm <<"])"<<endl;
#endif
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
	if (modelType == FLOATING) {
		maxSigDigits = 16;
	} else if (modelType == FLOATING_SINGLE) {
		maxSigDigits = 6;
	} else if (modelType == FIXED) {

		double dgtsd = std::log(getScale()) / std::log(double(10.0));
		const int dgts = static_cast<int>(
			dgtsd > 0 ? std::ceil(dgtsd)
			          : std::floor(dgtsd)
		);
		maxSigDigits = dgts;
	}
	return maxSigDigits;
}


/*private*/
void
PrecisionModel::setScale(double newScale)
		// throw IllegalArgumentException
{
	if ( newScale <= 0 )
		throw util::IllegalArgumentException("PrecisionModel scale cannot be 0"); 
    scale = std::fabs(newScale);
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

  
string
PrecisionModel::toString() const
{
	ostringstream s;
  	if (modelType == FLOATING) {
  		s<<"Floating";
  	} else if (modelType == FLOATING_SINGLE) {
  		s<<"Floating-Single";
  	} else if (modelType == FIXED) {
		s <<"Fixed (Scale=" << getScale()
		  << " OffsetX=" << getOffsetX()
		  << " OffsetY=" << getOffsetY()
		  << ")";
	} else {
		s<<"UNKNOWN";
	}
	return s.str();
}

bool operator==(const PrecisionModel& a, const PrecisionModel& b) {
	return a.isFloating() == b.isFloating() &&
			a.getScale() == b.getScale();
}

/*public*/
int
PrecisionModel::compareTo(const PrecisionModel *other) const
{
	int sigDigits=getMaximumSignificantDigits();
	int otherSigDigits=other->getMaximumSignificantDigits();
	return sigDigits<otherSigDigits? -1: (sigDigits==otherSigDigits? 0:1);
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.46  2006/05/04 14:05:31  strk
 * * source/headers/geos/util/math.h: provided an util::round() method being an inline proxy to call appropriate default rounding function for the whole GEOS codebase. Currently pointing at util::java_math_round() being the last being used.
 *
 * Revision 1.45  2006/04/21 17:04:09  strk
 * Fixed constructor by type to always initialize scale factor
 *
 * Revision 1.44  2006/04/07 12:37:51  mloskot
 * Added java_math_round() function for Asymmetric Arithmetic Rounding. Small fixes in Unit Tests.
 *
 * Revision 1.43  2006/04/06 12:36:52  strk
 * removed spurious lines from previous commi
 *
 * Revision 1.42  2006/04/06 12:34:07  strk
 * Port info, more debugging lines, doxygen comments
 *
 * Revision 1.41  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.40  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.39  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.38  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.37  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.36  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.35  2006/02/28 14:34:03  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.34  2006/02/27 09:05:32  strk
 * Doxygen comments, a few inlines and general cleanups
 *
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

