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

#pragma once

#include <array>
#include <cstdint>

#include <geos/export.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CircularArc.h>

namespace geos::algorithm {

class GEOS_DLL CircularArcIntersector {
public:
    using CoordinateXY = geom::CoordinateXY;
    using CoordinateXYZM = geom::CoordinateXYZM;
    using CircularArc = geom::CircularArc;
    using Envelope = geom::Envelope;

    enum intersection_type : uint8_t {
        NO_INTERSECTION = 0,
        ONE_POINT_INTERSECTION = 1,
        TWO_POINT_INTERSECTION = 2,
        COCIRCULAR_INTERSECTION = 3,
    };

    intersection_type getResult() const
    {
        return result;
    }

    const CoordinateXYZM& getPoint(std::uint8_t i) const
    {
        return intPt[i];
    }

    const CircularArc& getArc(std::uint8_t i) const
    {
        return intArc[i];
    }

    std::uint8_t getNumPoints() const
    {
        return nPt;
    }

    std::uint8_t getNumArcs() const
    {
        return nArc;
    }

    /** Determines whether and where a circular arc intersects a line segment.
     *
     * Sets the appropriate value of intersection_type and stores the intersection points, if any.
     *
     * @param arc The circular arc
     * @param seq A CoordinateSequence containing the points of the line segment
     * @param pos0 The index of the first point in the line segment
     * @param pos1 The index of the second point in the line segment
     * @param useSegEndpoints Whether to preferentially take Z/M values from
     *        the endpoints of the line segment rather than the arc.
     */
    void intersects(const CircularArc& arc, const geom::CoordinateSequence& seq, std::size_t pos0, std::size_t pos1, bool useSegEndpoints);

    /** Determines whether and where two circular arcs intersect.
     *
     * 	Sets the appropriate value of intersection_type and stores the intersection
     *  points and/or arcs, if any.
     */
    void intersects(const CircularArc& arc1, const CircularArc& arc2);

    /** Determines whether and where two line segments intersect
     *
     * 	Sets the appropriate value of intersection_type and stores the intersection
     *  points, if any.
     */
    void intersects(const geom::CoordinateSequence& p, std::size_t p0, std::size_t p1,
                    const geom::CoordinateSequence& q, std::size_t q0, std::size_t q1);

private:
    void reset() {
        nPt = 0;
        nArc = 0;
    }

    /** Add an arc intersection of two cocircular arcs between the specified angles.
     *
     * The input arcs are provided so that Z/M values can be assigned to the created arc.
     * When the endpoints of the new arc correspond with those of the inputs, Z/M values
     * will be preferentially taken from arc1.
     */
    void addArcIntersection(double startAngle, double endAngle, int orientation, const CircularArc& arc1, const CircularArc& arc2);

    /** Add a point intersection between two arcs.
     *
     * The input arcs are provided so that Z/M values can be assigned to the intersection point.
     * When the intersection point corresponds matches one of the input arc endpoints,
     * Z/M values will be taken from that endpoint, with arc1 having priority over arc2.
     * If the intersection point does not equal the endpoint of either arc, its Z/M values
     * will be interpolated.
     */
    void addIntersection(const CoordinateXY& computedIntPt, const CircularArc& arc1, const CircularArc& arc2);

    /** Add a point intersection between an arc and a segment.
     *
     * The input arc and segment are provided so that Z/M values can be assigned to the intersection point.
     * When the intersection point corresponds to the arc or segment endpoints, Z/M values
     * will be taken from that endpoint. Priority will be given to the arc endpoints unless
     * `useSegEndpoint` is true. If the intersection point does not equal the endpoint of the arc
     * or the segment, its Z/M values will be interpolated.
     */
    void addIntersection(const CoordinateXY& computedIntPt, const CircularArc& lhs, const geom::CoordinateSequence& seq, std::size_t pos0, std::size_t pos1, bool useSegEndpoints);

    /** Determines whether and where two circles intersect a line segment.
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
    circleIntersects(const CoordinateXY& center, double r, const CoordinateXY& p0, const CoordinateXY& p1, CoordinateXY& isect0, CoordinateXY& isect1);

    std::array<CoordinateXYZM, 2> intPt;
    std::array<CircularArc, 2> intArc;
    intersection_type result = NO_INTERSECTION;
    std::uint8_t nPt = 0;
    std::uint8_t nArc = 0;
};

}
