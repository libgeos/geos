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

    static LineToCurveParams getDefault() {
        return {};
    }

    // Get the radius tolerance. For p[x] to be considered a continuation of the same arc, it must
    // be within the tolerance of the arc radius as computed from p[0], p[1], p[2].
    double getRadiusTolerance() const {
        return radiusTolerance;
    }

    void setRadiusTolerance(double tol) {
        radiusTolerance = tol;
    }

    // Get the angle step tolerance. For p[x] to be considered a continuation of the same arc, the angle
    // formed by p[x-1] - center - p[x] must be within this fraction of the angle formed by
    // p[x-2] - center - p[x-1].
    double getAngleStepTolerance() const {
        return angleStepTolerance;
    }

    void setAngleStepTolerance(double tol) {
        angleStepTolerance = tol;
    }

private:
    double radiusTolerance{1e-6};
    double angleStepTolerance{1e-3};
};

}