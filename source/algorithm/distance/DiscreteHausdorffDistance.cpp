/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/distance/DiscreteHausdorffDistance.java 1.4 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/geom/CoordinateSequence.h>

//#include <geos/algorithm/CGAlgorithms.h>
//#include <geos/geom/Geometry.h>
//#include <geos/geom/Polygon.h>
//#include <geos/geom/GeometryCollection.h>
//#include <geos/geom/Location.h>
//#include <geos/geom/LineString.h>

#include <typeinfo>
#include <cassert>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace distance { // geos.algorithm.distance

void
DiscreteHausdorffDistance::MaxDensifiedByFractionDistanceFilter::filter_ro(
	const geom::CoordinateSequence& seq, size_t index)
{
    /**
     * This logic also handles skipping Point geometries
     */
    if (index == 0)
      return;

    const geom::Coordinate& p0 = seq.getAt(index - 1);
    const geom::Coordinate& p1 = seq.getAt(index);

    double delx = (p1.x - p0.x)/numSubSegs;
    double dely = (p1.y - p0.y)/numSubSegs;

    for (size_t i = 0; i < numSubSegs; ++i) {
      double x = p0.x + i*delx;
      double y = p0.y + i*dely;
      Coordinate pt(x, y);
      minPtDist.initialize();
      EuclideanDistanceToPoint::computeDistance(geom, pt, minPtDist);
      maxPtDist.setMaximum(minPtDist);
    }

}


} // namespace geos.algorithm.distance
} // namespace geos.algorithm
} // namespace geos

