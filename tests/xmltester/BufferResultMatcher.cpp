/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: jtstest/testrunner/BufferResultMatcher.java rev rev 1.6 (JTS-1.11)
 *
 **********************************************************************/

#include "BufferResultMatcher.h"

#include <geos/geom/Geometry.h>
#include <geos/geom/HeuristicOverlay.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>

#include <cmath>

namespace geos {
namespace xmltester {

bool
BufferResultMatcher::isBufferResultMatch(const geom::Geometry& actualBuffer,
        const geom::Geometry& expectedBuffer,
        double distance)
{
    if(actualBuffer.isEmpty() && expectedBuffer.isEmpty()) {
        return true;
    }

    /*
     * MD - need some more checks here - symDiffArea won't catch
     * very small holes ("tears")
     * near the edge of computed buffers (which can happen
     * in current version of JTS (1.8)).
     * This can probably be handled by testing
     * that every point of the actual buffer is at least a certain
     * distance away from the geometry boundary.
     */
    if(! isSymDiffAreaInTolerance(actualBuffer, expectedBuffer)) {
        std::cerr << "isSymDiffAreaInTolerance failed" << std::endl;
        return false;
    }

    if(! isBoundaryHausdorffDistanceInTolerance(actualBuffer,
            expectedBuffer, distance)) {
        std::cerr << "isBoundaryHasudorffDistanceInTolerance failed" << std::endl;
        return false;
    }

    return true;
}

bool
BufferResultMatcher::isSymDiffAreaInTolerance(
    const geom::Geometry& actualBuffer,
    const geom::Geometry& expectedBuffer)
{
    typedef std::unique_ptr<geom::Geometry> GeomPtr;

    using namespace operation::overlay;
    using geos::geom::HeuristicOverlay;

    double area = expectedBuffer.getArea();
    GeomPtr diff = HeuristicOverlay(&actualBuffer, &expectedBuffer,
                            OverlayOp::opSYMDIFFERENCE);

    double areaDiff = diff->getArea();

    // can't get closer than difference area = 0 !
    // This also handles case when symDiff is empty
    if(areaDiff <= 0.0) {
        return true;
    }

    if(area <= 0) {
        return false;
    }
    double frac = areaDiff / area;

    bool ret = frac < MAX_RELATIVE_AREA_DIFFERENCE;
    if(! ret) {
        std::cerr << "symDiffArea frac: " << frac << " tolerated " << MAX_RELATIVE_AREA_DIFFERENCE << std::endl;
    }
    return ret;
}

bool
BufferResultMatcher::isBoundaryHausdorffDistanceInTolerance(
    const geom::Geometry& actualBuffer,
    const geom::Geometry& expectedBuffer, double distance)
{
    typedef std::unique_ptr<geom::Geometry> GeomPtr;

    using geos::algorithm::distance::DiscreteHausdorffDistance;

    GeomPtr actualBdy(actualBuffer.getBoundary());
    GeomPtr expectedBdy(expectedBuffer.getBoundary());

    DiscreteHausdorffDistance haus(*actualBdy, *expectedBdy);
    haus.setDensifyFraction(0.25);

    double maxDistanceFound = haus.orientedDistance();

    double expectedDistanceTol = fabs(distance) / MAX_HAUSDORFF_DISTANCE_FACTOR;
    if(expectedDistanceTol < MIN_DISTANCE_TOLERANCE) {
        expectedDistanceTol = MIN_DISTANCE_TOLERANCE;
    }

    if(maxDistanceFound > expectedDistanceTol) {
        std::cerr << "maxDistanceFound: " << maxDistanceFound << " tolerated " << expectedDistanceTol << std::endl;
        return false;
    }

    return true;
}

} // namespace geos::xmltester
} // namespace geos
