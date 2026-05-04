/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024-2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Angle.h>
#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/Quadrant.h>
#include <geos/math/DD.h>

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::LineSegment;

namespace geos {
namespace algorithm {

CoordinateXY
CircularArcs::getMidpoint(const CoordinateXY& p0, const CoordinateXY& p2, const CoordinateXY& center, double radius, bool isCCW)
{
    double start = getAngle(p0, center);
    double stop = getAngle(p2, center);
    double mid = getMidpointAngle(start, stop, isCCW);
    return createPoint(center, radius, mid);
}

double
CircularArcs::getAngle(const CoordinateXY& p, const CoordinateXY& center)
{
    return std::atan2(p.y - center.y, p.x - center.x);
}

CoordinateXY
CircularArcs::getDirectionPoint(const geom::CoordinateXY& center, double radius, double theta, bool isCCW)
{
    const double dt = geos::MATH_PI / 4 * (isCCW ? 1: -1);

    // To get a point that is exactly on the tangent to this arc at p0, we would create a point
    // at radius * sqrt(2). During overlay, this can produce a situation where noded edges originating
    // from the same point have direction points that are in the same direction. Nudging the direction point
    // slightly in the direction of the arc resolves this issue.
    return createPoint(center, radius * std::sqrt(2) * (1 - 1e-6), theta + dt);
}

double
CircularArcs::getMidpointAngle(double theta0, double theta2, bool isCCW)
{
    if (!isCCW) {
        return getMidpointAngle(theta2, theta0, true);
    }

    double mid = (theta0 + theta2) / 2;
    if (!Angle::isWithinCCW(mid, theta0, theta2)) {
        mid += MATH_PI;
    }

    return mid;
}

CoordinateXY
CircularArcs::createPoint(const CoordinateXY& center, double radius, double theta)
{
    return { center.x + radius* std::cos(theta), center.y + radius* std::sin(theta) };
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
CircularArcs::interpolateZM(const CoordinateSequence& seq, std::size_t i0,
                            const CoordinateXY &center, bool isCCW,
                            CoordinateXY &pt, double &z, double &m)
{
    using geom::Ordinate;

    const CoordinateXY& p0 = seq.getAt<CoordinateXY>(i0);
    const CoordinateXY& p1 = seq.getAt<CoordinateXY>(i0 + 1);
    const CoordinateXY& p2 = seq.getAt<CoordinateXY>(i0 + 2);

    // Read Z, M from control point
    double z1, m1;
    seq.applyAt(i0 + 1, [&z1, &m1](const auto& arcPt) {
        z1 = arcPt.template get<Ordinate::Z>();
        m1 = arcPt.template get<Ordinate::M>();
    });
    // Test point = control point?
    // Take Z, M from the control point
    if (p1.equals2D(pt)) {
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
    if (p0.equals2D(pt)) {
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
    if (p2.equals2D(pt)) {
        z = z2;
        m = m2;
        return;
    }

    double theta0 = getAngle(p0, center);
    const double theta1 = getAngle(p1, center);
    double theta2 = getAngle(p2, center);
    const double theta = getAngle(pt, center);

    if (!isCCW) {
        std::swap(theta0, theta2);
        std::swap(z0, z2);
        std::swap(m0, m2);
    }

    if (std::isnan(z1)) {
        // Interpolate between p0 /  p2
        const double frac = Angle::fractionCCW(theta, theta0, theta2);
        z = interpolateValue(z0, z2, frac);
    } else if (Angle::isWithinCCW(theta, theta0, theta1)) {
        // Interpolate between p0 / p1
        const double frac = Angle::fractionCCW(theta, theta0, theta1);
        z = interpolateValue(z0, z1, frac);
    } else {
        // Interpolate between p1 / p2
        const double frac = Angle::fractionCCW(theta, theta1, theta2);
        z = interpolateValue(z1, z2, frac);
    }

    if (std::isnan(m1)) {
        // Interpolate between p0 /  p2
        const double frac = Angle::fractionCCW(theta, theta0, theta2);
        m = interpolateValue(m0, m2, frac);
    } else if (Angle::isWithinCCW(theta, theta0, theta1)) {
        // Interpolate between p0 / p1
        const double frac = Angle::fractionCCW(theta, theta0, theta1);
        m = interpolateValue(m0, m1, frac);
    } else {
        // Interpolate between p1 / p2
        const double frac = Angle::fractionCCW(theta, theta1, theta2);
        m = interpolateValue(m1, m2, frac);
    }
}

template<typename T>
CoordinateXY getCenterImpl(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    // Circumcenter formulas from Graphics Gems III
    T p0x{p0.x};
    T p0y{p0.y};
    T p1x{p1.x};
    T p1y{p1.y};
    T p2x{p2.x};
    T p2y{p2.y};

    T ax = p1x - p2x;
    T ay = p1y - p2y;
    T bx = p2x - p0x;
    T by = p2y - p0y;
    T cx = p0x - p1x;
    T cy = p0y - p1y;

    T d1 = -(bx*cx + by*cy);
    T d2 = -(cx*ax + cy*ay);
    T d3 = -(ax*bx + ay*by);

    T e1 = d2*d3;
    T e2 = d3*d1;
    T e3 = d1*d2;
    T e = e1 + e2 + e3;

    T G3x = p0.x + p1.x + p2.x;
    T G3y = p0.y + p1.y + p2.y;
    T Hx = (e1*p0.x + e2*p1.x + e3*p2.x) / e;
    T Hy = (e1*p0.y + e2*p1.y + e3*p2.y) / e;

    T rx = 0.5*(G3x - Hx);
    T ry = 0.5*(G3y - Hy);

    if constexpr (std::is_same_v<T, math::DD>) {
        return {rx.doubleValue(), ry.doubleValue()};
    } else {
        return {rx, ry};
    }
}

CoordinateXY
CircularArcs::getCenter(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2)
{
    if (p0.equals2D(p2)) {
        // Closed circle
        return { 0.5*(p0.x + p1.x), 0.5*(p0.y + p1.y) };
    }

    return getCenterImpl<double>(p0, p1, p2);
}

void
CircularArcs::expandEnvelope(geom::Envelope& e, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
                             const geom::CoordinateXY& p2)
{
    using geom::Quadrant;

    e.expandToInclude(p0);
    e.expandToInclude(p1);
    e.expandToInclude(p2);

    CoordinateXY center = getCenter(p0, p1, p2);

    // zero-length arc
    if (center.equals2D(p0) || center.equals2D(p1)) {
        return;
    }

    // collinear
    if (!std::isfinite(center.x) || !std::isfinite(center.y)) {
        return;
    }

    //* 1 | 0
    //* --+--
    //* 2 | 3
    const auto pa0 = Quadrant::pseudoAngle(center, p0);
    const auto pa1 = Quadrant::pseudoAngle(center, p1);
    const auto pa2 = Quadrant::pseudoAngle(center, p2);

    auto q0 = static_cast<int>(pa0);
    auto q2 = static_cast<int>(pa2);
    double R = center.distance(p1);

    if (q0 == q2) {
        // Start and end quadrants are the same. Either the arc crosses all
        // the axes, or none of the axes.

        const bool isBetween = pa1 > std::min(pa0, pa2) && pa1 < std::max(pa0, pa2);

        if (!isBetween) {
            e.expandToInclude({center.x, center.y + R});
            e.expandToInclude({center.x - R, center.y});
            e.expandToInclude({center.x, center.y - R});
            e.expandToInclude({center.x + R, center.y});
        }

        return;
    }

    auto orientation = Orientation::index(p0, p1, p2);

    if (orientation == Orientation::CLOCKWISE) {
        std::swap(q0, q2);
    }

    for (auto q = q0 + 1; (q % 4) != ((q2+1) % 4); q++) {
        switch (q % 4) {
        case Quadrant::NW:
            e.expandToInclude({center.x, center.y + R});
            break;
        case Quadrant::SW:
            e.expandToInclude({center.x - R, center.y});
            break;
        case Quadrant::SE:
            e.expandToInclude({center.x, center.y - R});
            break;
        case Quadrant::NE:
            e.expandToInclude({center.x + R, center.y});
            break;
        }
    }
}

int
CircularArcs::circleIntersectsSegment(const CoordinateXY& center, double r,
                                      const CoordinateXY& p0, const CoordinateXY& p1,
                                      CoordinateXY& ret0, CoordinateXY& ret1)
{
    const double& x0 = center.x;
    const double& y0 = center.y;

    Envelope segEnv(p0, p1);

    CoordinateXY isect0, isect1;
    int n = 0;

    if (p1.x == p0.x) {
        // vertical line
        double x = p1.x;

        double A = 1;
        double B = -2*y0;
        double C = x*x - 2*x*x0 + x0*x0 + y0*y0 - r*r;

        double d = std::sqrt(B*B - 4*A*C);
        double Y1 = (-B + d)/(2*A);
        double Y2 = (-B - d)/(2*A);

        isect0 = {x, Y1};
        isect1 = {x, Y2};
    }
    else {
        double m = (p1.y - p0.y) / (p1.x - p0.x);
        double b = p1.y - p1.x*m;

        // Ax^2 + Bx + C = 0
        double A = 1 + m*m;
        double B = -2*x0 + 2*m*b - 2*m*y0;
        double C = x0*x0 + b*b - 2*b*y0 + y0*y0 - r*r;

        double d = std::sqrt(B*B - 4*A*C);
        double X1 = (-B + d)/(2*A);
        double X2 = (-B - d)/(2*A);

        // TODO use robust quadratic solver such as https://github.com/archermarx/quadratic ?
        // auto [X1, X2] = quadratic::solve(A, B, C);

        isect0 = {X1, m* X1 + b};
        isect1 = {X2, m* X2 + b};
    }

    if (segEnv.intersects(isect0)) {
        ret0 = isect0;
        if (segEnv.intersects(isect1) && !isect1.equals2D(isect0)) {
            ret1 = isect1;
            n = 2;
        } else {
            n = 1;
        }
    } else if (segEnv.intersects(isect1)) {
        ret0 = isect1;
        n = 1;
    }

    return n;
}

bool
CircularArcs::containsAngle(double t0, double t2, bool isCCW, double theta)
{
    if (theta == t0 || theta == t2) {
        return true;
    }

    if (isCCW) {
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
CircularArcs::containsPointOnCircle(const CoordinateXY& center,
                                    const CoordinateXY& p0, const CoordinateXY& p2, bool isCCW,
                                    const CoordinateXY& q)
{
    if (p0.equals2D(p2)) {
        // closed circle contains all points
        return true;
    }

    const double t0 = getAngle(p0, center);
    const double t2 = getAngle(p2, center);
    const double theta = getAngle(q, center);

    return containsAngle(t0, t2, isCCW, theta);
}

double
CircularArcs::distanceArcPoint(const CoordinateXY& c, double r,
                               const CoordinateXY& p0, const CoordinateXY& p2, bool isCCW,
                               const CoordinateXY &q)
{
    const CoordinateXY p = closestPointArcPoint(c, r, p0, p2, isCCW, q);
    return p.distance(q);
}

CoordinateXY
CircularArcs::closestPointArcPoint(const CoordinateXY& c, double r,
                                   const CoordinateXY& p0, const CoordinateXY& p2, bool isCCW,
                                   const CoordinateXY &q)
{
    if (!std::isfinite(r)) {
        return LineSegment::closestPoint(p0, p2, q);
    }

    const double distanceToCenter = q.distance(c);
    if (distanceToCenter == 0) {
        return p0; // arbitrary
    }

    CoordinateXY i;
    i.x = c.x + (q.x - c.x) * (r / distanceToCenter);
    i.y = c.y + (q.y - c.y) * (r / distanceToCenter);

    if (i.equals2D(p0) || i.equals2D(p2) || containsPointOnCircle(c, p0, p2, isCCW, i)) {
        return i;
    }

    if (q.distance(p0) < q.distance(p2)) {
        return p0;
    }

    return p2;
}

double
CircularArcs::distanceArcSegment(const CoordinateXY& c, double r,
                                 const CoordinateXY& p0, const CoordinateXY& p2, bool isCCW,
                                 const CoordinateXY& q0, const CoordinateXY& q1)
{
    const auto pts = closestPointsArcSegment(c, r, p0, p2, isCCW, q0, q1);
    return pts[0].distance(pts[1]);
}

std::array<CoordinateXY, 2>
CircularArcs::closestPointsArcSegment(const CoordinateXY& c, double r,
                                      const CoordinateXY& p0, const CoordinateXY& p2, bool isCCW,
                                      const CoordinateXY& q0, const CoordinateXY& q1)
{
    if (!std::isfinite(r)) {
        return geom::LineSegment::closestPoints(p0, p2, q0, q1);
    }

    // Calculate distance between line and circle center
    const CoordinateXY d = geom::LineSegment::closestPoint(q0, q1, c);
    const double dist_cd = d.distance(c);

    if (dist_cd <= r) {
        // Segment is at least partially within circle; check to see if it intersects
        CoordinateXY isect0, isect1;
        auto n = circleIntersectsSegment(c, r, q0, q1, isect0, isect1);
        if (n > 0 && containsPointOnCircle(c, p0, p2, isCCW, isect0)) {
            return {isect0, isect0};
        }
        if (n > 1 && containsPointOnCircle(c, p0, p2, isCCW, isect1)) {
            return {isect1, isect1};
        }

        // Closest point is one of the segment endpoints.
        const auto e0 = closestPointArcPoint(c, r, p0, p2, isCCW, q0);
        const auto e1 = closestPointArcPoint(c, r, p0, p2, isCCW, q1);
        if (e0.distance(q0) < e1.distance(q1)) {
            return {e0, q0};
        }

        return {e1, q1};
    }

    // Find closest point on circle
    const CoordinateXY g{c.x + (d.x - c.x) * (r / dist_cd),
                   c.y + (d.y - c.y) * (r / dist_cd)};

    const bool ptInArc = containsPointOnCircle(c, p0, p2, isCCW, g);

    if (ptInArc) {
        return {g, d};
    }

    // Closest distance is between the segment and one of the arc endpoints
    const auto e0 = LineSegment::closestPoint(q0, q1, p0);
    const auto e1 = LineSegment::closestPoint(q0, q1, p2);

    if (e0.distance(p0) < e1.distance(p2)) {
        return {e0, p0};
    }

    return {e1, p2};
}

double
CircularArcs::distanceArcArc(const CoordinateXY& ca, double ra,
                             const CoordinateXY& a0, const CoordinateXY& a2, bool aCCW,
                             const CoordinateXY& cb, double rb,
                             const CoordinateXY& b0, const CoordinateXY& b2, bool bCCW)
{
    const auto pts = closestPointsArcArc(ca, ra, a0, a2, aCCW, cb, rb, b0, b2, bCCW);
    return pts[0].distance(pts[1]);
}

std::array<CoordinateXY, 2>
CircularArcs::closestPointsArcArc(const CoordinateXY& ca, double ra,
                                  const CoordinateXY& a0, const CoordinateXY& a2, bool aCCW,
                                  const CoordinateXY& cb, double rb,
                                  const CoordinateXY& b0, const CoordinateXY& b2, bool bCCW)
{
    if (!std::isfinite(ra)) {
        if (!std::isfinite(rb)) {
            return LineSegment::closestPoints(a0, a2, b0, b2);
        } else {
            auto pts = closestPointsArcSegment(cb, rb, b0, b2, bCCW, a0, a2);
            std::swap(pts[0], pts[1]);
            return pts;
        }
    } else if (!std::isfinite(rb)) {
        return closestPointsArcSegment(ca, ra, a0, a2, aCCW, b0, b2);
    }

    const double d = ca.distance(cb);

    const bool disjoint = d > (ra + rb);
    const bool contained = d < std::abs(ra - rb);
    const bool concentric = (d == 0);
    const bool cocircular = concentric && (ra == rb);
    const bool overlapping = !(disjoint || contained || concentric);

    std::vector<CoordinateXY> ptsA{a0, a2};
    std::vector<CoordinateXY> ptsB{b0, b2};

    if (overlapping || cocircular) {
        auto isect = arcIntersectionPoint(ca, ra, a0, a2, aCCW, cb, rb, b0, b2, bCCW);
        if (isect.has_value()) {
            return { isect.value(), isect.value() };
        }
    }

    if (!concentric) {
        // Add points on A and B that intersect line from center-a to center-b
        {
            double theta = getAngle(cb, ca);
            double at0 = getAngle(a0, ca);
            double at2 = getAngle(a2, ca);

            if (containsAngle(at0, at2, aCCW, theta)) {
                ptsA.push_back(createPoint(ca, ra, theta));
            }
            if (containsAngle(at0, at2, aCCW, theta + MATH_PI)) {
                ptsA.push_back(createPoint(ca, ra, theta + MATH_PI));
            }
        }

        {
            double theta = getAngle(ca, cb);
            double bt0 = getAngle(b0, cb);
            double bt2 = getAngle(b2, cb);

            if (containsAngle(bt0, bt2, bCCW, theta)) {
                ptsB.push_back(createPoint(cb, rb, theta));
            }
            if (containsAngle(bt0, bt2, bCCW, theta + MATH_PI)) {
                ptsB.push_back(createPoint(cb, rb, theta + MATH_PI));
            }

        }
    }

    // Add points on A that intersect line from CA to B0
    if (containsPointOnCircle(ca, a0, a2, aCCW, b0)) {
        ptsA.push_back(CircularArcs::createPoint(ca, ra, getAngle(b0, ca)));
    }

    // Add points on A that intersect line from CA to B2
    if (containsPointOnCircle(ca, a0, a2, aCCW, b2)) {
        ptsA.push_back(CircularArcs::createPoint(ca, ra, getAngle(b2, ca)));
    }

    // Add points on B that intersect line from CB to A0
    if (containsPointOnCircle(cb, b0, b2, bCCW, a0)) {
        ptsB.push_back(CircularArcs::createPoint(cb, rb, getAngle(a0, cb)));
    }

    // Add points on B that intersect line from CB to A2
    if (containsPointOnCircle(cb, b0, b2, bCCW, a2)) {
        ptsB.push_back(CircularArcs::createPoint(cb, rb, getAngle(a2, cb)));
    }

    double minDist = std::numeric_limits<double>::max();

    std::array<CoordinateXY, 2> ret;

    for (const auto& pa : ptsA) {
        for (const auto& pb : ptsB) {
            const double dist = pa.distance(pb);
            if (dist < minDist) {
                minDist = dist;
                ret[0] = pa;
                ret[1] = pb;
            }
        }
    }

    return ret;
}

std::optional<CoordinateXY>
CircularArcs::cocircularArcsIntersectionPoint(const CoordinateXY& center, double radius,
                                              const CoordinateXY& a0, const CoordinateXY& a2, bool aCCW,
                                              const CoordinateXY& b0, const CoordinateXY& b2, bool bCCW)
{
    // The body of this function is adapted and simplified from CircularArcIntersector.

    // Make both inputs counter-clockwise for the purpose of determining intersections
    const double ap0 = Angle::normalizePositive(getAngle(aCCW ? a0 : a2, center));
    const double ap1 = Angle::normalizePositive(getAngle(aCCW ? a2 : a0, center));
    const double bp0 = Angle::normalizePositive(getAngle(bCCW ? b0 : b2, center));
    const double bp1 = Angle::normalizePositive(getAngle(bCCW ? b2 : b0, center));

    // Check if B starts within A
    if (Angle::isWithinCCW(bp0, ap0, ap1)) {
        return createPoint(center, radius, bp0);
    }

    // Check if B ends within A
    if (Angle::isWithinCCW(bp1, ap0, ap1)) {
        return createPoint(center, radius, bp1);
    }

    // Check if A starts within B
    if (Angle::isWithinCCW(ap0, bp0 , bp1))
    {
        return createPoint(center, radius, ap0);
    }

    return std::nullopt;
}

std::optional<CoordinateXY>
CircularArcs::arcIntersectionPoint(const CoordinateXY& ca, double ra,
                                   const CoordinateXY& a0, const CoordinateXY& a2, bool aCCW,
                                   const CoordinateXY& cb, double rb,
                                   const CoordinateXY& b0, const CoordinateXY& b2, bool bCCW)
{
    // The body of this function is adapted and simplified from CircularArcIntersector.

    const auto d = ca.distance(cb);

    if (d > ra + rb) {
        // Circles are disjoint
        return std::nullopt;
    }

    if (d < std::abs(ra - rb)) {
        // One circle contained within the other; arcs cannot intersect
        return std::nullopt;
    }

    // a: the distance from c1 to the "radical line", which connects the two intersection points
    // Expression rewritten by Herbie, https://herbie.uwplse.org/demo/
    // const double a = (d*d + r1*r1 - r2*r2) / (2*d);
    const double a = std::fma(ra-rb, (ra + rb) / (d+d), d*0.5);

    if (a == 0 || (d == 0 && ra == rb)) {
        return cocircularArcsIntersectionPoint(ca, ra, a0, a2, aCCW, b0, b2, bCCW);
    }

    // Explicitly check endpoint intersections
    if (a0.equals2D(b0) || a0.equals2D(b2)) {
        return a0;
    }
    if (a2.equals2D(b0) || a2.equals2D(b2)) {
        return a2;
    }

    // Compute interior intersection points.
    const double dx = cb.x - ca.x;
    const double dy = cb.y - ca.y;

    // point where a line between the two circle center points intersects
    // the radical line
    CoordinateXY p{ca.x + a * dx/d, ca.y+a* dy/d};

    // distance from p to the intersection points
    const double h = std::sqrt(ra*ra - a*a);

    CoordinateXY isect0{p.x + h* dy/d, p.y - h* dx/d };
    CoordinateXY isect1{p.x - h* dy/d, p.y + h* dx/d };

    if (containsPointOnCircle(ca, a0, a2, aCCW, isect0) &&
        containsPointOnCircle(cb, b0, b2, bCCW, isect0)) {
        return isect0;
    }

    if (containsPointOnCircle(ca, a0, a2, aCCW, isect1) &&
        containsPointOnCircle(cb, b0, b2, bCCW, isect1)) {
        return isect1;
    }

    return std::nullopt;
}

}
}
