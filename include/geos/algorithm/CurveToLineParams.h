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

    CurveToLineParams() :
      m_maxStepDegrees(4.0),
      m_maxDeviation(DoubleInfinity) {}

    CurveToLineParams& setMaxStepDegrees(double value) {
        if (!(value > 0)) {
            throw util::IllegalArgumentException("Step size must be positive");
        }

        m_maxStepDegrees = value;

        return *this;
    }

    CurveToLineParams& setMaxDeviation(double value) {
        if (!(value > 0)) {
            throw util::IllegalArgumentException("Max deviation must be positive");
        }

        m_maxDeviation = value;

        return *this;
    }

    static CurveToLineParams maxDeviation(double dev) {
        return CurveToLineParams()
          .setMaxStepDegrees(DoubleInfinity)
          .setMaxDeviation(dev);
    }

    static CurveToLineParams stepSizeDegrees(double stepSize) {
        return CurveToLineParams()
          .setMaxStepDegrees(stepSize)
          .setMaxDeviation(DoubleInfinity);
    }

    double getStepSizeDegrees(const geom::CircularArc& arc) const {
        const double deviationRatio = 1 - m_maxDeviation / arc.getRadius();
        const double maxStepFromDeviation = deviationRatio < 0 ? 180 : std::acos(deviationRatio) * 360 / MATH_PI;

        return std::min(m_maxStepDegrees, maxStepFromDeviation);
    }

private:

    double m_maxStepDegrees;
    double m_maxDeviation;

};

}