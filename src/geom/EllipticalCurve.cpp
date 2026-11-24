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

#include <geos/geom/EllipticalCurve.h>

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
#include <vector>

namespace geos {
namespace geom {

namespace {

constexpr double EPS = std::numeric_limits<double>::epsilon();

// An angle within this many ulps of a multiple of pi / 2 is taken as on it.
constexpr double QUADRANT_ULPS = 4.0;

// Angles that differ from a full turn by this many ulps make a full ellipse.
constexpr double FULL_TURN_ULPS = 8.0;

// Endpoint snap tolerance, in ulps of the placement extent.
constexpr double ENDPOINT_SNAP_ULPS = 32.0;

// Length panels start at most this wide, in radians of parameter angle.
constexpr double LENGTH_PANEL_RADIANS = MATH_PI / 8.0;

// Bisection depth limit for one length panel.
constexpr int MAX_LENGTH_DEPTH = 20;

// Relative agreement between one Gauss panel and its two halves.
constexpr double LENGTH_RELATIVE = 1e-14;

// Rounding floor for the length, in ulps of the placement extent.
constexpr double NOISE_ULPS = 64.0;

// Most segments one linearization makes.
constexpr double MAX_SEGMENTS = 1e6;

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

// cos and sin of t, exact at the multiples of pi / 2 that t rounds to.
void
cosSin(double t, double& c, double& s)
{
    const double q = t / (MATH_PI / 2.0);
    const double k = std::nearbyint(q);
    if (std::abs(q - k) <= QUADRANT_ULPS * EPS * std::max(1.0, std::abs(q))) {
        switch (static_cast<int>(std::fmod(std::fmod(k, 4.0) + 4.0, 4.0))) {
        case 0: c = 1.0; s = 0.0; return;
        case 1: c = 0.0; s = 1.0; return;
        case 2: c = -1.0; s = 0.0; return;
        default: c = 0.0; s = -1.0; return;
        }
    }
    c = std::cos(t);
    s = std::sin(t);
}

// The plan columns a u and b v of the placement.
struct Frame {
    const AffinePlacement& loc;
    double a;
    double b;

    Coordinate point(double t) const
    {
        double c = 0.0;
        double s = 0.0;
        cosSin(t, c, s);
        const double ac = a * c;
        const double bs = b * s;
        return Coordinate(loc.origin.x + ac * loc.u.x + bs * loc.v.x,
                          loc.origin.y + ac * loc.u.y + bs * loc.v.y,
                          loc.origin.z + ac * loc.u.z + bs * loc.v.z);
    }

    double planSpeed(double t) const
    {
        double c = 0.0;
        double s = 0.0;
        cosSin(t, c, s);
        return std::hypot(-a * s * loc.u.x + b * c * loc.v.x,
                          -a * s * loc.u.y + b * c * loc.v.y);
    }

    // Largest singular value of the plan map (cos, sin) -> plan offset,
    // which bounds the plan speed and the plan acceleration.
    double planNorm() const
    {
        const double ux = a * loc.u.x;
        const double uy = a * loc.u.y;
        const double vx = b * loc.v.x;
        const double vy = b * loc.v.y;
        const double p = ux * ux + uy * uy;
        const double q = vx * vx + vy * vy;
        const double r = ux * vx + uy * vy;
        return std::sqrt(0.5 * (p + q + std::hypot(p - q, 2.0 * r)));
    }

