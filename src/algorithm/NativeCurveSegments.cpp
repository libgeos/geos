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

#include <geos/algorithm/NativeCurveSegments.h>

#include <geos/constants.h>
#include <geos/geom/CircularArc.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/Clothoid.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Curve.h>
#include <geos/geom/EllipticalCurve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/NurbsCurve.h>
#include <geos/geom/SimpleCurve.h>
#include <geos/geom/SpiralCurve.h>
#include <geos/geom/Surface.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include "ClothoidMath.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {

namespace {

constexpr double PI = geos::MATH_PI;

// Largest parameter sweep of one elliptic arc piece. Anything below pi
// keeps every piece a minor arc, whatever the ellipse's shape.
constexpr double MAX_ELLIPSE_PIECE = 2.0 * PI / 3.0;

// Largest turn of the tangent over one fitted Bezier, in radians.
constexpr double MAX_BEZIER_TURN = 1.2;

// Fits are accepted at this fraction of the tolerance, so that the final,
// denser measurement rarely has to split a segment again.
constexpr double ACCEPT_FRACTION = 0.9;

constexpr std::size_t FIT_SAMPLES = 64;
constexpr std::size_t FINAL_SAMPLES = 256;
constexpr std::size_t MAX_SEGMENTS = 200000;

// An ellipse whose placed axes differ by less than this, relatively, is a circle.
constexpr double CIRCLE_RELATIVE_TOLERANCE = 1e-12;

// A rational quadratic whose normalised weight is this close to 1 is a parabola.
constexpr double PARABOLA_WEIGHT_TOLERANCE = 1e-12;

// A rational quadratic ellipse with half angle below this is too close to a
// parabola to place its centre reliably, so it is approximated instead.
constexpr double MIN_CONIC_HALF_ANGLE = 1e-3;

inline CoordinateXY
operator+(const CoordinateXY& a, const CoordinateXY& b)
{
    return CoordinateXY(a.x + b.x, a.y + b.y);
}

inline CoordinateXY
operator-(const CoordinateXY& a, const CoordinateXY& b)
{
    return CoordinateXY(a.x - b.x, a.y - b.y);
}

inline CoordinateXY
operator*(double k, const CoordinateXY& a)
{
    return CoordinateXY(k * a.x, k * a.y);
}

inline double
cross(const CoordinateXY& a, const CoordinateXY& b)
{
    return a.x * b.y - a.y * b.x;
}

inline double
dot(const CoordinateXY& a, const CoordinateXY& b)
{
    return a.x * b.x + a.y * b.y;
}

inline double
norm(const CoordinateXY& a)
{
    return std::hypot(a.x, a.y);
}

inline bool
isFinite(const CoordinateXY& a)
{
    return std::isfinite(a.x) && std::isfinite(a.y);
}

// Angle a - b folded into [0, 2 pi).
double
foldTurn(double a)
{
    const double twoPi = 2.0 * PI;
    double r = std::fmod(a, twoPi);
    if (r < 0.0) {
        r += twoPi;
    }
    return r;
}

NativeCurveSegment
makeLine(const CoordinateXY& a, const CoordinateXY& b)
{
    NativeCurveSegment seg;
    seg.type = NativeCurveSegment::Type::LINE;
    seg.start = a;
    seg.end = b;
    return seg;
}

// ---------------------------------------------------------------------
// Cubic Bezier

struct Bez {
    CoordinateXY p0, c1, c2, p3;

    CoordinateXY at(double t) const
    {
        const double s = 1.0 - t;
        const double b0 = s * s * s;
        const double b1 = 3.0 * s * s * t;
        const double b2 = 3.0 * s * t * t;
        const double b3 = t * t * t;
        return CoordinateXY(b0 * p0.x + b1 * c1.x + b2 * c2.x + b3 * p3.x,
                            b0 * p0.y + b1 * c1.y + b2 * c2.y + b3 * p3.y);
    }

    CoordinateXY d1(double t) const
    {
        const double s = 1.0 - t;
        const CoordinateXY e0 = 3.0 * (c1 - p0);
        const CoordinateXY e1 = 3.0 * (c2 - c1);
        const CoordinateXY e2 = 3.0 * (p3 - c2);
        const double b0 = s * s;
        const double b1 = 2.0 * s * t;
        const double b2 = t * t;
        return CoordinateXY(b0 * e0.x + b1 * e1.x + b2 * e2.x,
                            b0 * e0.y + b1 * e1.y + b2 * e2.y);
    }

    CoordinateXY d2(double t) const
    {
        const CoordinateXY f0 = 6.0 * (c2 - 2.0 * c1 + p0);
        const CoordinateXY f1 = 6.0 * (p3 - 2.0 * c2 + c1);
        const double s = 1.0 - t;
        return CoordinateXY(s * f0.x + t * f1.x, s * f0.y + t * f1.y);
    }
};

NativeCurveSegment
segmentFromBezier(const Bez& b, bool exact, double deviation)
{
    NativeCurveSegment seg;
    seg.type = NativeCurveSegment::Type::CUBIC_BEZIER;
    seg.start = b.p0;
    seg.end = b.p3;
    seg.control1 = b.c1;
    seg.control2 = b.c2;
    seg.exact = exact;
    seg.maxDeviation = exact ? 0.0 : deviation;
    return seg;
}

// ---------------------------------------------------------------------
// A smooth planar curve parameterised by s in [lo, hi], with its first
// and second derivative with respect to s.

struct Sample {
    CoordinateXY p;
    CoordinateXY d1;
    CoordinateXY d2;
};

class ParamCurve {
public:
    virtual ~ParamCurve() = default;
    virtual Sample eval(double s) const = 0;
};

// Clothoid, travelled by arc length tau in [0, |t1 - t0|].
class ClothoidParam : public ParamCurve {
public:
    explicit ClothoidParam(const geom::Clothoid& c)
        : m_origin(c.getReferenceLocation().origin.x, c.getReferenceLocation().origin.y)
        , m_u(c.getReferenceLocation().u.x, c.getReferenceLocation().u.y)
        , m_v(c.getReferenceLocation().v.x, c.getReferenceLocation().v.y)
        , m_scale(std::abs(c.getScaleFactor()))
        , m_t0(c.getStartDistance())
        , m_t1(c.getEndDistance())
        , m_dir(c.getEndDistance() >= c.getStartDistance() ? 1.0 : -1.0)
    {
    }

