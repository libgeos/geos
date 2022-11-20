/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cassert>
#include <cmath>

namespace geos {
namespace index { // geos::index
namespace strtree { // geos::index::strtree

/// A contiguous portion of 1D-space. Used internally by SIRtree.
//
/// @see SIRtree
///
template<typename T>
class GEOS_DLL IntervalBase {
public:
    IntervalBase(T newMin, T newMax) : imin(newMin), imax(newMax) {
        assert(std::isnan(newMin) || std::isnan(newMax) || imin <= imax);
    }

    T getMin() const { return imin; }
    T getMax() const { return imax; }
    T getWidth() const { return imax - imin; }
    T getCentre() const { return (imin + imax) / 2; }
    IntervalBase<T>* expandToInclude(const IntervalBase<T>* other) {
        imax = std::max(imax, other->imax);
        imin = std::min(imin, other->imin);
        return this;
    }
    bool intersects(const IntervalBase<T>* other) const {
        return !(other->imin > imax || other->imax < imin);
    }
    bool equals(const IntervalBase<T>* other) const {
        return imin == other->imin && imax == other->imax;
    }
protected:
    T imin;
    T imax;
};

class GEOS_DLL Interval : public IntervalBase<double> {
    using IntervalBase<double>::IntervalBase;
};

class GEOS_DLL FloatInterval : public IntervalBase<float> {
public:
    FloatInterval(double min, double max) :
        IntervalBase<float>(static_cast<float>(min), static_cast<float>(max)) {
            //if (static_cast<double>(imax) < max) {
            //    imax = std::nextafter(imax, std::numeric_limits<float>::infinity());
            //}
            //if (static_cast<double>(imin) > min) {
            //    imin = std::nextafter(imin, -std::numeric_limits<float>::infinity());
            //}
    }

};


} // namespace geos::index::strtree
} // namespace geos::index
} // namespace geos

