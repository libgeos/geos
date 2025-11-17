/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/DouglasPeuckerLineSimplifier.java rev. 1.4
 *
 **********************************************************************/

#include <geos/simplify/DouglasPeuckerLineSimplifier.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LinearRing.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>

#include <vector>
#include <memory> // for unique_ptr

using geos::geom::CoordinateSequence;

namespace geos {

/// Line simplification algorithms
namespace simplify { // geos::simplify

/*public static*/
std::unique_ptr<CoordinateSequence>
DouglasPeuckerLineSimplifier::simplify(
    const CoordinateSequence& nPts,
    double distanceTolerance,
    bool preserveClosedEndpoint)
{
    DouglasPeuckerLineSimplifier simp(nPts);
    simp.setDistanceTolerance(distanceTolerance);
    simp.setPreserveClosedEndpoint(preserveClosedEndpoint);
    return simp.simplify();
}

/*public*/
DouglasPeuckerLineSimplifier::DouglasPeuckerLineSimplifier(
    const CoordinateSequence& nPts)
    :
    pts(nPts)
{
}

/*public*/
void
DouglasPeuckerLineSimplifier::setDistanceTolerance(
    double nDistanceTolerance)
{
    if (std::isnan(nDistanceTolerance)) {
        throw util::IllegalArgumentException("Tolerance must not be NaN");
    }
    distanceTolerance = nDistanceTolerance;
}

void
DouglasPeuckerLineSimplifier::setPreserveClosedEndpoint(bool preserve)
{
    preserveEndpoint = preserve;
}

/*public*/
std::unique_ptr<CoordinateSequence>
DouglasPeuckerLineSimplifier::simplify()
{
    auto coordList = detail::make_unique<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    // empty coordlist is the simplest, won't simplify further
    if(pts.isEmpty()) {
        return coordList;
    }

    usePt = std::vector<bool>(pts.size(), true);
    simplifySection(0, pts.size() - 1);

    // Add continuous ranges of retained points from pts to coordList
    std::size_t from = 0;
    while (from < pts.size() && !usePt[from]) {
        from++;
    }
    for(std::size_t to = from + 1; to <= pts.size(); to++) {
        if (to == pts.size() || !usePt[to]) {
            coordList->add(pts, from, to - 1);
            while (to < pts.size() && !usePt[to]) {
                to++;
            }
            from = to;
        }
    }

    // TODO avoid copying entire sequence?
    bool simplifyRing = !preserveEndpoint && pts.isRing();
    if (simplifyRing && coordList->size() > geom::LinearRing::MINIMUM_VALID_SIZE) {
        geom::LineSegment seg(coordList->getAt(coordList->size() - 2), coordList->getAt(1));
        if (seg.distance(coordList->getAt(0)) <= distanceTolerance) {
            auto ret = detail::make_unique<CoordinateSequence>(0, pts.hasZ(), pts.hasM());
            ret->add(*coordList, 1, coordList->size() - 2);
            ret->closeRing();
            coordList = std::move(ret);
        }
    }

    return coordList;
}

/*private*/
void
DouglasPeuckerLineSimplifier::simplifySection(
    std::size_t i,
    std::size_t j)
{
    if((i + 1) == j) {

        return;
    }

    geom::LineSegment seg(pts[i], pts[j]);
    double maxDistance = -1.0;

    std::size_t maxIndex = i;

    for(std::size_t k = i + 1; k < j; k++) {
        double distance = seg.distance(pts[k]);
        if(distance > maxDistance) {
            maxDistance = distance;
            maxIndex = k;
        }
    }
    if(maxDistance <= distanceTolerance) {
        for(std::size_t k = i + 1; k < j; k++) {
            usePt[k] = false;
        }
    }
    else {
        simplifySection(i, maxIndex);
        simplifySection(maxIndex, j);
    }
}

} // namespace geos::simplify
} // namespace geos
