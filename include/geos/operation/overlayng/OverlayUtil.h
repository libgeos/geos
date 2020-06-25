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

#pragma once

#include <geos/operation/overlayng/InputGeometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

#include <geos/export.h>
#include <array>
#include <memory>
#include <vector>



using namespace geos::geom;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Utility methods for overlay processing.
 *
 * @author mdavis
 *
 */
class GEOS_DLL OverlayUtil {

private:

    static constexpr double SAFE_ENV_BUFFER_FACTOR = 0.1;
    static constexpr int SAFE_ENV_GRID_FACTOR = 3;

    static bool overlapEnvelope(int opCode, const InputGeometry& inputGeom, const PrecisionModel* pm, Envelope& rsltEnvelope);
    static double safeExpandDistance(const Envelope* env, const PrecisionModel* pm);
    static void safeEnv(const Envelope* env, const PrecisionModel* pm, Envelope& rsltEnvelope);

public:

    static bool isFloating(const PrecisionModel* pm);



};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

