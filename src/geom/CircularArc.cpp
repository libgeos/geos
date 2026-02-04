/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Angle.h>
#include <geos/geom/CircularArc.h>
#include <geos/triangulate/quadedge/TrianglePredicate.h>
#include <sstream>

namespace geos::geom {

CircularArc::CircularArc() :
    m_seq(nullptr),
    m_center_known(true),
    m_radius_known(true),
    m_orientation_known(true),
    m_own_coordinates(true)
{}

template<typename CoordType>
static CircularArc createFromPoints(const CoordType& p0, const CoordType& p2, const CoordinateXY& center, double radius, int orientation)
{
    static_assert(std::is_base_of_v<CoordinateXY, CoordType>);

    constexpr bool hasZ = CoordType::template has<Ordinate::Z>();
    constexpr bool hasM = CoordType::template has<Ordinate::M>();

    auto seq = std::make_unique<CoordinateSequence>(3, hasZ, hasM);

    CoordType p1(geos::algorithm::CircularArcs::getMidpoint(p0, p2, center, radius, orientation == algorithm::Orientation::COUNTERCLOCKWISE));
    if constexpr (hasZ) {
        p1.z = 0.5*(p0.z + p2.z);
    }
    if constexpr (hasM) {
        p1.m = 0.5*(p0.m + p2.m);
    }

    seq->setAt(p0, 0);
    seq->setAt(p1, 1);
    seq->setAt(p2, 2);

    CircularArc ret(std::move(seq), 0);

    return ret;
}

CircularArc
CircularArc::create(const CoordinateXY& p0, const CoordinateXY& p2, const CoordinateXY& center, double radius, int orientation) {
    return createFromPoints(p0, p2, center, radius, orientation);
}

CircularArc
CircularArc::create(const Coordinate& p0, const Coordinate& p2, const CoordinateXY& center, double radius, int orientation) {
    return createFromPoints(p0, p2, center, radius, orientation);
}

CircularArc
CircularArc::create(const CoordinateXYM& p0, const CoordinateXYM& p2, const CoordinateXY& center, double radius, int orientation) {
    return createFromPoints(p0, p2, center, radius, orientation);
}

CircularArc
CircularArc::create(const CoordinateXYZM& p0, const CoordinateXYZM& p2, const CoordinateXY& center, double radius, int orientation) {
    return createFromPoints(p0, p2, center, radius, orientation);
}

CircularArc::CircularArc(const CoordinateSequence& seq, std::size_t pos) :
    m_seq(&seq),
    m_pos(pos),
    m_own_coordinates(false) {}

CircularArc::CircularArc(std::unique_ptr<CoordinateSequence> seq, std::size_t pos) :
    CircularArc(*seq, pos)
{
    m_own_coordinates = true;
    seq.release();
}

CircularArc::CircularArc(const CoordinateSequence& seq, std::size_t pos, const CoordinateXY& center, double radius, int orientation) :
    m_seq(&seq),
    m_pos(pos),
    m_center(center),
    m_radius(radius),
    m_orientation(orientation),
    m_center_known(true),
    m_radius_known(true),
    m_orientation_known(true),
    m_own_coordinates(false)
{}

CircularArc::CircularArc(std::unique_ptr<CoordinateSequence> seq, std::size_t pos, const CoordinateXY& center, double radius, int orientation) :
    CircularArc(*seq, pos, center, radius, orientation)
{
    m_own_coordinates = true;
    seq.release();
}

CircularArc::CircularArc(const CircularArc& other) :
    m_seq(new CoordinateSequence(0, other.getCoordinateSequence()->hasZ(), other.getCoordinateSequence()->hasM())),
    m_pos(0),
    m_center(other.m_center),
    m_radius(other.m_radius),
    m_orientation(other.m_orientation),
    m_center_known(other.m_center_known),
    m_radius_known(other.m_radius_known),
    m_orientation_known(other.m_orientation_known),
    m_own_coordinates(true)
{
    CoordinateSequence* seq = const_cast<CoordinateSequence*>(m_seq);
    seq->reserve(3);
    seq->add(*other.getCoordinateSequence(), other.getCoordinatePosition(), other.getCoordinatePosition() + 2);
}

CircularArc::CircularArc(CircularArc&& other) noexcept {
    m_seq = other.m_seq;
    m_pos = other.m_pos;
    m_center = other.m_center;
    m_radius = other.m_radius;
    m_orientation = other.m_orientation;
    m_center_known = other.m_center_known;
    m_radius_known = other.m_radius_known;
    m_orientation_known = other.m_orientation_known;
    m_own_coordinates = other.m_own_coordinates;

    if (other.m_own_coordinates) {
        other.m_own_coordinates = false;
    }
}

CircularArc&
CircularArc::operator=(const CircularArc& other)
{
    if (m_own_coordinates) {
        delete m_seq;
    }

    m_seq = new CoordinateSequence(0, other.getCoordinateSequence()->hasZ(), other.getCoordinateSequence()->hasM());
    m_pos = other.m_pos;
    m_own_coordinates = true;
    m_orientation = other.m_orientation;
    m_orientation_known = other.m_orientation_known;
    m_center = other.m_center;
    m_center_known = other.m_center_known;
    m_radius = other.m_radius;
    m_radius_known = other.m_radius_known;

    CoordinateSequence* seq = const_cast<CoordinateSequence*>(m_seq);
    seq->reserve(3);
    seq->add(*other.getCoordinateSequence(), other.getCoordinatePosition(), other.getCoordinatePosition() + 2);

    return *this;
}

CircularArc&
CircularArc::operator=(CircularArc&& other) noexcept
{
    if (m_own_coordinates) {
        delete m_seq;
    }

    m_seq = other.m_seq;
    m_pos = other.m_pos;
    m_own_coordinates = other.m_own_coordinates;
    m_orientation = other.m_orientation;
    m_orientation_known = other.m_orientation_known;
    m_center = other.m_center;
    m_center_known = other.m_center_known;
    m_radius = other.m_radius;
    m_radius_known = other.m_radius_known;

    if (m_own_coordinates) {
        other.m_own_coordinates = false;
    }

    return *this;
}

CircularArc::~CircularArc()
{
    if (m_own_coordinates) {
        delete m_seq;
    }
}

bool
CircularArc::containsAngle(double theta) const {
    auto t0 = theta0();
    auto t2 = theta2();

    if (theta == t0 || theta == t2) {
        return true;
    }

    if (getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE) {
        std::swap(t0, t2);
    }

    t2 -= t0;
    theta -= t0;

    if (t2 < 0) {
        t2 += 2*MATH_PI;
    }
    if (theta < 0) {
        theta += 2*MATH_PI;
    }

    return theta >= t2;
}

bool
CircularArc::containsPoint(const CoordinateXY& q) const
{
    if (q == p0() || q == p1() || q == p2()) {
        return true;
    }

    //auto dist = std::abs(q.distance(getCenter()) - getRadius());

    //if (dist > 1e-8) {
    //    return false;
    //}

    if (triangulate::quadedge::TrianglePredicate::isInCircleRobust(p0(), p1(), p2(), q) != geom::Location::BOUNDARY) {
        return false;
    }

    return containsPointOnCircle(q);
}

double
CircularArc::getAngle() const
{
    if (isCircle()) {
        return 2*MATH_PI;
    }

    /// Even Rouault:
    /// potential optimization?: using crossproduct(p0 - center, p2 - center) = radius * radius * sin(angle)
    /// could yield the result by just doing a single asin(), instead of 2 atan2()
    /// actually one should also likely compute dotproduct(p0 - center, p2 - center) = radius * radius * cos(angle),
    /// and thus angle = atan2(crossproduct(p0 - center, p2 - center) , dotproduct(p0 - center, p2 - center) )
    auto t0 = theta0();
    auto t2 = theta2();

    if (getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE) {
        std::swap(t0, t2);
    }

    if (t0 < t2) {
        t0 += 2*MATH_PI;
    }

    auto diff = t0-t2;

    return diff;
}

double
CircularArc::getArea() const {
    if (isLinear()) {
        return 0;
    }

    auto R = getRadius();
    auto theta = getAngle();
    return R*R/2*(theta - std::sin(theta));
}

double
CircularArc::getLength() const {
    if (isLinear()) {
        return p0().distance(p2());
    }

    return getAngle()*getRadius();
}


void
CircularArc::addLinearizedPoints(CoordinateSequence& seq, double stepDegrees) const
{
    if (isLinear()) {
        seq.add(*getCoordinateSequence(), getCoordinatePosition() + 1, getCoordinatePosition() + 2);
        return;
    }

    const double angle = getAngle();
    const bool isCCW = getOrientation() == geos::algorithm::Orientation::COUNTERCLOCKWISE;
    const double stepRad = stepDegrees * MATH_PI / 180.0;
    const int nSegments = std::max(static_cast<int>(std::ceil(angle / stepRad)), 2);

    double adjStepRad = angle / nSegments;

    const CoordinateXY& center = getCenter();
    const double radius = getRadius();

    // To ensure that the vertices in the linearized arc are independent of the
    // arc orientation, we process the arc in a CCW manner regardless of its
    // original orientation.
    const double startAngle = isCCW ? theta0() : theta2();

    const bool hasZ = getCoordinateSequence()->hasZ();
    const bool hasM = getCoordinateSequence()->hasM();

    for (int i = 1; i < nSegments; i++) {
        const int j = isCCW ? i: nSegments - i;
        const double theta = startAngle + j*adjStepRad;

        CoordinateXYZM pt{geos::algorithm::CircularArcs::createPoint(center, radius, theta)};

        if (hasZ || hasM) {
            interpolateZM(pt, pt.z, pt.m);
        }

        seq.add(pt);
    }

    seq.add(*getCoordinateSequence(), getCoordinatePosition() + 2, getCoordinatePosition() + 2);
}

static double
interpolateValue(double a1, double a2, double frac)
{
    frac = std::clamp(frac, 0.0, 1.0);
    if (std::isnan(a1)) {
        return a2;
    }
    if (std::isnan(a2)) {
        return a1;
    }
    return a1 + frac * (a2 - a1);
}

void
CircularArc::interpolateZM(const CoordinateXY &pt, double &z, double &m) const
{
    using geom::Ordinate;

    const CoordinateSequence& seq = *getCoordinateSequence();
    std::size_t i0 = getCoordinatePosition();

    // Read Z, M from control point
    double z1, m1;
    seq.applyAt(i0 + 1, [&z1, &m1](const auto& arcPt) {
        z1 = arcPt.template get<Ordinate::Z>();
        m1 = arcPt.template get<Ordinate::M>();
    });
    // Test point = control point?
    // Take Z, M from the control point
    if (p1().equals2D(pt)) {
        z = z1;
        m = m1;
        return;
    }

    // Read Z, M from start point
    double z0, m0;
    seq.applyAt(i0, [&z0, &m0](const auto& arcPt) {
        z0 = arcPt.template get<Ordinate::Z>();
        m0 = arcPt.template get<Ordinate::M>();
    });
    // Test point = start point?
    // Take Z, M from the start point
    if (p0().equals2D(pt)) {
        z = z0;
        m = m0;
        return;
    }

    // Read Z, M from end point
    double z2, m2;
    seq.applyAt(i0 + 2, [&z2, &m2](const auto& arcPt) {
        z2 = arcPt.template get<Ordinate::Z>();
        m2 = arcPt.template get<Ordinate::M>();
    });
    // Test point = end point?
    // Take Z, M from the end point
    if (p2().equals2D(pt)) {
        z = z2;
        m = m2;
        return;
    }

    double norm_theta0 = theta0();
    const double norm_theta1 = theta1();
    double norm_theta2 = theta2();
    const double theta = algorithm::CircularArcs::getAngle(pt, getCenter());

    if (!isCCW()) {
        std::swap(norm_theta0, norm_theta2);
        std::swap(z0, z2);
        std::swap(m0, m2);
    }

    if (std::isnan(z1)) {
        // Interpolate between p0 /  p2
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta0, norm_theta2);
        z = interpolateValue(z0, z2, frac);
    } else if (algorithm::Angle::isWithinCCW(theta, norm_theta0, norm_theta1)) {
        // Interpolate between p0 / p1
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta0, norm_theta1);
        z = interpolateValue(z0, z1, frac);
    } else {
        // Interpolate between p1 / p2
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta1, norm_theta2);
        z = interpolateValue(z1, z2, frac);
    }

    if (std::isnan(m1)) {
        // Interpolate between p0 /  p2
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta0, norm_theta2);
        m = interpolateValue(m0, m2, frac);
    } else if (algorithm::Angle::isWithinCCW(theta, norm_theta0, norm_theta1)) {
        // Interpolate between p0 / p1
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta0, norm_theta1);
        m = interpolateValue(m0, m1, frac);
    } else {
        // Interpolate between p1 / p2
        const double frac = algorithm::Angle::fractionCCW(theta, norm_theta1, norm_theta2);
        m = interpolateValue(m1, m2, frac);
    }
}

