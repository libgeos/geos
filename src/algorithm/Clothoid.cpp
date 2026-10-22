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

#include <algorithm/ClothoidMath.h>

#include <geos/constants.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/IllegalArgumentException.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace geos {
namespace algorithm {
namespace clothoid {

namespace {

constexpr int FRESNEL_GL_N = 16;
// Power series is accurate on this window. Past it the terms reach
// e^(x^2 / 2) before they cancel.
constexpr double FRESNEL_SERIES_LIMIT = 3.0;
// Asymptotic expansion is accurate past this: its smallest term is
// about e^(-x^2 / 2), below 1e-17 here.
constexpr double FRESNEL_ASYMPTOTIC_LIMIT = 9.0;
// Widest Gauss panel between the two windows.
constexpr double FRESNEL_PANEL = 0.5;
// Phase change allowed across one Gauss panel.
constexpr double FRESNEL_PANEL_PHASE = 2.0;
// Stop a series when the next term is this small relative to the sum.
constexpr double SERIES_TERM_RELATIVE = 1e-18;
// Segment counts past this are not a useful linearization.
constexpr double MAX_SEGMENT_COUNT = 1.0e7;

// 16-point Gauss–Legendre nodes and weights on [-1, 1].
constexpr double FRESNEL_XI[FRESNEL_GL_N] = {
    -9.89400934991649939e-01,
        -9.44575023073232600e-01,
        -8.65631202387831755e-01,
        -7.55404408355002999e-01,
        -6.17876244402643771e-01,
        -4.58016777657227370e-01,
        -2.81603550779258915e-01,
        -9.50125098376374544e-02,
        9.50125098376374544e-02,
        2.81603550779258915e-01,
        4.58016777657227370e-01,
        6.17876244402643771e-01,
        7.55404408355002999e-01,
        8.65631202387831755e-01,
        9.44575023073232600e-01,
        9.89400934991649939e-01
    };

constexpr double FRESNEL_WI[FRESNEL_GL_N] = {
    2.71524594117540374e-02,
    6.22535239386477063e-02,
    9.51585116824925914e-02,
    1.24628971255534030e-01,
    1.49595988816576764e-01,
    1.69156519395002619e-01,
    1.82603415044923612e-01,
    1.89450610455068585e-01,
    1.89450610455068585e-01,
    1.82603415044923612e-01,
    1.69156519395002619e-01,
    1.49595988816576764e-01,
    1.24628971255534030e-01,
    9.51585116824925914e-02,
    6.22535239386477063e-02,
    2.71524594117540374e-02
};

double
fresnelSeriesC(double x)
{
    const double x4 = (x * x) * (x * x);
    double term = x;
    double sum = term;
    for (int n = 1; n < 80; ++n) {
        const double two = static_cast<double>(2 * n);
        term *= -x4 / (two * (two - 1.0) * 4.0);
        term *= (4.0 * n - 3.0) / (4.0 * n + 1.0);
        sum += term;
        if (std::abs(term) <= SERIES_TERM_RELATIVE * std::max(1.0, std::abs(sum))) {
            break;
        }
    }
    return sum;
}

double
fresnelSeriesS(double x)
{
    const double x4 = (x * x) * (x * x);
    double term = (x * x * x) / 6.0;
    double sum = term;
    for (int n = 1; n < 80; ++n) {
        const double twoN = static_cast<double>(2 * n);
        term *= -x4 / ((twoN + 1.0) * twoN * 4.0);
        term *= (4.0 * n - 1.0) / (4.0 * n + 3.0);
        sum += term;
        if (std::abs(term) <= SERIES_TERM_RELATIVE * std::max(1.0, std::abs(sum))) {
            break;
        }
    }
    return sum;
}

void
fresnelPanel(double a, double b, double& c, double& s)
{
    const double mid = 0.5 * (a + b);
    const double half = 0.5 * (b - a);
    double sc = 0.0;
    double ss = 0.0;
    for (int i = 0; i < FRESNEL_GL_N; ++i) {
        const double t = mid + half * FRESNEL_XI[i];
        const double phase = 0.5 * t * t;
        const double w = FRESNEL_WI[i] * half;
        sc += w * std::cos(phase);
        ss += w * std::sin(phase);
    }
    c = sc;
    s = ss;
}

// C2 and S2 for 0 <= x < FRESNEL_ASYMPTOTIC_LIMIT.
void
fresnelNear(double x, double& c, double& s)
{
    if (x <= FRESNEL_SERIES_LIMIT) {
        c = fresnelSeriesC(x);
        s = fresnelSeriesS(x);
        return;
    }
    c = fresnelSeriesC(FRESNEL_SERIES_LIMIT);
    s = fresnelSeriesS(FRESNEL_SERIES_LIMIT);
    // The phase rate is t, so narrow the panels as t grows.
    double t = FRESNEL_SERIES_LIMIT;
    while (t < x) {
        const double width = std::min(FRESNEL_PANEL, FRESNEL_PANEL_PHASE / (t + FRESNEL_PANEL));
        const double u = std::min(x, t + width);
        double dc = 0.0;
        double ds = 0.0;
        fresnelPanel(t, u, dc, ds);
        c += dc;
        s += ds;
        t = u;
    }
}

// C2 and S2 for x >= FRESNEL_ASYMPTOTIC_LIMIT from the auxiliary functions
//   C2 = sqrt(pi)/2 + f sin(x^2/2) - g cos(x^2/2)
//   S2 = sqrt(pi)/2 - f cos(x^2/2) - g sin(x^2/2)
// with f and g summed from the asymptotic series (2n-1)!! / x^(2n+1).
void
fresnelFar(double x, double cosPhase, double sinPhase, double& c, double& s)
{
    const double inv = 1.0 / x;
    const double inv2 = inv * inv;
    double term = inv;
    double f = term;
    double g = 0.0;
    for (int n = 1; n < 100; ++n) {
        const double next = term * (2.0 * n - 1.0) * inv2;
        if (!(next < term)) {
            break;
        }
        term = next;
        const double signedTerm = ((n / 2) % 2 == 0) ? term : -term;
        if (n % 2 == 0) {
            f += signedTerm;
        }
        else {
            g += signedTerm;
        }
        if (term <= SERIES_TERM_RELATIVE * inv) {
            break;
        }
    }
    const double half = 0.5 * std::sqrt(MATH_PI);
    c = half + f * sinPhase - g * cosPhase;
    s = half - f * cosPhase - g * sinPhase;
}

} // namespace

void
halfSquarePhase(double t, double scale, double& c, double& s)
{
    // theta = (t / A)^2 / 2 with the quotient and the square carried in two
    // parts, so that theta keeps its low digits at any distance.
    const double a = std::abs(scale);
    const double x = t / a;
    const double r = std::fma(-x, a, t) / a;
    const double hi = x * x;
    const double lo = std::fma(x, x, -hi) + 2.0 * x * r;
    const double cosHi = std::cos(0.5 * hi);
    const double sinHi = std::sin(0.5 * hi);
    const double cosLo = std::cos(0.5 * lo);
    const double sinLo = std::sin(0.5 * lo);
    c = cosHi * cosLo - sinHi * sinLo;
    s = sinHi * cosLo + cosHi * sinLo;
}

geom::CoordinateXY
constructiveOffset(double t, double scale)
{
    const double a = std::abs(scale);
    if (!(t != 0.0) || !(a > 0.0) || !std::isfinite(a)) {
        return geom::CoordinateXY(0.0, 0.0);
    }
    const double x = std::abs(t) / a;
    if (!(x <= MAX_FRESNEL_ARGUMENT)) {
        throw util::IllegalArgumentException(
            "Clothoid distance is too far from the inflection to evaluate");
    }
    double c = 0.0;
    double s = 0.0;
    if (x < FRESNEL_ASYMPTOTIC_LIMIT) {
        fresnelNear(x, c, s);
    }
    else {
        double cosPhase = 0.0;
        double sinPhase = 0.0;
        halfSquarePhase(t, a, cosPhase, sinPhase);
        fresnelFar(x, cosPhase, sinPhase, c, s);
    }
    // C2 and S2 are odd.
    const double sign = (t < 0.0) ? -1.0 : 1.0;
    return geom::CoordinateXY(sign * a * c, sign * a * s);
}

std::size_t
linearizeSegmentCount(const Parameters& curve, double maxStepRadians, double maxDeviation)
{
    if (!(curve.length > 0.0) || !std::isfinite(curve.length)) {
        return 0;
    }
    const double kappaMax = std::max(std::abs(curve.k0), std::abs(curve.k1));
    if (!(kappaMax > 0.0) || !std::isfinite(kappaMax)) {
        return 1;
    }

    double nNeed = 2.0;
    if (std::isfinite(maxDeviation) && maxDeviation > 0.0) {
        nNeed = std::max(nNeed, curve.length * std::sqrt(kappaMax / (8.0 * maxDeviation)));
    }
    if (std::isfinite(maxStepRadians) && maxStepRadians > 0.0) {
        nNeed = std::max(nNeed, kappaMax * curve.length / maxStepRadians);
    }
    if (!(nNeed < MAX_SEGMENT_COUNT)) {
        throw util::IllegalArgumentException("Clothoid linearization segment count is too large");
    }
    return static_cast<std::size_t>(std::ceil(nNeed));
}

}
}
}
