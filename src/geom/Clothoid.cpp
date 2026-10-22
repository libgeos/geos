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

#include <geos/geom/Clothoid.h>

#include <algorithm/ClothoidMath.h>
#include <geos/algorithm/CurveToLineParams.h>
#include <geos/constants.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace geos {
namespace geom {

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

// Relative gap allowed when testing a similarity frame.
constexpr double SIMILARITY_RELATIVE = 1e-12;

// Endpoint snap tolerance, in ulps of the reference location extent.
// Evaluation rounding is about one ulp. Reading 16-digit WKT perturbs the
// origin, directions, scale and distances by about two ulps each.
constexpr double ENDPOINT_SNAP_ULPS = 32.0;

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

CoordinateXY
fresnelOffset(double t, double a2)
{
    return algorithm::clothoid::constructiveOffset(t, a2);
}

struct Frame {
    double locX, locY, locZ;
    double ux, uy, uz;
    double vx, vy, vz;
    double scale;
    double t0, t1;
    double startM, endM;
};

Frame
frameOf(const ClothoidReferenceLocation& location,
        double scale, double t0, double t1, double startM, double endM)
{
    Frame frame;
    frame.locX = location.origin.x;
    frame.locY = location.origin.y;
    frame.locZ = location.origin.z;
    frame.ux = location.u.x;
    frame.uy = location.u.y;
    frame.uz = location.u.z;
    frame.vx = location.v.x;
    frame.vy = location.v.y;
    frame.vz = location.v.z;
    frame.scale = scale;
    frame.t0 = t0;
    frame.t1 = t1;
    frame.startM = startM;
    frame.endM = endM;
    return frame;
}

void
loadConstructiveFrame(const Frame& frame, algorithm::clothoid::Parameters& curve)
{
    // A negative scale is stored for round-trip and evaluated as |A|.
    const double a2 = frame.scale * frame.scale;
    const CoordinateXY offset = fresnelOffset(frame.t0, a2);
    const double headingDt = (frame.t0 * frame.t0) / (2.0 * a2);
    const double direction = (frame.t1 >= frame.t0) ? 1.0 : -1.0;
    curve.x0 = offset.x;
    curve.y0 = offset.y;
    curve.theta0 = (direction > 0.0) ? headingDt : headingDt + MATH_PI;
    curve.length = std::abs(frame.t1 - frame.t0);
    curve.k0 = direction * frame.t0 / a2;
    curve.k1 = direction * frame.t1 / a2;
}

void
mapAffine(const Frame& frame, double xc, double yc, double& x, double& y, double& z)
{
    x = frame.locX + xc * frame.ux + yc * frame.vx;
    y = frame.locY + xc * frame.uy + yc * frame.vy;
    z = frame.locZ + xc * frame.uz + yc * frame.vz;
}

void
pointOnFrame(const Frame& frame, double travel, double& x, double& y, double& z, double& m)
{
    algorithm::clothoid::Parameters curve;
    loadConstructiveFrame(frame, curve);
    double xc = curve.x0;
    double yc = curve.y0;
    if (curve.length > 0.0 && travel > 0.0) {
        const double s = std::min(travel, curve.length);
        const CoordinateXY at = algorithm::clothoid::position(curve, s);
        xc = at.x;
        yc = at.y;
    }
    mapAffine(frame, xc, yc, x, y, z);
    if (!(curve.length > 0.0)) {
        m = frame.startM;
        return;
    }
    const double frac = std::min(1.0, std::max(0.0, travel / curve.length));
    m = frame.startM + (frame.endM - frame.startM) * frac;
}

bool
similarityScale(const Frame& frame, double& scale, int& orientation)
{
    const double u2 = frame.ux * frame.ux + frame.uy * frame.uy + frame.uz * frame.uz;
    const double v2 = frame.vx * frame.vx + frame.vy * frame.vy + frame.vz * frame.vz;
    const double dot = frame.ux * frame.vx + frame.uy * frame.vy + frame.uz * frame.vz;
    const double uLen = std::sqrt(u2);
    const double vLen = std::sqrt(v2);
    if (!(uLen > 0.0) || !(vLen > 0.0)) {
        return false;
    }
    const double tol = SIMILARITY_RELATIVE * std::max(1.0, uLen);
    if (std::abs(uLen - vLen) > tol || std::abs(dot) > tol * uLen) {
        return false;
    }
    scale = uLen;
    const double crossZ = frame.ux * frame.vy - frame.uy * frame.vx;
    orientation = (crossZ < 0.0) ? -1 : 1;
    return true;
}

double
operatorNorm(const Frame& frame)
{
    const double a = frame.ux * frame.ux + frame.uy * frame.uy + frame.uz * frame.uz;
    const double b = frame.vx * frame.vx + frame.vy * frame.vy + frame.vz * frame.vz;
    const double c = frame.ux * frame.vx + frame.uy * frame.vy + frame.uz * frame.vz;
    const double disc = std::sqrt(std::max(0.0, (a - b) * (a - b) + 4.0 * c * c));
    return std::sqrt(0.5 * (a + b + disc));
}

double
speedAt(const Frame& frame, double t)
{
    const double a2 = frame.scale * frame.scale;
    const double theta = (t * t) / (2.0 * a2);
    const double c = std::cos(theta);
    const double s = std::sin(theta);
    const double dx = c * frame.ux + s * frame.vx;
    const double dy = c * frame.uy + s * frame.vy;
    const double dz = c * frame.uz + s * frame.vz;
    return std::hypot(dx, dy, dz);
}

double
integrateSpeed(const Frame& frame)
{
    const double a2 = frame.scale * frame.scale;
    const double tLo = std::min(frame.t0, frame.t1);
    const double tHi = std::max(frame.t0, frame.t1);
    if (!(tHi > tLo) || !(a2 > 0.0)) {
        return 0.0;
    }
    const double dTheta = std::abs(tHi * tHi - tLo * tLo) / (2.0 * a2);
    if (!std::isfinite(dTheta)) {
        throw util::IllegalArgumentException("Clothoid turn is too large to integrate");
    }
    const double panelCount = std::ceil(dTheta / algorithm::clothoid::QUADRATURE_PANEL_RADIANS);
    if (!(panelCount <= static_cast<double>(algorithm::clothoid::QUADRATURE_MAX_PANELS))) {
        throw util::IllegalArgumentException("Clothoid turn is too large to integrate");
    }
    const std::size_t panels = std::max<std::size_t>(
                                   1, panelCount > 0.0 ? static_cast<std::size_t>(panelCount) : 1);
    const double width = (tHi - tLo) / static_cast<double>(panels);
    double sum = 0.0;
    for (std::size_t p = 0; p < panels; ++p) {
        const double a = tLo + width * static_cast<double>(p);
        const double mid = a + 0.5 * width;
        const double half = 0.5 * width;
        for (int i = 0; i < GL_N; ++i) {
            sum += GL_W[i] * half * speedAt(frame, mid + half * GL_X[i]);
        }
    }
    return sum;
}

double
headingOf(const Frame& frame, double t)
{
    const double a2 = frame.scale * frame.scale;
    const double theta = (t * t) / (2.0 * a2);
    const double c = std::cos(theta);
    const double s = std::sin(theta);
    double dx = c * frame.ux + s * frame.vx;
    double dy = c * frame.uy + s * frame.vy;
    if (frame.t1 < frame.t0) {
        dx = -dx;
        dy = -dy;
    }
    if (!(std::hypot(dx, dy) > 0.0)) {
        return 0.0;
    }
    return std::atan2(dy, dx);
}

double
curvatureOf(const Frame& frame, double t)
{
    const double a2 = frame.scale * frame.scale;
    const double direction = (frame.t1 >= frame.t0) ? 1.0 : -1.0;
    const double kappa = direction * t / a2;
    double scale = 1.0;
    int orientation = 1;
    if (similarityScale(frame, scale, orientation) && scale > 0.0) {
        return orientation * kappa / scale;
    }
    return kappa;
}

std::unique_ptr<CoordinateSequence>
endpointsFor(const Frame& frame, bool hasZ, bool hasM)
{
    auto seq = std::make_unique<CoordinateSequence>(static_cast<std::size_t>(2), hasZ, hasM);
    double x0 = 0.0;
    double y0 = 0.0;
    double z0 = 0.0;
    double m0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    double z1 = 0.0;
    double m1 = 0.0;
    pointOnFrame(frame, 0.0, x0, y0, z0, m0);
    const double length = std::abs(frame.t1 - frame.t0);
    pointOnFrame(frame, length, x1, y1, z1, m1);
    CoordinateXYZM start(x0, y0,
                         hasZ ? z0 : DoubleNotANumber,
                         hasM ? m0 : DoubleNotANumber);
    CoordinateXYZM end(x1, y1,
                       hasZ ? z1 : DoubleNotANumber,
                       hasM ? m1 : DoubleNotANumber);
    seq->setAt(start, 0);
    seq->setAt(end, 1);
    return seq;
}

} // namespace

