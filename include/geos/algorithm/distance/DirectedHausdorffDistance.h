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
 * (locationtech/jts DirectedHausdorffDistance)
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
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
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
 * Computes the directed Hausdorff distance from one geometry to another.
 * The directed Hausdorff distance is the maximum distance any point
 * on a query geometry A can be from a target geometry B.
 * Equivalently, every point in the query geometry is within that distance
 * of the target geometry.
 * The class can compute a pair of points at which the distance is attained:
 * [ farthest A point, nearest B point ].
 *
 * The directed Hausdorff distance (DHD) is defined as:
 *
 *     DHD(A,B) = max a ∈ A (max b ∈ B (distance(a, b) )
 *
 * DHD is asymmetric: DHD(A,B) may not be equal to DHD(B,A).
 * Hence it is not a distance metric.
 * The Hausdorff distance is a symmetric distance metric defined as:
 *
 *     HD(A,B) = max(DHD(A,B), DHD(B,A))
 *
 * This can be computed via the
 * hausdorffDistancePoints(Geometry, Geometry) function.
 *
 * Points, lines and polygons are supported as input.
 * If the query geometry is polygonal,
 * the point at maximum distance may occur in the interior of a query polygon.
 * For a polygonal target geometry the point always lies on the boundary.
 *
 * The directed Hausdorff distance can be used to test
 * whether a geometry lies fully within a given distance of another one.
 * The isFullyWithinDistance(Geometry, double) function
 * is provided to execute this test efficiently.
 * It implements heuristic checks and short-circuiting to improve performance.
 *
 * The class can be used in prepared mode.
 * Creating an instance on a target geometry caches indexes for that geometry.
 * Then farthestPoints(Geometry)
 * or isFullyWithinDistance(Geometry, double)
 * can be called efficiently for multiple query geometries.
 *
 * If the Hausdorff distance is attained at a non-vertex of the query geometry,
 * the location must be approximated.
 * The algorithm uses a distance tolerance to control the approximation accuracy.
 * The tolerance is automatically determined to balance between accuracy and performance.
 * If more accuracy is desired some function signatures are provided
 * which allow specifying a distance tolerance.
 *
 * This algorithm is easier to use, more accurate,
 * and much faster than DiscreteHausdorffDistance.
 *
 * \author Martin Davis
 */
class GEOS_DLL DirectedHausdorffDistance {
public:
    /// JTS returns Coordinate[]; std::array is the C++ equivalent pair.
    using PointPair = std::array<geom::CoordinateXY, 2>;

    /**
     * Computes the directed Hausdorff distance
     * of a query geometry A from a target one B.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @return the directed Hausdorff distance,
     * or NaN if an input is empty
     */
    static double distance(const geom::Geometry& a, const geom::Geometry& b);

    /**
     * Computes the directed Hausdorff distance
     * of a query geometry A from a target one B,
     * up to a given distance accuracy.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @param tolerance the accuracy distance tolerance
     * @return the directed Hausdorff distance,
     * or NaN if an input is empty
     */
    static double distance(const geom::Geometry& a, const geom::Geometry& b,
                           double tolerance);

    /**
     * Computes a line containing a pair of points which attain the directed Hausdorff distance
     * of a query geometry A from a target one B.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @return a pair of points [ptA, ptB] demonstrating the distance,
     * or empty if an input is empty
     */
    static std::optional<PointPair> distancePoints(
        const geom::Geometry& a, const geom::Geometry& b);

    /**
     * Computes a line containing a pair of points which attain the directed Hausdorff distance
     * of a query geometry A from a target one B, up to a given distance accuracy.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @param tolerance the accuracy distance tolerance
     * @return a pair of points [ptA, ptB] demonstrating the distance,
     * or empty if an input is empty
     */
    static std::optional<PointPair> distancePoints(
        const geom::Geometry& a, const geom::Geometry& b, double tolerance);

    /**
     * Computes the symmetric Hausdorff distance between two geometries.
     * This is the maximum of the two directed Hausdorff distances.
     *
     * @param a a geometry
     * @param b a geometry
     * @return the Hausdorff distance, or NaN if an input is empty
     */
    static double hausdorffDistance(const geom::Geometry& a, const geom::Geometry& b);

    /**
     * Computes a pair of points which attain the symmetric Hausdorff distance
     * between two geometries.
     * This is the maximum of the two directed Hausdorff distances.
     *
     * @param a a geometry
     * @param b a geometry
     * @return a pair of points [ptA, ptB] demonstrating the Hausdorff distance,
     * or empty if an input is empty
     */
    static std::optional<PointPair> hausdorffDistancePoints(
        const geom::Geometry& a, const geom::Geometry& b);

    /**
     * Computes whether a query geometry lies fully within a given distance of a target geometry.
     * Equivalently, detects whether any point of the query geometry is farther
     * from the target than the specified distance.
     * This is the case if DHD(A, B) > maxDistance.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @param maxDistance the distance limit
     * @return true if the query geometry lies fully within the distance of the target
     */
    static bool isFullyWithinDistance(
        const geom::Geometry& a, const geom::Geometry& b, double maxDistance);

    /**
     * Computes whether a query geometry lies fully within a given distance of a target geometry,
     * up to a given distance accuracy.
     * Equivalently, detects whether any point of the query geometry is farther
     * from the target than the specified distance.
     * This is the case if DHD(A, B) > maxDistance.
     *
     * @param a the query geometry
     * @param b the target geometry
     * @param maxDistance the distance limit
     * @param tolerance the accuracy distance tolerance
     * @return true if the query geometry lies fully within the distance of the target
     */
    static bool isFullyWithinDistance(
        const geom::Geometry& a, const geom::Geometry& b,
        double maxDistance, double tolerance);

    /**
     * Create a new instance for a target geometry.
     *
     * @param geom the geometry to compute the distance from
     */
    explicit DirectedHausdorffDistance(const geom::Geometry& geom);

    DirectedHausdorffDistance(const DirectedHausdorffDistance&) = delete;
    DirectedHausdorffDistance& operator=(const DirectedHausdorffDistance&) = delete;

    ~DirectedHausdorffDistance();

    /**
     * Computes a pair of points which attain the directed Hausdorff distance
     * of a query geometry A from the target B.
     * If either geometry is empty the result is empty.
     *
     * @param geom the query geometry
     * @return a pair of points [ptA, ptB] attaining the distance,
     * or empty if an input is empty
     */
    std::optional<PointPair> farthestPoints(const geom::Geometry& geom);

    /**
     * Computes a pair of points which attain the directed Hausdorff distance
     * of a query geometry A from the target B,
     * up to a given distance accuracy.
     * If either geometry is empty the result is empty.
     *
     * @param geom the query geometry
     * @param tolerance the approximation distance tolerance
     * @return a pair of points [ptA, ptB] attaining the distance,
     * or empty if an input is empty
     */
    std::optional<PointPair> farthestPoints(const geom::Geometry& geom, double tolerance);

    /**
     * Tests whether a query geometry lies fully within a given distance of the target geometry.
     * Equivalently, detects whether any point of the query geometry is farther
     * from the target than the specified distance.
     * This is the case if DHD(A, B) > maxDistance.
     *
     * @param geom the query geometry
     * @param maxDistance the distance limit
     * @return true if the query geometry lies fully within the distance of the target
     */
    bool isFullyWithinDistance(const geom::Geometry& geom, double maxDistance);

    /**
     * Tests whether a query geometry lies fully within a given distance of the target geometry,
     * up to a given distance accuracy.
     * Equivalently, detects whether any point of the query geometry is farther
     * from the target than the specified distance.
     * This is the case if DHD(A, B) > maxDistance.
     *
     * @param geom the query geometry
     * @param maxDistance the distance limit
     * @param tolerance the accuracy distance tolerance
     * @return true if the query geometry lies fully within the distance of the target
     */
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
