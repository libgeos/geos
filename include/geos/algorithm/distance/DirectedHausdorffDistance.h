/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Martin Davis
 * Copyright (C) 2026 Jeroen Bloemscheer
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/distance/DirectedHausdorffDistance.java
 * (locationtech/jts#1182)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <array>
#include <memory>
#include <optional>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace geos {
namespace geom {
class Envelope;
class Geometry;
}
}

namespace geos {
namespace algorithm {
namespace distance {

/**
 * Computes the directed Hausdorff distance from a query geometry A
 * to a target geometry B.
 *
 * The directed Hausdorff distance is the maximum distance any point
 * on A can be from B:
 *
 *     h(A,B) = max_{a in A} min_{b in B} distance(a, b)
 *
 * It is asymmetric. The symmetric Hausdorff distance is
 * max(h(A,B), h(B,A)).
 *
 * Empty operands yield NaN (and a missing realizing pair).
 * A negative tolerance throws IllegalArgumentException.
 * Zero tolerance is allowed (zero-size input).
 *
 * This is the locus (continuous) algorithm from JTS 1182.
 * Do not confuse it with DiscreteHausdorffDistance, which only
 * samples vertices (optionally densified). GEOSHausdorffDistance
 * remains the discrete C API.
 *
 * The class-comment formula in JTS that writes max_a (max_b ...)
 * is farthest-pair; the algorithm implemented here is max-min.
 * farthestPoints keeps the JTS name but returns the max-min realizing pair.
 *
 * Mixed-dimension collections follow JTS: only getDimension() == P
 * routes to computeForPoints. A GeometryCollection of points and
 * lines drops the points (JTS TODO: handle mixed geoms with points).
 *
 * \author Martin Davis
 * \author Jeroen Bloemscheer (GEOS port)
 */
class GEOS_DLL DirectedHausdorffDistance {
public:
    using PointPair = std::array<geom::CoordinateXY, 2>;

    static double distance(const geom::Geometry& a, const geom::Geometry& b);
    static double distance(const geom::Geometry& a, const geom::Geometry& b,
                           double tolerance);

    static std::optional<PointPair> distancePoints(
        const geom::Geometry& a, const geom::Geometry& b);
    static std::optional<PointPair> distancePoints(
        const geom::Geometry& a, const geom::Geometry& b, double tolerance);

    static double hausdorffDistance(const geom::Geometry& a, const geom::Geometry& b);
    static std::optional<PointPair> hausdorffDistancePoints(
        const geom::Geometry& a, const geom::Geometry& b);

    static bool isFullyWithinDistance(
        const geom::Geometry& a, const geom::Geometry& b, double maxDistance);
    static bool isFullyWithinDistance(
        const geom::Geometry& a, const geom::Geometry& b,
        double maxDistance, double tolerance);

    /** Prepared instance: indexes the target once. */
    explicit DirectedHausdorffDistance(const geom::Geometry& geom);

    DirectedHausdorffDistance(const DirectedHausdorffDistance&) = delete;
    DirectedHausdorffDistance& operator=(const DirectedHausdorffDistance&) = delete;

    ~DirectedHausdorffDistance();

    std::optional<PointPair> farthestPoints(const geom::Geometry& geom);
    std::optional<PointPair> farthestPoints(const geom::Geometry& geom, double tolerance);

    bool isFullyWithinDistance(const geom::Geometry& geom, double maxDistance);
    bool isFullyWithinDistance(
        const geom::Geometry& geom, double maxDistance, double tolerance);

private:
    class TargetDistance;
    friend class DHDSegment;

    static double pairDistance(const std::optional<PointPair>& pts);
    static PointPair pair(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1);
    static double computeTolerance(const geom::Geometry& geom);
    static bool isBeyond(
        const geom::Envelope& envA, const geom::Envelope& envB, double maxDistance);
    static bool isValidLimit(double limit);
    static bool isBeyondLimit(double maxDist, double maxDistanceLimit);
    static bool isWithinLimit(double maxDist, double maxDistanceLimit);

    std::optional<PointPair> computeDistancePoints(
        const geom::Geometry& geom, double tolerance, double maxDistanceLimit);
    std::optional<PointPair> computeForPoints(
        const geom::Geometry& geom, double maxDistanceLimit);
    std::optional<PointPair> computeForEdges(
        const geom::Geometry& geom, double tolerance, double maxDistanceLimit);
    std::optional<PointPair> computeForAreaInterior(
        const geom::Geometry& geom, double tolerance);

    const geom::Geometry& target;
    std::unique_ptr<TargetDistance> targetDistance;
};

} // namespace distance
} // namespace algorithm
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
