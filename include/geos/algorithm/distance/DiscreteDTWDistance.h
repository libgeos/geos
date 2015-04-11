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
#include <geos/algorithm/distance/PointPairDistance.h>
#include <geos/algorithm/distance/DistanceToPoint.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/util/math.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequenceFilter.h>

#include <cstddef>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace geos {
  namespace algorithm {
    // class RayCrossingCounter;
  }
  namespace geom {
    class Geometry;
    class Coordinate;
    // class CoordinateSequence;
  }
  namespace index {
    namespace intervalrtree {
      // class SortedPackedIntervalRTree;
    }
  }
}

namespace geos {
namespace algorithm {
namespace distance {
/** \brief
 * An algorithm for computing a gaussian-penalized Dynamic Time Warp distance metric.
 *
 * Given two linestrings, the algorithm computes the aggregated DTW penalty
 * that represents the difference between the linestring sequences.
 * @TODO (mredmond): better documentation here
 */
class GEOS_DLL DiscreteDTWDistance
{
public:
  static double distance(const geom::LineString& l0,
                         const geom::LineString& l1);

  DiscreteDTWDistance(const geom::LineString& l0,
                      const geom::LineString& l1)
    :
    l0(l0),
    l1(l1)
  {}

private:
  // Uses class-scope variables set in the instance.
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
