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
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/geom/Geometry.h>

using geos::geom::Geometry;
using geos::noding::SegmentExtractingNoder;

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/* public static */
std::unique_ptr<Geometry>
CoverageUnion::geomunion(const Geometry* coverage)
{
    SegmentExtractingNoder sen;

    // a precision model is not needed since no noding is done
    return OverlayNG::geomunion(coverage, nullptr, &sen);
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