    double extent() const
    {
        return std::max(std::abs(loc.origin.x), std::abs(loc.origin.y)) + planNorm();
    }
};

double
gaussPanel(const Frame& frame, double lo, double hi)
{
    const double mid = 0.5 * (lo + hi);
    const double half = 0.5 * (hi - lo);
    double sum = 0.0;
    for (int i = 0; i < GL_N; ++i) {
        sum += GL_W[i] * half * frame.planSpeed(mid + half * GL_X[i]);
    }
    return sum;
}

double
adaptiveLength(const Frame& frame, double lo, double hi, double whole, double noise, int depth)
{
    const double mid = 0.5 * (lo + hi);
    const double left = gaussPanel(frame, lo, mid);
    const double right = gaussPanel(frame, mid, hi);
    const double halves = left + right;
    if (depth >= MAX_LENGTH_DEPTH || !(mid > lo) || !(hi > mid)
            || std::abs(halves - whole) <= std::max(LENGTH_RELATIVE * halves, noise)) {
        return halves;
    }
    return adaptiveLength(frame, lo, mid, left, noise, depth + 1)
           + adaptiveLength(frame, mid, hi, right, noise, depth + 1);
}

} // namespace

EllipticalCurve::EllipticalCurve(const AffinePlacement& location,
                                 double uAxisLength, double vAxisLength,
                                 double startAngle, double endAngle,
                                 bool hasZ, bool hasM,
                                 double startM, double endM,
                                 const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_location{Coordinate(location.origin.x, location.origin.y, hasZ ? location.origin.z : 0.0),
                 Coordinate(location.u.x, location.u.y, hasZ ? location.u.z : 0.0),
                 Coordinate(location.v.x, location.v.y, hasZ ? location.v.z : 0.0)},
      m_uAxis(uAxisLength),
      m_vAxis(vAxisLength),
      m_startAngle(startAngle),
      m_endAngle(endAngle),
      m_startM(hasM ? startM : 0.0),
      m_endM(hasM ? endM : 0.0)
{
    validateConstruction();
    points = evaluateEndpoints();
    envelope = computeEllipticalEnvelope();
}

EllipticalCurve::EllipticalCurve(bool hasZ, bool hasM, const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_location{Coordinate(0.0, 0.0, 0.0), Coordinate(0.0, 0.0, 0.0), Coordinate(0.0, 0.0, 0.0)},
      m_uAxis(0.0),
      m_vAxis(0.0),
      m_startAngle(0.0),
      m_endAngle(0.0),
      m_startM(0.0),
      m_endM(0.0)
{
}

std::unique_ptr<EllipticalCurve>
EllipticalCurve::clone() const
{
    return std::unique_ptr<EllipticalCurve>(cloneImpl());
}

std::string
EllipticalCurve::getGeometryType() const
{
    return "EllipticalCurve";
}

GeometryTypeId
EllipticalCurve::getGeometryTypeId() const
{
    return GEOS_ELLIPTICALCURVE;
}

bool
EllipticalCurve::isFullEllipse() const
{
    return !isEmpty() && sweepsFullTurn();
}

bool
EllipticalCurve::sweepsFullTurn() const
{
    const double scale = std::max({2.0 * MATH_PI, std::abs(m_startAngle), std::abs(m_endAngle)});
    return std::abs(std::abs(m_endAngle - m_startAngle) - 2.0 * MATH_PI) <= FULL_TURN_ULPS * EPS * scale;
}

CoordinateXYZM
EllipticalCurve::getPointAtAngle(double t) const
{
    if (isEmpty()) {
        throw util::IllegalArgumentException("Cannot evaluate an empty EllipticalCurve");
    }
    if (!std::isfinite(t)) {
        throw util::IllegalArgumentException("EllipticalCurve angle must be finite");
    }
    const Coordinate c = Frame{m_location, m_uAxis, m_vAxis}.point(t);
    return CoordinateXYZM(c.x, c.y, hasZ() ? c.z : DoubleNotANumber, DoubleNotANumber);
}

double
EllipticalCurve::getLength() const
{
    if (isEmpty()) {
        return 0.0;
    }
    const Frame frame{m_location, m_uAxis, m_vAxis};
    const double lo = std::min(m_startAngle, m_endAngle);
    const double hi = std::max(m_startAngle, m_endAngle);
    if (!(hi > lo)) {
        return 0.0;
    }
    const double noise = NOISE_ULPS * EPS * frame.extent();
    const int panels = std::max(1, static_cast<int>(std::ceil((hi - lo) / LENGTH_PANEL_RADIANS)));
    double length = 0.0;
    for (int i = 0; i < panels; ++i) {
        const double a = lo + (hi - lo) * (static_cast<double>(i) / panels);
        const double b = (i + 1 == panels) ? hi : lo + (hi - lo) * (static_cast<double>(i + 1) / panels);
        length += adaptiveLength(frame, a, b, gaussPanel(frame, a, b), noise, 0);
    }
    return length;
}

std::unique_ptr<EllipticalCurve>
EllipticalCurve::affineTransform(double m00, double m01, double m02,
                                 double m10, double m11, double m12) const
{
    const double map[] = {m00, m01, m02, m10, m11, m12};
    for (double value : map) {
        if (!std::isfinite(value)) {
            throw util::IllegalArgumentException("EllipticalCurve affine map must be finite");
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
    auto image = getFactory()->createEllipticalCurve(location, m_uAxis, m_vAxis,
                 m_startAngle, m_endAngle, hasZ(), hasM(), m_startM, m_endM);
    image->setSRID(getSRID());
    return image;
}

bool
EllipticalCurve::hasCurvedComponents() const
{
    if (isEmpty() || !(m_endAngle != m_startAngle)) {
        return false;
    }
    // Collinear directions still trace a line back and forth beyond the
    // chord. Only a plan image that is a single point is its endpoints.
    const AffinePlacement& p = m_location;
    return p.u.x != 0.0 || p.u.y != 0.0 || p.v.x != 0.0 || p.v.y != 0.0;
}

void
EllipticalCurve::normalize()
{
    if (isEmpty() || isClosed()) {
        return;
    }
    if (getEndCoordinate().compareTo(getStartCoordinate()) < 0) {
        std::unique_ptr<EllipticalCurve> reversed(reverseImpl());
        points = reversed->points;
        envelope = reversed->envelope;
        std::swap(m_startAngle, m_endAngle);
        std::swap(m_startM, m_endM);
    }
}

void
EllipticalCurve::apply_rw(const CoordinateFilter*)
{
    throw util::UnsupportedOperationException(
        "EllipticalCurve parameters are not edited through coordinate filters.");
}

void
EllipticalCurve::apply_rw(CoordinateSequenceFilter&)
{
    throw util::UnsupportedOperationException(
        "EllipticalCurve parameters are not edited through coordinate filters.");
}

bool
EllipticalCurve::equalsExact(const Geometry* other, double tolerance) const
{
    if (!SimpleCurve::equalsExact(other, tolerance)) {
        return false;
    }
    const auto* e = detail::down_cast<const EllipticalCurve*>(other);
    auto near = [tolerance](double a, double b) {
        return std::abs(a - b) <= tolerance;
    };
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = e->m_location;
    return near(p.origin.x, q.origin.x) && near(p.origin.y, q.origin.y) && near(p.origin.z, q.origin.z)
           && near(p.u.x, q.u.x) && near(p.u.y, q.u.y) && near(p.u.z, q.u.z)
           && near(p.v.x, q.v.x) && near(p.v.y, q.v.y) && near(p.v.z, q.v.z)
           && near(m_uAxis, e->m_uAxis) && near(m_vAxis, e->m_vAxis)
           && near(m_startAngle, e->m_startAngle) && near(m_endAngle, e->m_endAngle)
           && near(m_startM, e->m_startM) && near(m_endM, e->m_endM);
}

bool
EllipticalCurve::equalsIdentical(const Geometry* other) const
{
    if (!SimpleCurve::equalsIdentical(other)) {
        return false;
    }
    const auto& e = static_cast<const EllipticalCurve&>(*other);
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = e.m_location;
    return identicalDouble(p.origin.x, q.origin.x) && identicalDouble(p.origin.y, q.origin.y)
           && identicalDouble(p.origin.z, q.origin.z)
           && identicalDouble(p.u.x, q.u.x) && identicalDouble(p.u.y, q.u.y) && identicalDouble(p.u.z, q.u.z)
           && identicalDouble(p.v.x, q.v.x) && identicalDouble(p.v.y, q.v.y) && identicalDouble(p.v.z, q.v.z)
           && identicalDouble(m_uAxis, e.m_uAxis) && identicalDouble(m_vAxis, e.m_vAxis)
           && identicalDouble(m_startAngle, e.m_startAngle) && identicalDouble(m_endAngle, e.m_endAngle)
           && identicalDouble(m_startM, e.m_startM) && identicalDouble(m_endM, e.m_endM);
}

int
EllipticalCurve::compareToSameClass(const Geometry* geom) const
{
    const auto* e = detail::down_cast<const EllipticalCurve*>(geom);
    const int base = SimpleCurve::compareToSameClass(geom);
    if (base != 0) {
        return base;
    }
    const AffinePlacement& p = m_location;
    const AffinePlacement& q = e->m_location;
    const double fields[][2] = {
        {p.origin.x, q.origin.x}, {p.origin.y, q.origin.y}, {p.origin.z, q.origin.z},
        {p.u.x, q.u.x}, {p.u.y, q.u.y}, {p.u.z, q.u.z},
        {p.v.x, q.v.x}, {p.v.y, q.v.y}, {p.v.z, q.v.z},
        {m_uAxis, e->m_uAxis}, {m_vAxis, e->m_vAxis},
        {m_startAngle, e->m_startAngle}, {m_endAngle, e->m_endAngle},
        {m_startM, e->m_startM}, {m_endM, e->m_endM}
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
EllipticalCurve::appendLinearizedPoints(CoordinateSequence& seq,
                                        const algorithm::CurveToLineParams& params,
                                        std::size_t first) const
{
    if (isEmpty()) {
        return;
    }
    const Frame frame{m_location, m_uAxis, m_vAxis};
    const double sweep = m_endAngle - m_startAngle;

    // Equal steps of parameter angle, each at most the maximum step, and
    // short enough that the chord stays within the maximum deviation: a
    // step h deviates at most h^2 / 8 times the largest acceleration, which
    // the plan norm bounds.
    double step = params.getMaxStepDegrees() * MATH_PI / 180.0;
    const double norm = frame.planNorm();
    if (norm > 0.0 && std::isfinite(params.getMaxDeviation())) {
        step = std::min(step, std::sqrt(8.0 * params.getMaxDeviation() / norm));
    }
    double count = 1.0;
    if (step > 0.0 && std::isfinite(step)) {
        count = std::min(MAX_SEGMENTS, std::max(1.0, std::ceil(std::abs(sweep) / step)));
    }
    const std::size_t segments = static_cast<std::size_t>(count);

    std::vector<Coordinate> pts;
    pts.reserve(segments + 1);
    // The ends are the stored endpoints, which a CompoundCurve may have
    // snapped onto its neighbors.
    pts.push_back(points->getAt<Coordinate>(0));
    for (std::size_t i = 1; i < segments; ++i) {
        pts.push_back(frame.point(m_startAngle + sweep * (static_cast<double>(i) / static_cast<double>(segments))));
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
EllipticalCurve::addLinearizedPoints(CoordinateSequence& seq,
                                     const algorithm::CurveToLineParams& params) const
{
    appendLinearizedPoints(seq, params, 1);
}

LineString*
EllipticalCurve::getLinearizedImpl(const algorithm::CurveToLineParams& params) const
{
    if (isEmpty()) {
        return getFactory()->createLineString(hasZ(), hasM()).release();
    }
    auto seq = std::make_unique<CoordinateSequence>(0, hasZ(), hasM());
    appendLinearizedPoints(*seq, params, 0);
    return getFactory()->createLineString(std::move(seq)).release();
}

EllipticalCurve*
EllipticalCurve::reverseImpl() const
{
    if (isEmpty()) {
        return cloneImpl();
    }
    // Swapping the angles traces the same arc backwards (7.6.1 Desc 21).
    return getFactory()->createEllipticalCurve(
               m_location, m_uAxis, m_vAxis, m_endAngle, m_startAngle,
               hasZ(), hasM(), m_endM, m_startM).release();
}

std::unique_ptr<CoordinateSequence>
EllipticalCurve::evaluateEndpoints() const
{
    const Frame frame{m_location, m_uAxis, m_vAxis};
    const Coordinate start = frame.point(m_startAngle);
    // A full ellipse closes exactly, whatever cos and sin of the end angle
    // round to.
    const Coordinate end = sweepsFullTurn() ? start : frame.point(m_endAngle);
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
EllipticalCurve::computeEllipticalEnvelope() const
{
    Envelope env;
    if (isEmpty()) {
        return env;
    }
    env.expandToInclude(points->getAt<CoordinateXY>(0));
    env.expandToInclude(points->getAt<CoordinateXY>(1));

    const Frame frame{m_location, m_uAxis, m_vAxis};
    const double lo = std::min(m_startAngle, m_endAngle);
    const double hi = std::max(m_startAngle, m_endAngle);
    // x(t) = x0 + A cos(t) + B sin(t) is extreme where tan(t) = B / A, at
    // atan2(B, A) + k pi; likewise for y.
    auto addExtremes = [&](double coeffCos, double coeffSin) {
        if (!(std::hypot(coeffCos, coeffSin) > 0.0)) {
            return;
        }
        const double base = std::atan2(coeffSin, coeffCos);
        const double kFirst = std::ceil((lo - base) / MATH_PI);
        for (double k = kFirst; ; k += 1.0) {
            const double t = base + k * MATH_PI;
            if (!(t <= hi)) {
                break;
            }
            if (t >= lo) {
                env.expandToInclude(frame.point(t));
            }
        }
    };
    addExtremes(m_uAxis * m_location.u.x, m_vAxis * m_location.v.x);
    addExtremes(m_uAxis * m_location.u.y, m_vAxis * m_location.v.y);
    return env;
}

void
EllipticalCurve::validateConstruction() const
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
                "EllipticalCurve reference location must be finite");
        }
    }
    if (!std::isfinite(m_uAxis) || !std::isfinite(m_vAxis) || !(m_uAxis > 0.0) || !(m_vAxis > 0.0)) {
        throw util::IllegalArgumentException(
            "EllipticalCurve axis lengths must be finite and positive");
    }
    if (!std::isfinite(m_startAngle) || !std::isfinite(m_endAngle)
            || !std::isfinite(m_endAngle - m_startAngle)) {
        throw util::IllegalArgumentException("EllipticalCurve angles must be finite");
    }
    const double scale = std::max({2.0 * MATH_PI, std::abs(m_startAngle), std::abs(m_endAngle)});
    if (std::abs(m_endAngle - m_startAngle) > 2.0 * MATH_PI + FULL_TURN_ULPS * EPS * scale) {
        throw util::IllegalArgumentException(
            "EllipticalCurve angles must not differ by more than a full turn");
    }
    if (hasM() && (!std::isfinite(m_startM) || !std::isfinite(m_endM))) {
        throw util::IllegalArgumentException(
            "EllipticalCurve STARTM and ENDM must both be finite");
    }
}

void
EllipticalCurve::snapEndpoint(std::size_t index, const CoordinateXY& target)
{
    const CoordinateXY& stored = points->getAt<CoordinateXY>(index);
    if (stored == target) {
        return;
    }
    const Frame frame{m_location, m_uAxis, m_vAxis};
    const double tolerance = ENDPOINT_SNAP_ULPS * EPS * frame.extent();
    if (!(stored.distance(target) <= tolerance)) {
        return;
    }
    auto seq = points->clone();
    seq->setOrdinate(index, CoordinateSequence::X, target.x);
    seq->setOrdinate(index, CoordinateSequence::Y, target.y);
    // A full ellipse stays closed.
    if (isFullEllipse()) {
        const std::size_t other = 1 - index;
        seq->setOrdinate(other, CoordinateSequence::X, target.x);
        seq->setOrdinate(other, CoordinateSequence::Y, target.y);
    }
    points = std::move(seq);
    envelope.expandToInclude(target);
}

}
}
