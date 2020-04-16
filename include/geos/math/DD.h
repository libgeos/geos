/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Crunchy Data
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_MATH_DD_H
#define GEOS_MATH_DD_H

#include <cmath>

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
        double SPLIT = 134217729.0; // 2^27+1, for IEEE double
        double hi;
        double lo;

        int magnitude(double x) const;
        int signum() const;
        DD rint() const;


    public:
        DD(double p_hi, double p_lo) : hi(p_hi), lo(p_lo) {};
        DD(double x) : hi(x), lo(0.0) {};
        DD(const DD &dd) : hi(dd.hi), lo(dd.lo) {};
        DD() : hi(0.0), lo(0.0) {};

        bool operator==(const DD &rhs) const
        {
            return hi == rhs.hi && lo == rhs.lo;
        }

        bool operator!=(const DD &rhs) const
        {
            return hi != rhs.hi || lo != rhs.lo;
        }

        bool operator<(const DD &rhs) const
        {
            return (hi < rhs.hi) || (hi == rhs.hi && lo < rhs.lo);
        }

        bool operator<=(const DD &rhs) const
        {
            return (hi < rhs.hi) || (hi == rhs.hi && lo <= rhs.lo);
        }

        bool operator>(const DD &rhs) const
        {
            return (hi > rhs.hi) || (hi == rhs.hi && lo > rhs.lo);
        }

        bool operator>=(const DD &rhs) const
        {
            return (hi > rhs.hi) || (hi == rhs.hi && lo >= rhs.lo);
        }

        friend DD operator+ (const DD &lhs, const DD &rhs);
        friend DD operator+ (const DD &lhs, double rhs);
        friend DD operator- (const DD &lhs, const DD &rhs);
        friend DD operator- (const DD &lhs, double rhs);
        friend DD operator* (const DD &lhs, const DD &rhs);
        friend DD operator* (const DD &lhs, double rhs);
        friend DD operator/ (const DD &lhs, const DD &rhs);
        friend DD operator/ (const DD &lhs, double rhs);

        static DD determinant(const DD &x1, const DD &y1, const DD &x2, const DD &y2);
        static DD determinant(double x1, double y1, double x2, double y2);
        static DD abs(const DD &d);
        static DD pow(const DD &d, int exp);
        static DD trunc(const DD &d);

        bool isNaN() const;
        bool isNegative() const;
        bool isPositive() const;
        bool isZero() const;
        double doubleValue() const;
        double ToDouble() const { return doubleValue(); }
        int intValue() const;
        DD negate() const;
        DD reciprocal() const;
        DD floor() const;
        DD ceil() const;

        void selfAdd(const DD &d);
        void selfAdd(double p_hi, double p_lo);
        void selfAdd(double y);

        void selfSubtract(const DD &d);
        void selfSubtract(double p_hi, double p_lo);
        void selfSubtract(double y);

        void selfMultiply(double p_hi, double p_lo);
        void selfMultiply(const DD &d);
        void selfMultiply(double y);

        void selfDivide(double p_hi, double p_lo);
        void selfDivide(const DD &d);
        void selfDivide(double y);
};


} // namespace geos::math
} // namespace geos


#endif // GEOS_MATH_DD_H
