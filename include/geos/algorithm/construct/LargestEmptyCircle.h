/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/LargestEmptyCircle.java
 * https://github.com/locationtech/jts/commit/98274a7ea9b40651e9de6323dc10fb2cac17a245
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/Envelope.h>
#include <geos/algorithm/construct/IndexedDistanceToPoint.h>
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
* Constructs the Largest Empty Circle for a set of obstacle geometries,
* up to a specified tolerance. 
* The obstacles may be any combination of point, linear and polygonal geometries.
*
* The Largest Empty Circle (LEC) is the largest circle 
* whose interior does not intersect with any obstacle
* and whose center lies within a polygonal boundary.
* The circle center is the point in the interior of the boundary 
* which has the farthest distance from the obstacles 
* (up to the accuracy of the distance tolerance).
* The circle itself is determined by the center point
* and a point lying on an obstacle determining the circle radius.
* 
* The polygonal boundary may be supplied explicitly.
* If it is not specified the convex hull of the obstacles is used as the boundary.
* 
* To compute an LEC which lies wholly within
* a polygonal boundary, include the boundary of the polygon(s) as an obstacle.
*
* The implementation uses a successive-approximation technique
* over a grid of square cells covering the obstacles and boundary.
* The grid is refined using a branch-and-bound algorithm. Point
* containment and distance are computed in a performant way
* by using spatial indexes.
*
* \author Martin Davis
*/
class GEOS_DLL LargestEmptyCircle {
    using IndexedFacetDistance = geos::operation::distance::IndexedFacetDistance;

public:

    /**
    * Creates a new instance of a Largest Empty Circle construction.
    * The obstacles may be any collection of points, lines and polygons.
    * The constructed circle center lies within the convex hull of the obstacles.
    *
    * @param p_obstacles a geometry representing the obstacles
    * @param p_tolerance the distance tolerance for computing the circle center point
    */
    LargestEmptyCircle(const geom::Geometry* p_obstacles, double p_tolerance);

    /**
    * Creates a new instance of a Largest Empty Circle construction,
    * interior-disjoint to a set of obstacle geometries 
    * and having its center within a polygonal boundary.
    * The obstacles may be any collection of points, lines and polygons.
    * If the boundary is null or empty the convex hull
    * of the obstacles is used as the boundary.
    *
    * @param p_obstacles a geometry representing the obstacles
    * @param p_boundary a polygonal geometry to contain the LEC center
    * @param p_tolerance the distance tolerance for computing the circle center point
    */
    LargestEmptyCircle(const geom::Geometry* p_obstacles, const geom::Geometry* p_boundary, double p_tolerance);

    ~LargestEmptyCircle() = default;

    /**
    * Computes the center point of the Largest Empty Circle
    * within a set of obstacles, up to a given tolerance distance.
    * The obstacles may be any collection of points, lines and polygons.
    *
    * @param p_obstacles a geometry representing the obstacles
    * @param p_tolerance the distance tolerance for computing the center point
    * @return the center point of the Largest Empty Circle
    */
    static std::unique_ptr<geom::Point> getCenter(const geom::Geometry* p_obstacles, double p_tolerance);

    /**
    * Computes a radius line of the Largest Empty Circle
    * within a set of obstacles, up to a given distance tolerance.
    * The obstacles may be any collection of points, lines and polygons.
    *
    * @param p_obstacles a geometry representing the obstacles
    * @param p_tolerance the distance tolerance for computing the center point
    * @return a line from the center of the circle to a point on the edge
    */
    static std::unique_ptr<geom::LineString> getRadiusLine(const geom::Geometry* p_obstacles, double p_tolerance);

    std::unique_ptr<geom::Point> getCenter();
    std::unique_ptr<geom::Point> getRadiusPoint();
    std::unique_ptr<geom::LineString> getRadiusLine();


private:

    /* private members */
    double tolerance;
    const geom::Geometry* obstacles;
    std::unique_ptr<geom::Geometry> boundary;
    const geom::GeometryFactory* factory;
    geom::Envelope gridEnv;
    bool done;
    std::unique_ptr<algorithm::locate::IndexedPointInAreaLocator> boundaryPtLocater;
    IndexedDistanceToPoint obstacleDistance;
    std::unique_ptr<IndexedFacetDistance> boundaryDistance;
    geom::CoordinateXY centerPt;
    geom::CoordinateXY radiusPt;

    /**
    * Computes the signed distance from a point to the constraints
    * (obstacles and boundary).
    * Points outside the boundary polygon are assigned a negative distance.
    * Their containing cells will be last in the priority queue
    * (but will still end up being tested since they may be refined).
    *
    * @param c the point to compute the distance for
    * @return the signed distance to the constraints (negative indicates outside the boundary)
    */
    double distanceToConstraints(const geom::Coordinate& c);
    double distanceToConstraints(double x, double y);
    void initBoundary();
    void compute();

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
        Cell(double p_x, double p_y, double p_hSize, double p_distanceToConstraints)
            : x(p_x)
            , y(p_y)
            , hSize(p_hSize)
            , distance(p_distanceToConstraints)
            , maxDist(p_distanceToConstraints + (p_hSize*SQRT2))
        {};

        geom::Envelope getEnvelope() const
        {
            geom::Envelope env(x-hSize, x+hSize, y-hSize, y+hSize);
            return env;
        }

        bool isFullyOutside() const
        {
            return maxDist < 0.0;
        }
        bool isOutside() const
        {
            return distance < 0.0;
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
            return maxDist < rhs.maxDist;
        }
        bool operator> (const Cell& rhs) const
        {
            return maxDist > rhs.maxDist;
        }
        bool operator==(const Cell& rhs) const
        {
            return maxDist == rhs.maxDist;
        }
    };

    bool mayContainCircleCenter(const Cell& cell, const Cell& farthestCell);
    void createInitialGrid(const geom::Envelope* env, std::priority_queue<Cell>& cellQueue);
    Cell createCentroidCell(const geom::Geometry* geom);

};


} // geos::algorithm::construct
} // geos::algorithm
} // geos
