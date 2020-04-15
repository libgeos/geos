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

        // DD parse(std::string &str);
        int magnitude(double x);

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

        bool isNaN() const;
        bool isNegative() const;
        bool isPositive() const;
        bool isZero() const;
        double doubleValue() const;
        int intValue() const;

        DD negate() const;
        DD reciprocal() const;
        DD floor() const;
        DD ceil() const;
        int signum() const;
        DD rint() const;
        DD trunc() const;
        DD abs() const;
        DD sqr() const;

        void selfSqr();

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

        double ToDouble() const { return doubleValue(); }

};

} // namespace geos::math
} // namespace geos


#endif // GEOS_MATH_DD_H