    double length() const
    {
        return std::abs(m_t1 - m_t0);
    }

    Sample eval(double tau) const override
    {
        const double t = m_t0 + m_dir * tau;
        const CoordinateXY off = clothoid::constructiveOffset(t, m_scale);
        double c = 0.0;
        double s = 0.0;
        clothoid::halfSquarePhase(t, m_scale, c, s);
        Sample out;
        out.p = m_origin + off.x * m_u + off.y * m_v;
        const CoordinateXY d = c * m_u + s * m_v;
        out.d1 = m_dir * d;
        const CoordinateXY n = c * m_v - s * m_u;
        out.d2 = (t / (m_scale * m_scale)) * n;
        return out;
    }

private:
    CoordinateXY m_origin, m_u, m_v;
    double m_scale, m_t0, m_t1, m_dir;
};

double
spiralShape(geom::SpiralType type, double x)
{
    switch (type) {
    case geom::SpiralType::CLOTHOID:
        return x;
    case geom::SpiralType::BLOSS:
        return 3.0 * x * x - 2.0 * x * x * x;
    case geom::SpiralType::BIQUADRATIC:
        return x <= 0.5 ? 2.0 * x * x : 1.0 - 2.0 * (1.0 - x) * (1.0 - x);
    case geom::SpiralType::SINE:
        return x - std::sin(2.0 * PI * x) / (2.0 * PI);
    case geom::SpiralType::COSINE:
        return (1.0 - std::cos(PI * x)) / 2.0;
    }
    return x;
}

// The integral of spiralShape from 0 to x.
double
spiralShapeIntegral(geom::SpiralType type, double x)
{
    switch (type) {
    case geom::SpiralType::CLOTHOID:
        return x * x / 2.0;
    case geom::SpiralType::BLOSS:
        return x * x * x - x * x * x * x / 2.0;
    case geom::SpiralType::BIQUADRATIC:
        return x <= 0.5 ? 2.0 * x * x * x / 3.0
               : x - 0.5 + (2.0 / 3.0) * (1.0 - x) * (1.0 - x) * (1.0 - x);
    case geom::SpiralType::SINE:
        return x * x / 2.0 + (std::cos(2.0 * PI * x) - 1.0) / (4.0 * PI * PI);
    case geom::SpiralType::COSINE:
        return x / 2.0 - std::sin(PI * x) / (2.0 * PI);
    }
    return x * x / 2.0;
}

// Spiral curve, by fraction t in [0, 1] of its length.
class SpiralParam : public ParamCurve {
public:
    explicit SpiralParam(const geom::SpiralCurve& c)
        : m_curve(c)
        , m_u(c.getReferenceLocation().u.x, c.getReferenceLocation().u.y)
        , m_v(c.getReferenceLocation().v.x, c.getReferenceLocation().v.y)
        , m_length(c.getSpiralLength())
        , m_k0(c.getStartCurvature())
        , m_k1(c.getEndCurvature())
        , m_type(c.getSpiralType())
    {
    }

    Sample eval(double t) const override
    {
        const double tc = std::min(1.0, std::max(0.0, t));
        const geom::CoordinateXYZM pt = m_curve.getPointAtFraction(tc);
        const double theta = m_length * (m_k0 * tc + (m_k1 - m_k0) * spiralShapeIntegral(m_type, tc));
        const double kappa = m_k0 + (m_k1 - m_k0) * spiralShape(m_type, tc);
        const double c = std::cos(theta);
        const double s = std::sin(theta);
        Sample out;
        out.p = CoordinateXY(pt.x, pt.y);
        out.d1 = m_length * (c * m_u + s * m_v);
        out.d2 = (m_length * m_length * kappa) * (c * m_v - s * m_u);
        return out;
    }

private:
    const geom::SpiralCurve& m_curve;
    CoordinateXY m_u, m_v;
    double m_length, m_k0, m_k1;
    geom::SpiralType m_type;
};

// One Bezier span of a (possibly rational) NURBS, by u in [0, 1]. The
// control values are homogeneous: (w x, w y, w).
using HPoint = std::array<double, 3>;

class RationalBezierParam : public ParamCurve {
public:
    explicit RationalBezierParam(std::vector<HPoint> pts)
        : m_h(std::move(pts))
    {
    }

    Sample eval(double u) const override
    {
        const std::size_t n = m_h.size();
        HPoint v0 = bernstein(m_h, u);
        HPoint v1 = {0.0, 0.0, 0.0};
        HPoint v2 = {0.0, 0.0, 0.0};
        if (n >= 2) {
            v1 = bernstein(difference(m_h), u);
        }
        if (n >= 3) {
            v2 = bernstein(difference(difference(m_h)), u);
        }
        const double w = v0[2];
        const double w1 = v1[2];
        const double w2 = v2[2];
        Sample out;
        out.p = CoordinateXY(v0[0] / w, v0[1] / w);
        const CoordinateXY a1(v1[0], v1[1]);
        const CoordinateXY a2(v2[0], v2[1]);
        out.d1 = (1.0 / w) * (a1 - w1 * out.p);
        out.d2 = (1.0 / w) * (a2 - 2.0 * w1 * out.d1 - w2 * out.p);
        return out;
    }

private:
    // Derivative control values: n - 1 differences scaled by the degree.
    static std::vector<HPoint> difference(const std::vector<HPoint>& h)
    {
        std::vector<HPoint> d;
        const double deg = static_cast<double>(h.size() - 1);
        for (std::size_t i = 0; i + 1 < h.size(); i++) {
            d.push_back({deg * (h[i + 1][0] - h[i][0]),
                         deg * (h[i + 1][1] - h[i][1]),
                         deg * (h[i + 1][2] - h[i][2])});
        }
        return d;
    }

    static HPoint bernstein(std::vector<HPoint> h, double u)
    {
        // de Casteljau
        for (std::size_t r = 1; r < h.size(); r++) {
            for (std::size_t i = 0; i + r < h.size(); i++) {
                for (std::size_t k = 0; k < 3; k++) {
                    h[i][k] = (1.0 - u) * h[i][k] + u * h[i + 1][k];
                }
            }
        }
        return h[0];
    }