Clothoid::Clothoid(const ClothoidReferenceLocation& location,
                   double scaleFactor,
                   double startDistance,
                   double endDistance,
                   bool hasZ, bool hasM,
                   double startM, double endM,
                   const GeometryFactory& factory)
    : SimpleCurve(endpointsFor(frameOf(location, scaleFactor, startDistance, endDistance,
                                       hasM ? startM : 0.0, hasM ? endM : 0.0),
                               hasZ, hasM),
                  false, factory),
      m_origin(location.origin.x, location.origin.y, hasZ ? location.origin.z : 0.0),
      m_u(location.u.x, location.u.y, hasZ ? location.u.z : 0.0),
      m_v(location.v.x, location.v.y, hasZ ? location.v.z : 0.0),
      m_scale(scaleFactor),
      m_t0(startDistance),
      m_t1(endDistance),
      m_startM(hasM ? startM : 0.0),
      m_endM(hasM ? endM : 0.0)
{
    validateConstruction();
    envelope = computeClothoidEnvelope();
}

Clothoid::Clothoid(bool hasZ, bool hasM, const GeometryFactory& factory)
    : SimpleCurve(std::make_unique<CoordinateSequence>(static_cast<std::size_t>(0), hasZ, hasM), false, factory),
      m_origin(0.0, 0.0, 0.0),
      m_u(0.0, 0.0, 0.0),
      m_v(0.0, 0.0, 0.0),
      m_scale(0.0),
      m_t0(0.0),
      m_t1(0.0),
      m_startM(0.0),
      m_endM(0.0)
{
}

