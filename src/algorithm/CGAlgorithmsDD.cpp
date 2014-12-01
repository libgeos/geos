/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mateusz Loskot <mateusz@loskot.net>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithmsDD.java r789 (JTS-1.14)
 *
 **********************************************************************/
//#define GEOS_ENABLE_DD_TTMATH 1
#define GEOS_ENABLE_DD_DOUBLEDOUBLE 1

#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/geom/Coordinate.h>
#if defined(GEOS_ENABLE_DD_TTMATH)
#include "ttmath/ttmath.h"
#elif defined(GEOS_ENABLE_DD_DOUBLEDOUBLE)
#include "doubledouble/doubledouble.h"
#endif
#include <sstream>

using namespace geos::geom;

namespace {
    double const DP_SAFE_EPSILON =  1e-15;

#if defined(GEOS_ENABLE_DD_TTMATH)
    // Close to DoubleDouble equivalent used by JTS
    typedef ttmath::Big<TTMATH_BITS(32), TTMATH_BITS(128)> DD;
    //typedef ttmath::Big<TTMATH_BITS(64), TTMATH_BITS(128)> DD;
    //typedef ttmath::Big<TTMATH_BITS(32), TTMATH_BITS(256)> DD;
    //typedef ttmath::Big<TTMATH_BITS(64), TTMATH_BITS(256)> DD;
    
#elif defined(GEOS_ENABLE_DD_DOUBLEDOUBLE)
    typedef doubledouble DD;
#else
#error "Missing DoubleDouble implementation"
#endif

    inline int SigNumDD(DD const& dd)
    {
        static DD const zero(0.0);
        if (dd < zero) {
            return -1;
        }
        if (dd > zero) {
            return 1;
        }
        return 0;
    }

    inline std::string ToStringDD(DD const& dd)
    {
#if defined(GEOS_ENABLE_DD_TTMATH)
        return dd.ToString();
#else
        std::ostringstream oss;
        oss << dd;
        return oss.str();
#endif
    }
}

namespace geos {
namespace algorithm { // geos::algorithm

int CGAlgorithmsDD::orientationIndex(const Coordinate& p1,const Coordinate& p2,const Coordinate& q)
{
    // fast filter for orientation index
    // avoids use of slow extended-precision arithmetic in many cases
    //int index = orientationIndexFilter(p1, p2, q);
    //if (index <= 1) return index;

    // normalize coordinates
    DD dx1 = DD(p2.x) + DD(-p1.x);
    DD dy1 = DD(p2.y) + DD(-p1.y);
    DD dx2 = DD(q.x) + DD(-p2.x);
    DD dy2 = DD(q.y) + DD(-p2.y);

    DD mx1y2(dx1 * dy2);
    DD my1x2(dy1 * dx2);
    DD d = mx1y2 - my1x2;
    // sign of determinant - unrolled for performance
    //std::cout << ToStringDD(d) << std::endl;
    return SigNumDD(d);
}

int CGAlgorithmsDD::orientationIndexFilter(const Coordinate& pa, const Coordinate& pb, const Coordinate& pc)
{
    double detsum;

    double const detleft = (pa.x - pc.x) * (pb.y - pc.y);
    double const detright = (pa.y - pc.y) * (pb.x - pc.x);
    double const det = detleft - detright;

    if (detleft > 0.0) {
        if (detright <= 0.0) {
            return signum(det);
        } else {
            detsum = detleft + detright;
        }
    } else if (detleft < 0.0) {
        if (detright >= 0.0) {
            return signum(det);
        } else {
            detsum = -detleft - detright;
        }
    } else {
        return signum(det);
    }

    double const errbound = DP_SAFE_EPSILON * detsum;
    if ((det >= errbound) || (-det >= errbound)) {
        return signum(det);
    }

    return 2;
}

} // namespace geos::algorithm
} // namespace geos
