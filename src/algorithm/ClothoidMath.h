/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Jeroen Bloemscheer
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>

#include <cstddef>

namespace geos {
namespace algorithm {
namespace clothoid {

// One Gauss panel per this many radians of heading change.
constexpr double QUADRATURE_PANEL_RADIANS = 0.25;

// Past this the turn is not a practical curve (4096 * 0.25 = 1024 rad).
constexpr std::size_t QUADRATURE_MAX_PANELS = 4096;

constexpr double MAX_TURN_RADIANS =
    static_cast<double>(QUADRATURE_MAX_PANELS) * QUADRATURE_PANEL_RADIANS;

// |A| below this underflows a constructive coordinate (k = 1e308 -> A ~ 1e-154).
constexpr double MIN_ABS_SCALE = 1e-100;

// |A| above this overflows a unit constructive coordinate.
constexpr double MAX_ABS_SCALE = 1e100;

struct Parameters {
    double x0;
    double y0;
    double theta0;
    double k0;
    double k1;
    double length;
};

geom::CoordinateXY position(const Parameters& curve, double s);

geom::CoordinateXY constructiveOffset(double t, double a2);

std::size_t linearizeSegmentCount(
    const Parameters& curve,
    double maxStepRadians,
    double maxDeviation);

}
}
}
