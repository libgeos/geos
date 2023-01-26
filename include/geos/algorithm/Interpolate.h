/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2016 Vivid Solutions Inc.
 * Copyright (C) 2023 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <cmath>

namespace geos {
namespace algorithm {

class GEOS_DLL Interpolate {

private:

    template<geom::Ordinate Ord, typename CoordType>
    static double
    interpolate(const geom::CoordinateXY& p, const CoordType& p1, const CoordType& p2)
    {
        double p1z = p1.template get<Ord>();
        double p2z = p2.template get<Ord>();

        if (std::isnan(p1z)) {
            return p2z; // may be NaN
        }
        if (std::isnan(p2z)) {
            return p1z; // may be NaN
        }
        if (p.equals2D(p1)) {
            return p1z; // not NaN
        }
        if (p.equals2D(p2)) {
            return p2z; // not NaN
        }
        double dz = p2z - p1z;
        if (dz == 0.0) {
            return p1z;
        }

        // interpolate Z from distance of p along p1-p2
        double dx = (p2.x - p1.x);
        double dy = (p2.y - p1.y);
        // seg has non-zero length since p1 < p < p2
        double seglen = (dx * dx + dy * dy);
        double xoff = (p.x - p1.x);
        double yoff = (p.y - p1.y);
        double plen = (xoff * xoff + yoff * yoff);
        double frac = std::sqrt(plen / seglen);
        double zoff = dz * frac;
        double zInterpolated = p1z + zoff;

        return zInterpolated;
    }

    template<geom::Ordinate Ord, typename C1, typename C2>
    static double
    interpolate(const geom::CoordinateXY& p, const C1& p1, const C1& p2, const C2& q1, const C2& q2)
    {
        double zp = interpolate<Ord>(p, p1, p2);
        double zq = interpolate<Ord>(p, q1, q2);

        if (std::isnan(zp)) {
          return zq; // may be NaN
        }
        if (std::isnan(zq)) {
          return zp; // may be NaN
        }

        return (zp + zq) / 2.0;
    }

    template<geom::Ordinate Ord, typename C1, typename C2>
    static double
    get(const C1& p, const C2& q)
    {
        double a = p.template get<Ord>();
        double b = q.template get<Ord>();
        if (std::isnan(a)) {
            return b;
        }
        return a;
    }

    template<geom::Ordinate Ord, typename C1, typename C2>
    static double
    getOrInterpolate(const C1& p, const C2& p1, const C2& p2)
    {
        double z = p.template get<Ord>();
        if (!std::isnan(z)) return z;
        return interpolate<Ord>(p, p1, p2);
    }

    static double
    interpolate(const geom::CoordinateXY& p, const geom::CoordinateXY& p1, const geom::CoordinateXY& p2)
    {
        (void) p; (void) p1; (void) p2;
        return DoubleNotANumber;
    }

public:
    /// Interpolate a Z value for a coordinate from two other coordinates.
    template<typename CoordType>
    static double
    zInterpolate(const geom::CoordinateXY& p, const CoordType& p1, const CoordType& p2)
    {
        return interpolate<geom::Ordinate::Z>(p, p1, p2);
    }

    /// Calculate an average interpolated Z value from two pairs of other coordinates.
    template<typename C1, typename C2>
    static double
    zInterpolate(const geom::CoordinateXY& p, const C1& p1, const C1& p2, const C2& q1, const C2& q2)
    {
        return interpolate<geom::Ordinate::Z>(p, p1, p2, q1, q2);
    }

    /// Interpolate an M value for a coordinate from two other coordinates.
    template<typename CoordType>
    static double
    mInterpolate(const geom::CoordinateXY& p, const CoordType& p1, const CoordType& p2)
    {
        return interpolate<geom::Ordinate::M>(p, p1, p2);
    }

    /// Calculate an average interpolated M value from two pairs of other coordinates.
    template<typename C1, typename C2>
    static double
    mInterpolate(const geom::CoordinateXY& p, const C1& p1, const C1& p2, const C2& q1, const C2& q2)
    {
        return interpolate<geom::Ordinate::M>(p, p1, p2, q1, q2);
    }

    /// Return the first non-NaN Z value from two coordinates, or NaN if both values are NaN.
    template<typename C1, typename C2>
    static double
    zGet(const C1& p, const C2& q)
    {
        return get<geom::Ordinate::Z>(p, q);
    }

    /// Return the first non-NaN M value from two coordinates, or NaN if both values are NaN.
    template<typename C1, typename C2>
    static double
    mGet(const C1& p, const C2& q)
    {
        return get<geom::Ordinate::M>(p, q);
    }

    /// Return a coordinates's non-NaN Z value or interpolate it from two other coordinates if it is NaN.
    template<typename C1, typename C2>
    static double
    zGetOrInterpolate(const C1& p, const C2& p1, const C2& p2)
    {
        return getOrInterpolate<geom::Ordinate::Z>(p, p1, p2);
    }

    /// Return a coordinates's non-NaN M value or interpolate it from two other coordinates if it is NaN.
    template<typename C1, typename C2>
    static double
    mGetOrInterpolate(const C1& p, const C2& p1, const C2& p2)
    {
        return getOrInterpolate<geom::Ordinate::M>(p, p1, p2);
    }

};

}
}
