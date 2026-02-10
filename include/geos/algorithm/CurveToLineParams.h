/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/CircularArc.h>

namespace geos::algorithm {

class GEOS_DLL CurveToLineParams {

public:
    enum class TOLERANCE_TYPE {
        STEP_DEGREES,
        MAX_DEVIATION,
    };

    CurveToLineParams(TOLERANCE_TYPE tolType, double tolValue) :
        toleranceType(tolType),
        toleranceValue(tolValue) {}

    static CurveToLineParams maxDeviation(double dev) {
        return CurveToLineParams(TOLERANCE_TYPE::MAX_DEVIATION, dev);
    }

    static CurveToLineParams stepSizeDegrees(double stepSize) {
        return CurveToLineParams(TOLERANCE_TYPE::STEP_DEGREES, stepSize);
    }

    double getStepSizeDegrees(const geom::CircularArc& arc) const {
        if (toleranceType == TOLERANCE_TYPE::STEP_DEGREES) {
            return toleranceValue;
        }

        if (toleranceType == TOLERANCE_TYPE::MAX_DEVIATION) {
            return std::acos(1 - toleranceValue / arc.getRadius()) * 360 / MATH_PI;
        }

        throw util::GEOSException("Invalid tolerance type");
    }

private:
    const TOLERANCE_TYPE toleranceType;
    const double toleranceValue;

};

}