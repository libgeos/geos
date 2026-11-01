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

#include <geos/geom/NurbsCurve.h>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/constants.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace geos {
namespace geom {

bool
NurbsPoint::hasWeight() const
{
    return !std::isnan(weight);
}

double
NurbsPoint::effectiveWeight() const
{
    return hasWeight() ? weight : 1.0;
}

namespace {

int
compareDouble(double a, double b)
{
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

bool
identicalDouble(double a, double b)
{
    if (std::isnan(a) || std::isnan(b)) {
        return std::isnan(a) && std::isnan(b);
    }
    return a == b;
}

// Endpoint snap tolerance, in ulps of the control point extent. Dividing by
// the weight rounds once; de Boor's convex combinations add about one ulp a
// level.
constexpr double ENDPOINT_SNAP_ULPS = 32.0;

// Each knot span is first cut into 2 p pieces, capped here, so that no piece
// turns by a full revolution between the tangents it samples.
constexpr int MAX_INITIAL_PIECES = 64;

// Bisection depth limit for linearization within one initial piece.
constexpr int MAX_LINEARIZE_DEPTH = 24;

// Bisection depth limit for the envelope of one Bezier segment.
constexpr int MAX_ENVELOPE_DEPTH = 64;

// Envelope slack, in ulps of the control point extent, below which a
// segment's control box is taken as is.
constexpr double ENVELOPE_ULPS = 4.0;

// Bisection depth limit for the length of one knot span.
constexpr int MAX_LENGTH_DEPTH = 30;

// Relative agreement between one Gauss panel and its two halves.
constexpr double LENGTH_RELATIVE = 1e-14;

// Rounding floor, in ulps of the control point extent. Lengths, chords and
// deviations below it are rounding noise: a curve that is a point has a
// speed and tangents of that size, which never meet a relative test.
constexpr double NOISE_ULPS = 64.0;

// 8-point Gauss–Legendre nodes and weights on [-1, 1].
constexpr int GL_N = 8;
constexpr double GL_X[GL_N] = {
    -0.96028985649753623168,
        -0.79666647741362673959,
        -0.52553240991632898582,
        -0.18343464249564980494,
        0.18343464249564980494,
        0.52553240991632898582,
        0.79666647741362673959,
        0.96028985649753623168
    };
constexpr double GL_W[GL_N] = {
    0.10122853629037625915,
    0.22238103445337447054,
    0.31370664587788728734,
    0.36268378337836198297,
    0.36268378337836198297,
    0.31370664587788728734,
    0.22238103445337447054,
    0.10122853629037625915
};

// A control point in homogeneous coordinates (w x, w y, w z, w).
struct Homogeneous {
    double x, y, z, w;

    Homogeneous lerp(const Homogeneous& other, double alpha) const
    {
        const double beta = 1.0 - alpha;
        return Homogeneous{
            beta * x + alpha * other.x,
            beta * y + alpha * other.y,
            beta * z + alpha * other.z,
            beta * w + alpha * other.w};
    }
};

// Read-only view of a valid, non-empty curve for evaluation.
class Evaluator {
public:
    Evaluator(int degree,
              const std::vector<NurbsPoint>& controlPoints,
              const std::vector<double>& knots)
        : p(static_cast<std::size_t>(degree)),
          n(controlPoints.size()),
          U(knots)
    {
        hp.reserve(n);
        for (const NurbsPoint& cp : controlPoints) {
            const double z = std::isnan(cp.weightedPoint.z) ? 0.0 : cp.weightedPoint.z;
            hp.push_back(Homogeneous{cp.weightedPoint.x, cp.weightedPoint.y, z, cp.effectiveWeight()});
        }
    }

    double domainStart() const
    {
        return U[p];
    }

    double domainEnd() const
    {
        return U[n];
    }

    // Spans k in [p, n) with u_k < u_{k+1}: the pieces of the curve.
    std::vector<std::size_t> spans() const
    {
        std::vector<std::size_t> result;
        for (std::size_t k = p; k < n; ++k) {
            if (U[k] < U[k + 1]) {
                result.push_back(k);
            }
        }
        return result;
    }

    // The span holding u, taking the domain end from the last span.
    std::size_t findSpan(double u) const
    {
        if (u >= U[n]) {
            std::size_t k = n - 1;
            while (!(U[k] < U[k + 1])) {
                --k;
            }
            return k;
        }
        if (u <= U[p]) {
            std::size_t k = p;
            while (!(U[k] < U[k + 1])) {
                ++k;
            }
            return k;
        }
        // Largest k in [p, n) with u_k <= u.
        const auto it = std::upper_bound(U.begin() + static_cast<std::ptrdiff_t>(p),
                                         U.begin() + static_cast<std::ptrdiff_t>(n), u);
        return static_cast<std::size_t>(it - U.begin()) - 1;
    }

    // The blossom of span k at the p arguments t, by de Boor's scheme with a
    // different argument at each level. Every weight is in [0, 1] when the
    // arguments lie in [u_k, u_{k+1}].
    Homogeneous blossom(std::size_t k, const double* t) const
    {
        std::vector<Homogeneous> d(hp.begin() + static_cast<std::ptrdiff_t>(k - p),
                                   hp.begin() + static_cast<std::ptrdiff_t>(k + 1));
        for (std::size_t r = 1; r <= p; ++r) {
            for (std::size_t j = p; j >= r; --j) {
                const std::size_t i = j + k - p;
                const double alpha = (t[r - 1] - U[i]) / (U[i + p + 1 - r] - U[i]);
                d[j] = d[j - 1].lerp(d[j], alpha);
            }
        }
        return d[p];
    }

    // The homogeneous point and its derivative in u. After p - 1 levels of
    // de Boor the last two points are the blossoms (u^{p-1}, u_k) and
    // (u^{p-1}, u_{k+1}); the derivative is p times their difference over
    // the span width.
    void evaluate(double u, Homogeneous& point, Homogeneous& derivative) const
    {
        const std::size_t k = findSpan(u);
        std::vector<Homogeneous> d(hp.begin() + static_cast<std::ptrdiff_t>(k - p),
                                   hp.begin() + static_cast<std::ptrdiff_t>(k + 1));
        for (std::size_t r = 1; r < p; ++r) {
            for (std::size_t j = p; j >= r; --j) {
                const std::size_t i = j + k - p;
                const double alpha = (u - U[i]) / (U[i + p + 1 - r] - U[i]);
                d[j] = d[j - 1].lerp(d[j], alpha);
            }
        }
        const double width = U[k + 1] - U[k];
        const double alpha = (u - U[k]) / width;
        point = d[p - 1].lerp(d[p], alpha);
        const double scale = static_cast<double>(p) / width;
        derivative = Homogeneous{
            scale * (d[p].x - d[p - 1].x),
            scale * (d[p].y - d[p - 1].y),
            scale * (d[p].z - d[p - 1].z),
            scale * (d[p].w - d[p - 1].w)};
    }

    Coordinate point(double u) const
    {
        Homogeneous h{};
        Homogeneous dh{};
        evaluate(u, h, dh);
        return Coordinate(h.x / h.w, h.y / h.w, h.z / h.w);
    }

    // The cartesian point and its derivative: C = A / w,
    // C' = (A' - w' C) / w.
    void pointAndTangent(double u, Coordinate& c, Coordinate& dc) const
    {
        Homogeneous h{};
        Homogeneous dh{};
        evaluate(u, h, dh);
        c = Coordinate(h.x / h.w, h.y / h.w, h.z / h.w);
        dc = Coordinate((dh.x - dh.w * c.x) / h.w,
                        (dh.y - dh.w * c.y) / h.w,
                        (dh.z - dh.w * c.z) / h.w);
    }

    // The rational Bezier control points of span k: the blossoms
    // (u_k^{p-j}, u_{k+1}^j) for j = 0 .. p.
    std::vector<Homogeneous> bezier(std::size_t k) const
    {
        std::vector<Homogeneous> result;
        result.reserve(p + 1);
        std::vector<double> t(p);
        for (std::size_t j = 0; j <= p; ++j) {
            std::fill(t.begin(), t.begin() + static_cast<std::ptrdiff_t>(p - j), U[k]);
            std::fill(t.begin() + static_cast<std::ptrdiff_t>(p - j), t.end(), U[k + 1]);
            result.push_back(blossom(k, t.data()));
        }
        return result;
    }

    std::size_t degree() const
    {
        return p;
    }

    const std::vector<double>& knots() const
    {
        return U;
    }

    // Largest cartesian control coordinate, which bounds the curve.
    double extent() const
    {
        double result = 0.0;
        for (const Homogeneous& h : hp) {
            result = std::max({result, std::abs(h.x / h.w), std::abs(h.y / h.w)});
        }
        return result;
    }

private:
    std::size_t p;
    std::size_t n;
    const std::vector<double>& U;
    std::vector<Homogeneous> hp;
};

double
planSpeed(const Evaluator& curve, double u)
{
    Coordinate c;
    Coordinate dc;
    curve.pointAndTangent(u, c, dc);
    return std::hypot(dc.x, dc.y);
}

double
gaussPanel(const Evaluator& curve, double a, double b)
{
    const double mid = 0.5 * (a + b);
    const double half = 0.5 * (b - a);
    double sum = 0.0;
    for (int i = 0; i < GL_N; ++i) {
        sum += GL_W[i] * half * planSpeed(curve, mid + half * GL_X[i]);
    }
    return sum;
}

double
adaptiveLength(const Evaluator& curve, double a, double b, double whole,
               double noise, int depth)
{
    const double mid = 0.5 * (a + b);
    const double left = gaussPanel(curve, a, mid);
    const double right = gaussPanel(curve, mid, b);
    const double halves = left + right;
    if (depth >= MAX_LENGTH_DEPTH || !(mid > a) || !(b > mid)
            || std::abs(halves - whole) <= std::max(LENGTH_RELATIVE * halves, noise)) {
        return halves;
    }
    return adaptiveLength(curve, a, mid, left, noise, depth + 1)
           + adaptiveLength(curve, mid, b, right, noise, depth + 1);
}

double
distanceToSegment(const Coordinate& p, const Coordinate& a, const Coordinate& b)
{
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double len2 = dx * dx + dy * dy;
    double t = 0.0;
    if (len2 > 0.0) {
        t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len2;
        t = std::min(1.0, std::max(0.0, t));
    }
    return std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
}

// Unsigned plan angle between two directions; 0 when either has no length.
double
turnBetween(const Coordinate& d0, const Coordinate& d1)
{
    const double cross = d0.x * d1.y - d0.y * d1.x;
    const double dot = d0.x * d1.x + d0.y * d1.y;
    if (cross == 0.0 && dot == 0.0) {
        return 0.0;
    }
    return std::abs(std::atan2(cross, dot));
}

struct Sample {
    double u;
    Coordinate c;
    Coordinate dc;
};

// Append parameters in (a.u, b.u] so that each piece deviates from its
// chord by at most maxDeviation and turns by at most maxTurn.
void
refine(const Evaluator& curve, const Sample& a, const Sample& b,
       double maxDeviation, double maxTurn, double noise, int depth, std::vector<Sample>& out)
{
    Sample m;
    m.u = 0.5 * (a.u + b.u);
    curve.pointAndTangent(m.u, m.c, m.dc);
    bool accept = depth >= MAX_LINEARIZE_DEPTH || !(m.u > a.u) || !(b.u > m.u);
    if (!accept) {
        const double turn = turnBetween(a.dc, m.dc) + turnBetween(m.dc, b.dc);
        double deviation = distanceToSegment(m.c, a.c, b.c);
        if (deviation <= maxDeviation) {
            deviation = std::max(deviation,
                                 distanceToSegment(curve.point(0.5 * (a.u + m.u)), a.c, b.c));
            deviation = std::max(deviation,
                                 distanceToSegment(curve.point(0.5 * (m.u + b.u)), a.c, b.c));
        }
        // A piece within rounding of a point has noise for tangents.
        const bool negligible = a.c.distance(b.c) <= noise && deviation <= noise;
        accept = negligible || (turn <= maxTurn && deviation <= maxDeviation);
    }
    if (accept) {
        out.push_back(b);
        return;
    }
    refine(curve, a, m, maxDeviation, maxTurn, noise, depth + 1, out);
    refine(curve, m, b, maxDeviation, maxTurn, noise, depth + 1, out);
}

void
splitBezier(const std::vector<Homogeneous>& bez,
            std::vector<Homogeneous>& left, std::vector<Homogeneous>& right)
{
    const std::size_t count = bez.size();
    std::vector<Homogeneous> work(bez);
    left.assign(count, Homogeneous{});
    right.assign(count, Homogeneous{});
    left[0] = work[0];
    right[count - 1] = work[count - 1];
    for (std::size_t r = 1; r < count; ++r) {
        for (std::size_t j = 0; j < count - r; ++j) {
            work[j] = work[j].lerp(work[j + 1], 0.5);
        }
        left[r] = work[0];
        right[count - 1 - r] = work[count - 1 - r];
    }
}

// Expand env to cover a rational Bezier segment with positive weights. The
// segment lies in the box of its cartesian control points; where that box
// leaves env by more than slack, the segment is split and its midpoint,
// which lies on the curve, is added.
void
expandBezier(const std::vector<Homogeneous>& bez, Envelope& env, double slack, int depth)
{
    Envelope box;
    for (const Homogeneous& h : bez) {
        box.expandToInclude(h.x / h.w, h.y / h.w);
    }
    if (env.covers(&box)) {
        return;
    }
    const bool tight = box.getMinX() >= env.getMinX() - slack
                       && box.getMaxX() <= env.getMaxX() + slack
                       && box.getMinY() >= env.getMinY() - slack
                       && box.getMaxY() <= env.getMaxY() + slack;
    if (tight || depth >= MAX_ENVELOPE_DEPTH) {
        env.expandToInclude(&box);
        return;
    }
    std::vector<Homogeneous> left;
    std::vector<Homogeneous> right;
    splitBezier(bez, left, right);
    const Homogeneous& mid = right.front();
    env.expandToInclude(mid.x / mid.w, mid.y / mid.w);
    expandBezier(left, env, slack, depth + 1);
    expandBezier(right, env, slack, depth + 1);
}

std::vector<double>
expandKnots(const std::vector<NurbsKnot>& knots)
{
    std::vector<double> result;
    for (const NurbsKnot& knot : knots) {
        result.insert(result.end(), knot.multiplicity, knot.value);
    }
    return result;
}

} // namespace

NurbsCurve::NurbsCurve(int degree,
                       std::vector<NurbsPoint> controlPoints,
                       std::vector<NurbsKnot> knots,
                       bool hasZ, bool hasM,
                       double startM, double endM,
                       const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_degree(degree),
      m_controlPoints(std::move(controlPoints)),
      m_knots(std::move(knots)),
      m_startM(hasM ? startM : 0.0),
      m_endM(hasM ? endM : 0.0)
{
    for (NurbsPoint& cp : m_controlPoints) {
        cp.weightedPoint.z = hasZ ? cp.weightedPoint.z : DoubleNotANumber;
    }
    validateConstruction();
    m_expandedKnots = expandKnots(m_knots);
    points = evaluateEndpoints();
    envelope = computeNurbsEnvelope();
}

NurbsCurve::NurbsCurve(bool hasZ, bool hasM, const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_degree(0),
      m_startM(0.0),
      m_endM(0.0)
{
}

std::unique_ptr<NurbsCurve>
NurbsCurve::clone() const
{
    return std::unique_ptr<NurbsCurve>(cloneImpl());
}

std::string
NurbsCurve::getGeometryType() const
{
    return "NurbsCurve";
}

GeometryTypeId
NurbsCurve::getGeometryTypeId() const
{
    return GEOS_NURBSCURVE;
}

bool
NurbsCurve::isRational() const
{
    return !m_controlPoints.empty() && m_controlPoints.front().hasWeight();
}

double
NurbsCurve::getStartParameter() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return m_expandedKnots[static_cast<std::size_t>(m_degree)];
}

double
NurbsCurve::getEndParameter() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return m_expandedKnots[m_controlPoints.size()];
}

CoordinateXYZM
NurbsCurve::getPointAtParameter(double u) const
{
    if (isEmpty()) {
        throw util::IllegalArgumentException("Cannot evaluate an empty NurbsCurve");
    }
    if (std::isnan(u)) {
        throw util::IllegalArgumentException("NurbsCurve parameter must not be NaN");
    }
    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const double clamped = std::min(curve.domainEnd(), std::max(curve.domainStart(), u));
    const Coordinate c = curve.point(clamped);
    return CoordinateXYZM(c.x, c.y, hasZ() ? c.z : DoubleNotANumber, DoubleNotANumber);
}

double
NurbsCurve::getLength() const
{
    if (isEmpty()) {
        return 0.0;
    }
    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const std::vector<double>& U = curve.knots();
    const double noise = NOISE_ULPS * std::numeric_limits<double>::epsilon() * curve.extent();
    double length = 0.0;
    for (std::size_t k : curve.spans()) {
        length += adaptiveLength(curve, U[k], U[k + 1], gaussPanel(curve, U[k], U[k + 1]), noise, 0);
    }
    return length;
}

std::unique_ptr<NurbsCurve>
NurbsCurve::affineTransform(double m00, double m01, double m02,
                            double m10, double m11, double m12) const
{
    const double map[] = {m00, m01, m02, m10, m11, m12};
    for (double value : map) {
        if (!std::isfinite(value)) {
            throw util::IllegalArgumentException("NurbsCurve affine map must be finite");
        }
    }
    if (isEmpty()) {
        return clone();
    }
    // A weighted point (w x, w y) maps to (w x', w y'), where the
    // translation is scaled by the weight.
    std::vector<NurbsPoint> mapped(m_controlPoints);
    for (NurbsPoint& cp : mapped) {
        const double w = cp.effectiveWeight();
        const double wx = cp.weightedPoint.x;
        const double wy = cp.weightedPoint.y;
        cp.weightedPoint.x = m00 * wx + m01 * wy + m02 * w;
        cp.weightedPoint.y = m10 * wx + m11 * wy + m12 * w;
    }
    auto image = getFactory()->createNurbsCurve(m_degree, std::move(mapped), m_knots,
                 hasZ(), hasM(), m_startM, m_endM);
    image->setSRID(getSRID());
    return image;
}

bool
NurbsCurve::hasCurvedComponents() const
{
    if (isEmpty()) {
        return false;
    }
    // Collinear control points can still trace a line back and forth beyond
    // the chord. Only a curve whose control points share one plan position
    // is its two endpoints.
    const NurbsPoint& first = m_controlPoints.front();
    const double x0 = first.weightedPoint.x / first.effectiveWeight();
    const double y0 = first.weightedPoint.y / first.effectiveWeight();
    for (const NurbsPoint& cp : m_controlPoints) {
        const double w = cp.effectiveWeight();
        if (cp.weightedPoint.x / w != x0 || cp.weightedPoint.y / w != y0) {
            return true;
        }
    }
    return false;
}

void
NurbsCurve::normalize()
{
    if (isEmpty() || isClosed()) {
        return;
    }
    if (getEndCoordinate().compareTo(getStartCoordinate()) < 0) {
        std::unique_ptr<NurbsCurve> reversed(reverseImpl());
        points = reversed->points;
        envelope = reversed->envelope;
        m_controlPoints = std::move(reversed->m_controlPoints);
        m_knots = std::move(reversed->m_knots);
        m_expandedKnots = std::move(reversed->m_expandedKnots);
        m_startM = reversed->m_startM;
        m_endM = reversed->m_endM;
    }
}

void
NurbsCurve::apply_rw(const CoordinateFilter*)
{
    throw util::UnsupportedOperationException(
        "NurbsCurve control points are not edited through coordinate filters.");
}

void
NurbsCurve::apply_rw(CoordinateSequenceFilter&)
{
    throw util::UnsupportedOperationException(
        "NurbsCurve control points are not edited through coordinate filters.");
}

bool
NurbsCurve::equalsExact(const Geometry* other, double tolerance) const
{
    if (!SimpleCurve::equalsExact(other, tolerance)) {
        return false;
    }
    const NurbsCurve* nurbs = detail::down_cast<const NurbsCurve*>(other);
    if (m_degree != nurbs->m_degree
            || m_controlPoints.size() != nurbs->m_controlPoints.size()
            || m_knots.size() != nurbs->m_knots.size()) {
        return false;
    }
    auto near = [tolerance](double a, double b) {
        return std::abs(a - b) <= tolerance;
    };
    for (std::size_t i = 0; i < m_controlPoints.size(); ++i) {
        const NurbsPoint& a = m_controlPoints[i];
        const NurbsPoint& b = nurbs->m_controlPoints[i];
        if (!near(a.weightedPoint.x, b.weightedPoint.x)
                || !near(a.weightedPoint.y, b.weightedPoint.y)
                || (hasZ() && !near(a.weightedPoint.z, b.weightedPoint.z))
                || !near(a.effectiveWeight(), b.effectiveWeight())) {
            return false;
        }
    }
    for (std::size_t i = 0; i < m_knots.size(); ++i) {
        if (!near(m_knots[i].value, nurbs->m_knots[i].value)
                || m_knots[i].multiplicity != nurbs->m_knots[i].multiplicity) {
            return false;
        }
    }
    return near(m_startM, nurbs->m_startM) && near(m_endM, nurbs->m_endM);
}

bool
NurbsCurve::equalsIdentical(const Geometry* other) const
{
    if (!SimpleCurve::equalsIdentical(other)) {
        return false;
    }
    const auto& nurbs = static_cast<const NurbsCurve&>(*other);
    if (m_degree != nurbs.m_degree
            || m_controlPoints.size() != nurbs.m_controlPoints.size()
            || m_knots.size() != nurbs.m_knots.size()) {
        return false;
    }
    for (std::size_t i = 0; i < m_controlPoints.size(); ++i) {
        const NurbsPoint& a = m_controlPoints[i];
        const NurbsPoint& b = nurbs.m_controlPoints[i];
        if (!identicalDouble(a.weightedPoint.x, b.weightedPoint.x)
                || !identicalDouble(a.weightedPoint.y, b.weightedPoint.y)
                || !identicalDouble(a.weightedPoint.z, b.weightedPoint.z)
                || !identicalDouble(a.weight, b.weight)) {
            return false;
        }
    }
    for (std::size_t i = 0; i < m_knots.size(); ++i) {
        if (!identicalDouble(m_knots[i].value, nurbs.m_knots[i].value)
                || m_knots[i].multiplicity != nurbs.m_knots[i].multiplicity) {
            return false;
        }
    }
    return identicalDouble(m_startM, nurbs.m_startM)
           && identicalDouble(m_endM, nurbs.m_endM);
}

int
NurbsCurve::compareToSameClass(const Geometry* geom) const
{
    const auto* nurbs = detail::down_cast<const NurbsCurve*>(geom);
    const int base = SimpleCurve::compareToSameClass(geom);
    if (base != 0) {
        return base;
    }
    int cmp = compareDouble(m_degree, nurbs->m_degree);
    if (cmp != 0) {
        return cmp;
    }
    cmp = compareDouble(static_cast<double>(m_controlPoints.size()),
                        static_cast<double>(nurbs->m_controlPoints.size()));
    if (cmp != 0) {
        return cmp;
    }
    cmp = compareDouble(static_cast<double>(m_knots.size()),
                        static_cast<double>(nurbs->m_knots.size()));
    if (cmp != 0) {
        return cmp;
    }
    for (std::size_t i = 0; i < m_controlPoints.size(); ++i) {
        const NurbsPoint& a = m_controlPoints[i];
        const NurbsPoint& b = nurbs->m_controlPoints[i];
        const double fields[][2] = {
            {a.weightedPoint.x, b.weightedPoint.x},
            {a.weightedPoint.y, b.weightedPoint.y},
            {a.weightedPoint.z, b.weightedPoint.z},
            {a.effectiveWeight(), b.effectiveWeight()}
        };
        for (const auto& field : fields) {
            cmp = compareDouble(field[0], field[1]);
            if (cmp != 0) {
                return cmp;
            }
        }
    }
    for (std::size_t i = 0; i < m_knots.size(); ++i) {
        cmp = compareDouble(m_knots[i].value, nurbs->m_knots[i].value);
        if (cmp != 0) {
            return cmp;
        }
        cmp = compareDouble(static_cast<double>(m_knots[i].multiplicity),
                            static_cast<double>(nurbs->m_knots[i].multiplicity));
        if (cmp != 0) {
            return cmp;
        }
    }
    cmp = compareDouble(m_startM, nurbs->m_startM);
    if (cmp != 0) {
        return cmp;
    }
    return compareDouble(m_endM, nurbs->m_endM);
}

void
NurbsCurve::appendLinearizedPoints(CoordinateSequence& seq,
                                   const algorithm::CurveToLineParams& params,
                                   std::size_t first) const
{
    if (isEmpty()) {
        return;
    }

    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const std::vector<double>& U = curve.knots();
    const double maxTurn = params.getMaxStepDegrees() * MATH_PI / 180.0;
    const double maxDeviation = params.getMaxDeviation();
    const int pieces = std::min(MAX_INITIAL_PIECES, 2 * m_degree);
    const double noise = NOISE_ULPS * std::numeric_limits<double>::epsilon() * curve.extent();

    std::vector<Sample> samples;
    Sample start;
    start.u = curve.domainStart();
    curve.pointAndTangent(start.u, start.c, start.dc);
    samples.push_back(start);
    for (std::size_t k : curve.spans()) {
        for (int i = 0; i < pieces; ++i) {
            Sample b;
            b.u = (i + 1 == pieces)
                  ? U[k + 1]
                  : U[k] + (U[k + 1] - U[k]) * (static_cast<double>(i + 1) / pieces);
            curve.pointAndTangent(b.u, b.c, b.dc);
            const Sample a = samples.back();
            refine(curve, a, b, maxDeviation, maxTurn, noise, 0, samples);
        }
    }
    // The ends are the stored endpoints, which a CompoundCurve may have
    // snapped onto its neighbors.
    samples.front().c = points->getAt<Coordinate>(0);
    samples.back().c = points->getAt<Coordinate>(1);

    // M runs linearly in length along the linearized curve.
    std::vector<double> travel(samples.size(), 0.0);
    for (std::size_t i = 1; i < samples.size(); ++i) {
        travel[i] = travel[i - 1] + samples[i].c.distance(samples[i - 1].c);
    }
    const double total = travel.back();

    for (std::size_t i = first; i < samples.size(); ++i) {
        double m = DoubleNotANumber;
        if (hasM()) {
            if (i + 1 == samples.size()) {
                m = m_endM;
            }
            else {
                const double frac = total > 0.0 ? travel[i] / total : 0.0;
                m = m_startM + (m_endM - m_startM) * frac;
            }
        }
        seq.add(CoordinateXYZM(samples[i].c.x, samples[i].c.y,
                               hasZ() ? samples[i].c.z : DoubleNotANumber, m));
    }
}

void
NurbsCurve::addLinearizedPoints(CoordinateSequence& seq,
                                const algorithm::CurveToLineParams& params) const
{
    appendLinearizedPoints(seq, params, 1);
}

LineString*
NurbsCurve::getLinearizedImpl(const algorithm::CurveToLineParams& params) const
{
    if (isEmpty()) {
        return getFactory()->createLineString(hasZ(), hasM()).release();
    }

    auto seq = std::make_unique<CoordinateSequence>(0, hasZ(), hasM());
    appendLinearizedPoints(*seq, params, 0);
    return getFactory()->createLineString(std::move(seq)).release();
}

NurbsCurve*
NurbsCurve::reverseImpl() const
{
    if (isEmpty()) {
        return cloneImpl();
    }
    // C(u) traced backwards is C(-u) with the control points and the
    // negated knots in reverse order. Negation is exact, so reversing twice
    // gives back the same curve.
    std::vector<NurbsPoint> controlPoints(m_controlPoints.rbegin(), m_controlPoints.rend());
    std::vector<NurbsKnot> knots;
    knots.reserve(m_knots.size());
    for (auto it = m_knots.rbegin(); it != m_knots.rend(); ++it) {
        knots.push_back(NurbsKnot{-it->value, it->multiplicity});
    }
    return getFactory()->createNurbsCurve(
               m_degree, std::move(controlPoints), std::move(knots),
               hasZ(), hasM(), m_endM, m_startM).release();
}

std::unique_ptr<CoordinateSequence>
NurbsCurve::evaluateEndpoints() const
{
    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const Coordinate start = curve.point(curve.domainStart());
    const Coordinate end = curve.point(curve.domainEnd());
    auto seq = std::make_unique<CoordinateSequence>(static_cast<std::size_t>(2), hasZ(), hasM());
    seq->setAt(CoordinateXYZM(start.x, start.y,
                              hasZ() ? start.z : DoubleNotANumber,
                              hasM() ? m_startM : DoubleNotANumber), 0);
    seq->setAt(CoordinateXYZM(end.x, end.y,
                              hasZ() ? end.z : DoubleNotANumber,
                              hasM() ? m_endM : DoubleNotANumber), 1);
    return seq;
}

Envelope
NurbsCurve::computeNurbsEnvelope() const
{
    Envelope env;
    if (isEmpty()) {
        return env;
    }
    env.expandToInclude(points->getAt<CoordinateXY>(0));
    env.expandToInclude(points->getAt<CoordinateXY>(1));

    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const std::vector<std::size_t> spans = curve.spans();
    std::vector<std::vector<Homogeneous>> segments;
    segments.reserve(spans.size());
    // The span ends lie on the curve; adding them all first lets most
    // control boxes fall inside at once.
    for (std::size_t k : spans) {
        segments.push_back(curve.bezier(k));
        const Homogeneous& h = segments.back().back();
        env.expandToInclude(h.x / h.w, h.y / h.w);
    }
    const double slack = ENVELOPE_ULPS * std::numeric_limits<double>::epsilon() * curve.extent();
    for (const auto& segment : segments) {
        expandBezier(segment, env, slack, 0);
    }
    return env;
}

void
NurbsCurve::validateConstruction() const
{
    if (m_degree < 1 || m_degree > MAX_DEGREE) {
        throw util::IllegalArgumentException(
            "NurbsCurve degree must be from 1 to " + std::to_string(MAX_DEGREE));
    }
    const std::size_t p = static_cast<std::size_t>(m_degree);
    const std::size_t n = m_controlPoints.size();
    if (n < p + 1) {
        throw util::IllegalArgumentException(
            "NurbsCurve needs at least degree + 1 control points");
    }

    const bool rational = m_controlPoints.front().hasWeight();
    for (const NurbsPoint& cp : m_controlPoints) {
        if (cp.hasWeight() != rational) {
            throw util::IllegalArgumentException(
                "NurbsCurve control points must all have a weight or none");
        }
        const double w = cp.effectiveWeight();
        if (!std::isfinite(w) || !(w > 0.0)) {
            throw util::IllegalArgumentException(
                "NurbsCurve weights must be finite and positive");
        }
        const Coordinate& wp = cp.weightedPoint;
        if (!std::isfinite(wp.x) || !std::isfinite(wp.y)
                || (hasZ() && !std::isfinite(wp.z))
                || !std::isfinite(wp.x / w) || !std::isfinite(wp.y / w)
                || (hasZ() && !std::isfinite(wp.z / w))) {
            throw util::IllegalArgumentException(
                "NurbsCurve control points must be finite");
        }
    }

    std::size_t total = 0;
    for (std::size_t i = 0; i < m_knots.size(); ++i) {
        const NurbsKnot& knot = m_knots[i];
        if (!std::isfinite(knot.value)) {
            throw util::IllegalArgumentException("NurbsCurve knot values must be finite");
        }
        if (i > 0 && !(m_knots[i - 1].value < knot.value)) {
            throw util::IllegalArgumentException(
                "NurbsCurve knot values must be strictly increasing");
        }
        if (knot.multiplicity < 1 || knot.multiplicity > p + 1) {
            throw util::IllegalArgumentException(
                "NurbsCurve knot multiplicity must be from 1 to degree + 1");
        }
        total += knot.multiplicity;
    }
    if (total != n + p + 1) {
        throw util::IllegalArgumentException(
            "NurbsCurve total knot multiplicity must be control points + degree + 1");
    }

    const std::vector<double> U = expandKnots(m_knots);
    const double domainStart = U[p];
    const double domainEnd = U[n];
    if (!(domainStart < domainEnd)) {
        throw util::IllegalArgumentException("NurbsCurve knot domain must not be empty");
    }
    for (const NurbsKnot& knot : m_knots) {
        if (knot.value > domainStart && knot.value < domainEnd && knot.multiplicity > p) {
            throw util::IllegalArgumentException(
                "NurbsCurve interior knot multiplicity must not exceed the degree");
        }
    }
    // Every span width is a divisor in de Boor's scheme.
    for (std::size_t k = p; k < n; ++k) {
        const double width = U[k + 1] - U[k];
        if (width > 0.0 && !std::isfinite(static_cast<double>(p) / width)) {
            throw util::IllegalArgumentException("NurbsCurve knot span is too narrow");
        }
    }

    if (hasM() && (!std::isfinite(m_startM) || !std::isfinite(m_endM))) {
        throw util::IllegalArgumentException(
            "NurbsCurve STARTM and ENDM must both be finite");
    }
}

void
NurbsCurve::snapEndpoint(std::size_t index, const CoordinateXY& target)
{
    const CoordinateXY& stored = points->getAt<CoordinateXY>(index);
    if (stored == target) {
        return;
    }
    const Evaluator curve(m_degree, m_controlPoints, m_expandedKnots);
    const double tolerance = ENDPOINT_SNAP_ULPS * std::numeric_limits<double>::epsilon() * curve.extent();
    if (!(stored.distance(target) <= tolerance)) {
        return;
    }
    auto seq = points->clone();
    seq->setOrdinate(index, CoordinateSequence::X, target.x);
    seq->setOrdinate(index, CoordinateSequence::Y, target.y);
    points = std::move(seq);
    envelope.expandToInclude(target);
}

}
}
