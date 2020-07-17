/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/OverlayNGSnapIfNeeded.h>

#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayUtil.h>
#include <geos/operation/overlayng/PrecisionUtil.h>
#include <geos/operation/union/UnionStrategy.h>
#include <geos/operation/union/UnaryUnionOp.h>
#include <geos/noding/snap/SnappingNoder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/TopologyException.h>

#include <stdexcept>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;


/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::Intersection(const Geometry* g0, const Geometry* g1)
{
    return Overlay(g0, g1, OverlayNG::INTERSECTION);
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::Union(const Geometry* g0, const Geometry* g1)
{
    return Overlay(g0, g1, OverlayNG::UNION);
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::Difference(const Geometry* g0, const Geometry* g1)
{
    return Overlay(g0, g1, OverlayNG::DIFFERENCE);
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::SymDifference(const Geometry* g0, const Geometry* g1)
{
    return Overlay(g0, g1, OverlayNG::SYMDIFFERENCE);
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::Union(const Geometry* a)
{
    geounion::UnaryUnionOp op(*a);
    SRUnionStrategy unionSRFun;
    op.setUnionFunction(&unionSRFun);
    return op.Union();
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::Overlay(const Geometry* geom0, const Geometry* geom1, int opCode)
{
    std::unique_ptr<Geometry> result;
    std::runtime_error exOriginal("");

    /**
     * First try overlay with a FLOAT noder, which is fastest and causes least
     * change to geometry coordinates
     * By default the noder is validated, which is required in order
     * to detect certain invalid noding situations which otherwise
     * cause incorrect overlay output.
     */
    try {
        geom::PrecisionModel PM_FLOAT;
        result = OverlayNG::overlay(geom0, geom1, opCode, &PM_FLOAT);

        // Simple noding with no validation
        // There are cases where this succeeds with invalid noding (e.g. STMLF 1608).
        // So currently it is NOT safe to run overlay without noding validation
        //result = OverlayNG.overlay(geom0, geom1, opCode, createFloatingNoValidNoder());

        return result;
    }
    catch (std::runtime_error ex) {
        /**
        * Capture original exception,
        * so it can be rethrown if the remaining strategies all fail.
        */
        exOriginal = ex;
    }

    /**
     * On failure retry using snapping noding with a "safe" tolerance.
     * if this throws an exception just let it go,
     * since it is something that is not a TopologyException
     */
    result = overlaySnapTries(geom0, geom1, opCode);
    if (result != nullptr)
        return result;

    throw exOriginal;
}


/*private static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::overlaySnapTries(const Geometry* geom0, const Geometry* geom1, int opCode)
{
    std::unique_ptr<Geometry> result;
    double snapTol = snapTolerance(geom0, geom1);

    for (std::size_t i = 0; i < NUM_SNAP_TRIES; i++) {

        result = overlaySnapping(geom0, geom1, opCode, snapTol);
        if (result != nullptr) return result;

      /**
       * Now try snapping each input individually,
       * and then doing the overlay.
       */
      result = overlaySnapBoth(geom0, geom1, opCode, snapTol);
      if (result != nullptr) return result;

      // increase the snap tolerance and try again
      snapTol = snapTol * 10;
    }
    // failed to compute overlay
    return nullptr;
}

/*private static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::overlaySnapping(const Geometry* geom0, const Geometry* geom1, int opCode, double snapTol)
{
    try {
        return overlaySnapTol(geom0, geom1, opCode, snapTol);
    }
    catch (geos::util::TopologyException ex) {
        //---- ignore this exception, just return a nullptr result
    }
    return nullptr;
}

/*private static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::overlaySnapBoth(const Geometry* geom0, const Geometry* geom1, int opCode, double snapTol)
{
    try {
        std::unique_ptr<Geometry> snap0 = overlaySnapTol(geom0, nullptr, OverlayNG::UNION, snapTol);
        std::unique_ptr<Geometry> snap1 = overlaySnapTol(geom1, nullptr, OverlayNG::UNION, snapTol);
        return overlaySnapTol(snap0.get(), snap1.get(), opCode, snapTol);
    }
    catch (geos::util::TopologyException ex) {
        //---- ignore this exception, just return a nullptr result
    }
    return nullptr;
}

/*private static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::overlaySnapTol(const Geometry* geom0, const Geometry* geom1, int opCode, double snapTol)
{
    noding::snap::SnappingNoder snapNoder(snapTol);
    return OverlayNG::overlay(geom0, geom1, opCode, &snapNoder);
}

/*public static*/
double
OverlayNGSnapIfNeeded::snapTolerance(const Geometry* geom0, const Geometry* geom1)
{
    double tol0 = snapTolerance(geom0);
    double tol1 = snapTolerance(geom1);
    double snapTol = std::max(tol0,  tol1);
    return snapTol;
}

/*private static*/
double
OverlayNGSnapIfNeeded::snapTolerance(const Geometry* geom)
{
    double magnitude = ordinateMagnitude(geom);
    return magnitude / SNAP_TOL_FACTOR;
}


/*private static*/
double
OverlayNGSnapIfNeeded::ordinateMagnitude(const Geometry* geom)
{
    if (geom == nullptr) return 0;
    const Envelope* env = geom->getEnvelopeInternal();
    double magMax = std::max(
        std::abs(env->getMaxX()),
        std::abs(env->getMaxY())
        );
    double magMin = std::max(
        std::abs(env->getMinX()),
        std::abs(env->getMinY())
        );
    return std::max(magMax, magMin);
}


/*private static*/
std::unique_ptr<Geometry>
OverlayNGSnapIfNeeded::overlaySR(const Geometry* geom0, const Geometry* geom1, int opCode)
{
    std::unique_ptr<Geometry> result;
    try {
        // start with operation using floating PM
        geom::PrecisionModel PM_FLOAT;
        result = OverlayNG::overlay(geom0, geom1, opCode, &PM_FLOAT);
        return result;
    }
    catch (geos::util::TopologyException ex) {
        // ignore this exception, since the operation will be rerun
        //System.out.println("Overlay failed");
    }
    // on failure retry with a "safe" fixed PM
    // this should not throw an exception, but if it does just let it go
    double scaleSafe = PrecisionUtil::safeScale(geom0, geom1);
    PrecisionModel pmSafe(scaleSafe);
    result = OverlayNG::overlay(geom0, geom1, opCode, &pmSafe);
    return result;
}






} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
