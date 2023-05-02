/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/CoverageUnion.h>

#include <geos/noding/SegmentExtractingNoder.h>
#include <geos/noding/BoundaryChainNoder.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/geom/Geometry.h>
#include <geos/util/TopologyException.h>

using geos::geom::Geometry;
using geos::noding::SegmentExtractingNoder;
using geos::noding::BoundaryChainNoder;

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/* public static */
std::unique_ptr<Geometry>
CoverageUnion::geomunion(const Geometry* coverage)
{
    double area_in = coverage->getArea();
    std::unique_ptr<Geometry> result;

    // a precision model is not needed since no noding is done
    //-- linear networks require a segment-extracting noder
    if (coverage->getDimension() < 2) {
        SegmentExtractingNoder sen;
        result = OverlayNG::geomunion(coverage, nullptr, &sen);
    }
    else {
        BoundaryChainNoder bcn;
        result = OverlayNG::geomunion(coverage, nullptr, &bcn);
    }

    double area_out = result->getArea();

    if (std::abs((area_out - area_in)/area_in) > AREA_PCT_DIFF_TOL) {
        throw geos::util::TopologyException("CoverageUnion cannot process overlapping inputs.");
    }

    return result;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
