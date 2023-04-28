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

#include <geos/algorithm/construct/LargestEmptyCircle.h>
#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/operation/distance/IndexedFacetDistance.h>
#include <geos/util/Interrupt.h>

#include <typeinfo> // for dynamic_cast
#include <cassert>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace construct { // geos.algorithm.construct

LargestEmptyCircle::LargestEmptyCircle(const Geometry* p_obstacles, double p_tolerance)
    : LargestEmptyCircle(p_obstacles, nullptr, p_tolerance)
{
}

LargestEmptyCircle::LargestEmptyCircle(const Geometry* p_obstacles, const Geometry* p_boundary, double p_tolerance)
    : tolerance(p_tolerance)
    , obstacles(p_obstacles)
    , factory(p_obstacles->getFactory())
    , obstacleDistance(p_obstacles)
    , done(false)
{
    if (obstacles->isEmpty()) {
        throw util::IllegalArgumentException("Empty obstacles geometry is not supported");
    }
    if (! p_boundary || p_boundary->isEmpty()) {
        boundary = obstacles->convexHull();
    }
    else {
        boundary = p_boundary->clone();
    }
}

/* public static */
std::unique_ptr<Point>
LargestEmptyCircle::getCenter(const Geometry* p_obstacles, double p_tolerance)
{
    LargestEmptyCircle lec(p_obstacles, p_tolerance);
    return lec.getCenter();
}

/* public static */
std::unique_ptr<LineString>
LargestEmptyCircle::getRadiusLine(const Geometry* p_obstacles, double p_tolerance)
{
    LargestEmptyCircle lec(p_obstacles, p_tolerance);
    return lec.getRadiusLine();
}

/* public */
std::unique_ptr<Point>
LargestEmptyCircle::getCenter()
{
    compute();
    return std::unique_ptr<Point>(factory->createPoint(centerPt));
}

/* public */
std::unique_ptr<Point>
LargestEmptyCircle::getRadiusPoint()
{
    compute();
    return std::unique_ptr<Point>(factory->createPoint(radiusPt));
}

/* public */
std::unique_ptr<LineString>
LargestEmptyCircle::getRadiusLine()
{
    compute();

    auto cl = detail::make_unique<CoordinateSequence>(2u);
    cl->setAt(centerPt, 0);
    cl->setAt(radiusPt, 1);
    return factory->createLineString(std::move(cl));
}


/* private */
void
LargestEmptyCircle::createInitialGrid(const Envelope* env, std::priority_queue<Cell>& cellQueue)
{
    if (!env->isFinite()) {
        throw util::GEOSException("Non-finite envelope encountered.");
    }

    double cellSize = std::max(env->getWidth(), env->getHeight());
    double hSide = cellSize / 2.0;

    // Collapsed geometries just end up using the centroid
    // as the answer and skip all the other machinery
    if (cellSize == 0) return;

    CoordinateXY c;
    env->centre(c);
    cellQueue.emplace(c.x, c.y, hSide, distanceToConstraints(c.x, c.y));
}

/* private */
bool
LargestEmptyCircle::mayContainCircleCenter(const Cell& cell, const Cell& farthestCell)
{
    /**
     * Every point in the cell lies outside the boundary,
     * so they cannot be the center point
     */
    if (cell.isFullyOutside())
        return false;

    /**
     * The cell is outside, but overlaps the boundary
     * so it may contain a point which should be checked.
     * This is only the case if the potential overlap distance
     * is larger than the tolerance.
     */
    if (cell.isOutside()) {
        bool isOverlapSignificant = cell.getMaxDistance() > tolerance;
        return isOverlapSignificant;
    }

    /**
     * Cell is inside the boundary. It may contain the center
     * if the maximum possible distance is greater than the current distance
     * (up to tolerance).
     */
    double potentialIncrease = cell.getMaxDistance() - farthestCell.getDistance();
    return potentialIncrease > tolerance;
}


