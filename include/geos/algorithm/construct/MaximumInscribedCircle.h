/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2020 Martin Davis
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/MaximumInscribedCircle.java
 * https://github.com/locationtech/jts/commit/f0b9a808bdf8a973de435f737e37b7a221e231cb
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/Envelope.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/operation/distance/IndexedFacetDistance.h>

#include <memory>
#include <queue>



namespace geos {
namespace geom {
class Coordinate;
class Envelope;
class Geometry;
class GeometryFactory;
class LineString;
class Point;
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace construct { // geos::algorithm::construct

/**
 * Constructs the Maximum Inscribed Circle for a
 * polygonal Geometry, up to a specified tolerance.
 * The Maximum Inscribed Circle is determined by a point in the interior of the area
 * which has the farthest distance from the area boundary,
 * along with a boundary point at that distance.
 *
 * In the context of geography the center of the Maximum Inscribed Circle
 * is known as the **Pole of Inaccessibility**.
 * A cartographic use case is to determine a suitable point
 * to place a map label within a polygon.
 *
 * The radius length of the Maximum Inscribed Circle is a
 * measure of how "narrow" a polygon is. It is the
 * distance at which the negative buffer becomes empty.
 *
 * The class supports testing whether a polygon is "narrower"
 * than a specified distance via
 * isRadiusWithin(Geometry, double) or
 * isRadiusWithin(double).
 * Testing for the maximum radius is generally much faster
 * than computing the actual radius value, since short-circuiting
 * is used to limit the approximation iterations.
 *
 * The class supports polygons with holes and multipolygons.
 *
 * The implementation uses a successive-approximation technique
 * over a grid of square cells covering the area geometry.
 * The grid is refined using a branch-and-bound algorithm.
 * Point containment and distance are computed in a performant
 * way by using spatial indexes.
 *
 * Future Enhancements
 *
 *   * Support a polygonal constraint on placement of center point,
 *     for example to produce circle-packing constructions,
 *     or support multiple labels.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL MaximumInscribedCircle {

    using IndexedPointInAreaLocator = geos::algorithm::locate::IndexedPointInAreaLocator;
    using IndexedFacetDistance = geos::operation::distance::IndexedFacetDistance;

public:

    MaximumInscribedCircle(const geom::Geometry* polygonal, double tolerance);
    ~MaximumInscribedCircle() = default;

    /**
    * Gets the center point of the maximum inscribed circle
    * (up to the tolerance distance).
    *
    * @return the center point of the maximum inscribed circle
    */
    std::unique_ptr<geom::Point> getCenter();

    /**
    * Gets a point defining the radius of the Maximum Inscribed Circle.
    * This is a point on the boundary which is
    * nearest to the computed center of the Maximum Inscribed Circle.
    * The line segment from the center to this point
    * is a radius of the constructed circle, and this point
    * lies on the boundary of the circle.
    *
    * @return a point defining the radius of the Maximum Inscribed Circle
    */
    std::unique_ptr<geom::Point> getRadiusPoint();

    /**
    * Gets a line representing a radius of the Largest Empty Circle.
    *
    * @return a 2-point line from the center of the circle to a point on the edge
    */
    std::unique_ptr<geom::LineString> getRadiusLine();

    /**
    * Tests if the radius of the maximum inscribed circle
    * is no longer than the specified distance.
    * This method determines the distance tolerance automatically
    * as a fraction of the maxRadius value.
    * After this method is called the center and radius
    * points provide locations demonstrating where
    * the radius exceeds the specified maximum.
    *
    * @param maxRadius the (non-negative) radius value to test
    * @return true if the max in-circle radius is no longer than the max radius
    */
    bool isRadiusWithin(double maxRadius);

    /**
    * Computes the center point of the Maximum Inscribed Circle
    * of a polygonal geometry, up to a given tolerance distance.
    *
    * @param polygonal a polygonal geometry
    * @param tolerance the distance tolerance for computing the center point
    * @return the center point of the maximum inscribed circle
    */
    static std::unique_ptr<geom::Point> getCenter(const geom::Geometry* polygonal, double tolerance);

    /**
    * Computes a radius line of the Maximum Inscribed Circle
    * of a polygonal geometry, up to a given tolerance distance.
    *
    * @param polygonal a polygonal geometry
    * @param tolerance the distance tolerance for computing the center point
    * @return a line from the center to a point on the circle
    */
    static std::unique_ptr<geom::LineString> getRadiusLine(const geom::Geometry* polygonal, double tolerance);

    /**
    * Tests if the radius of the maximum inscribed circle
    * is no longer than the specified distance.
    * This method determines the distance tolerance automatically
    * as a fraction of the maxRadius value.
    *
    * @param polygonal a polygonal geometry
    * @param maxRadius the radius value to test
    * @return true if the max in-circle radius is no longer than the max radius
    */
    static bool isRadiusWithin(const geom::Geometry* polygonal, double maxRadius);

    /**
     * Computes the maximum number of iterations allowed.
     * Uses a heuristic based on the area of the input geometry
     * and the tolerance distance.
     * The number of tolerance-sized cells that cover the input geometry area
     * is computed, times a safety factor.
     * This prevents massive numbers of iterations and created cells
     * for casees where the input geometry has extremely small area
     * (e.g. is very thin).
     *
     * @param geom the input geometry
     * @param toleranceDist the tolerance distance
     * @return the maximum number of iterations allowed
     */
    static std::size_t computeMaximumIterations(const geom::Geometry* geom, double toleranceDist);

private:

    /* private members */
    const geom::Geometry* inputGeom;
    std::unique_ptr<geom::Geometry> inputGeomBoundary;
    double tolerance;
    IndexedFacetDistance indexedDistance;
    IndexedPointInAreaLocator ptLocator;
    const geom::GeometryFactory* factory;
    bool done;
    geom::CoordinateXY centerPt;
    geom::CoordinateXY radiusPt;
    double maximumRadius = -1;

    /* private constant */
    static constexpr double MAX_RADIUS_FRACTION = 0.0001;

    /* private methods */
    double distanceToBoundary(const geom::Point& pt);
    double distanceToBoundary(double x, double y);
    void compute();
    void computeApproximation();
    void createResult(const geom::CoordinateXY& c, const geom::CoordinateXY& r);

    /* private class */
    class Cell {
    private:
        static constexpr double SQRT2 = 1.4142135623730951;
        double x;
        double y;
        double hSize;
        double distance;
        double maxDist;

    public:
        Cell(double p_x, double p_y, double p_hSize, double p_distanceToBoundary)
            : x(p_x)
            , y(p_y)
            , hSize(p_hSize)
            , distance(p_distanceToBoundary)
            , maxDist(p_distanceToBoundary+(p_hSize*SQRT2))
        {};

        geom::Envelope getEnvelope() const
        {
            geom::Envelope env(x-hSize, x+hSize, y-hSize, y+hSize);
            return env;
        }

        double getMaxDistance() const
        {
            return maxDist;
        }
        double getDistance() const
        {
            return distance;
        }
        double getHSize() const
        {
            return hSize;
        }
        double getX() const
        {
            return x;
        }
        double getY() const
        {
            return y;
        }

        bool operator< (const Cell& rhs) const
        {
            return maxDist <  rhs.maxDist;
        }

        bool operator> (const Cell& rhs) const
        {
            return maxDist >  rhs.maxDist;
        }

        bool operator==(const Cell& rhs) const
        {
            return maxDist == rhs.maxDist;
        }

        /**
         * The Cell priority queue is sorted by the natural order of maxDistance.
         * std::priority_queue sorts with largest first,
         * which is what is needed for this algorithm.
         */
        using CellQueue = std::priority_queue<Cell>;
    };

    void createInitialGrid(const geom::Envelope* env, Cell::CellQueue& cellQueue);
    Cell createInteriorPointCell(const geom::Geometry* geom);

};


} // geos::algorithm::construct
} // geos::algorithm
} // geos
