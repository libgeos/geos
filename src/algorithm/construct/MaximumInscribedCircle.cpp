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
 * https://github.com/locationtech/jts/commit/f0b9a808bdf8a973de435f737e37b7a221e231cb
 *
 **********************************************************************/

#include <geos/algorithm/construct/MaximumInscribedCircle.h>
#include <geos/algorithm/construct/ExactMaxInscribedCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
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
        throw util::IllegalArgumentException("Input must be a Polygon or MultiPolygon");
    }

    if (polygonal->isEmpty()) {
        throw util::IllegalArgumentException("Empty input is not supported");
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
bool
MaximumInscribedCircle::isRadiusWithin(const Geometry* polygonal, double maxRadius)
{
    MaximumInscribedCircle mic(polygonal, -1);
    return mic.isRadiusWithin(maxRadius);
}

/* public static */
std::size_t
MaximumInscribedCircle::computeMaximumIterations(const Geometry* geom, double toleranceDist)
{
    double diam = geom->getEnvelopeInternal()->getDiameter();
    double ncells = diam / toleranceDist;
    //-- Using log of ncells allows control over number of iterations
    int factor = (int) std::log(ncells);
    if (factor < 1) factor = 1;
    return (std::size_t) (2000 + 2000 * factor);
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
MaximumInscribedCircle::distanceToBoundary(double x, double y)
{
    Coordinate coord(x, y);
    std::unique_ptr<Point> pt(factory->createPoint(coord));
    return distanceToBoundary(*pt.get());
}

/* private */
double
MaximumInscribedCircle::distanceToBoundary(const Point& pt)
{
    double dist = indexedDistance.distance(&pt);
    // double dist = inputGeomBoundary->distance(pt.get());
    bool isOutside = (Location::EXTERIOR == ptLocator.locate(pt.getCoordinate()));
    if (isOutside) return -dist;
    return dist;
}

/* private */
MaximumInscribedCircle::Cell
MaximumInscribedCircle::createInteriorPointCell(const Geometry* geom)
{
    std::unique_ptr<Point> p = geom->getInteriorPoint();
    Cell cell(p->getX(), p->getY(), 0, distanceToBoundary(*p.get()));
    return cell;
}


/* public */
bool
MaximumInscribedCircle::isRadiusWithin(double maxRadius)
{
    if (maxRadius < 0) {
        throw util::IllegalArgumentException("Radius length must be non-negative");
    }
    //-- handle 0 corner case, to provide maximum domain
    if (maxRadius == 0) {
        return false;
    }
    maximumRadius = maxRadius;

    /**
     * Check if envelope dimension is smaller than diameter
     */
    const Envelope* env = inputGeom->getEnvelopeInternal();
    double maxDiam = 2 * maximumRadius;
    if (env->getWidth() < maxDiam || env->getHeight() < maxDiam) {
        return true;
    }

    tolerance = maxRadius * MAX_RADIUS_FRACTION;
    compute();
    double radius = centerPt.distance(radiusPt);
    return radius <= maximumRadius;
}


/* private */
void
MaximumInscribedCircle::compute()
{
    // check if already computed
    if (done) return;

    /**
     * Handle flat geometries.
     */
    if (inputGeom->getArea() == 0.0) {
        const CoordinateXY* c = inputGeom->getCoordinate();
        createResult(*c, *c);
        return;
    }

    /**
     * Optimization for small simple convex polygons
     */
    if (ExactMaxInscribedCircle::isSupported(inputGeom)) {
        auto polygonal = static_cast<const Polygon*>(inputGeom);
        std::pair<CoordinateXY, CoordinateXY> centreRadius = ExactMaxInscribedCircle::computeRadius(polygonal);
        createResult(centreRadius.first, centreRadius.second);
        return;
    }

    //-- only needed for approximation
    if (tolerance <= 0) {
        throw util::IllegalArgumentException("Tolerance must be positive");
    }

    computeApproximation();
}


/* private */
void MaximumInscribedCircle::createResult(
    const CoordinateXY& c, const CoordinateXY& r)
{
    centerPt = c;
    radiusPt = r;
}


/* private */
void
MaximumInscribedCircle::computeApproximation()
{
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

        //-- if cell must be closer than farthest, terminate since all remaining cells in queue are even closer.
        if (cell.getMaxDistance() < farthestCell.getDistance())
            break;

        // update the center cell if the candidate is further from the boundary
        if (cell.getDistance() > farthestCell.getDistance()) {
            farthestCell = cell;
        }

        //-- search termination when checking max radius predicate
        if (maximumRadius >= 0) {
            //-- found a inside point further than max radius
            if (farthestCell.getDistance() > maximumRadius)
                break;
        //-- no cells can have larger radius
        if (cell.getMaxDistance() < maximumRadius)
            break;
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
