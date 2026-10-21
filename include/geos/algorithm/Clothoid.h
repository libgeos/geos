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

#include <geos/export.h>

#include <cstddef>
#include <vector>

namespace geos {
namespace algorithm {
namespace clothoid {

/**
 * Arc-length clothoid with linear curvature.
 *
 * kappa(s) = k0 + (k1 - k0) * (s / length)
 * theta(s) = theta0 + k0 * s + 0.5 * (k1 - k0) * s^2 / length
 *
 * Position is the unit-speed integral of (cos theta, sin theta).
 */
struct GEOS_DLL Parameters {
    double x0;
    double y0;
    double theta0;
    double k0;
    double k1;
    double length;
};

struct GEOS_DLL Moments {
    double P;
    double Q;
    double R;
    double T;
    double S2c;
    double S2s;
};

struct GEOS_DLL LengthSolve {
    double length;
    int iterations;
    bool converged;
};

constexpr double LENGTH_TOLERANCE = 1e-13;
constexpr int LENGTH_MAX_ITERATIONS = 50;

/**
 * fresnelC2(x) = integral from 0 to x of cos(t^2 / 2) dt.
 * fresnelS2(x) = integral from 0 to x of sin(t^2 / 2) dt.
 *
 * Power series on [-5, 5]; 16-point Gauss-Legendre panels outside.
 * Both are odd.
 */
GEOS_DLL double fresnelC2(double x);
GEOS_DLL double fresnelS2(double x);

GEOS_DLL double heading(const Parameters& curve, double s);

GEOS_DLL double curvature(const Parameters& curve, double s);

GEOS_DLL double endHeading(const Parameters& curve);

/// Unit-speed position at station s.
GEOS_DLL void position(const Parameters& curve, double s, double& x, double& y);

/// Moment integrals of the Halley residual on tau in [0, 1].
GEOS_DLL Moments moments(double length, double k0, double k1);

/// Halley solve of f(L) = L^2 (P^2 + Q^2) - d^2 for the chord length.
GEOS_DLL LengthSolve solveLengthHalley(
    double x0, double y0, double x1, double y1,
    double k0, double k1,
    double tolerance = LENGTH_TOLERANCE,
    int maxIterations = LENGTH_MAX_ITERATIONS);

/// Solve the chord length, then rotate the local frame onto the chord.
struct GEOS_DLL ChordPlacement {
    Parameters parameters;
    LengthSolve solve;
};

GEOS_DLL ChordPlacement placeOnChord(
    double x0, double y0, double x1, double y1,
    double k0, double k1,
    double tolerance = LENGTH_TOLERANCE,
    int maxIterations = LENGTH_MAX_ITERATIONS);

/// Stations in (0, length) where theta(s) is a multiple of pi/2.
GEOS_DLL void axisExtremaStations(const Parameters& curve, std::vector<double>& stations);

/// Segment count for a uniform arc-length sample. Straight curves return 1.
GEOS_DLL std::size_t linearizeSegmentCount(
    const Parameters& curve,
    double maxStepRadians,
    double maxDeviation);

}
}
}
