/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2020 Crunchy Data
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_MATH_TOPOLOGYEXCEPTION_H
#define GEOS_MATH_TOPOLOGYEXCEPTION_H

#include <cassert>

namespace geos {
namespace math { // geos.math

/**
 * \class DD
 *
 * \brief
 * Wrapper for DoubleDouble higher precision mathematics
 * operations.
 */
class GEOS_DLL DD {
    private:
        double hi;
        double lo;

        DD parse(std::string &str);
        int magnitude(double x);

    public:
        DD(double p_hi, double p_lo) : hi(p_hi), lo(p_lo) {};
        DD(double x) : hi(x), lo(0.0) {};
        DD(DD &dd) : hi(dd.hi), lo(dd.lo) {};

        bool operator==(DD const &rhs) const
        {
            return hi == rhs.hi && lo == rhs.lo;
        }

        bool operator!=(DD const &rhs) const
        {
            return hi != rhs.hi || lo != rhs.lo;
        }

        bool operator<(DD const &rhs) const
        {
            return (hi < rhs.hi) || (hi == rhs.hi && lo < rhs.lo);
        }

        bool operator<=(DD const &rhs) const
        {
            return (hi < rhs.hi) || (hi == rhs.hi && lo <= rhs.lo);
        }

        bool operator>(DD const &rhs) const
        {
            return (hi > rhs.hi) || (hi == rhs.hi && lo > rhs.lo);
        }

        bool operator>=(DD const &rhs) const
        {
            return (hi > rhs.hi) || (hi == rhs.hi && lo >= rhs.lo);
        }

        DD operator+(DD const &rhs) const;
        DD operator+(double rhs) const;


        bool isNaN();
        bool isNegative();
        bool isPositive();
        bool isZero();
        double doubleValue();
        int intValue();

        void selfAdd(DD const &d);
        void selfAdd(double p_hi, double p_lo);
        void selfAdd(double y);
};

} // namespace geos::math
} // namespace geos


#endif // GEOS_MATH_TOPOLOGYEXCEPTION_H
