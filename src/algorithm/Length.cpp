/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Length.java @ 2017-09-04
 *
 **********************************************************************/

#include <cmath>
#include <vector>

#include <geos/algorithm/Length.h>

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
double
Length::ofLine(const geom::CoordinateSequence* pts)
{
    // optimized for processing CoordinateSequences
    std::size_t n = pts->size();
    if(n <= 1) {
        return 0.0;
    }

    double len = 0.0;
    
    #ifdef HAVE_OPEN_SIMD
    #pragma omp simd reduction(+:len)
    #endif
    for(std::size_t i = 1; i < n; i++) {
        const geom::CoordinateXY& pi = pts->getAt<geom::CoordinateXY>(i);
        const geom::CoordinateXY& pi_1 = pts->getAt<geom::CoordinateXY>(i-1);
        
        double dx = pi.x - pi_1.x;
        double dy = pi.y - pi_1.y;

        len += std::sqrt(dx * dx + dy * dy);
    }
    return len;
}

double
Length::ofLine(const std::vector<geom::CoordinateXY>& pts)
{
    if (pts.size() < 2) {
        return 0;
    }

    double len = 0;
    
    #ifdef HAVE_OPEN_SIMD
    #pragma omp simd reduction(+:len)
    #endif
    for(std::size_t i = 1; i < pts.size(); i++) {
        const geom::CoordinateXY& pi = pts[i];
        const geom::CoordinateXY& pi_1 = pts[i-1];
        
        double dx = pi.x - pi_1.x;
        double dy = pi.y - pi_1.y;

        len += std::sqrt(dx * dx + dy * dy);
    }

    return len;
}

} // namespace geos.algorithm
} //namespace geos

