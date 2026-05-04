/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <array>
#include <optional>

namespace geos {
namespace geom {
class CoordinateSequence;
class Envelope;
}
}

namespace geos {
namespace algorithm {

class GEOS_DLL CircularArcs {
public:

    /// Return the circle center of an arc defined by three points
    static geom::CoordinateXY getCenter(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
                                        const geom::CoordinateXY& p2);

    static double getAngle(const geom::CoordinateXY& pt, const geom::CoordinateXY& center);

    static geom::CoordinateXY getDirectionPoint(const geom::CoordinateXY& center, double radius, double theta, bool isCCW);

    static double getMidpointAngle(double theta0, double theta2, bool isCCW);

    static geom::CoordinateXY getMidpoint(const geom::CoordinateXY& p0, const geom::CoordinateXY& p2, const geom::CoordinateXY& center, double radius, bool isCCW);

    /// Expand an envelope to include an arc defined by three points
    static void expandEnvelope(geom::Envelope& e, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
                               const geom::CoordinateXY& p2);

    /// Return the point defined by a circle center, radius, and angle
    static geom::CoordinateXY createPoint(const geom::CoordinateXY& center, double radius, double theta);

    /** Interpolate Z/M values for a point added to an arc.
     *
     * @param seq CoordinateSequence containing the arc points
     * @param i0 Index of the first point in the arc
     * @param center The center point of the circle defining the arc
     * @param isCCW Whether the arc is counterclockwise
     * @param pt The point to interpolate Z/M values for
     * @param z The interpolated Z value
     * @param m The interpolated M value
     */
    static void interpolateZM(const geom::CoordinateSequence &seq, size_t i0, const geom::CoordinateXY &center, bool isCCW, geom::CoordinateXY &pt, double
                              &z, double &m);

    /** Determines whether and where a circle intersects a line segment.
     *
     * @param center The center point of the circle
     * @param r The radius of the circle
     * @param p0 The first point of the line segment
     * @param p1 The second point of the line segment
     * @param isect0 Set to the first intersection point, if it exists
     * @param isect1 Set to the second intersection point, if it exists
     * @return The number of intersection points
     */
    static int
    circleIntersectsSegment(const geom::CoordinateXY& center, double r,
                            const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
                            geom::CoordinateXY& isect0, geom::CoordinateXY& isect1);

    /** Returns whether an angle is included between two other angles.
     *
     * @param t0 starting angle (radians)
     * @param t2 ending engle (radians)
     * @param isCCW whether the interval from t0 to t2 is counterclockwise
     * @param theta angle to test (radians)
     * @return true if theta is included between t0 and t2
     */
    static bool
    containsAngle(double t0, double t2, bool isCCW, double theta);

    /** Returns whether a point on a circle is included between two arc endpoints.
     *  It is assumed that the point lies on the circle; this is not tested.
     *
     * @param center The center coordinate of the circle
     * @param p0 The first arc endpoint
     * @param p2 The second arc endpoint
     * @param isCCW Whether the arc between p0 and p2 is counterclockwise
     * @param q The point to check for inclusion
     * @return true if q is on the arc, false otherwise
     */
    static bool
    containsPointOnCircle(const geom::CoordinateXY& center,
                          const geom::CoordinateXY& p0, const geom::CoordinateXY& p2, bool isCCW,
                          const geom::CoordinateXY& q);

    /** Calculates the distance between an arc and a point.
     *
     * @param c The center coordinate of the circle
     * @param r The radius of the circle
     * @param p0 The first arc endpoint
     * @param p2 The second arc endpoint
     * @param isCCW Whether the arc between p0 and p2 is counterclockwise
     * @param q The point to calculate distance to
     * @return the distance between the arc and the point
     */
    static double
    distanceArcPoint(const geom::CoordinateXY& c, double r,
                     const geom::CoordinateXY& p0, const geom::CoordinateXY& p2, bool isCCW,
                     const geom::CoordinateXY& q);

    /** Calculates the distance between an arc and a line segment.
     * @param c The center coordinate of the circle
     * @param r The radius of the circle
     * @param p0 The first arc endpoint
     * @param p2 The second arc endpoint
     * @param isCCW Whether the arc between p0 and p2 is counterclockwise
     * @param q0 The first point of the line segment
     * @param q1 The second point of the line segment
     * @return the distance between the arc and the line segment
     */
    static double
    distanceArcSegment(const geom::CoordinateXY& c, double r,
                       const geom::CoordinateXY& p0, const geom::CoordinateXY& p2, bool isCCW,
                       const geom::CoordinateXY& q0, const geom::CoordinateXY& q1);