    std::vector<HPoint> m_h;
};

// ---------------------------------------------------------------------
// Bezier fitting

struct Fit {
    Bez bez;
    double deviation = 0.0;
    bool g2 = false;
};

CoordinateXY
unitTangent(const ParamCurve& pc, const Sample& smp, double s, double toward, double avgSpeed)
{
    const double len = norm(smp.d1);
    if (std::isfinite(len) && len > 1e-8 * avgSpeed && len > 0.0) {
        return (1.0 / len) * smp.d1;
    }
    // A stationary point: look a little way along the curve instead.
    const double step = 1e-4 * (toward - s);
    const Sample probe = pc.eval(s + step);
    const CoordinateXY d = probe.p - smp.p;
    const double dl = norm(d);
    if (dl > 0.0) {
        return (1.0 / dl) * d;
    }
    return CoordinateXY(1.0, 0.0);
}

double
curvatureOf(const Sample& smp)
{
    const double len = norm(smp.d1);
    if (!(len > 0.0)) {
        return 0.0;
    }
    return cross(smp.d1, smp.d2) / (len * len * len);
}

// Newton projection of q onto a curve given by eval(x) -> (p, d1, d2).
template <typename EvalFn>
double
projectDistance(const EvalFn& evalAt, const CoordinateXY& q, double x, double lo, double hi)
{
    double xc = x;
    for (int it = 0; it < 4; it++) {
        const Sample smp = evalAt(xc);
        const CoordinateXY r = smp.p - q;
        const double f = dot(r, smp.d1);
        const double fp = dot(smp.d1, smp.d1) + dot(r, smp.d2);
        if (!(std::abs(fp) > 0.0) || !std::isfinite(fp)) {
            break;
        }
        double nx = xc - f / fp;
        nx = std::min(hi, std::max(lo, nx));
        if (nx == xc) {
            break;
        }
        xc = nx;
    }
    return norm(evalAt(xc).p - q);
}

// The sampled Hausdorff distance between the exact curve on [sa, sb] and the
// Bezier: n + 1 samples of each, every one projected onto the other curve
// near where the previous one matched. Also the total turn of the exact
// tangent.
double
measureDeviation(const ParamCurve& pc, double sa, double sb, const Bez& bez,
                 std::size_t n, double* turnOut)
{
    const std::size_t m = 4 * n;
    std::vector<Sample> exact(m + 1);
    std::vector<CoordinateXY> bpts(m + 1);
    for (std::size_t k = 0; k <= m; k++) {
        const double f = static_cast<double>(k) / static_cast<double>(m);
        exact[k] = pc.eval(sa + (sb - sa) * f);
        bpts[k] = bez.at(f);
    }
    if (turnOut) {
        double turn = 0.0;
        double prevAngle = 0.0;
        bool havePrev = false;
        for (std::size_t k = 0; k <= m; k += 4) {
            const CoordinateXY& d = exact[k].d1;
            if (!(norm(d) > 0.0)) {
                continue;
            }
            const double ang = std::atan2(d.y, d.x);
            if (havePrev) {
                double dA = ang - prevAngle;
                while (dA > PI) {
                    dA -= 2.0 * PI;
                }
                while (dA < -PI) {
                    dA += 2.0 * PI;
                }
                turn += std::abs(dA);
            }
            prevAngle = ang;
            havePrev = true;
        }
        *turnOut = turn;
    }

    auto bezEval = [&](double t) {
        Sample smp;
        smp.p = bez.at(t);
        smp.d1 = bez.d1(t);
        smp.d2 = bez.d2(t);
        return smp;
    };
    auto exactEval = [&](double s) {
        return pc.eval(s);
    };

    // Both polylines run along the same curve in the same direction, so the
    // nearest point of one to a sample of the other is looked for only in a
    // window of the other's samples that follows the previous match. That
    // keeps a sample of a curve that comes back close to itself from
    // matching the wrong branch, and costs O(window) per sample.
    const std::size_t window = std::max<std::size_t>(16, m / 16);
    auto nearestIn = [&](auto&& pointAt, const CoordinateXY& q, std::size_t expected,
                         std::size_t& best, double& bestD) {
        const std::size_t lo = expected > window ? expected - window : 0;
        const std::size_t hi = std::min(m, expected + window);
        bestD = std::numeric_limits<double>::infinity();
        for (std::size_t k = lo; k <= hi; k++) {
            const CoordinateXY r = pointAt(k) - q;
            const double d = r.x * r.x + r.y * r.y;
            if (d < bestD) {
                bestD = d;
                best = k;
            }
        }
    };
    auto exactAt = [&](std::size_t k) {
        return exact[k].p;
    };
    auto bezierAt = [&](std::size_t k) {
        return bpts[k];
    };

    double dev = 0.0;
    std::size_t prevBez = 0;
    std::size_t prevExact = 0;
    for (std::size_t i = 0; i <= n; i++) {
        // exact sample -> Bezier
        {
            std::size_t best = prevBez;
            double bestD = 0.0;
            nearestIn(bezierAt, exact[4 * i].p, std::max(prevBez, 4 * i), best, bestD);
            prevBez = best;
            const double d = projectDistance(bezEval, exact[4 * i].p,
                                             static_cast<double>(best) / static_cast<double>(m), 0.0, 1.0);
            dev = std::max(dev, std::min(d, std::sqrt(bestD)));
        }
        // Bezier sample -> exact
        {
            std::size_t best = prevExact;
            double bestD = 0.0;
            nearestIn(exactAt, bpts[4 * i], std::max(prevExact, 4 * i), best, bestD);
            prevExact = best;
            const double x0 = sa + (sb - sa) * static_cast<double>(best) / static_cast<double>(m);
            const double d = projectDistance(exactEval, bpts[4 * i], x0, std::min(sa, sb), std::max(sa, sb));
            dev = std::max(dev, std::min(d, std::sqrt(bestD)));
        }
    }
    return dev;
}

// A cubic Bezier from s = sa to s = sb with end positions and tangents
// taken from the curve, and handle lengths chosen to match the curvature
// at both ends (G2) when asked and possible, otherwise by the circular
// arc rule (G1).
bool
fitCandidates(const ParamCurve& pc, double sa, double sb, bool g2, std::vector<Bez>& out)
{
    const Sample a = pc.eval(sa);
    const Sample b = pc.eval(sb);
    if (!isFinite(a.p) || !isFinite(b.p)) {
        return false;
    }
    const CoordinateXY d = b.p - a.p;
    const double dist = norm(d);
    const double avgSpeed = dist / std::abs(sb - sa);
    const CoordinateXY t0 = unitTangent(pc, a, sa, sb, avgSpeed);
    const CoordinateXY t1 = unitTangent(pc, b, sb, sa, avgSpeed);
    // t1 must run forward, so flip the probe direction used at the far end.
    const CoordinateXY t1f = norm(b.d1) > 1e-8 * avgSpeed ? t1 : -1.0 * t1;

    const double turn = std::atan2(cross(t0, t1f), dot(t0, t1f));
    const double cosQuarter = std::cos(turn / 4.0);
    const double h = dist / (3.0 * cosQuarter * cosQuarter);

    Bez g1;
    g1.p0 = a.p;
    g1.p3 = b.p;
    g1.c1 = a.p + h * t0;
    g1.c2 = b.p - h * t1f;
    out.push_back(g1);

    if (!g2 || !(dist > 0.0)) {
        return true;
    }

    const double k0 = curvatureOf(a);
    const double k1 = curvatureOf(b);
    if (!(std::abs(k0) * dist > 1e-9) && !(std::abs(k1) * dist > 1e-9)) {
        return true;
    }
    const double c0 = cross(t0, d);
    const double c1 = cross(d, t1f);
    const double x = cross(t0, t1f);

    double ha = h;
    double hb = h;
    bool converged = false;
    for (int it = 0; it < 40; it++) {
        const double f1 = (2.0 / 3.0) * (c0 - hb * x) - k0 * ha * ha;
        const double f2 = (2.0 / 3.0) * (c1 - ha * x) - k1 * hb * hb;
        const double scale = std::abs(c0) + std::abs(c1) + dist * 1e-12;
        if (std::abs(f1) + std::abs(f2) < 1e-11 * scale + 1e-300) {
            converged = true;
            break;
        }
        const double j11 = -2.0 * k0 * ha;
        const double j12 = -(2.0 / 3.0) * x;
        const double j21 = -(2.0 / 3.0) * x;
        const double j22 = -2.0 * k1 * hb;
        const double det = j11 * j22 - j12 * j21;
        if (!(std::abs(det) > 1e-300)) {
            break;
        }
        double da = (f1 * j22 - f2 * j12) / det;
        double db = (j11 * f2 - j21 * f1) / det;
        // Damped step that keeps both handles positive.
        double lambda = 1.0;
        while (lambda > 1e-6 && (ha - lambda * da <= 0.0 || hb - lambda * db <= 0.0)) {
            lambda *= 0.5;
        }
        if (lambda <= 1e-6) {
            break;
        }
        ha -= lambda * da;
        hb -= lambda * db;
        if (!(ha < 10.0 * dist) || !(hb < 10.0 * dist)) {
            break;
        }
    }
    if (converged && ha > 0.0 && hb > 0.0 && std::isfinite(ha) && std::isfinite(hb)) {
        Bez g;
        g.p0 = a.p;
        g.p3 = b.p;
        g.c1 = a.p + ha * t0;
        g.c2 = b.p - hb * t1f;
        out.push_back(g);
    }
    return true;
}

// Fit one Bezier over [sa, sb]. The curvature-matched (G2) candidate is used
// when it was asked for and the solve converged, otherwise the G1 candidate.
// Returns whether the candidate is within `fraction * tol`.
bool
tryFit(const ParamCurve& pc, double sa, double sb, bool g2, double tol, double fraction,
       std::size_t samples, Fit& best)
{
    std::vector<Bez> cands;
    if (!fitCandidates(pc, sa, sb, g2, cands)) {
        return false;
    }
    // cands[0] is G1; a second entry, when present, is the G2 candidate.
    // When the curvature-matched fit exists it is the only one tried, so the
    // result does not flip between G1 and G2 from one Bezier to the next.
    const std::size_t lowest = cands.size() > 1 ? 1 : 0;
    for (std::size_t i = cands.size(); i-- > lowest;) {
        double turn = 0.0;
        const double dev = measureDeviation(pc, sa, sb, cands[i], samples, &turn);
        if (std::isfinite(dev) && turn <= MAX_BEZIER_TURN && dev <= fraction * tol) {
            best.bez = cands[i];
            best.deviation = dev;
            best.g2 = (i > 0);
            return true;
        }
    }
    return false;
}

// Fit cubic Beziers over [s0, s1] so that each stays within tol of the curve.
void
approximateRange(const ParamCurve& pc, double s0, double s1, const NativeCurveParams& params,
                 std::vector<NativeCurveSegment>& out)
{
    const double tol = params.tolerance;
    const bool g2 = params.matchCurvature;
    const double span = s1 - s0;

    // Greedy: take the longest piece that fits, found by bisection. "Fits" is
    // not monotone in the length (the G2 solve can fail to converge and the
    // turn cap can bind), so this is a good piece, not provably the longest.
    std::vector<double> breaks;
    breaks.push_back(s0);
    double pos = s0;
    Fit fit;
    while (std::abs(s1 - pos) > 0.0) {
        if (breaks.size() > MAX_SEGMENTS) {
            throw util::UnsupportedOperationException(
                "Curve needs too many Bezier segments for the requested tolerance");
        }
        if (tryFit(pc, pos, s1, g2, tol, ACCEPT_FRACTION, FIT_SAMPLES, fit)) {
            pos = s1;
        }
        else {
            double lo = pos;
            double hi = s1;
            for (int it = 0; it < 40; it++) {
                const double mid = 0.5 * (lo + hi);
                if (mid == lo || mid == hi) {
                    break;
                }
                if (tryFit(pc, pos, mid, g2, tol, ACCEPT_FRACTION, FIT_SAMPLES, fit)) {
                    lo = mid;
                }
                else {
                    hi = mid;
                }
            }
            if (std::abs(lo - pos) <= 1e-12 * std::abs(span)) {
                throw util::UnsupportedOperationException(
                    "Cannot fit a cubic Bezier within the requested tolerance");
            }
            pos = lo;
        }
        breaks.push_back(pos);
    }

    // Prefer equal pieces when the greedy count suffices for them.
    const std::size_t n = breaks.size() - 1;
    if (n > 1) {
        bool allOk = true;
        for (std::size_t i = 0; i < n && allOk; i++) {
            const double a = s0 + span * static_cast<double>(i) / static_cast<double>(n);
            const double b = (i + 1 == n) ? s1 : s0 + span * static_cast<double>(i + 1) / static_cast<double>(n);
            allOk = tryFit(pc, a, b, g2, tol, ACCEPT_FRACTION, FIT_SAMPLES, fit);
        }
        if (allOk) {
            for (std::size_t i = 0; i <= n; i++) {
                breaks[i] = (i == n) ? s1 : s0 + span * static_cast<double>(i) / static_cast<double>(n);
            }
        }
    }

    // Emit, re-measuring densely and splitting a piece that is above tol.
    struct Emitter {
        const ParamCurve& pc;
        const NativeCurveParams& params;
        std::vector<NativeCurveSegment>& out;
        int depth;

        void emit(double a, double b)
        {
            Fit f;
            const bool ok = tryFit(pc, a, b, params.matchCurvature, params.tolerance, 1.0,
                                   FIT_SAMPLES, f);
            double dense = 0.0;
            if (ok) {
                double turn = 0.0;
                dense = measureDeviation(pc, a, b, f.bez, FINAL_SAMPLES, &turn);
            }
            if (ok && dense <= params.tolerance) {
                NativeCurveSegment seg = segmentFromBezier(f.bez, false, dense);
                out.push_back(seg);
                return;
            }
            if (depth > 60 || out.size() > MAX_SEGMENTS) {
                throw util::UnsupportedOperationException(
                    "Cannot fit a cubic Bezier within the requested tolerance");
            }
            const double mid = 0.5 * (a + b);
            Emitter sub{pc, params, out, depth + 1};
            sub.emit(a, mid);
            sub.emit(mid, b);
        }
    };
    Emitter em{pc, params, out, 0};
    for (std::size_t i = 0; i + 1 < breaks.size(); i++) {
        em.emit(breaks[i], breaks[i + 1]);
    }
}

// ---------------------------------------------------------------------
// Ellipse arcs

// An arc of c + A cos t + B sin t from t = th0 to t = th1, as circular or
// elliptic arcs. `start` and `end` are the stored end points of the whole
// arc, used exactly for the first and last piece.
void
appendEllipseArcs(std::vector<NativeCurveSegment>& out,
                  const CoordinateXY& c, const CoordinateXY& A, const CoordinateXY& B,
                  double th0, double th1,
                  const CoordinateXY& start, const CoordinateXY& end)
{
    const double det = cross(A, B);
    const double sxx = A.x * A.x + B.x * B.x;
    const double syy = A.y * A.y + B.y * B.y;
    const double sxy = A.x * A.y + B.x * B.y;
    const double mean = 0.5 * (sxx + syy);
    const double rad = std::sqrt(0.25 * (sxx - syy) * (sxx - syy) + sxy * sxy);
    const double lam1 = mean + rad;
    const double lam2 = std::max(0.0, mean - rad);
    const double semiMajor = std::sqrt(lam1);
    const double semiMinor = std::sqrt(lam2);
    if (!(semiMajor > 0.0) || !(semiMinor > 1e-12 * semiMajor) || !std::isfinite(semiMajor)) {
        throw util::UnsupportedOperationException(
            "Elliptic arc with a singular placement has no ellipse form");
    }
    const double phi = 0.5 * std::atan2(2.0 * sxy, sxx - syy);
    const bool isCircle = (semiMajor - semiMinor) <= CIRCLE_RELATIVE_TOLERANCE * semiMajor;
    const double sweep = th1 - th0;
    const bool ccw = (sweep > 0.0) == (det > 0.0);

    auto pointAt = [&](double t) {
        return c + std::cos(t) * A + std::sin(t) * B;
    };

    std::size_t pieces = static_cast<std::size_t>(std::ceil(std::abs(sweep) / MAX_ELLIPSE_PIECE - 1e-9));
    if (pieces < 1) {
        pieces = 1;
    }
    for (std::size_t k = 0; k < pieces; k++) {
        const double fa = static_cast<double>(k) / static_cast<double>(pieces);
        const double fb = static_cast<double>(k + 1) / static_cast<double>(pieces);
        const double ta = th0 + sweep * fa;
        const double tb = th0 + sweep * fb;
        NativeCurveSegment seg;
        seg.start = (k == 0) ? start : pointAt(ta);
        seg.end = (k + 1 == pieces) ? end : pointAt(tb);
        seg.center = c;
        seg.clockwise = !ccw;
        if (isCircle) {
            seg.type = NativeCurveSegment::Type::CIRCULAR_ARC;
            seg.radius = 0.5 * (semiMajor + semiMinor);
            seg.interior = pointAt(0.5 * (ta + tb));
        }
        else {
            seg.type = NativeCurveSegment::Type::ELLIPTIC_ARC;
            seg.semiMajorAxis = semiMajor;
            seg.axisRatio = semiMinor / semiMajor;
            seg.rotation = phi;
            seg.minor = true;
            seg.interior = pointAt(0.5 * (ta + tb));
        }
        out.push_back(seg);
    }
}

void
appendCircularArc(std::vector<NativeCurveSegment>& out, const geom::CircularArc& arc)
{
    const CoordinateXY p0(arc.p0().x, arc.p0().y);
    const CoordinateXY p1(arc.p1().x, arc.p1().y);
    const CoordinateXY p2(arc.p2().x, arc.p2().y);
    if (arc.isLinear()) {
        out.push_back(makeLine(p0, p2));
        return;
    }
    const CoordinateXY c = arc.getCenter();
    const double radius = arc.getRadius();
    const bool ccw = arc.isCCW();
    if (arc.isCircle()) {
        // A complete circle: two half circles, split at the antipode.
        const CoordinateXY q = 2.0 * c - p0;
        const CoordinateXY r0 = p0 - c;
        const CoordinateXY r1 = q - c;
        const double sgn = ccw ? 1.0 : -1.0;
        NativeCurveSegment a;
        a.type = NativeCurveSegment::Type::CIRCULAR_ARC;
        a.start = p0;
        a.end = q;
        a.interior = c + CoordinateXY(-sgn * r0.y, sgn * r0.x);
        a.center = c;
        a.radius = radius;
        a.clockwise = !ccw;
        NativeCurveSegment b = a;
        b.start = q;
        b.end = p2;
        b.interior = c + CoordinateXY(-sgn * r1.y, sgn * r1.x);
        out.push_back(a);
        out.push_back(b);
        return;
    }
    NativeCurveSegment seg;
    seg.type = NativeCurveSegment::Type::CIRCULAR_ARC;
    seg.start = p0;
    seg.end = p2;
    seg.interior = p1;
    seg.center = c;
    seg.radius = radius;
    seg.clockwise = !ccw;
    out.push_back(seg);
}

// ---------------------------------------------------------------------
// NURBS

struct BezierSpan {
    std::vector<HPoint> h; // homogeneous control values
};

CoordinateXY
cartesian(const HPoint& h)
{
    return CoordinateXY(h[0] / h[2], h[1] / h[2]);
}

// Elevate a quadratic Bezier to a cubic exactly.
Bez
elevateQuadratic(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    Bez b;
    b.p0 = p0;
    b.c1 = (1.0 / 3.0) * (p0 + 2.0 * p1);
    b.c2 = (1.0 / 3.0) * (2.0 * p1 + p2);
    b.p3 = p2;
    return b;
}

// If the polynomial Bezier with these control points has degree 3 or less,
// give its cubic form.
bool
reduceToCubic(const std::vector<CoordinateXY>& pts, Bez& cubic)
{
    const std::size_t n = pts.size();
    double extent = 0.0;
    for (std::size_t i = 1; i < n; i++) {
        extent = std::max(extent, norm(pts[i] - pts[0]));
    }
    if (!(extent > 0.0)) {
        return false;
    }
    std::vector<CoordinateXY> diff = pts;
    for (std::size_t order = 1; order < n; order++) {
        for (std::size_t i = 0; i + order < n; i++) {
            diff[i] = diff[i + 1] - diff[i];
        }
        if (order >= 4) {
            for (std::size_t i = 0; i + order < n; i++) {
                // Rounding in a k-th difference grows like 2^k.
                if (norm(diff[i]) > 1e-12 * extent * std::ldexp(1.0, static_cast<int>(order))) {
                    return false;
                }
            }
        }
    }
    auto poly = [&](double t) {
        std::vector<CoordinateXY> w = pts;
        for (std::size_t r = 1; r < n; r++) {
            for (std::size_t i = 0; i + r < n; i++) {
                w[i] = (1.0 - t) * w[i] + t * w[i + 1];
            }
        }
        return w[0];
    };
    const CoordinateXY p0 = pts.front();
    const CoordinateXY p3 = pts.back();
    const CoordinateXY x = 27.0 * poly(1.0 / 3.0) - 8.0 * p0 - p3;
    const CoordinateXY y = 27.0 * poly(2.0 / 3.0) - p0 - 8.0 * p3;
    cubic.p0 = p0;
    cubic.p3 = p3;
    cubic.c1 = (1.0 / 18.0) * (2.0 * x - y);
    cubic.c2 = (1.0 / 18.0) * (2.0 * y - x);
    return true;
}

// True when the weights w_i, divided by r^i with r = (w_n / w_0)^(1/n), are
// all equal. Such a rational Bezier is a projective reparametrisation of the
// polynomial Bezier with the same control points (equal weights being the
// case r = 1), so it traces exactly that polynomial curve.
bool
polynomialWeights(const std::vector<HPoint>& h)
{
    const std::size_t n = h.size() - 1;
    const double r = std::pow(h[n][2] / h[0][2], 1.0 / static_cast<double>(n));
    double ri = 1.0;
    for (std::size_t i = 0; i <= n; i++) {
        if (std::abs(h[i][2] / h[0][2] / ri - 1.0) > 1e-13) {
            return false;
        }
        ri *= r;
    }
    return true;
}

void
appendNurbsSpan(std::vector<NativeCurveSegment>& out, const BezierSpan& span,
                const NativeCurveParams& params)
{
    const std::vector<HPoint>& h = span.h;
    const std::size_t deg = h.size() - 1;
    std::vector<CoordinateXY> pts;
    for (const HPoint& p : h) {
        pts.push_back(cartesian(p));
    }

    if (deg == 1) {
        out.push_back(makeLine(pts[0], pts[1]));
        return;
    }

    if (polynomialWeights(h)) {
        if (deg == 2) {
            out.push_back(segmentFromBezier(elevateQuadratic(pts[0], pts[1], pts[2]), true, 0.0));
            return;
        }
        if (deg == 3) {
            Bez b;
            b.p0 = pts[0];
            b.c1 = pts[1];
            b.c2 = pts[2];
            b.p3 = pts[3];
            out.push_back(segmentFromBezier(b, true, 0.0));
            return;
        }
        Bez cubic;
        if (reduceToCubic(pts, cubic)) {
            out.push_back(segmentFromBezier(cubic, true, 0.0));
            return;
        }
    }
    else if (deg == 2) {
        // Rational quadratic that is not a parabola: an ellipse or a hyperbola.
        const double w = h[1][2] / std::sqrt(h[0][2] * h[2][2]);
        if (std::abs(w - 1.0) <= PARABOLA_WEIGHT_TOLERANCE) {
            out.push_back(segmentFromBezier(elevateQuadratic(pts[0], pts[1], pts[2]), true, 0.0));
            return;
        }
        if (w < 1.0) {
            const double alpha = std::acos(w);
            const CoordinateXY mid = 0.5 * (pts[0] + pts[2]);
            const double chord = norm(pts[2] - pts[0]);
            const double bulge = norm(pts[1] - mid);
            if (alpha >= MIN_CONIC_HALF_ANGLE && chord > 0.0 && bulge > 1e-12 * chord
                    && std::abs(cross(pts[1] - pts[0], pts[2] - pts[0])) > 1e-12 * chord * chord) {
                const double sa = std::sin(alpha);
                const double ca = std::cos(alpha);
                const CoordinateXY B = (1.0 / (2.0 * sa)) * (pts[2] - pts[0]);
                const CoordinateXY A = (ca / (sa * sa)) * (pts[1] - mid);
                const CoordinateXY c = pts[0] - ca * A + sa * B;
                appendEllipseArcs(out, c, A, B, -alpha, alpha, pts[0], pts[2]);
                return;
            }
        }
    }

    // No exact form: a Bezier fit, by u in [0, 1].
    RationalBezierParam pc(h);
    approximateRange(pc, 0.0, 1.0, params, out);
}

void
appendNurbs(std::vector<NativeCurveSegment>& out, const geom::NurbsCurve& nc,
            const NativeCurveParams& params)
{
    for (const std::vector<geom::NurbsPoint>& span : nc.getBezierSpans()) {
        BezierSpan bs;
        for (const geom::NurbsPoint& cp : span) {
            bs.h.push_back({cp.weightedPoint.x, cp.weightedPoint.y, cp.effectiveWeight()});
        }
        appendNurbsSpan(out, bs, params);
    }
}

// ---------------------------------------------------------------------
// Dispatch

CoordinateXY
frontXY(const geom::SimpleCurve& c)
{
    const geom::CoordinateSequence& cs = *c.getCoordinatesRO();
    return CoordinateXY(cs.getX(0), cs.getY(0));
}

CoordinateXY
backXY(const geom::SimpleCurve& c)
{
    const geom::CoordinateSequence& cs = *c.getCoordinatesRO();
    const std::size_t i = cs.size() - 1;
    return CoordinateXY(cs.getX(i), cs.getY(i));
}

// The last segment appended keeps `end` equal to the stored end point of
// the curve, and the first segment starts at the stored start point.
void
pinEnds(std::vector<NativeCurveSegment>& segs, std::size_t first,
        const CoordinateXY& start, const CoordinateXY& end)
{
    if (segs.size() <= first) {
        return;
    }
    segs[first].start = start;
    segs.back().end = end;
}

void
appendSimpleCurve(std::vector<NativeCurveSegment>& out, const geom::SimpleCurve& curve,
                  const NativeCurveParams& params)
{
    if (curve.isEmpty()) {
        return;
    }
    const std::size_t first = out.size();
    switch (curve.getGeometryTypeId()) {
    case geom::GEOS_LINESTRING:
    case geom::GEOS_LINEARRING: {
        const geom::CoordinateSequence& cs = *curve.getCoordinatesRO();
        for (std::size_t i = 0; i + 1 < cs.size(); i++) {
            out.push_back(makeLine(CoordinateXY(cs.getX(i), cs.getY(i)),
                                   CoordinateXY(cs.getX(i + 1), cs.getY(i + 1))));
        }
        return;
    }
    case geom::GEOS_CIRCULARSTRING: {
        const auto& cstr = static_cast<const geom::CircularString&>(curve);
        for (const geom::CircularArc& arc : cstr.getArcs()) {
            appendCircularArc(out, arc);
        }
        break;
    }
    case geom::GEOS_ELLIPTICALCURVE: {
        const auto& ec = static_cast<const geom::EllipticalCurve&>(curve);
        const geom::AffinePlacement& loc = ec.getReferenceLocation();
        const double a = ec.getUAxisLength();
        const double b = ec.getVAxisLength();
        const CoordinateXY A(a * loc.u.x, a * loc.u.y);
        const CoordinateXY B(b * loc.v.x, b * loc.v.y);
        appendEllipseArcs(out, CoordinateXY(loc.origin.x, loc.origin.y), A, B,
                          ec.getStartAngle(), ec.getEndAngle(), frontXY(curve), backXY(curve));
        break;
    }
    case geom::GEOS_NURBSCURVE:
        appendNurbs(out, static_cast<const geom::NurbsCurve&>(curve), params);
        break;
    case geom::GEOS_CLOTHOID: {
        const auto& cl = static_cast<const geom::Clothoid&>(curve);
        if (!cl.hasRegularPlacement()) {
            throw util::UnsupportedOperationException(
                "Clothoid with a singular placement has no curve form");
        }
        ClothoidParam pc(cl);
        if (!(pc.length() > 0.0)) {
            out.push_back(makeLine(frontXY(curve), backXY(curve)));
            break;
        }
        approximateRange(pc, 0.0, pc.length(), params, out);
        break;
    }
    case geom::GEOS_SPIRALCURVE: {
        const auto& sp = static_cast<const geom::SpiralCurve&>(curve);
        const geom::AffinePlacement& loc = sp.getReferenceLocation();
        const double det = loc.u.x * loc.v.y - loc.u.y * loc.v.x;
        if (!(std::abs(det) > 1e-12 * std::hypot(loc.u.x, loc.u.y) * std::hypot(loc.v.x, loc.v.y))) {
            throw util::UnsupportedOperationException(
                "Spiral curve with a singular placement has no curve form");
        }
        SpiralParam pc(sp);
        approximateRange(pc, 0.0, 1.0, params, out);
        break;
    }
    default:
        throw util::UnsupportedOperationException(
            "Curve type " + curve.getGeometryType() + " has no native curve segment form");
    }
    pinEnds(out, first, frontXY(curve), backXY(curve));
}

// Make segments connect exactly: each starts where the previous ended.
void
connect(std::vector<NativeCurveSegment>& segs)
{
    for (std::size_t i = 1; i < segs.size(); i++) {
        segs[i].start = segs[i - 1].end;
    }
}

// Points along a segment, for orientation tests only.
void
samplePoints(const NativeCurveSegment& seg, std::vector<CoordinateXY>& pts)
{
    const std::size_t n = 32;
    switch (seg.type) {
    case NativeCurveSegment::Type::LINE:
        pts.push_back(seg.end);
        return;
    case NativeCurveSegment::Type::CUBIC_BEZIER: {
        Bez b;
        b.p0 = seg.start;
        b.c1 = seg.control1;
        b.c2 = seg.control2;
        b.p3 = seg.end;
        for (std::size_t i = 1; i <= n; i++) {
            pts.push_back(b.at(static_cast<double>(i) / static_cast<double>(n)));
        }
        return;
    }
    case NativeCurveSegment::Type::CIRCULAR_ARC:
    case NativeCurveSegment::Type::ELLIPTIC_ARC: {
        const bool ell = seg.type == NativeCurveSegment::Type::ELLIPTIC_ARC;
        const double a = ell ? seg.semiMajorAxis : seg.radius;
        const double bb = ell ? seg.semiMajorAxis * seg.axisRatio : seg.radius;
        const double cr = std::cos(seg.rotation);
        const double sr = std::sin(seg.rotation);
        auto angleOf = [&](const CoordinateXY& p) {
            const CoordinateXY r = p - seg.center;
            const double x = (r.x * cr + r.y * sr) / a;
            const double y = (-r.x * sr + r.y * cr) / bb;
            return std::atan2(y, x);
        };
        const double a0 = angleOf(seg.start);
        const double a1 = angleOf(seg.interior);
        const double a2 = angleOf(seg.end);
        double sweep;
        if (seg.clockwise) {
            sweep = -(foldTurn(a0 - a1) + foldTurn(a1 - a2));
        }
        else {
            sweep = foldTurn(a1 - a0) + foldTurn(a2 - a1);
        }
        for (std::size_t i = 1; i <= n; i++) {
            const double t = a0 + sweep * static_cast<double>(i) / static_cast<double>(n);
            const double x = a * std::cos(t);
            const double y = bb * std::sin(t);
            pts.push_back(seg.center + CoordinateXY(x * cr - y * sr, x * sr + y * cr));
        }
        return;
    }
    }
}

double
signedAreaApprox(const NativeCurvePath& path)
{
    std::vector<CoordinateXY> pts;
    if (path.segments.empty()) {
        return 0.0;
    }
    pts.push_back(path.segments.front().start);
    for (const NativeCurveSegment& s : path.segments) {
        samplePoints(s, pts);
    }
    // Relative to the first point, so that large coordinates (projected
    // systems reach 1e7) do not cancel.
    const CoordinateXY origin = pts.front();
    double area = 0.0;
    for (std::size_t i = 0; i + 1 < pts.size(); i++) {
        area += cross(pts[i] - origin, pts[i + 1] - origin);
    }
    return 0.5 * area;
}

void
reversePath(NativeCurvePath& path)
{
    std::reverse(path.segments.begin(), path.segments.end());
    for (NativeCurveSegment& s : path.segments) {
        s = s.reversed();
    }
}

void
convertInto(const geom::Geometry& g, const NativeCurveParams& params,
            std::vector<NativeCurvePath>& out);

NativeCurvePath
buildPath(const geom::Curve& curve, const NativeCurveParams& params, bool ring)
{
    NativeCurvePath path;
    if (curve.getGeometryTypeId() == geom::GEOS_COMPOUNDCURVE) {
        const auto& cc = static_cast<const geom::CompoundCurve&>(curve);
        for (std::size_t i = 0; i < cc.getNumCurves(); i++) {
            appendSimpleCurve(path.segments, *cc.getCurveN(i), params);
        }
    }
    else {
        const auto* sc = dynamic_cast<const geom::SimpleCurve*>(&curve);
        if (!sc) {
            throw util::UnsupportedOperationException(
                "Curve type " + curve.getGeometryType() + " has no native curve segment form");
        }
        appendSimpleCurve(path.segments, *sc, params);
    }
    connect(path.segments);
    path.closed = ring;
    return path;
}

void
addSurface(const geom::Surface& surf, const NativeCurveParams& params,
           std::vector<NativeCurvePath>& out)
{
    if (surf.isEmpty()) {
        return;
    }
    {
        NativeCurvePath shell = buildPath(*surf.getExteriorRing(), params, true);
        // Exterior rings run clockwise.
        if (signedAreaApprox(shell) > 0.0) {
            reversePath(shell);
        }
        out.push_back(std::move(shell));
    }
    for (std::size_t i = 0; i < surf.getNumInteriorRing(); i++) {
        NativeCurvePath hole = buildPath(*surf.getInteriorRingN(i), params, true);
        // Holes run counterclockwise.
        if (signedAreaApprox(hole) < 0.0) {
            reversePath(hole);
        }
        out.push_back(std::move(hole));
    }
}

void
convertInto(const geom::Geometry& g, const NativeCurveParams& params,
            std::vector<NativeCurvePath>& out)
{
    if (g.isEmpty()) {
        return;
    }
    switch (g.getGeometryTypeId()) {
    case geom::GEOS_POINT:
    case geom::GEOS_MULTIPOINT:
        return;
    case geom::GEOS_LINESTRING:
    case geom::GEOS_LINEARRING:
    case geom::GEOS_CIRCULARSTRING:
    case geom::GEOS_ELLIPTICALCURVE:
    case geom::GEOS_NURBSCURVE:
    case geom::GEOS_CLOTHOID:
    case geom::GEOS_SPIRALCURVE:
    case geom::GEOS_COMPOUNDCURVE:
        out.push_back(buildPath(static_cast<const geom::Curve&>(g), params, false));
        return;
    case geom::GEOS_POLYGON:
    case geom::GEOS_CURVEPOLYGON:
        addSurface(static_cast<const geom::Surface&>(g), params, out);
        return;
    case geom::GEOS_MULTILINESTRING:
    case geom::GEOS_MULTICURVE:
    case geom::GEOS_MULTIPOLYGON:
    case geom::GEOS_MULTISURFACE: {
        const auto& gc = static_cast<const geom::GeometryCollection&>(g);
        for (std::size_t i = 0; i < gc.getNumGeometries(); i++) {
            convertInto(*gc.getGeometryN(i), params, out);
        }
        return;
    }
    default:
        throw util::UnsupportedOperationException(
            "Geometry type " + g.getGeometryType() + " has no native curve segment form");
    }
}

void
checkParams(const NativeCurveParams& params)
{
    if (!(params.tolerance > 0.0) || !std::isfinite(params.tolerance)) {
        throw util::IllegalArgumentException("Curve conversion tolerance must be positive and finite");
    }
}

} // anonymous namespace