std::unique_ptr<Clothoid>
Clothoid::clone() const
{
    return std::unique_ptr<Clothoid>(cloneImpl());
}

std::string
Clothoid::getGeometryType() const
{
    return "Clothoid";
}

GeometryTypeId
Clothoid::getGeometryTypeId() const
{
    return GEOS_CLOTHOID;
}

double
Clothoid::getLength() const
{
    if (isEmpty()) {
        return 0.0;
    }
    Frame frame = frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM);
    double scale = 1.0;
    int orientation = 1;
    if (similarityScale(frame, scale, orientation)) {
        return scale * std::abs(m_t1 - m_t0);
    }
    return integrateSpeed(frame);
}

double
Clothoid::getStartHeading() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return travelHeading(m_t0);
}

double
Clothoid::getStartCurvature() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return travelCurvature(m_t0);
}

double
Clothoid::getEndCurvature() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return travelCurvature(m_t1);
}

double
Clothoid::getEndHeading() const
{
    if (isEmpty()) {
        return 0.0;
    }
    return travelHeading(m_t1);
}

ClothoidReferenceLocation
Clothoid::getReferenceLocation() const
{
    ClothoidReferenceLocation location;
    location.origin = m_origin;
    location.u = m_u;
    location.v = m_v;
    return location;
}

bool
Clothoid::hasCurvedComponents() const
{
    if (isEmpty() || !(std::abs(m_t1 - m_t0) > 0.0)) {
        return false;
    }
    const double cross = m_u.x * m_v.y - m_u.y * m_v.x;
    const double crossZ = m_u.y * m_v.z - m_u.z * m_v.y;
    const double crossY = m_u.z * m_v.x - m_u.x * m_v.z;
    return cross != 0.0 || crossZ != 0.0 || crossY != 0.0;
}

