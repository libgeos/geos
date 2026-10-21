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

#include <geos/algorithm/Clothoid.h>
#include <geos/algorithm/CurveToLineParams.h>
#include <geos/constants.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>
#include <geos/util/IllegalArgumentException.h>

#include <algorithm>
#include <cmath>
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

void
fresnelOffset(double t, double a2, double& x, double& y, double& headingDt)
{
    headingDt = (t * t) / (2.0 * a2);
    if (t == 0.0) {
        x = 0.0;
        y = 0.0;
        return;
    }
    const double len = std::abs(t);
    algorithm::clothoid::Parameters curve;
    curve.x0 = 0.0;
    curve.y0 = 0.0;
    curve.theta0 = 0.0;
    curve.k0 = 0.0;
    curve.k1 = len / a2;
    curve.length = len;
    algorithm::clothoid::position(curve, len, x, y);
    if (t < 0.0) {
        x = -x;
        y = -y;
    }
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
    frame.locX = location.x;
    frame.locY = location.y;
    frame.locZ = location.z;
    frame.ux = location.ux;
    frame.uy = location.uy;
    frame.uz = location.uz;
    frame.vx = location.vx;
    frame.vy = location.vy;
    frame.vz = location.vz;
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
    double x0 = 0.0;
    double y0 = 0.0;
    double headingDt = 0.0;
    fresnelOffset(frame.t0, a2, x0, y0, headingDt);
    const double direction = (frame.t1 >= frame.t0) ? 1.0 : -1.0;
    curve.x0 = x0;
    curve.y0 = y0;
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
        algorithm::clothoid::position(curve, s, xc, yc);
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
    const double tol = 1e-12 * std::max(1.0, uLen);
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
    int panels = static_cast<int>(std::ceil(dTheta / 0.25));
    if (panels < 1) {
        panels = 1;
    }
    if (panels > 4096) {
        panels = 4096;
    }
    const double width = (tHi - tLo) / static_cast<double>(panels);
    double sum = 0.0;
    for (int p = 0; p < panels; ++p) {
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
      m_locX(location.x),
      m_locY(location.y),
      m_locZ(hasZ ? location.z : 0.0),
      m_ux(location.ux),
      m_uy(location.uy),
      m_uz(hasZ ? location.uz : 0.0),
      m_vx(location.vx),
      m_vy(location.vy),
      m_vz(hasZ ? location.vz : 0.0),
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
      m_locX(0.0),
      m_locY(0.0),
      m_locZ(0.0),
      m_ux(0.0),
      m_uy(0.0),
      m_uz(0.0),
      m_vx(0.0),
      m_vy(0.0),
      m_vz(0.0),
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
Clothoid::getStartTangent() const
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
Clothoid::getEndTangent() const
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
    location.x = m_locX;
    location.y = m_locY;
    location.z = m_locZ;
    location.ux = m_ux;
    location.uy = m_uy;
    location.uz = m_uz;
    location.vx = m_vx;
    location.vy = m_vy;
    location.vz = m_vz;
    return location;
}

bool
Clothoid::hasCurvedComponents() const
{
    if (isEmpty() || !(std::abs(m_t1 - m_t0) > 0.0)) {
        return false;
    }
    if (m_t0 == 0.0 && m_t1 == 0.0) {
        return false;
    }
    const double cross = m_ux * m_vy - m_uy * m_vx;
    const double crossZ = m_uy * m_vz - m_uz * m_vy;
    const double crossY = m_uz * m_vx - m_ux * m_vz;
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
        m_locX = reversed->m_locX;
        m_locY = reversed->m_locY;
        m_locZ = reversed->m_locZ;
        m_ux = reversed->m_ux;
        m_uy = reversed->m_uy;
        m_uz = reversed->m_uz;
        m_vx = reversed->m_vx;
        m_vy = reversed->m_vy;
        m_vz = reversed->m_vz;
        m_scale = reversed->m_scale;
        m_t0 = reversed->m_t0;
        m_t1 = reversed->m_t1;
        m_startM = reversed->m_startM;
        m_endM = reversed->m_endM;
    }
}

