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

#include <geos/geom/SpiralCurve.h>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/constants.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace geos {
namespace geom {

namespace {

constexpr double EPS = std::numeric_limits<double>::epsilon();

// One integration panel per this many radians of turn.
constexpr double PANEL_RADIANS = 0.25;

// Past this turn the spiral is not a practical curve (4096 panels).
constexpr double MAX_TURN_RADIANS = 1024.0;

// Endpoint snap tolerance, in ulps of the placement extent.
constexpr double ENDPOINT_SNAP_ULPS = 32.0;

// Bisection steps for a parameter in [0, 1]; 2^-64 is below an ulp.
constexpr int BISECTION_STEPS = 64;

// Most segments one linearization makes.
constexpr double MAX_SEGMENTS = 1e6;

constexpr double TWO_PI = 2.0 * MATH_PI;

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

// The curvature shape f, rising from f(0) = 0 to f(1) = 1.
double
shape(SpiralCurve::Type type, double t)
{
    switch (type) {
    case SpiralCurve::Type::CLOTHOID:
        return t;
    case SpiralCurve::Type::BLOSS:
        return t * t * (3.0 - 2.0 * t);
    case SpiralCurve::Type::BIQUADRATIC:
        if (t <= 0.5) {
            return 2.0 * t * t;
        }
        return 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
    case SpiralCurve::Type::SINE:
        return t - std::sin(TWO_PI * t) / TWO_PI;
    case SpiralCurve::Type::COSINE:
        return 0.5 * (1.0 - std::cos(MATH_PI * t));
    }
    return t;
}

// The integral of f from 0 to t.
double
shapeIntegral(SpiralCurve::Type type, double t)
{
    switch (type) {
    case SpiralCurve::Type::CLOTHOID:
        return 0.5 * t * t;
    case SpiralCurve::Type::BLOSS:
        return t * t * t * (1.0 - 0.5 * t);
    case SpiralCurve::Type::BIQUADRATIC:
        if (t <= 0.5) {
            return 2.0 * t * t * t / 3.0;
        }
        return t - 0.5 + 2.0 * (1.0 - t) * (1.0 - t) * (1.0 - t) / 3.0;
    case SpiralCurve::Type::SINE:
        return 0.5 * t * t + (std::cos(TWO_PI * t) - 1.0) / (TWO_PI * TWO_PI);
    case SpiralCurve::Type::COSINE:
        return 0.5 * t - std::sin(MATH_PI * t) / TWO_PI;
    }
    return 0.5 * t * t;
}

// Largest singular value of the plan part of the placement.
double
planNorm(const AffinePlacement& loc)
{
    const double p = loc.u.x * loc.u.x + loc.u.y * loc.u.y;
    const double q = loc.v.x * loc.v.x + loc.v.y * loc.v.y;
    const double r = loc.u.x * loc.v.x + loc.u.y * loc.v.y;
    return std::sqrt(0.5 * (p + q + std::hypot(p - q, 2.0 * r)));
}

// An even panel count, so that t = 1/2, where the biquadratic shape
// changes its formula, is a panel boundary.
std::size_t
panelCount(double turn)
{
    const double half = std::ceil(turn / (2.0 * PANEL_RADIANS));
    return 2 * static_cast<std::size_t>(std::max(1.0, half));
}

} // namespace

std::string
SpiralCurve::typeName(Type type)
{
    switch (type) {
    case Type::CLOTHOID: return "clothoid";
    case Type::BLOSS: return "bloss";
    case Type::BIQUADRATIC: return "biquadratic";
    case Type::SINE: return "sine";
    case Type::COSINE: return "cosine";
    }
    return "clothoid";
}

SpiralCurve::Type
SpiralCurve::typeFromName(const std::string& name)
{
    std::string lower = name;
    for (char& c : lower) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    for (Type type : {Type::CLOTHOID, Type::BLOSS, Type::BIQUADRATIC, Type::SINE, Type::COSINE}) {
        if (lower == typeName(type)) {
            return type;
        }
    }
    throw util::IllegalArgumentException(
        "Spiral type must be clothoid, bloss, biquadratic, sine or cosine, not '" + name + "'");
}

SpiralCurve::SpiralCurve(const AffinePlacement& location,
                         double length, double startCurvature, double endCurvature,
                         Type type,
                         bool hasZ, bool hasM,
                         double startM, double endM,
                         const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_location{Coordinate(location.origin.x, location.origin.y, hasZ ? location.origin.z : 0.0),
                 Coordinate(location.u.x, location.u.y, hasZ ? location.u.z : 0.0),
                 Coordinate(location.v.x, location.v.y, hasZ ? location.v.z : 0.0)},
      m_length(length),
      m_k0(startCurvature),
      m_k1(endCurvature),
      m_type(type),
      m_startM(hasM ? startM : 0.0),
      m_endM(hasM ? endM : 0.0)
{
    validateConstruction();

    // Integrate the unit tangent once, panel by panel; every point is then
    // a short integral from the panel boundary before it.
    const std::size_t n = panelCount(m_length * std::max(std::abs(m_k0), std::abs(m_k1)));
    m_panelPoints.reserve(n + 1);
    m_panelPoints.emplace_back(0.0, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        const double a = static_cast<double>(i) / static_cast<double>(n);
        const double b = static_cast<double>(i + 1) / static_cast<double>(n);
        const double mid = 0.5 * (a + b);
        const double half = 0.5 * (b - a);
        double x = 0.0;
        double y = 0.0;
        for (int k = 0; k < GL_N; ++k) {
            const double theta = heading(mid + half * GL_X[k]);
            x += GL_W[k] * std::cos(theta);
            y += GL_W[k] * std::sin(theta);
        }
        const CoordinateXY& prev = m_panelPoints.back();
        m_panelPoints.emplace_back(prev.x + m_length * half * x, prev.y + m_length * half * y);
    }

    auto seq = std::make_unique<CoordinateSequence>(static_cast<std::size_t>(2), hasZ, hasM);
    const Coordinate end = placedPoint(1.0);
    // The start is the placement origin exactly.
    seq->setAt(CoordinateXYZM(m_location.origin.x, m_location.origin.y,
                              hasZ ? m_location.origin.z : DoubleNotANumber,
                              hasM ? m_startM : DoubleNotANumber), 0);
    seq->setAt(CoordinateXYZM(end.x, end.y,
                              hasZ ? end.z : DoubleNotANumber,
                              hasM ? m_endM : DoubleNotANumber), 1);
    points = std::move(seq);
    envelope = computeSpiralEnvelope();
}

SpiralCurve::SpiralCurve(bool hasZ, bool hasM, const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_location{Coordinate(0.0, 0.0, 0.0), Coordinate(0.0, 0.0, 0.0), Coordinate(0.0, 0.0, 0.0)},
      m_length(0.0),
      m_k0(0.0),
      m_k1(0.0),
      m_type(Type::CLOTHOID),
      m_startM(0.0),
      m_endM(0.0)
{
}

std::unique_ptr<SpiralCurve>
SpiralCurve::clone() const
{
    return std::unique_ptr<SpiralCurve>(cloneImpl());
}

std::string
SpiralCurve::getGeometryType() const
{
    return "SpiralCurve";
}

GeometryTypeId
SpiralCurve::getGeometryTypeId() const
{
    return GEOS_SPIRALCURVE;
}

double
SpiralCurve::heading(double t) const
{
    return m_length * (m_k0 * t + (m_k1 - m_k0) * shapeIntegral(m_type, t));
}

CoordinateXY
SpiralCurve::localPoint(double t) const
{
    const std::size_t n = m_panelPoints.size() - 1;
    const double clamped = std::min(1.0, std::max(0.0, t));
    std::size_t i = static_cast<std::size_t>(clamped * static_cast<double>(n));
    if (i >= n) {
        return m_panelPoints[n];
    }
    const double a = static_cast<double>(i) / static_cast<double>(n);
    if (clamped == a) {
        return m_panelPoints[i];
    }
    const double mid = 0.5 * (a + clamped);
    const double half = 0.5 * (clamped - a);
    double x = 0.0;
    double y = 0.0;
    for (int k = 0; k < GL_N; ++k) {
        const double theta = heading(mid + half * GL_X[k]);
        x += GL_W[k] * std::cos(theta);
        y += GL_W[k] * std::sin(theta);
    }
    return CoordinateXY(m_panelPoints[i].x + m_length * half * x,
                        m_panelPoints[i].y + m_length * half * y);
}

Coordinate
SpiralCurve::placedPoint(double t) const
{
    const CoordinateXY p = localPoint(t);
    const AffinePlacement& loc = m_location;
    return Coordinate(loc.origin.x + p.x * loc.u.x + p.y * loc.v.x,
                      loc.origin.y + p.x * loc.u.y + p.y * loc.v.y,
                      loc.origin.z + p.x * loc.u.z + p.y * loc.v.z);
}

CoordinateXYZM
SpiralCurve::getPointAtFraction(double t) const
{
    if (isEmpty()) {
        throw util::IllegalArgumentException("Cannot evaluate an empty SpiralCurve");
    }
    if (std::isnan(t)) {
        throw util::IllegalArgumentException("SpiralCurve fraction must not be NaN");
    }
    const Coordinate c = placedPoint(t);
    return CoordinateXYZM(c.x, c.y, hasZ() ? c.z : DoubleNotANumber, DoubleNotANumber);
}

double
SpiralCurve::getLength() const
{
    if (isEmpty()) {
        return 0.0;
    }
    // The plan speed is L |cos(theta) u + sin(theta) v|, integrated over the
    // same panels as the points.
    const AffinePlacement& loc = m_location;
    const std::size_t n = m_panelPoints.size() - 1;
    double length = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double a = static_cast<double>(i) / static_cast<double>(n);
        const double b = static_cast<double>(i + 1) / static_cast<double>(n);
        const double mid = 0.5 * (a + b);
        const double half = 0.5 * (b - a);
        for (int k = 0; k < GL_N; ++k) {
            const double theta = heading(mid + half * GL_X[k]);
            const double c = std::cos(theta);
            const double s = std::sin(theta);
            length += GL_W[k] * half * std::hypot(c * loc.u.x + s * loc.v.x, c * loc.u.y + s * loc.v.y);
        }
    }
    return m_length * length;
}

std::unique_ptr<SpiralCurve>
SpiralCurve::affineTransform(double m00, double m01, double m02,
                             double m10, double m11, double m12) const
{
    const double map[] = {m00, m01, m02, m10, m11, m12};
    for (double value : map) {
        if (!std::isfinite(value)) {
            throw util::IllegalArgumentException("SpiralCurve affine map must be finite");
        }
    }
    if (isEmpty()) {
        return clone();
    }
    // The origin is a point and takes the translation; u and v are
    // directions and take only the linear part.
    const AffinePlacement& p = m_location;
    AffinePlacement location;
    location.origin = Coordinate(m00 * p.origin.x + m01 * p.origin.y + m02,
                                 m10 * p.origin.x + m11 * p.origin.y + m12,
                                 p.origin.z);
    location.u = Coordinate(m00 * p.u.x + m01 * p.u.y, m10 * p.u.x + m11 * p.u.y, p.u.z);
    location.v = Coordinate(m00 * p.v.x + m01 * p.v.y, m10 * p.v.x + m11 * p.v.y, p.v.z);
    auto image = getFactory()->createSpiralCurve(location, m_length, m_k0, m_k1, m_type,
                 hasZ(), hasM(), m_startM, m_endM);
    image->setSRID(getSRID());
    return image;
}

bool
SpiralCurve::hasCurvedComponents() const
{
    if (isEmpty() || (m_k0 == 0.0 && m_k1 == 0.0)) {
        return false;
    }
    // Collinear directions still trace the curve back and forth along a
    // line. Only a plan image that is a single point is its endpoints.
    const AffinePlacement& p = m_location;
    return p.u.x != 0.0 || p.u.y != 0.0 || p.v.x != 0.0 || p.v.y != 0.0;
}

void
SpiralCurve::normalize()
{
    if (isEmpty() || isClosed()) {
        return;
    }
    if (getEndCoordinate().compareTo(getStartCoordinate()) < 0) {
        std::unique_ptr<SpiralCurve> reversed(reverseImpl());
        points = reversed->points;
        envelope = reversed->envelope;
        m_location = reversed->m_location;
        m_k0 = reversed->m_k0;
        m_k1 = reversed->m_k1;
        m_startM = reversed->m_startM;
        m_endM = reversed->m_endM;
        m_panelPoints = std::move(reversed->m_panelPoints);
    }
}

void
SpiralCurve::apply_rw(const CoordinateFilter*)
{
    throw util::UnsupportedOperationException(
        "SpiralCurve parameters are not edited through coordinate filters.");
}

void
SpiralCurve::apply_rw(CoordinateSequenceFilter&)
{
    throw util::UnsupportedOperationException(
        "SpiralCurve parameters are not edited through coordinate filters.");
}

bool
SpiralCurve::equalsExact(const Geometry* other, double tolerance) const
{
    if (!SimpleCurve::equalsExact(other, tolerance)) {
        return false;
    }
    const auto* s = detail::down_cast<const SpiralCurve*>(other);
    auto near = [tolerance](double a, double b) {
        return std::abs(a - b) <= tolerance;
    };
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = s->m_location;
    return m_type == s->m_type
           && near(p.origin.x, q.origin.x) && near(p.origin.y, q.origin.y) && near(p.origin.z, q.origin.z)
           && near(p.u.x, q.u.x) && near(p.u.y, q.u.y) && near(p.u.z, q.u.z)
           && near(p.v.x, q.v.x) && near(p.v.y, q.v.y) && near(p.v.z, q.v.z)
           && near(m_length, s->m_length) && near(m_k0, s->m_k0) && near(m_k1, s->m_k1)
           && near(m_startM, s->m_startM) && near(m_endM, s->m_endM);
}

bool
SpiralCurve::equalsIdentical(const Geometry* other) const
{
    if (!SimpleCurve::equalsIdentical(other)) {
        return false;
    }
    const auto& s = static_cast<const SpiralCurve&>(*other);
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = s.m_location;
    return m_type == s.m_type
           && identicalDouble(p.origin.x, q.origin.x) && identicalDouble(p.origin.y, q.origin.y)
           && identicalDouble(p.origin.z, q.origin.z)
           && identicalDouble(p.u.x, q.u.x) && identicalDouble(p.u.y, q.u.y) && identicalDouble(p.u.z, q.u.z)
           && identicalDouble(p.v.x, q.v.x) && identicalDouble(p.v.y, q.v.y) && identicalDouble(p.v.z, q.v.z)
           && identicalDouble(m_length, s.m_length)
           && identicalDouble(m_k0, s.m_k0) && identicalDouble(m_k1, s.m_k1)
           && identicalDouble(m_startM, s.m_startM) && identicalDouble(m_endM, s.m_endM);
}

int
SpiralCurve::compareToSameClass(const Geometry* geom) const
{
    const auto* s = detail::down_cast<const SpiralCurve*>(geom);
    const int base = SimpleCurve::compareToSameClass(geom);
    if (base != 0) {
        return base;
    }
    const int typeCmp = compareDouble(static_cast<double>(m_type), static_cast<double>(s->m_type));
    if (typeCmp != 0) {
        return typeCmp;
    }
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = s->m_location;
    const double fields[][2] = {
        {p.origin.x, q.origin.x}, {p.origin.y, q.origin.y}, {p.origin.z, q.origin.z},
        {p.u.x, q.u.x}, {p.u.y, q.u.y}, {p.u.z, q.u.z},
        {p.v.x, q.v.x}, {p.v.y, q.v.y}, {p.v.z, q.v.z},
        {m_length, s->m_length}, {m_k0, s->m_k0}, {m_k1, s->m_k1},
        {m_startM, s->m_startM}, {m_endM, s->m_endM}
    };
    for (const auto& field : fields) {
        const int cmp = compareDouble(field[0], field[1]);
        if (cmp != 0) {
            return cmp;
        }
    }
    return 0;
}

void
SpiralCurve::appendLinearizedPoints(CoordinateSequence& seq,
                                    const algorithm::CurveToLineParams& params,
                                    std::size_t first) const
{
    if (isEmpty()) {
        return;
    }
    // Equal steps of length, each turning at most the maximum step and
    // short enough that its chord deviates at most the maximum deviation:
    // a step h deviates at most k h^2 / 8 in the local frame, and the plan
    // norm bounds how far the placement stretches that.
    const double kmax = std::max(std::abs(m_k0), std::abs(m_k1));
    double count = 1.0;
    if (kmax > 0.0) {
        double step = params.getMaxStepDegrees() * MATH_PI / 180.0 / kmax;
        const double norm = planNorm(m_location);
        if (norm > 0.0 && std::isfinite(params.getMaxDeviation())) {
            step = std::min(step, std::sqrt(8.0 * params.getMaxDeviation() / (norm * kmax)));
        }
        if (step > 0.0 && std::isfinite(step)) {
            count = std::min(MAX_SEGMENTS, std::max(1.0, std::ceil(m_length / step)));
        }
    }
    const std::size_t segments = static_cast<std::size_t>(count);

    std::vector<Coordinate> pts;
    pts.reserve(segments + 1);
    // The ends are the stored endpoints, which a CompoundCurve may have
    // snapped onto its neighbors.
    pts.push_back(points->getAt<Coordinate>(0));
    for (std::size_t i = 1; i < segments; ++i) {
        pts.push_back(placedPoint(static_cast<double>(i) / static_cast<double>(segments)));
    }
    pts.push_back(points->getAt<Coordinate>(1));

    // M runs linearly in length along the linearized curve.
    std::vector<double> travel(pts.size(), 0.0);
    for (std::size_t i = 1; i < pts.size(); ++i) {
        travel[i] = travel[i - 1] + pts[i].distance(pts[i - 1]);
    }
    const double total = travel.back();
    for (std::size_t i = first; i < pts.size(); ++i) {
        double m = DoubleNotANumber;
        if (hasM()) {
            if (i + 1 == pts.size()) {
                m = m_endM;
            }
            else {
                const double frac = total > 0.0 ? travel[i] / total : 0.0;
                m = m_startM + (m_endM - m_startM) * frac;
            }
        }
        seq.add(CoordinateXYZM(pts[i].x, pts[i].y, hasZ() ? pts[i].z : DoubleNotANumber, m));
    }
}

void
SpiralCurve::addLinearizedPoints(CoordinateSequence& seq,
                                 const algorithm::CurveToLineParams& params) const
{
    appendLinearizedPoints(seq, params, 1);
}

LineString*
SpiralCurve::getLinearizedImpl(const algorithm::CurveToLineParams& params) const
{
    if (isEmpty()) {
        return getFactory()->createLineString(hasZ(), hasM()).release();
    }
    auto seq = std::make_unique<CoordinateSequence>(0, hasZ(), hasM());
    appendLinearizedPoints(*seq, params, 0);
    return getFactory()->createLineString(std::move(seq)).release();
}

SpiralCurve*
SpiralCurve::reverseImpl() const
{
    if (isEmpty()) {
        return cloneImpl();
    }
    // Backwards from the end, the local direction is d = -(cos, sin) of the
    // end heading and the local left of d is (-d.y, d.x), with the same
    // handedness as (u, v). Traced backwards the curvature at t is
    // -(k0 + (k1 - k0) f(1 - t)) = -k1 + (k1 - k0) f(t), since
    // f(1 - t) = 1 - f(t): the same type, from -k1 to -k0.
    const double theta = heading(1.0);
    const double dx = -std::cos(theta);
    const double dy = -std::sin(theta);
    const AffinePlacement& p = m_location;
    AffinePlacement location;
    const Coordinate end = placedPoint(1.0);
    location.origin = Coordinate(points->getX(1), points->getY(1), end.z);
    location.u = Coordinate(dx * p.u.x + dy * p.v.x, dx * p.u.y + dy * p.v.y, dx * p.u.z + dy * p.v.z);
    location.v = Coordinate(-dy * p.u.x + dx * p.v.x, -dy * p.u.y + dx * p.v.y, -dy * p.u.z + dx * p.v.z);
    return getFactory()->createSpiralCurve(location, m_length, -m_k1, -m_k0, m_type,
                                           hasZ(), hasM(), m_endM, m_startM).release();
}

Envelope
SpiralCurve::computeSpiralEnvelope() const
{
    Envelope env;
    if (isEmpty()) {
        return env;
    }
    env.expandToInclude(points->getAt<CoordinateXY>(0));
    env.expandToInclude(points->getAt<CoordinateXY>(1));

    // The curvature k0 + (k1 - k0) f(t) is monotone, so the heading has at
    // most two monotone pieces, split where the curvature crosses zero.
    std::vector<double> cuts = {0.0};
    const double dk = m_k1 - m_k0;
    if ((m_k0 < 0.0 && m_k1 > 0.0) || (m_k0 > 0.0 && m_k1 < 0.0)) {
        const double target = -m_k0 / dk;
        double lo = 0.0;
        double hi = 1.0;
        for (int i = 0; i < BISECTION_STEPS; ++i) {
            const double mid = 0.5 * (lo + hi);
            (shape(m_type, mid) < target ? lo : hi) = mid;
        }
        cuts.push_back(0.5 * (lo + hi));
    }
    cuts.push_back(1.0);

    // x(t) is extreme where the plan tangent cos(theta) u.x + sin(theta) v.x
    // vanishes: theta = atan2(v.x, u.x) + pi / 2 + k pi; likewise for y.
    auto addExtremes = [&](double uc, double vc) {
        if (!(std::hypot(uc, vc) > 0.0)) {
            return;
        }
        const double base = std::atan2(vc, uc) + MATH_PI / 2.0;
        for (std::size_t piece = 0; piece + 1 < cuts.size(); ++piece) {
            const double a = cuts[piece];
            const double b = cuts[piece + 1];
            const double ha = heading(a);
            const double hb = heading(b);
            const double lo = std::min(ha, hb);
            const double hi = std::max(ha, hb);
            const bool rising = hb >= ha;
            for (double k = std::ceil((lo - base) / MATH_PI); ; k += 1.0) {
                const double target = base + k * MATH_PI;
                if (!(target <= hi)) {
                    break;
                }
                double tl = a;
                double th = b;
                for (int i = 0; i < BISECTION_STEPS; ++i) {
                    const double mid = 0.5 * (tl + th);
                    ((heading(mid) < target) == rising ? tl : th) = mid;
                }
                env.expandToInclude(placedPoint(0.5 * (tl + th)));
            }
        }
    };
    addExtremes(m_location.u.x, m_location.v.x);
    addExtremes(m_location.u.y, m_location.v.y);
    return env;
}

void
SpiralCurve::validateConstruction() const
{
    const AffinePlacement& p = m_location;
    const double coords[] = {
        p.origin.x, p.origin.y, p.origin.z,
        p.u.x, p.u.y, p.u.z,
        p.v.x, p.v.y, p.v.z
    };
    for (double value : coords) {
        if (!std::isfinite(value)) {
            throw util::IllegalArgumentException(
                "SpiralCurve reference location must be finite");
        }
    }
    if (!std::isfinite(m_length) || !(m_length > 0.0)) {
        throw util::IllegalArgumentException("SpiralCurve length must be finite and positive");
    }
    if (!std::isfinite(m_k0) || !std::isfinite(m_k1)) {
        throw util::IllegalArgumentException("SpiralCurve curvatures must be finite");
    }
    const double turn = m_length * std::max(std::abs(m_k0), std::abs(m_k1));
    if (!std::isfinite(turn) || turn > MAX_TURN_RADIANS) {
        throw util::IllegalArgumentException("SpiralCurve turn is too large to integrate");
    }
    if (hasM() && (!std::isfinite(m_startM) || !std::isfinite(m_endM))) {
        throw util::IllegalArgumentException(
            "SpiralCurve STARTM and ENDM must both be finite");
    }
}

void
SpiralCurve::snapEndpoint(std::size_t index, const CoordinateXY& target)
{
    const CoordinateXY& stored = points->getAt<CoordinateXY>(index);
    if (stored == target) {
        return;
    }
    const double extent = std::max(std::abs(m_location.origin.x), std::abs(m_location.origin.y))
                          + planNorm(m_location) * m_length;
    const double tolerance = ENDPOINT_SNAP_ULPS * EPS * extent;
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