void
Clothoid::normalize()
{
    if (isEmpty() || isClosed()) {
        return;
    }
    if (getEndCoordinate().compareTo(getStartCoordinate()) < 0) {
        std::unique_ptr<Clothoid> reversed(reverseImpl());
        points = reversed->points;
        envelope = reversed->envelope;
        m_origin = reversed->m_origin;
        m_u = reversed->m_u;
        m_v = reversed->m_v;
        m_scale = reversed->m_scale;
        m_t0 = reversed->m_t0;
        m_t1 = reversed->m_t1;
        m_startM = reversed->m_startM;
        m_endM = reversed->m_endM;
    }
}

void
Clothoid::apply_rw(const CoordinateFilter*)
{
    throw util::UnsupportedOperationException(
        "Clothoid parameters are not edited through coordinate filters.");
}

void
Clothoid::apply_rw(CoordinateSequenceFilter&)
{
    throw util::UnsupportedOperationException(
        "Clothoid parameters are not edited through coordinate filters.");
}

bool
Clothoid::equalsExact(const Geometry* other, double tolerance) const
{
    if (!SimpleCurve::equalsExact(other, tolerance)) {
        return false;
    }
    const Clothoid* clothoid = detail::down_cast<const Clothoid*>(other);
    auto near = [tolerance](double a, double b) {
        return std::abs(a - b) <= tolerance;
    };
    return near(m_origin.x, clothoid->m_origin.x)
           && near(m_origin.y, clothoid->m_origin.y)
           && near(m_origin.z, clothoid->m_origin.z)
           && near(m_u.x, clothoid->m_u.x)
           && near(m_u.y, clothoid->m_u.y)
           && near(m_u.z, clothoid->m_u.z)
           && near(m_v.x, clothoid->m_v.x)
           && near(m_v.y, clothoid->m_v.y)
           && near(m_v.z, clothoid->m_v.z)
           && near(m_scale, clothoid->m_scale)
           && near(m_t0, clothoid->m_t0)
           && near(m_t1, clothoid->m_t1)
           && near(m_startM, clothoid->m_startM)
           && near(m_endM, clothoid->m_endM);
}

bool
Clothoid::equalsIdentical(const Geometry* other) const
{
    if (!SimpleCurve::equalsIdentical(other)) {
        return false;
    }
    const auto& clothoid = static_cast<const Clothoid&>(*other);
    return identicalDouble(m_origin.x, clothoid.m_origin.x)
           && identicalDouble(m_origin.y, clothoid.m_origin.y)
           && identicalDouble(m_origin.z, clothoid.m_origin.z)
           && identicalDouble(m_u.x, clothoid.m_u.x)
           && identicalDouble(m_u.y, clothoid.m_u.y)
           && identicalDouble(m_u.z, clothoid.m_u.z)
           && identicalDouble(m_v.x, clothoid.m_v.x)
           && identicalDouble(m_v.y, clothoid.m_v.y)
           && identicalDouble(m_v.z, clothoid.m_v.z)
           && identicalDouble(m_scale, clothoid.m_scale)
           && identicalDouble(m_t0, clothoid.m_t0)
           && identicalDouble(m_t1, clothoid.m_t1)
           && identicalDouble(m_startM, clothoid.m_startM)
           && identicalDouble(m_endM, clothoid.m_endM);
}

