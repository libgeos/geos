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
 * Last port: algorithm/construct/MaximumInscribedCircle.java
 * https://github.com/locationtech/jts/commit/98274a7ea9b40651e9de6323dc10fb2cac17a245
 *
 **********************************************************************/

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


/* public */
MaximumInscribedCircle::MaximumInscribedCircle(const Geometry* polygonal, double p_tolerance)
    : inputGeom(polygonal)
    , inputGeomBoundary(polygonal->getBoundary())
    , tolerance(p_tolerance)
    , indexedDistance(inputGeomBoundary.get())
    , ptLocator(*polygonal)
    , factory(polygonal->getFactory())
    , done(false)
{
    if (!(typeid(*polygonal) == typeid(Polygon) ||
          typeid(*polygonal) == typeid(MultiPolygon))) {
        throw util::IllegalArgumentException("Input geometry must be a Polygon or MultiPolygon");
    }

    if (polygonal->isEmpty()) {
        throw util::IllegalArgumentException("Empty input geometry is not supported");
    }
}


/* public static */
std::unique_ptr<Point>
MaximumInscribedCircle::getCenter(const Geometry* polygonal, double tolerance)
{
    MaximumInscribedCircle mic(polygonal, tolerance);
    return mic.getCenter();
}

/* public static */
std::unique_ptr<LineString>
MaximumInscribedCircle::getRadiusLine(const Geometry* polygonal, double tolerance)
{
    MaximumInscribedCircle mic(polygonal, tolerance);
    return mic.getRadiusLine();
}

/* public static */
std::size_t
MaximumInscribedCircle::computeMaximumIterations(const Geometry* geom, double toleranceDist)
{
    double diam = geom->getEnvelopeInternal()->getDiameter();
    double ncells = diam / toleranceDist;
    //-- Using log of ncells allows control over number of iterations
    std::size_t factor = (std::size_t) std::log(ncells);
    if (factor < 1) factor = 1;
    return 2000 + 2000 * factor;
}

/* public */
std::unique_ptr<Point>
MaximumInscribedCircle::getCenter()
{
    compute();
    return factory->createPoint(centerPt);
}

/* public */
std::unique_ptr<Point>
MaximumInscribedCircle::getRadiusPoint()
{
    compute();
    return factory->createPoint(radiusPt);
}

/* public */
std::unique_ptr<LineString>
MaximumInscribedCircle::getRadiusLine()
{
    compute();

    auto cl = detail::make_unique<CoordinateSequence>(2u);
    cl->setAt(centerPt, 0);
    cl->setAt(radiusPt, 1);
    return factory->createLineString(std::move(cl));
}

int INITIAL_GRID_SIDE = 25;

/* private */
void
MaximumInscribedCircle::createInitialGrid(const Envelope* env, Cell::CellQueue& cellQueue)
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
    cellQueue.emplace(c.x, c.y, hSide, distanceToBoundary(c.x, c.y));
}

/* private */
double
MaximumInscribedCircle::distanceToBoundary(const Coordinate& c)
{
    std::unique_ptr<Point> pt(factory->createPoint(c));
    double dist = indexedDistance.distance(pt.get());
    // double dist = inputGeomBoundary->distance(pt.get());
    bool isOutside = (Location::EXTERIOR == ptLocator.locate(&c));
    if (isOutside) return -dist;
    return dist;
}

/* private */
double
MaximumInscribedCircle::distanceToBoundary(double x, double y)
{
    Coordinate coord(x, y);
    return distanceToBoundary(coord);
}

/* private */
MaximumInscribedCircle::Cell
MaximumInscribedCircle::createInteriorPointCell(const Geometry* geom)
{
    Coordinate c;
    std::unique_ptr<Point> p = geom->getInteriorPoint();
    Cell cell(p->getX(), p->getY(), 0, distanceToBoundary(c));
    return cell;
}

/* private */
void
MaximumInscribedCircle::compute()
{

    // check if already computed
    if (done) return;

    // Priority queue of cells, ordered by maximum distance from boundary
    Cell::CellQueue cellQueue;

    createInitialGrid(inputGeom->getEnvelopeInternal(), cellQueue);

    // use the area centroid as the initial candidate center point
    Cell farthestCell = createInteriorPointCell(inputGeom);

    /**
     * Carry out the branch-and-bound search
     * of the cell space
     */
    std::size_t maxIter = computeMaximumIterations(inputGeom, tolerance);
    std::size_t iterationCount = 0;
    while (!cellQueue.empty() && iterationCount < maxIter) {
        // pick the most promising cell from the queue
        Cell cell = cellQueue.top();
        cellQueue.pop();
//    std::cout << iterationCount << "] Dist: " << cell.getDistance() << "  size: " << cell.getHSize() << std::endl;

        if ((iterationCount++ % 1000) == 0) {
            GEOS_CHECK_FOR_INTERRUPTS();
        }

        //-- if cell must be closer than furthest, terminate since all remaining cells in queue are even closer.
        if (cell.getMaxDistance() < farthestCell.getDistance())
            break;

        // update the center cell if the candidate is further from the boundary
        if (cell.getDistance() > farthestCell.getDistance()) {
            farthestCell = cell;
        }
        /**
        * Refine this cell if the potential distance improvement
        * is greater than the required tolerance.
        * Otherwise the cell is pruned (not investigated further),
        * since no point in it is further than
        * the current farthest distance.
        */
        double potentialIncrease = cell.getMaxDistance() - farthestCell.getDistance();
        if (potentialIncrease > tolerance) {
            // split the cell into four sub-cells
            double h2 = cell.getHSize() / 2;
            cellQueue.emplace(cell.getX()-h2, cell.getY()-h2, h2, distanceToBoundary(cell.getX()-h2, cell.getY()-h2));
            cellQueue.emplace(cell.getX()+h2, cell.getY()-h2, h2, distanceToBoundary(cell.getX()+h2, cell.getY()-h2));
            cellQueue.emplace(cell.getX()-h2, cell.getY()+h2, h2, distanceToBoundary(cell.getX()-h2, cell.getY()+h2));
            cellQueue.emplace(cell.getX()+h2, cell.getY()+h2, h2, distanceToBoundary(cell.getX()+h2, cell.getY()+h2));
        }
    }

    // the farthest cell is the best approximation to the MIC center
    Cell centerCell = farthestCell;
    centerPt.x = centerCell.getX();
    centerPt.y = centerCell.getY();

    // compute radius point
    std::unique_ptr<Point> centerPoint(factory->createPoint(centerPt));
    const auto& nearestPts = indexedDistance.nearestPoints(centerPoint.get());
    radiusPt = nearestPts->getAt(0);

    // flag computation
    done = true;
}




} // namespace geos.algorithm.construct
} // namespace geos.algorithm
} // namespace geos
