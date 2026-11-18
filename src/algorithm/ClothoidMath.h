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

// Past this |t / A| the heading (t / A)^2 / 2 is beyond the 159-bit 2 pi
// used to reduce it. A clothoid on one side of the inflection with a turn
// of at most 1024 rad stays below about 2e9, and one crossing it has a turn
// bounded on each side; the limit only binds for one of zero length.
constexpr double MAX_FRESNEL_ARGUMENT = 1e15;

struct Parameters {
    double k0;
    double k1;
    double length;
};

/// cos and sin of the constructive heading (t / A)^2 / 2, to about one ulp
/// at any |t / A| up to MAX_FRESNEL_ARGUMENT.
void halfSquarePhase(double t, double scale, double& c, double& s);

/// A (C2(t / A), S2(t / A)) with C2(x) = int_0^x cos(tau^2 / 2) d tau.
geom::CoordinateXY constructiveOffset(double t, double scale);

std::size_t linearizeSegmentCount(
    const Parameters& curve,
    double maxStepRadians,
    double maxDeviation);

}
}
}
