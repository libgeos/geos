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

class LengthSolve {
public:
    double length = 0.0;
    int iterations = 0;

    bool succeeded() const
    {
        return m_converged;
    }

    void invalidate()
    {
        m_converged = false;
    }

    static LengthSolve make(double lengthIn, int iterationsIn, bool convergedIn)
    {
        LengthSolve result;
        result.length = lengthIn;
        result.iterations = iterationsIn;
        result.m_converged = convergedIn;
        return result;
    }

private:
    // Not part of any caller-visible result.
    bool m_converged = false;
};

struct ChordPlacement {
    Parameters parameters;
    LengthSolve solve;
};

namespace {

constexpr int FRESNEL_GL_N = 16;
// Power series is accurate on this window; panels are used outside it.
constexpr double FRESNEL_SERIES_LIMIT = 5.0;
// Panel width for the Fresnel integral outside the series window.
constexpr double FRESNEL_PANEL = 0.5;
// Stop the series when the next term is this small relative to the sum.
constexpr double SERIES_TERM_RELATIVE = 1e-18;
// Heading change below which the constant-curvature closed form cancels.
constexpr double TAYLOR_TURN = 1e-8;
// Completed-square Fresnel argument past which the shift cancels.
constexpr double FRESNEL_ARGUMENT_LIMIT = 1e3;
// alpha*L^2 below this is straight at double precision.
constexpr double MIN_FRESNEL_PHASE = 1e-10;
// Halley step is rejected when the denominator is this small.
constexpr double HALLEY_DENOM = 1e-20;
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

// 32-point Gauss-Legendre nodes and weights on [0, 1].
constexpr int MOMENT_N = 32;

constexpr double MOMENT_T[MOMENT_N] = {
    1.36806907525921506e-03,
    7.19424422736580915e-03,
    1.76188722062468051e-02,
    3.25469620311301666e-02,
    5.18394221169739544e-02,
    7.53161931337150148e-02,
    1.02758102016028807e-01,
    1.33908940629855144e-01,
    1.68477866534892384e-01,
    2.06142121379618848e-01,
    2.46550045533885320e-01,
    2.89324361934682361e-01,
    3.34065698858936166e-01,
    3.80356318873931454e-01,
    4.27764019208601742e-01,
    4.75846167156130817e-01,
    5.24153832843869183e-01,
    5.72235980791398258e-01,
    6.19643681126068491e-01,
    6.65934301141063889e-01,
    7.10675638065317639e-01,
    7.53449954466114624e-01,
    7.93857878620381152e-01,
    8.31522133465107616e-01,
    8.66091059370144856e-01,
    8.97241897983971137e-01,
    9.24683806866285041e-01,
    9.48160577883025990e-01,
    9.67453037968869833e-01,
    9.82381127793753195e-01,
    9.92805755772634191e-01,
    9.98631930924740785e-01
};

constexpr double MOMENT_W[MOMENT_N] = {
    3.50930500473525288e-03,
    8.13719736545287162e-03,
    1.26960326546310120e-02,
    1.71369314565108823e-02,
    2.14179490111134178e-02,
    2.54990296311880457e-02,
    2.93420467392677825e-02,
    3.29111113881808415e-02,
    3.61728970544241690e-02,
    3.90969478935351139e-02,
    4.16559621134733535e-02,
    4.38260465022018916e-02,
    4.55869393478818899e-02,
    4.69221995404022554e-02,
    4.78193600396373542e-02,
    4.82700442573638297e-02,
    4.82700442573638297e-02,
    4.78193600396373542e-02,
    4.69221995404022554e-02,
    4.55869393478818899e-02,
    4.38260465022018916e-02,
    4.16559621134733535e-02,
    3.90969478935351139e-02,
    3.61728970544241690e-02,
    3.29111113881808415e-02,
    2.93420467392677825e-02,
    2.54990296311880457e-02,
    2.14179490111134178e-02,
    1.71369314565108823e-02,
    1.26960326546310120e-02,
    8.13719736545287162e-03,
    3.50930500473525288e-03
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

void
fresnelIncrement(double a, double b, double& c, double& s)
{
    if (b < a) {
        fresnelIncrement(b, a, c, s);
        c = -c;
        s = -s;
        return;
    }
    if (!(b > a)) {
        c = 0.0;
        s = 0.0;
        return;
    }
    if (std::abs(a) <= FRESNEL_SERIES_LIMIT && std::abs(b) <= FRESNEL_SERIES_LIMIT) {
        c = fresnelSeriesC(b) - fresnelSeriesC(a);
        s = fresnelSeriesS(b) - fresnelSeriesS(a);
        return;
    }

    c = 0.0;
    s = 0.0;
    double t = a;
    while (t < b) {
        const double u = std::min(b, t + FRESNEL_PANEL);
        double dc = 0.0;
        double ds = 0.0;
        fresnelPanel(t, u, dc, ds);
        c += dc;
        s += ds;
        t = u;
    }
}

double
curvatureRate(const Parameters& curve)
{
    if (!(curve.length > 0.0)) {
        return 0.0;
    }
    return (curve.k1 - curve.k0) / curve.length;
}

void
positionConstantCurvature(const Parameters& curve, double s, double& x, double& y)
{
    const double k = curve.k0;
    if (std::abs(k) * std::abs(s) < TAYLOR_TURN) {
        const double c = std::cos(curve.theta0);
        const double sn = std::sin(curve.theta0);
        const double s2 = s * s;
        const double s3 = s2 * s;
        x = curve.x0 + c * s - sn * k * s2 / 2.0 - c * k * k * s3 / 6.0;
        y = curve.y0 + sn * s + c * k * s2 / 2.0 - sn * k * k * s3 / 6.0;
        return;
    }
    const double th1 = curve.theta0 + k * s;
    x = curve.x0 + (std::sin(th1) - std::sin(curve.theta0)) / k;
    y = curve.y0 - (std::cos(th1) - std::cos(curve.theta0)) / k;
}

void
positionFresnel(const Parameters& curve, double s, double alpha, double& x, double& y)
{
    const double phi = curve.theta0 - (curve.k0 * curve.k0) / (2.0 * alpha);
    const double gamma = std::sqrt(std::abs(alpha));
    const double sigma = alpha >= 0.0 ? 1.0 : -1.0;
    const auto vOf = [&](double t) {
        return (t + curve.k0 / alpha) * gamma;
    };
    double dC = 0.0;
    double dSraw = 0.0;
    fresnelIncrement(vOf(0.0), vOf(s), dC, dSraw);
    dC /= gamma;
    const double dS = sigma * dSraw / gamma;
    const double cphi = std::cos(phi);
    const double sphi = std::sin(phi);
    x = curve.x0 + cphi * dC - sphi * dS;
    y = curve.y0 + sphi * dC + cphi * dS;
}

double
localHeading(const Parameters& curve, double s)
{
    const double alpha = curvatureRate(curve);
    return curve.theta0 + curve.k0 * s + 0.5 * alpha * s * s;
}

void
positionQuadrature(const Parameters& curve, double s, double& x, double& y)
{
    const double turn = std::abs(localHeading(curve, s) - curve.theta0);
    if (!std::isfinite(turn) || !std::isfinite(s)) {
        throw util::IllegalArgumentException("Clothoid turn is too large to integrate");
    }
    const double panelCount = std::ceil(turn / QUADRATURE_PANEL_RADIANS);
    if (!(panelCount <= static_cast<double>(QUADRATURE_MAX_PANELS))) {
        throw util::IllegalArgumentException("Clothoid turn is too large to integrate");
    }
    const std::size_t panels = std::max<std::size_t>(
                                   1, panelCount > 0.0 ? static_cast<std::size_t>(panelCount) : 1);
    const double h = s / static_cast<double>(panels);
    double sx = 0.0;
    double sy = 0.0;
    for (std::size_t i = 0; i < panels; ++i) {
        const double a = h * static_cast<double>(i);
        const double mid = a + 0.5 * h;
        const double half = 0.5 * h;
        for (int k = 0; k < FRESNEL_GL_N; ++k) {
            const double t = mid + half * FRESNEL_XI[k];
            const double th = localHeading(curve, t);
            const double w = FRESNEL_WI[k] * half;
            sx += w * std::cos(th);
            sy += w * std::sin(th);
        }
    }
    x = curve.x0 + sx;
    y = curve.y0 + sy;
}

bool
fresnelReductionIsStable(const Parameters& curve, double s, double alpha)
{
    if (!(std::abs(alpha) > 0.0) || !std::isfinite(alpha)) {
        return false;
    }
    const double gamma = std::sqrt(std::abs(alpha));
    const double v0 = (curve.k0 / alpha) * gamma;
    const double vs = (s + curve.k0 / alpha) * gamma;
    // Large Fresnel arguments are still defined, but a tiny rate makes the
    // completed-square shift cancel. Fall back to parameter quadrature then.
    if (std::abs(v0) > FRESNEL_ARGUMENT_LIMIT || std::abs(vs) > FRESNEL_ARGUMENT_LIMIT) {
        return false;
    }
    if (std::abs(alpha) * curve.length * curve.length < MIN_FRESNEL_PHASE) {
        return false;
    }
    return std::isfinite(v0) && std::isfinite(vs);
}

LengthSolve
finishUnconverged(double length, int iterations)
{
    return LengthSolve::make(length, iterations, false);
}

} // namespace

double
fresnelC2(double x)
{
    double c = 0.0;
    double s = 0.0;
    fresnelIncrement(0.0, x, c, s);
    return c;
}

double
fresnelS2(double x)
{
    double c = 0.0;
    double s = 0.0;
    fresnelIncrement(0.0, x, c, s);
    return s;
}

geom::CoordinateXY
position(const Parameters& curve, double s)
{
    if (!(s > 0.0) || !(curve.length > 0.0)) {
        return geom::CoordinateXY(curve.x0, curve.y0);
    }
    double x = 0.0;
    double y = 0.0;
    const double alpha = curvatureRate(curve);
    if (curve.k0 == curve.k1 || !fresnelReductionIsStable(curve, s, alpha)) {
        if (curve.k0 == curve.k1) {
            positionConstantCurvature(curve, s, x, y);
        }
        else {
            positionQuadrature(curve, s, x, y);
        }
    }
    else {
        positionFresnel(curve, s, alpha, x, y);
    }
    return geom::CoordinateXY(x, y);
}

geom::CoordinateXY
constructiveOffset(double t, double a2)
{
    if (!(t != 0.0) || !(a2 > 0.0) || !std::isfinite(a2)) {
        return geom::CoordinateXY(0.0, 0.0);
    }
    const double amplitude = std::sqrt(a2);
    const double argument = t / amplitude;
    if (!std::isfinite(argument) || !std::isfinite(amplitude)) {
        throw util::IllegalArgumentException("Clothoid scale is too small to evaluate");
    }
    return geom::CoordinateXY(amplitude * fresnelC2(argument), amplitude * fresnelS2(argument));
}

struct Moments {
    double P;
    double Q;
    double R;
    double T;
    double S2c;
    double S2s;
};

Moments
moments(double length, double k0, double k1)
{
    const double half = 0.5 * (k1 - k0);
    Moments m{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < MOMENT_N; ++i) {
        const double t = MOMENT_T[i];
        const double w = MOMENT_W[i];
        const double psi = k0 * t + half * t * t;
        const double phase = length * psi;
        const double c = std::cos(phase);
        const double s = std::sin(phase);
        m.P += w * c;
        m.Q += w * s;
        m.R += w * psi * c;
        m.T += w * psi * s;
        m.S2c += w * psi * psi * c;
        m.S2s += w * psi * psi * s;
    }
    return m;
}

LengthSolve
solveLengthHalley(
    double x0, double y0, double x1, double y1,
    double k0, double k1,
    double tolerance, int maxIterations)
{
    const double cx = x1 - x0;
    const double cy = y1 - y0;
    const double d2 = cx * cx + cy * cy;
    const double d = std::sqrt(d2);
    if (!(d > 0.0) || !std::isfinite(d) || !std::isfinite(k0) || !std::isfinite(k1)) {
        const bool converged = std::isfinite(d) && d == 0.0 && std::isfinite(k0) && std::isfinite(k1);
        return LengthSolve::make(0.0, 0, converged);
    }

    double length = d;
    const int budget = std::max(maxIterations, 0);
    for (int it = 1; it <= budget; ++it) {
        if (!std::isfinite(length)) {
            return finishUnconverged(length, it);
        }
        const Moments m = moments(length, k0, k1);
        const double r2 = m.P * m.P + m.Q * m.Q;
        const double qrpt = m.Q * m.R - m.P * m.T;
        const double f = length * length * r2 - d2;
        const double fp = 2.0 * length * r2 + 2.0 * length * length * qrpt;
        const double fpp = 2.0 * r2 + 8.0 * length * qrpt
                           + 2.0 * length * length * (m.R * m.R + m.T * m.T - m.P * m.S2c - m.Q * m.S2s);
        const double scale = std::max(d2, 1.0);
        if (std::abs(f) < tolerance * scale) {
            return LengthSolve::make(length, it, true);
        }
        const double denom = 2.0 * fp * fp - f * fpp;
        if (std::abs(denom) < HALLEY_DENOM || !(fp > 0.0)) {
            length *= 1.5;
            continue;
        }
        const double step = 2.0 * f * fp / denom;
        double next = length - step;
        if (!(next > 0.0)) {
            next = 0.5 * length;
        }
        length = next;
    }
    return finishUnconverged(length, budget);
}

ChordPlacement
placeOnChord(
    double x0, double y0, double x1, double y1,
    double k0, double k1,
    double tolerance, int maxIterations)
{
    ChordPlacement placed;
    placed.solve = solveLengthHalley(x0, y0, x1, y1, k0, k1, tolerance, maxIterations);
    placed.parameters.x0 = x0;
    placed.parameters.y0 = y0;
    placed.parameters.k0 = k0;
    placed.parameters.k1 = k1;
    placed.parameters.length = placed.solve.length;
    placed.parameters.theta0 = 0.0;

    if (!placed.solve.succeeded() || !(placed.solve.length > 0.0)) {
        const double chord = std::atan2(y1 - y0, x1 - x0);
        placed.parameters.theta0 = std::isfinite(chord) ? chord : 0.0;
        return placed;
    }

    Parameters local = placed.parameters;
    local.x0 = 0.0;
    local.y0 = 0.0;
    local.theta0 = 0.0;
    const geom::CoordinateXY localEnd = position(local, local.length);
    if (!(std::hypot(localEnd.x, localEnd.y) > 0.0)) {
        placed.solve.invalidate();
        return placed;
    }
    const double chord = std::atan2(y1 - y0, x1 - x0);
    const double localAngle = std::atan2(localEnd.y, localEnd.x);
    placed.parameters.theta0 = chord - localAngle;
    return placed;
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
