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

#include <geos/operation/overlayng/OverlayUtil.h>




namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public static*/
bool
OverlayUtil::isFloating(const PrecisionModel* pm)
{
    if (pm == nullptr) return true;
    return pm->isFloating();
}

/*private static*/
double
OverlayUtil::safeExpandDistance(const Envelope* env, const PrecisionModel* pm)
{
    double envExpandDist;
    if (isFloating(pm)) {
        // if PM is FLOAT then there is no scale factor, so add 10%
        double minSize = std::min(env->getHeight(), env->getWidth());
        envExpandDist = SAFE_ENV_BUFFER_FACTOR * minSize;
    }
    else {
        // if PM is fixed, add a small multiple of the grid size
        double gridSize = 1.0 / pm->getScale();
        envExpandDist = SAFE_ENV_GRID_FACTOR * gridSize;
    }
    return envExpandDist;
}

/*private static*/
void
OverlayUtil::safeEnv(const Envelope* env, const PrecisionModel* pm, Envelope& rsltEnvelope)
{
    double envExpandDist = safeExpandDistance(env, pm);
    rsltEnvelope = *env;
    rsltEnvelope.expandBy(envExpandDist);
    return;
}

/*private static*/
// bool
// OverlayUtil::overlapEnvelope(int opCode, const InputGeometry& inputGeom, const PrecisionModel* pm, Envelope& rsltEnvelope)
// {
//     switch (opCode) {
//         case OverlayNG::INTERSECTION:
//             // use safe envelopes for intersection to ensure they contain rounded coordinates
//             Envelope envA, envB;
//             safeEnv(inputGeom->getEnvelope(0), pm, envA);
//             safeEnv(inputGeom->getEnvelope(1), pm, envB);
//             envA.intersection(envB, rsltEnvelope);
//             return true;
//         case OverlayNG::DIFFERENCE:
//             safeEnv(inputGeom->getEnvelope(0), pm, rsltEnvelope);
//             return true;
//     }
//     // return false for UNION and SYMDIFFERENCE to indicate no clipping
//     return false;
// }

/*public static*/
// bool
// clippingEnvelope(int opCode, const InputGeometry& inputGeom, const PrecisionModel& pm, Envelope& rsltEnvelope)
// {
//     bool overlapEnv = overlapEnvelope(opCode, inputGeom, pm);
//     if (!overlapEnv)
//       return false;

//     Envelope clipEnv = RobustClipEnvelopeComputer.getEnvelope(
//         inputGeom.getGeometry(0),
//         inputGeom.getGeometry(1),
//         overlapEnv);

//     Envelope safeEnv = safeEnv( clipEnv, pm );
//     return safeEnv;
// }


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