int
Clothoid::compareToSameClass(const Geometry* geom) const
{
    const auto* clothoid = detail::down_cast<const Clothoid*>(geom);
    const int base = SimpleCurve::compareToSameClass(geom);
    if (base != 0) {
        return base;
    }
    const double fields[][2] = {
        {m_origin.x, clothoid->m_origin.x},
        {m_origin.y, clothoid->m_origin.y},
        {m_origin.z, clothoid->m_origin.z},
        {m_u.x, clothoid->m_u.x},
        {m_u.y, clothoid->m_u.y},
        {m_u.z, clothoid->m_u.z},
        {m_v.x, clothoid->m_v.x},
        {m_v.y, clothoid->m_v.y},
        {m_v.z, clothoid->m_v.z},
        {m_scale, clothoid->m_scale},
        {m_t0, clothoid->m_t0},
        {m_t1, clothoid->m_t1},
        {m_startM, clothoid->m_startM},
        {m_endM, clothoid->m_endM}
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
Clothoid::appendLinearizedPoints(CoordinateSequence& seq,
                                 const algorithm::CurveToLineParams& params,
                                 std::size_t first) const
{
    if (isEmpty()) {
        return;
    }

    const double stepRadians = params.getMaxStepDegrees() * MATH_PI / 180.0;
    Frame frame = frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM);
    algorithm::clothoid::Parameters curve;
    loadConstructiveFrame(frame, curve);
    const double sigma = operatorNorm(frame);
    double deviation = params.getMaxDeviation();
    if (sigma > 0.0 && std::isfinite(sigma)) {
        deviation /= sigma;
    }
    std::size_t segments = algorithm::clothoid::linearizeSegmentCount(
                               curve, stepRadians, deviation);
    if (segments == 0) {
        segments = 1;
    }

    for (std::size_t i = first; i <= segments; ++i) {
        const double s = curve.length * (static_cast<double>(i) / static_cast<double>(segments));
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double m = 0.0;
        pointOnFrame(frame, s, x, y, z, m);
        CoordinateXYZM ordinate(x, y,
                                hasZ() ? z : DoubleNotANumber,
                                hasM() ? m : DoubleNotANumber);
        seq.add(ordinate);
    }
}

void
Clothoid::addLinearizedPoints(CoordinateSequence& seq,
                              const algorithm::CurveToLineParams& params) const
{
    appendLinearizedPoints(seq, params, 1);
}

LineString*
Clothoid::getLinearizedImpl(const algorithm::CurveToLineParams& params) const
{
    if (isEmpty()) {
        return getFactory()->createLineString(hasZ(), hasM()).release();
    }

    auto seq = std::make_unique<CoordinateSequence>(0, hasZ(), hasM());
    appendLinearizedPoints(*seq, params, 0);
    return getFactory()->createLineString(std::move(seq)).release();
}

Clothoid*
Clothoid::reverseImpl() const
{
    if (isEmpty()) {
        return cloneImpl();
    }
    return getFactory()->createClothoid(
               getReferenceLocation(), m_scale, m_t1, m_t0,
               hasZ(), hasM(), m_endM, m_startM).release();
}

Envelope
Clothoid::computeClothoidEnvelope() const
{
    Envelope env;
    if (isEmpty()) {
        return env;
    }
    Frame frame = frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM);
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double m = 0.0;
    pointOnFrame(frame, 0.0, x, y, z, m);
    env.expandToInclude(x, y);
    const double length = std::abs(m_t1 - m_t0);
    pointOnFrame(frame, length, x, y, z, m);
    env.expandToInclude(x, y);

    const double a2 = m_scale * m_scale;
    if (!(a2 > 0.0) || !(length > 0.0)) {
        return env;
    }
    const double tLo = std::min(m_t0, m_t1);
    const double tHi = std::max(m_t0, m_t1);
    auto thetaOf = [a2](double t) {
        return (t * t) / (2.0 * a2);
    };
    double thetaMin = 0.0;
    double thetaMax = 0.0;
    if (tLo <= 0.0 && tHi >= 0.0) {
        thetaMin = 0.0;
        thetaMax = std::max(thetaOf(tLo), thetaOf(tHi));
    }
    else {
        thetaMin = std::min(thetaOf(tLo), thetaOf(tHi));
        thetaMax = std::max(thetaOf(tLo), thetaOf(tHi));
    }

    auto addIfInterior = [&](double t) {
        if (t > tLo && t < tHi) {
            const CoordinateXY constructive = fresnelOffset(t, a2);
            const double xc = constructive.x;
            const double yc = constructive.y;
            double px = 0.0;
            double py = 0.0;
            double pz = 0.0;
            mapAffine(frame, xc, yc, px, py, pz);
            env.expandToInclude(px, py);
        }
    };

    auto solveAxis = [&](double coeffCos, double coeffSin) {
        const double norm = std::hypot(coeffCos, coeffSin);
        if (!(norm > 0.0)) {
            return;
        }
        const double alpha = std::atan2(coeffSin, coeffCos);
        const double base = alpha + MATH_PI / 2.0;
        // The net turn is at most MAX_TURN_RADIANS. Absolute constructive
        // heading is not, so the half-turn index does not fit in int.
        // Walk the window [thetaMin, thetaMax] instead of casting it.
        const double halfTurns = (thetaMax - thetaMin) / MATH_PI;
        const double first = std::floor((thetaMin - base) / MATH_PI) - 1.0;
        if (!std::isfinite(halfTurns) || !std::isfinite(first)
                || halfTurns < 0.0
                || halfTurns > static_cast<double>(std::numeric_limits<int>::max() - 5)) {
            throw util::IllegalArgumentException(
                "Clothoid turn is too large to integrate");
        }
        const int steps = static_cast<int>(std::ceil(halfTurns)) + 5;
        for (int i = 0; i < steps; ++i) {
            const double theta = base + (first + static_cast<double>(i)) * MATH_PI;
            if (theta < thetaMin || theta > thetaMax) {
                continue;
            }
            if (theta == 0.0) {
                addIfInterior(0.0);
                continue;
            }
            if (theta < 0.0) {
                continue;
            }
            const double root = std::sqrt(theta * 2.0 * a2);
            addIfInterior(root);
            addIfInterior(-root);
        }
    };
    solveAxis(m_u.x, m_v.x);
    solveAxis(m_u.y, m_v.y);
    return env;
}