    /** Calculates the distance between two arcs.
     *
     * @param ca the center coordinate of the first arc's circle
     * @param ra the radius of the first arc's circle
     * @param a0 the first endpoint of the first arc
     * @param a2 the second endpoint of the first arc
     * @param aCCW whether the first arc is counterclockwise
     * @param cb the center coordinate of the second arc's circle
     * @param rb the radius of the second arc's circle
     * @param b0 the first endpoint of the second arc
     * @param b2 the second endpoint of the second arc
     * @param bCCW whether the second arc is counterclockwise
     * @return the distance between the two arcs
     */
    static double
    distanceArcArc(const geom::CoordinateXY& ca, double ra,
                   const geom::CoordinateXY& a0, const geom::CoordinateXY& a2, bool aCCW,
                   const geom::CoordinateXY& cb, double rb,
                   const geom::CoordinateXY& b0, const geom::CoordinateXY& b2, bool bCCW);

    /** Return the closest point on an arc to a specified point.
     *
     * @param c the center of the arc
     * @param r the radius of the arc
     * @param p0 the first endpoint of the arc
     * @param p2 the second endpoint of the arc
     * @param isCCW whether the arc is counterclockwise
     * @param q the point to find the closest point to
     * @return the closest point on the arc to the specified point
     */
    static geom::CoordinateXY
    closestPointArcPoint(const geom::CoordinateXY& c, double r,
                         const geom::CoordinateXY& p0, const geom::CoordinateXY& p2,
                         bool isCCW, const geom::CoordinateXY &q);

    /** Return the pair of points having the shortest distance between an arc and a line segment
     *
     * @param c the center of the arc
     * @param r the radius of the arc
     * @param p0 the first endpoint of the arc
     * @param p2 the second endpoint of the arc
     * @param isCCW whether the arc is counterclockwise
     * @param q0 the first endpoint of the line segment
     * @param q1 the second endpoint of the line segment
     * @return array whose first element is a point on the arc and second element is a point on the line segment
     */
    static std::array<geom::CoordinateXY, 2>
    closestPointsArcSegment(const geom::CoordinateXY& c, double r,
                            const geom::CoordinateXY& p0, const geom::CoordinateXY& p2, bool isCCW, const geom::CoordinateXY& q0, const geom::CoordinateXY& q1);


    /** Return the pair of points having the shortest distance between two arcs
     *
     * @param ca the center of the first arc
     * @param ra the radius of the first arc
     * @param a0 the first endpoint of the first arc
     * @param a2 the second endpoint of the first arc
     * @param aCCW whether the first arc is counterclockwise
     * @param cb the center of the second arc
     * @param rb the radius of the second arc
     * @param b0 the first endpoint of the second arc
     * @param b2 the second endpoint of the second arc
     * @param bCCW whether the second arc is counterclockwise
     * @return array whose first element is a point on the first arc and second element is a point on the second arc
     */
    static std::array<geom::CoordinateXY, 2> closestPointsArcArc(const geom::CoordinateXY& ca,
        double ra,
        const geom::CoordinateXY& a0,
        const geom::CoordinateXY& a2,
        bool aCCW,
        const geom::CoordinateXY& cb,
        double rb,
        const geom::CoordinateXY& b0,
        const geom::CoordinateXY& b2,
        bool bCCW);

    /** Returns a single XY point of intersection between two cocircular arcs, if one exists.
     *
     * To identify all intersection point(s) and/or arc(s), with handling of Z and M ordinates, use CircularArcIntersector.
     *
     * @param center the center coordinate of the arcs
     * @param radius the radius of the arcs
     * @param a0 the first endpoint of the first arc
     * @param a2 the second endpoint of the first arc
     * @param aCCW whether the first arc is counterclockwise
     * @param b0 the first endpoint of the second arc
     * @param b2 the second endpoint of the second arc
     * @param bCCW whether the second arc is counterclockwise
     * @return a point of intersection between the two arcs, if one exists
     */
    static std::optional<geom::CoordinateXY>
    cocircularArcsIntersectionPoint(const geom::CoordinateXY& center, double radius,
                                    const geom::CoordinateXY& a0, const geom::CoordinateXY& a2, bool aCCW,
                                    const geom::CoordinateXY& b0, const geom::CoordinateXY& b2, bool bCCW);

    /** Returns a single point of intersection between two arcs, if one exists.
     *
     * To identify all intersection point(s) and/or arc(s), with handling of Z and M ordinates, use CircularArcIntersector.
     *
     * @param ca the center of the first arc
     * @param ra the radius of the first arc
     * @param a0 the first endpoint of the first arc
     * @param a2 the second endpoint of the first arc
     * @param aCCW whether the first arc is counterclockwise
     * @param cb the center of the second arc
     * @param rb the radius of the second arc
     * @param b0 the first endpoint of the second arc
     * @param b2 the second endpoint of the second arc
     * @param bCCW whether the second arc is counterclockwise
     * @return a point of intersection between the two arcs, if one exists
     */
    static std::optional<geom::CoordinateXY>
    arcIntersectionPoint(const geom::CoordinateXY& ca, double ra,
                         const geom::CoordinateXY& a0, const geom::CoordinateXY& a2, bool aCCW,
                         const geom::CoordinateXY& cb, double rb,
                         const geom::CoordinateXY& b0, const geom::CoordinateXY& b2, bool bCCW);

};
}
}
