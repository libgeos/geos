/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2015  Matthew Redmond <mredmond@strava.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/distance/DiscreteDTWDistance.h>
#include <geos/util/AssertionFailedException.h>

#include <typeinfo>
#include <cassert>
#include <vector>
#include <limits>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace distance { // geos.algorithm.distance

/* static public */
double
DiscreteDTWDistance::distance(const geom::LineString& l0, const geom::LineString& l1)
{
	  DiscreteDTWDistance dist(l0, l1);
	  return dist.compute();
}

/* private */
double
DiscreteDTWDistance::compute()
{
    // TODO @mredmond: implement FastDTW (progressive resolution coarsening and projection of optimal warp path)
    // from https://gi.cebitec.uni-bielefeld.de/teaching/2007summer/jclub/papers/Salvador2004.pdf
    // 10-100x performance gains, turns algorithm from quadratic (O(n*m)) into quasi-linear (O(sqrt(n) * sqrt(m))).
    if ( l0.isEmpty() || l1.isEmpty() )
    {
        throw util::IllegalArgumentException("Linestring arguments should be non-empty.");
    }

    const size_t n = l0.getNumPoints();
    const size_t m = l1.getNumPoints();
    const CoordinateSequence* l0_seq = l0.getCoordinatesRO();
    const CoordinateSequence* l1_seq = l1.getCoordinatesRO();

    // This can be implemented to use constant space if we only keep track of the most recently updated rows -
    // however, if you use constant space, you lose the ability to trace the path backwards through the table,
    // which may be desirable in the future.
    std::vector<std::vector<double> > dtw_table(n+1, std::vector<double> (m+1, 0));
    dtw_table[0][0] = 0;
    // Boundary conditions are cost = +infinity
    for ( size_t i = 1; i <= n; ++i)
    {
        dtw_table[i][0] = std::numeric_limits<double>::infinity();
    }
    for ( size_t i = 1; i <= m; ++i)
    {
        dtw_table[0][i] = std::numeric_limits<double>::infinity();
    }

    for ( size_t i = 1; i <= n; ++i)
    {
        for ( size_t j = 1; j <= m; ++j)
        {
            double cost = (l0_seq->getAt(i-1)).distance(l1_seq->getAt(j-1));
            double path1_cost = dtw_table[i-1][j-1];
            double path2_cost = dtw_table[i][j-1];
            double path3_cost = dtw_table[i-1][j];
            dtw_table[i][j] = cost + std::min(std::min(path1_cost, path2_cost), path3_cost);
        }
    }
    return dtw_table[n][m];
}

} // namespace geos.algorithm.distance
} // namespace geos.algorithm
} // namespace geos