void
Clothoid::validateConstruction() const
{
    if (!std::isfinite(m_scale)
            || !(std::abs(m_scale) >= algorithm::clothoid::MIN_ABS_SCALE)
            || !(std::abs(m_scale) <= algorithm::clothoid::MAX_ABS_SCALE)) {
        throw util::IllegalArgumentException(
            "Clothoid scale factor must be finite and non-zero");
    }
    if (!std::isfinite(m_t0) || !std::isfinite(m_t1)) {
        throw util::IllegalArgumentException(
            "Clothoid start and end distance must be finite");
    }
    const double a2 = m_scale * m_scale;
    const double turn = std::abs(m_t1 * m_t1 - m_t0 * m_t0) / (2.0 * a2);
    if (!std::isfinite(turn) || turn > algorithm::clothoid::MAX_TURN_RADIANS) {
        throw util::IllegalArgumentException("Clothoid turn is too large to integrate");
    }
    const double coords[] = {
        m_origin.x, m_origin.y, m_origin.z,
        m_u.x, m_u.y, m_u.z,
        m_v.x, m_v.y, m_v.z
    };
    for (double value : coords) {
        if (!std::isfinite(value)) {
            throw util::IllegalArgumentException(
                "Clothoid reference location must be finite");
        }
    }
    if (hasM() && (!std::isfinite(m_startM) || !std::isfinite(m_endM))) {
        throw util::IllegalArgumentException(
            "Clothoid STARTM and ENDM must both be finite");
    }
}

void
Clothoid::snapEndpoint(std::size_t index, const CoordinateXY& target)
{
    const CoordinateXY& stored = points->getAt<CoordinateXY>(index);
    if (stored == target) {
        return;
    }
    // The endpoints are evaluated from the reference location, whose terms are
    // at most this large. Evaluating it again, or from 16-digit WKT, moves an
    // endpoint by about one ulp of it.
    Frame frame = frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM);
    const double extent = std::max(std::abs(m_origin.x), std::abs(m_origin.y))
                          + operatorNorm(frame) * std::max(std::abs(m_t0), std::abs(m_t1));
    const double tolerance = ENDPOINT_SNAP_ULPS * std::numeric_limits<double>::epsilon() * extent;
    if (!(stored.distance(target) <= tolerance)) {
        return;
    }
    auto seq = points->clone();
    seq->setOrdinate(index, CoordinateSequence::X, target.x);
    seq->setOrdinate(index, CoordinateSequence::Y, target.y);
    points = std::move(seq);
    envelope.expandToInclude(target);
}

void
Clothoid::pointOnCurve(double travel, double& x, double& y, double& z, double& m) const
{
    pointOnFrame(frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM),
                 travel, x, y, z, m);
}

double
Clothoid::travelHeading(double constructiveDistance) const
{
    return headingOf(frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM),
                     constructiveDistance);
}

double
Clothoid::travelCurvature(double constructiveDistance) const
{
    return curvatureOf(frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM),
                       constructiveDistance);
}

}
}