bool
CircularArc::isUpwardAtPoint(const CoordinateXY& q) const {
    auto quad = geom::Quadrant::quadrant(getCenter(), q);
    bool isUpward;

    if (getOrientation() == algorithm::Orientation::CLOCKWISE) {
        isUpward = (quad == geom::Quadrant::SW || quad == geom::Quadrant::NW);
    } else {
        isUpward = (quad == geom::Quadrant::SE || quad == geom::Quadrant::NE);
    }

    return isUpward;
}

CircularArc
CircularArc::reverse() const
{
    auto seq = std::make_unique<CoordinateSequence>(3, m_seq->hasZ(), m_seq->hasM());
    m_seq->applyAt(m_pos, [&seq](const auto& pt) {
        seq->setAt(pt, 2);
    });
    m_seq->applyAt(m_pos + 1, [&seq](const auto& pt) {
        seq->setAt(pt, 1);
    });
    m_seq->applyAt(m_pos + 2, [&seq](const auto& pt) {
        seq->setAt(pt, 0);
    });

    CircularArc ret(std::move(seq), 0);

    if (m_orientation_known) {
        if (m_orientation == algorithm::Orientation::COUNTERCLOCKWISE) {
            ret.m_orientation = algorithm::Orientation::CLOCKWISE;
        } else if (m_orientation == algorithm::Orientation::CLOCKWISE) {
            ret.m_orientation = algorithm::Orientation::COUNTERCLOCKWISE;
        } else {
            ret.m_orientation = algorithm::Orientation::COLLINEAR;
        }
        ret.m_orientation_known = true;
    }

    if (m_center_known) {
        ret.m_center = m_center;
        ret.m_center_known = true;
    }

    if (m_radius_known) {
        ret.m_radius = m_radius;
        ret.m_radius_known = true;
    }

    return ret;
}

