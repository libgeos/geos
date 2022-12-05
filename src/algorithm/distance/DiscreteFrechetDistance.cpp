/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2016  Shinichi SUGIYAMA <shin.sugi@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: original work
 *
 **********************************************************************/

#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/geom/CoordinateSequence.h>

#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>

#include <typeinfo>
#include <cassert>
#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace distance { // geos.algorithm.distance

/* static public */
double
DiscreteFrechetDistance::distance(const geom::Geometry& g0,
                                  const geom::Geometry& g1)
{
    DiscreteFrechetDistance dist(g0, g1);
    return dist.distance();
}

/* static public */
double
DiscreteFrechetDistance::distance(const geom::Geometry& g0,
                                  const geom::Geometry& g1,
                                  double densifyFrac)
{
    DiscreteFrechetDistance dist(g0, g1);
    dist.setDensifyFraction(densifyFrac);
    return dist.distance();
}

/* public */
void DiscreteFrechetDistance::setDensifyFraction(double dFrac)
{
    // !(dFrac > 0) written that way to catch NaN
    // and test on 1.0/dFrac to avoid a potential later undefined behaviour
    // when casting to std::size_t
    if(dFrac > 1.0 || !(dFrac > 0.0) ||
       util::round(1.0 / dFrac) >
           static_cast<double>(std::numeric_limits<std::size_t>::max())) {
        throw util::IllegalArgumentException(
            "Fraction is not in range (0.0 - 1.0]");
    }

    densifyFrac = dFrac;
}

/* private */

geom::Coordinate
DiscreteFrechetDistance::getSegmentAt(const CoordinateSequence& seq, std::size_t index)
{
    if(densifyFrac > 0.0) {
        // Validity of the cast to size_t has been verified in setDensifyFraction()
        std::size_t numSubSegs =  std::size_t(util::round(1.0 / densifyFrac));
        std::size_t i = index / numSubSegs;
        std::size_t j = index % numSubSegs;
        if(i >= seq.size() - 1) {
            return seq.getAt(seq.size() - 1);
        }
        const geom::Coordinate& p0 = seq.getAt(i);
        const geom::Coordinate& p1 = seq.getAt(i + 1);

        double delx = (p1.x - p0.x) / static_cast<double>(numSubSegs);
        double dely = (p1.y - p0.y) / static_cast<double>(numSubSegs);

        double x = p0.x + static_cast<double>(j) * delx;
        double y = p0.y + static_cast<double>(j) * dely;
        Coordinate pt(x, y);
        return pt;
    }
    else {
        return seq.getAt(index);
    }
}

PointPairDistance&
DiscreteFrechetDistance::getFrechetDistance(std::vector< std::vector<PointPairDistance> >& ca, std::size_t i, std::size_t j,
        const CoordinateSequence& p, const CoordinateSequence& q)
{
    PointPairDistance p_ptDist;
    if(! ca[i][j].getIsNull()) {
        return ca[i][j];
    }
    p_ptDist.initialize(getSegmentAt(p, i), getSegmentAt(q, j));
    if(i == 0 && j == 0) {
        ca[i][j] = p_ptDist;
    }
    else if(i > 0 && j == 0) {
        PointPairDistance nextDist = getFrechetDistance(ca, i - 1, 0, p, q);
        ca[i][j] = (nextDist.getDistance() > p_ptDist.getDistance()) ? nextDist : p_ptDist;
    }
    else if(i == 0 && j > 0) {
        PointPairDistance nextDist = getFrechetDistance(ca, 0, j - 1, p, q);
        ca[i][j] = (nextDist.getDistance() > p_ptDist.getDistance()) ? nextDist : p_ptDist;
    }
    else {
        PointPairDistance d1 = getFrechetDistance(ca, i - 1, j, p, q),
                          d2 = getFrechetDistance(ca, i - 1, j - 1, p, q),
                          d3 = getFrechetDistance(ca, i, j - 1, p, q);
        PointPairDistance& minDist = (d1.getDistance() < d2.getDistance()) ? d1 : d2;
        if(d3.getDistance() < minDist.getDistance()) {
            minDist = d3;
        }
        ca[i][j] = (minDist.getDistance() > p_ptDist.getDistance()) ? minDist : p_ptDist;
    }

    return ca[i][j];
}

void
DiscreteFrechetDistance::compute(
    const geom::Geometry& discreteGeom,
    const geom::Geometry& geom)
{
    if (discreteGeom.isEmpty() || geom.isEmpty()) {
        throw util::IllegalArgumentException("DiscreteFrechetDistance called with empty inputs.");
    }

    auto lp = discreteGeom.getCoordinates();
    auto lq = geom.getCoordinates();
    std::size_t pSize, qSize;
    if(densifyFrac > 0) {
        std::size_t numSubSegs =  std::size_t(util::round(1.0 / densifyFrac));
        pSize = numSubSegs * (lp->size() - 1) + 1;
        qSize = numSubSegs * (lq->size() - 1) + 1;
    }
    else {
        pSize = lp->size();
        qSize = lq->size();
    }
    std::vector< std::vector<PointPairDistance> > ca(pSize, std::vector<PointPairDistance>(qSize));
    for(std::size_t i = 0; i < pSize; i++) {
        for(std::size_t j = 0; j < qSize; j++) {
            ca[i][j].initialize();
        }
    }
    ptDist = getFrechetDistance(ca, pSize - 1, qSize - 1, *lp, *lq);
}

} // namespace geos.algorithm.distance
} // namespace geos.algorithm
} // namespace geos
