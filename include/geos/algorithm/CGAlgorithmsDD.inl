/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mateusz Loskot <mateusz@loskot.net>
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

#ifndef GEOS_ALGORITHM_CGALGORITHMDD_INL
#define GEOS_ALGORITHM_CGALGORITHMDD_INL

namespace geos {
namespace algorithm {

INLINE int
CGAlgorithmsDD::orientationIndexFilter(double pax, double pay,
                                       double pbx, double pby,
                                       double pcx, double pcy)
{
    /**
     * A value which is safely greater than the relative round-off
     * error in double-precision numbers
     */
    double constexpr DP_SAFE_EPSILON =  1e-15;

    double detsum;
    double const detleft = (pax - pcx) * (pby - pcy);
    double const detright = (pay - pcy) * (pbx - pcx);
    double const det = detleft - detright;

    if(detleft > 0.0) {
        if(detright <= 0.0) {
            return orientation(det);
        }
        else {
            detsum = detleft + detright;
        }
    }
    else if(detleft < 0.0) {
        if(detright >= 0.0) {
            return orientation(det);
        }
        else {
            detsum = -detleft - detright;
        }
    }
    else {
        return orientation(det);
    }

    double const errbound = DP_SAFE_EPSILON * detsum;
        if((det >= errbound) || (-det >= errbound)) {
            return orientation(det);
        }
        return CGAlgorithmsDD::FAILURE;
    }


}
}

#endif