bool
CircularArc::equals(const CircularArc &other, double tol) const
{
    if (getCoordinateSequence()->hasZ() != other.getCoordinateSequence()->hasZ()) {
        return false;
    }

    if (getCoordinateSequence()->hasM() != other.getCoordinateSequence()->hasM()) {
        return false;
    }

    if (getCenter().distance(other.getCenter()) > tol) {
        return false;
    }

    if (std::abs(getRadius() - other.getRadius()) > tol) {
        return false;
    }

    if (getOrientation() != other.getOrientation()) {
        return false;
    }

    CoordinateXYZM a, b;
    getCoordinateSequence()->getAt(getCoordinatePosition(), a);
    other.getCoordinateSequence()->getAt(other.getCoordinatePosition(), b);

    if (a.distance(b) > tol) {
        return false;
    }

    if ((!std::isnan(a.z) || !std::isnan(b.z)) && !(std::abs(a.z - b.z) <= tol)) {
        return false;
    }

    if ((!std::isnan(a.m) || !std::isnan(b.m)) && !(std::abs(a.m - b.m) <= tol)) {
        return false;
    }

    return true;
}

std::string
CircularArc::toString() const {
    std::stringstream ss;
    ss << "CIRCULARSTRING ";
    if (m_seq->hasZ()) {
        ss << "Z";
    }
    if (m_seq->hasM()) {
        ss << "M";
    }
    if (m_seq->hasZ() || m_seq->hasM()) {
        ss << " ";
    }
    ss << "(";
    m_seq->applyAt(m_pos, [&ss](const auto& pt) {
        ss << pt << ", " << *(&pt + 1) << ", " << *(&pt + 2);
    });
    ss << ")";
    return ss.str();
}


}
