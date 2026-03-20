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

namespace geos::algorithm {

class GEOS_DLL LineToCurveParams {

public:

    // Get the radius tolerance. For p[x] to be considered a continuation of the same arc,
    // its radius must be within tol * the arc radius as computed from p[0], p[1], p[2].
    double getRadiusTolerance() const {
        return radiusTolerance;
    }

    void setRadiusTolerance(double tol) {
        if (!(tol > 0)) {
            throw util::IllegalArgumentException("Radius tolerance must be positive");
        }
        radiusTolerance = tol;
    }

    double getMaxExteriorAngleDifferenceRadians() const {
        return maxExteriorAngleDifferenceRadians;
    }

    void setMaxExteriorAngleDifferenceRadians(double tol) {
        if (!(tol > 0)) {
            throw util::IllegalArgumentException("Angle tolerance must be positive");
        }
        maxExteriorAngleDifferenceRadians = tol;
    }

    void setMaxExteriorAngleDifferenceDegrees(double tol) {
        setMaxExteriorAngleDifferenceRadians(tol * MATH_PI / 180.0);
    }

    // Get the maximum angle step.
    double getMaxAngleDegrees() const {
        return maxAngleRadians * 180 / MATH_PI;
    }

    double getMaxAngleRadians() const {
        return maxAngleRadians;
    }

    void setMaxStepDegrees(double tol) {
        if (!(tol > 0)) {
            throw util::IllegalArgumentException("Angle step tolerance must be positive");
        }
        maxAngleRadians = tol * MATH_PI / 180.0;
    }

private:
    double radiusTolerance{1e-6};
    double maxAngleRadians{45.01 * MATH_PI / 180.0};
    double maxExteriorAngleDifferenceRadians{0.01};
};

}