NativeCurveSegment
NativeCurveSegment::reversed() const
{
    NativeCurveSegment r = *this;
    std::swap(r.start, r.end);
    switch (type) {
    case Type::CUBIC_BEZIER:
        std::swap(r.control1, r.control2);
        break;
    case Type::CIRCULAR_ARC:
    case Type::ELLIPTIC_ARC:
        r.clockwise = !clockwise;
        break;
    case Type::LINE:
        break;
    }
    return r;
}

NativeCurvePath
NativeCurveSegments::convertCurve(const geom::Curve& curve, const NativeCurveParams& params)
{
    checkParams(params);
    return buildPath(curve, params, false);
}

std::vector<NativeCurvePath>
NativeCurveSegments::convert(const geom::Geometry& geom, const NativeCurveParams& params)
{
    checkParams(params);
    std::vector<NativeCurvePath> out;
    convertInto(geom, params, out);
    return out;
}

std::size_t
NativeCurveSegments::countApproximations(const std::vector<NativeCurvePath>& paths)
{
    std::size_t n = 0;
    for (const NativeCurvePath& p : paths) {
        for (const NativeCurveSegment& s : p.segments) {
            if (!s.exact) {
                n++;
            }
        }
    }
    return n;
}

std::size_t
NativeCurveSegments::countSegments(const std::vector<NativeCurvePath>& paths)
{
    std::size_t n = 0;
    for (const NativeCurvePath& p : paths) {
        n += p.segments.size();
    }
    return n;
}

double
NativeCurveSegments::maxDeviation(const std::vector<NativeCurvePath>& paths)
{
    double dev = 0.0;
    for (const NativeCurvePath& p : paths) {
        for (const NativeCurveSegment& s : p.segments) {
            dev = std::max(dev, s.maxDeviation);
        }
    }
    return dev;
}

}
}