/* private */
double
LargestEmptyCircle::distanceToConstraints(const Coordinate& c)
{
    bool isOutside = ptLocator && (Location::EXTERIOR == ptLocator->locate(&c));
    std::unique_ptr<Point> pt(factory->createPoint(c));
    if (isOutside) {
        double boundaryDist = boundaryDistance->distance(pt.get());
        return -boundaryDist;

    }
    double dist = obstacleDistance.distance(pt.get());
    return dist;
}

/* private */
double
LargestEmptyCircle::distanceToConstraints(double x, double y)
{
    Coordinate coord(x, y);
    return distanceToConstraints(coord);
}

/* private */
LargestEmptyCircle::Cell
LargestEmptyCircle::createCentroidCell(const Geometry* geom)
{
    Coordinate c;
    geom->getCentroid(c);
    Cell cell(c.x, c.y, 0, distanceToConstraints(c));
    return cell;
}

/* private */
void
LargestEmptyCircle::initBoundary()
{
    gridEnv = *(boundary->getEnvelopeInternal());
    // if boundary does not enclose an area cannot create a ptLocator
    if (boundary->getDimension() >= 2) {
        ptLocator.reset(new algorithm::locate::IndexedPointInAreaLocator(*(boundary.get())));
        boundaryDistance.reset(new operation::distance::IndexedFacetDistance(boundary.get()));
    }
}

/* private */
void
LargestEmptyCircle::compute()
{
    // check if already computed
    if (done) return;

    initBoundary();
    // if ptLocator is not present then result is degenerate (represented as zero-radius circle)
    if (!ptLocator) {
        const CoordinateXY* pt = obstacles->getCoordinate();
        centerPt = *pt;
        radiusPt = *pt;
        done = true;
        return;
    }

    // Priority queue of cells, ordered by decreasing distance from constraints
    std::priority_queue<Cell> cellQueue;
    createInitialGrid(&gridEnv, cellQueue);

    Cell farthestCell = createCentroidCell(obstacles);

    /**
     * Carry out the branch-and-bound search
     * of the cell space
     */
    std::size_t maxIter = MaximumInscribedCircle::computeMaximumIterations(boundary.get(), tolerance);
    std::size_t iterationCount = 0;
    while (!cellQueue.empty() && iterationCount < maxIter) {

        // pick the most promising cell from the queue
        Cell cell = cellQueue.top();
        cellQueue.pop();

        if ((iterationCount++ % 1000) == 0) {
            GEOS_CHECK_FOR_INTERRUPTS();
        }

        // update the center cell if the candidate is further from the constraints
        if (cell.getDistance() > farthestCell.getDistance()) {
            farthestCell = cell;
        }

        /**
        * If this cell may contain a better approximation to the center
        * of the empty circle, then refine it (partition into subcells
        * which are added into the queue for further processing).
        * Otherwise the cell is pruned (not investigated further),
        * since no point in it can be further than the current farthest distance.
        */
        if (mayContainCircleCenter(cell, farthestCell)) {
            // split the cell into four sub-cells
            double h2 = cell.getHSize() / 2;
            cellQueue.emplace(cell.getX()-h2, cell.getY()-h2, h2, distanceToConstraints(cell.getX()-h2, cell.getY()-h2));
            cellQueue.emplace(cell.getX()+h2, cell.getY()-h2, h2, distanceToConstraints(cell.getX()+h2, cell.getY()-h2));
            cellQueue.emplace(cell.getX()-h2, cell.getY()+h2, h2, distanceToConstraints(cell.getX()-h2, cell.getY()+h2));
            cellQueue.emplace(cell.getX()+h2, cell.getY()+h2, h2, distanceToConstraints(cell.getX()+h2, cell.getY()+h2));
        }
    }

    // the farthest cell is the best approximation to the MIC center
    Cell centerCell = farthestCell;
    centerPt.x = centerCell.getX();
    centerPt.y = centerCell.getY();

    // compute radius point
    std::unique_ptr<Point> centerPoint(factory->createPoint(centerPt));
    const auto& nearestPts = obstacleDistance.nearestPoints(centerPoint.get());
    radiusPt = nearestPts->getAt(0);

    // flag computation
    done = true;
}



} // namespace geos.algorithm.construct
} // namespace geos.algorithm
} // namespace geos
