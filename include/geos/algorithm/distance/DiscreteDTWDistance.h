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

#ifndef GEOS_ALGORITHM_DISTANCE_DISCRETEDTWDISTANCE_H
#define GEOS_ALGORITHM_DISTANCE_DISCRETEDTWDISTANCE_H

#include <geos/export.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/util/math.h>

#include <cstddef>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace geos {
    namespace geom {
        class Geometry;
        class Coordinate;
    }
}

namespace geos {
namespace algorithm {
namespace distance {
/** \brief
 * An algorithm for computing a Dynamic Time Warp distance metric.
 *
 * Given two linestrings, the algorithm computes the (euclidean-cost-weighted) Dynamic Time Warp value
 * that represents the aggregated difference between the linestring sequences, according to the optimal warp path.
 * Useful for curve alignment (low values mean relatively similar linestring trajectories).
 * See http://en.wikipedia.org/wiki/Dynamic_time_warping for more.
 * We implement the most naive, brute force implementation first - more performant implementations to come soon.
 *
 * Runtime: O(n*m) where input linestring1 has n elements, and input linestring2 has m elements.
 * Space: O(n*m)
 *
 * Low hanging fruit for optimization:
 * - Decimate input linestrings to reduce sizes of n, m.
 * - Implement Sakoe-Chiba band to reduce space complexity and necessary distance computations.
 * - Implement FastDTW (see link on wiki page) to reduce to quasi-linear time O(sqrt(n) * sqrt(m)).
 */
class GEOS_DLL DiscreteDTWDistance
{
  public:
      static double distance(const geom::LineString& l0, const geom::LineString& l1);

      DiscreteDTWDistance(const geom::LineString& l0, const geom::LineString& l1):
          l0(l0),
          l1(l1)
      {}

  private:
      double compute();

      const geom::LineString& l0;
      const geom::LineString& l1;

      // Declare type as noncopyable
      DiscreteDTWDistance(const DiscreteDTWDistance& other);
      DiscreteDTWDistance& operator=(const DiscreteDTWDistance& rhs);
};

} // geos::algorithm::distance
} // geos::algorithm
} // geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // GEOS_ALGORITHM_DISTANCE_DISCRETEDTWDISTANCE_H