std::unique_ptr<Geometry>
Clothoid::convexHull() const
{
    if (hasCurvedComponents()) {
        throw util::UnsupportedOperationException(
            "Clothoid convex hull is not implemented. Call getLinearized first.");
    }
    return Geometry::convexHull();
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
    return near(m_locX, clothoid->m_locX)
        && near(m_locY, clothoid->m_locY)
        && near(m_locZ, clothoid->m_locZ)
        && near(m_ux, clothoid->m_ux)
        && near(m_uy, clothoid->m_uy)
        && near(m_uz, clothoid->m_uz)
        && near(m_vx, clothoid->m_vx)
        && near(m_vy, clothoid->m_vy)
        && near(m_vz, clothoid->m_vz)
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
    return identicalDouble(m_locX, clothoid.m_locX)
        && identicalDouble(m_locY, clothoid.m_locY)
        && identicalDouble(m_locZ, clothoid.m_locZ)
        && identicalDouble(m_ux, clothoid.m_ux)
        && identicalDouble(m_uy, clothoid.m_uy)
        && identicalDouble(m_uz, clothoid.m_uz)
        && identicalDouble(m_vx, clothoid.m_vx)
        && identicalDouble(m_vy, clothoid.m_vy)
        && identicalDouble(m_vz, clothoid.m_vz)
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
        {m_locX, clothoid->m_locX},
        {m_locY, clothoid->m_locY},
        {m_locZ, clothoid->m_locZ},
        {m_ux, clothoid->m_ux},
        {m_uy, clothoid->m_uy},
        {m_uz, clothoid->m_uz},
        {m_vx, clothoid->m_vx},
        {m_vy, clothoid->m_vy},
        {m_vz, clothoid->m_vz},
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

LineString*
Clothoid::getLinearizedImpl(const algorithm::CurveToLineParams& params) const
{
    if (isEmpty()) {
        return getFactory()->createLineString(hasZ(), hasM()).release();
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

    auto seq = std::make_unique<CoordinateSequence>(segments + 1, hasZ(), hasM());
    for (std::size_t i = 0; i <= segments; ++i) {
        const double s = curve.length * (static_cast<double>(i) / static_cast<double>(segments));
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double m = 0.0;
        pointOnFrame(frame, s, x, y, z, m);
        CoordinateXYZM ordinate(x, y,
                                hasZ() ? z : DoubleNotANumber,
                                hasM() ? m : DoubleNotANumber);
        seq->setAt(ordinate, i);
    }
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
    if ((thetaMax - thetaMin) / MATH_PI > 100000.0) {
        return env;
    }

    auto addIfInterior = [&](double t) {
        if (t > tLo && t < tHi) {
            double xc = 0.0;
            double yc = 0.0;
            double heading = 0.0;
            fresnelOffset(t, a2, xc, yc, heading);
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
        const int n0 = static_cast<int>(std::floor((thetaMin - base) / MATH_PI)) - 1;
        const int n1 = static_cast<int>(std::ceil((thetaMax - base) / MATH_PI)) + 1;
        for (int n = n0; n <= n1; ++n) {
            const double theta = base + static_cast<double>(n) * MATH_PI;
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
    solveAxis(m_ux, m_vx);
    solveAxis(m_uy, m_vy);
    return env;
}

void
Clothoid::validateConstruction() const
{
    if (!std::isfinite(m_scale) || m_scale == 0.0) {
        throw util::IllegalArgumentException(
            "Clothoid scale factor must be finite and non-zero");
    }
    if (!std::isfinite(m_t0) || !std::isfinite(m_t1)) {
        throw util::IllegalArgumentException(
            "Clothoid start and end distance must be finite");
    }
    const double coords[] = {
        m_locX, m_locY, m_locZ, m_ux, m_uy, m_uz, m_vx, m_vy, m_vz
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
Clothoid::loadConstructive(algorithm::clothoid::Parameters& curve) const
{
    loadConstructiveFrame(
        frameOf(getReferenceLocation(), m_scale, m_t0, m_t1, m_startM, m_endM), curve);
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
