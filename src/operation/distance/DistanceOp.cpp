/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/distance/DistanceOp.java r335 (JTS-1.12-)
 *
 **********************************************************************/

#include <geos/constants.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/distance/GeometryLocation.h>
#include <geos/operation/distance/ConnectedElementLocationFilter.h>
#include <geos/algorithm/Distance.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/util/SimpleCurveExtracter.h>
#include <geos/geom/util/SurfaceExtracter.h>
#include <geos/geom/util/PointExtracter.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <limits>
#include <vector>
#include <iostream>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif


using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance


/*public static (deprecated)*/
double
DistanceOp::distance(const Geometry* g0, const Geometry* g1)
{
    DistanceOp distOp(g0, g1);
    return distOp.distance();
}

/*public static*/
double
DistanceOp::distance(const Geometry& g0, const Geometry& g1)
{
    DistanceOp distOp(g0, g1);
    return distOp.distance();
}

/*public static*/
const std::array<GeometryLocation, 2>&
DistanceOp::nearestLocations()
{
    computeMinDistance();
    return minDistanceLocation;
}

/*public static*/
std::unique_ptr<CoordinateSequence>
DistanceOp::nearestPoints(const Geometry* g0, const Geometry* g1)
{
    DistanceOp distOp(g0, g1);
    return distOp.nearestPoints();
}

DistanceOp::DistanceOp(const Geometry* g0, const Geometry* g1):
    geom{{g0, g1}},
    terminateDistance(0.0),
    minDistance(DoubleInfinity)
{}

DistanceOp::DistanceOp(const Geometry& g0, const Geometry& g1):
    geom{{&g0, &g1}},
    terminateDistance(0.0),
    minDistance(DoubleInfinity)
{}

DistanceOp::DistanceOp(const Geometry& g0, const Geometry& g1, double tdist)
    :
    geom{{&g0, &g1}},
    terminateDistance(tdist < 0.0 ? 0.0 : tdist),
    minDistance(DoubleInfinity)
{}

/**
 * Report the distance between the closest points on the input geometries.
 *
 * @return the distance between the geometries, or positive infinity if one of them is empty.
 */
double
DistanceOp::distance()
{
    using geos::util::IllegalArgumentException;

    if(geom[0] == nullptr || geom[1] == nullptr) {
        throw IllegalArgumentException("null geometries are not supported");
    }
    if(geom[0]->isEmpty() || geom[1]->isEmpty()) {
        return std::numeric_limits<double>::infinity();
    }
    if(geom[0]->getGeometryTypeId() == GEOS_POINT && geom[1]->getGeometryTypeId() == GEOS_POINT) {
        return static_cast<const Point*>(geom[0])->getCoordinate()->distance(*static_cast<const Point*>(geom[1])->getCoordinate());
    }

    computeMinDistance();
    return minDistance;
}

/* public */
std::unique_ptr<CoordinateSequence>
DistanceOp::nearestPoints()
{
    // lazily creates minDistanceLocation
    computeMinDistance();

    auto& locs = minDistanceLocation;

    // Empty input geometries result in this behaviour
    if(locs[0].getGeometryComponent() == nullptr || locs[1].getGeometryComponent() == nullptr) {
        // either both or none are set..
        assert(locs[0].getGeometryComponent() == nullptr && locs[1].getGeometryComponent() == nullptr);

        return nullptr;
    }

    auto nearestPts = detail::make_unique<CoordinateSequence>(2u);
    nearestPts->setAt(locs[0].getCoordinate(), 0);
    nearestPts->setAt(locs[1].getCoordinate(), 1);

    return nearestPts;
}

void
DistanceOp::updateMinDistance(std::array<GeometryLocation, 2> & locGeom, bool flip)
{
    // if not set then don't update
    if(locGeom[0].getGeometryComponent() == nullptr) {
        assert(locGeom[1].getGeometryComponent() == nullptr);
#if GEOS_DEBUG
        std::cerr << "updateMinDistance called with loc[0] == null and loc[1] == null" << std::endl;
#endif
        return;
    }

    if(flip) {
        minDistanceLocation[0] = std::move(locGeom[1]);
        minDistanceLocation[1] = std::move(locGeom[0]);
    }
    else {
        minDistanceLocation[0] = std::move(locGeom[0]);
        minDistanceLocation[1] = std::move(locGeom[1]);
    }
}

/*private*/
void
DistanceOp::computeMinDistance()
{
    // only compute once!
    if(computed) {
        return;
    }

#if GEOS_DEBUG
    std::cerr << "---Start: " << geom[0]->toString() << " - " << geom[1]->toString() << std::endl;
#endif

    computeContainmentDistance();

    if(minDistance <= terminateDistance) {
        computed = true;
        return;
    }

    computeFacetDistance();
    computed = true;

#if GEOS_DEBUG
    std::cerr << "---End " << std::endl;
#endif
}

/*private*/
void
DistanceOp::computeContainmentDistance()
{
    std::vector<const Surface*> polys1;
    geom::util::SurfaceExtracter::getSurfaces(*(geom[1]), polys1);

#if GEOS_DEBUG
    std::cerr << "SurfaceExtracter found " << polys1.size() << " polygons in geometry 2" << std::endl;
#endif

    // NOTE:
    // Expected to fill minDistanceLocation items
    // if minDistance <= terminateDistance

    std::array<GeometryLocation, 2> locPtPoly;
    // test if either geometry has a vertex inside the other
    if(! polys1.empty()) {
        auto insideLocs0 = ConnectedElementLocationFilter::getLocations(geom[0]);
        computeInside(insideLocs0, polys1, locPtPoly);

        if(minDistance <= terminateDistance) {
            assert(locPtPoly[0].getGeometryComponent());
            assert(locPtPoly[1].getGeometryComponent());

            minDistanceLocation[0] = std::move(locPtPoly[0]);
            minDistanceLocation[1] = std::move(locPtPoly[1]);

            return;
        }
    }

    std::vector<const Surface*> polys0;
    geom::util::SurfaceExtracter::getSurfaces(*(geom[0]), polys0);

#if GEOS_DEBUG
    std::cerr << "SurfaceExtracter found " << polys0.size() << " polygons in geometry 1" << std::endl;
#endif


    if(! polys0.empty()) {
        auto insideLocs1 = ConnectedElementLocationFilter::getLocations(geom[1]);
        computeInside(insideLocs1, polys0, locPtPoly);
        if(minDistance <= terminateDistance) {
            // flip locations, since we are testing geom 1 VS geom 0
            assert(locPtPoly[0].getGeometryComponent());
            assert(locPtPoly[1].getGeometryComponent());

            minDistanceLocation[0] = std::move(locPtPoly[1]);
            minDistanceLocation[1] = std::move(locPtPoly[0]);

            return;
        }
    }
}


/*private*/
void
DistanceOp::computeInside(std::vector<GeometryLocation> & locs,
                          const std::vector<const Surface*>& polys,
                          std::array<GeometryLocation, 2> & locPtPoly)
{
    for(auto& loc : locs) {
        for(const auto& poly : polys) {
            const auto& pt = loc.getCoordinate();

			if (Location::EXTERIOR != ptLocator.locate(pt, static_cast<const Geometry*>(poly))) {
				minDistance = 0.0;
				locPtPoly[0] = loc;
                locPtPoly[1] = GeometryLocation(poly, pt);
				return;
			}
        }
    }
}

/*private*/
void
DistanceOp::computeFacetDistance()
{
    using geom::util::SimpleCurveExtracter;
    using geom::util::PointExtracter;

    std::array<GeometryLocation, 2> locGeom;

    /*
     * Geometries are not wholly inside, so compute distance from lines
     * and points of one to lines and points of the other
     */
    std::vector<const SimpleCurve*> lines0;
    std::vector<const SimpleCurve*> lines1;
    SimpleCurveExtracter::getCurves(*(geom[0]), lines0);
    SimpleCurveExtracter::getCurves(*(geom[1]), lines1);

#if GEOS_DEBUG
    std::cerr << "SimpleCurveExtracter found "
              << lines0.size() << " lines in geometry 1 and "
              << lines1.size() << " lines in geometry 2 "
              << std::endl;
#endif

    // exit whenever minDistance goes LE than terminateDistance
    computeMinDistanceLines(lines0, lines1, locGeom);
    updateMinDistance(locGeom, false);
    if(minDistance <= terminateDistance) {
#if GEOS_DEBUG
        std::cerr << "Early termination after line-line distance" << std::endl;
#endif
        return;
    }

    Point::ConstVect pts1;
    PointExtracter::getPoints(*(geom[1]), pts1);

#if GEOS_DEBUG
    std::cerr << "PointExtracter found " << pts1.size() << " points in geometry 2" << std::endl;
#endif

    locGeom[0] = GeometryLocation();
    locGeom[1] = GeometryLocation();
    computeMinDistanceLinesPoints(lines0, pts1, locGeom);
    updateMinDistance(locGeom, false);
    if(minDistance <= terminateDistance) {
#if GEOS_DEBUG
        std::cerr << "Early termination after lines0-points1 distance" << std::endl;
#endif
        return;
    }

    Point::ConstVect pts0;
    PointExtracter::getPoints(*(geom[0]), pts0);

#if GEOS_DEBUG
    std::cerr << "PointExtracter found " << pts0.size() << " points in geometry 1" << std::endl;
#endif

    locGeom[0] = GeometryLocation();
    locGeom[1] = GeometryLocation();
    computeMinDistanceLinesPoints(lines1, pts0, locGeom);
    updateMinDistance(locGeom, true);
    if(minDistance <= terminateDistance) {
#if GEOS_DEBUG
        std::cerr << "Early termination after lines1-points0 distance" << std::endl;
#endif
        return;
    }

    locGeom[0] = GeometryLocation();
    locGeom[1] = GeometryLocation();
    computeMinDistancePoints(pts0, pts1, locGeom);
    updateMinDistance(locGeom, false);

#if GEOS_DEBUG
    std::cerr << "termination after pts-pts distance" << std::endl;
#endif
}

/*private*/
void
DistanceOp::computeMinDistanceLines(
    const std::vector<const SimpleCurve*>& lines0,
    const std::vector<const SimpleCurve*>& lines1,
    std::array<GeometryLocation, 2> & locGeom)
{
    for(const SimpleCurve* line0 : lines0) {
        if (line0->getGeometryTypeId() == GEOS_CLOTHOID) {
            throw util::UnsupportedOperationException("Clothoid distance is not implemented");
        }
        const bool isCurved0 = line0->getGeometryTypeId() == GEOS_CIRCULARSTRING;

        for(const SimpleCurve* line1 : lines1) {
            if (line1->getGeometryTypeId() == GEOS_CLOTHOID) {
                throw util::UnsupportedOperationException("Clothoid distance is not implemented");
            }
            const bool isCurved1 = line1->getGeometryTypeId() == GEOS_CIRCULARSTRING;

            if (line0->isEmpty() || line1->isEmpty())
                continue;

            if (isCurved0) {
                const CircularString* cs0 = detail::down_cast<const CircularString*>(line0);
                if (isCurved1) {
                    const CircularString* cs1 = detail::down_cast<const CircularString*>(line1);
                    computeMinDistance(cs0, cs1, locGeom);
                } else {
                    computeMinDistance(cs0, detail::down_cast<const LineString*>(line1), locGeom);
                }
            } else {
                if (isCurved1) {
                    const CircularString* cs1 = detail::down_cast<const CircularString*>(line1);
                    computeMinDistance(detail::down_cast<const LineString*>(line0), cs1, locGeom);
                } else {
                    computeMinDistance(detail::down_cast<const LineString*>(line0), detail::down_cast<const LineString*>(line1), locGeom);
                }
            }

            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistancePoints(
    const Point::ConstVect& points0,
    const Point::ConstVect& points1,
    std::array<GeometryLocation, 2> & locGeom)
{
    for(const Point* pt0 : points0) {
        for(const Point* pt1 : points1) {

            if (pt1->isEmpty() || pt0->isEmpty())
                continue;

            double dist = pt0->getCoordinate()->distance(*(pt1->getCoordinate()));

#if GEOS_DEBUG
            std::cerr << "Distance "
                      << pt0->toString() << " - "
                      << pt1->toString() << ": "
                      << dist << ", minDistance: " << minDistance
                      << std::endl;
#endif

            if(dist < minDistance) {
                minDistance = dist;
                // this is wrong - need to determine closest points on both segments!!!
                locGeom[0] = GeometryLocation(pt0, 0, *(pt0->getCoordinate()));
                locGeom[1] = GeometryLocation(pt1, 0, *(pt1->getCoordinate()));
            }

            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistanceLinesPoints(
    const std::vector<const SimpleCurve*>& lines,
    const std::vector<const Point*>& points,
    std::array<GeometryLocation, 2> & locGeom)
{
    for(const SimpleCurve* sc : lines) {
        for(const Point* pt : points) {
            if (sc->isEmpty() || pt->isEmpty())
                continue;

            if (sc->getGeometryTypeId() == GEOS_CLOTHOID) {
                throw util::UnsupportedOperationException("Clothoid distance is not implemented");
            }
            if (sc->getGeometryTypeId() == GEOS_CIRCULARSTRING) {
                const CircularString* cs = detail::down_cast<const CircularString*>(sc);
                computeMinDistance(cs, pt, locGeom);
            } else {
                const LineString* line = detail::down_cast<const LineString*>(sc);
                computeMinDistance(line, pt, locGeom);
            }

            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistance(
    const LineString* line0,
    const LineString* line1,
    std::array<GeometryLocation, 2> & locGeom)
{
    using geos::algorithm::Distance;

    const Envelope* lineEnv0 = line0->getEnvelopeInternal();
    const Envelope* lineEnv1 = line1->getEnvelopeInternal();
    if(lineEnv0->distance(*lineEnv1) > minDistance) {
        return;
    }

    const CoordinateSequence* coord0 = line0->getCoordinatesRO();
    const CoordinateSequence* coord1 = line1->getCoordinatesRO();
    std::size_t npts0 = coord0->getSize();
    std::size_t npts1 = coord1->getSize();

    // brute force approach!
    for(std::size_t i = 0; i < npts0 - 1; ++i) {
        const CoordinateXY& p00 = coord0->getAt<CoordinateXY>(i);
        const CoordinateXY& p01 = coord0->getAt<CoordinateXY>(i+1);

        Envelope segEnv0(p00, p01);

        if (segEnv0.distanceSquared(*lineEnv1) > minDistance*minDistance) {
            continue;
        }

        for(std::size_t j = 0; j < npts1 - 1; ++j) {
            const CoordinateXY& p10 = coord1->getAt<CoordinateXY>(j);
            const CoordinateXY& p11 = coord1->getAt<CoordinateXY>(j+1);

            Envelope segEnv1(p10, p11);

            if (segEnv0.distanceSquared(segEnv1) > minDistance*minDistance) {
                continue;
            }

            double dist = Distance::segmentToSegment(p00, p01, p10, p11);
            if(dist < minDistance) {
                minDistance = dist;

                auto closestPt = LineSegment::closestPoints(p00, p01, p10, p11);

                locGeom[0] = GeometryLocation(line0, i, closestPt[0]);
                locGeom[1] = GeometryLocation(line1, j, closestPt[1]);
            }
            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistance(
    const CircularString* cs0,
    const LineString* line1,
    std::array<GeometryLocation, 2> & locGeom)
{
    const Envelope* lineEnv0 = cs0->getEnvelopeInternal();
    const Envelope* lineEnv1 = line1->getEnvelopeInternal();
    if(lineEnv0->distance(*lineEnv1) > minDistance) {
        return;
    }

    const CoordinateSequence* coord1 = line1->getCoordinatesRO();
    std::size_t npts1 = coord1->getSize();

    // brute force approach!
    for(const auto& arc : cs0->getArcs()) {
        const Envelope arcEnv = arc.getEnvelope();

        if (arcEnv.distanceSquared(*lineEnv1) > minDistance*minDistance) {
            continue;
        }

        for(std::size_t j = 0; j < npts1 - 1; ++j) {
            const CoordinateXY& q0 = coord1->getAt<CoordinateXY>(j);
            const CoordinateXY& q1 = coord1->getAt<CoordinateXY>(j+1);

            Envelope segEnv(q0, q1);

            if (arcEnv.distanceSquared(segEnv) > minDistance*minDistance) {
                continue;
            }

            const auto closestPts = arc.closestPoints(q0, q1);

            const double dist = closestPts[0].distance(closestPts[1]);
            if(dist < minDistance) {
                minDistance = dist;

                locGeom[0] = GeometryLocation(cs0, arc.getCoordinatePosition(), closestPts[0]);
                locGeom[1] = GeometryLocation(line1, j, closestPts[1]);
            }

            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistance(
    const LineString* line0,
    const CircularString* cs1,
    std::array<GeometryLocation, 2> & locGeom)
{
    computeMinDistance(cs1, line0, locGeom);
    std::swap(locGeom[0], locGeom[1]);
}

/*private*/
void
DistanceOp::computeMinDistance(
    const CircularString* cs0,
    const CircularString* cs1,
    std::array<GeometryLocation, 2> & locGeom)
{
    const Envelope* geomEnv0 = cs0->getEnvelopeInternal();
    const Envelope* geomEnv1 = cs1->getEnvelopeInternal();

    if(geomEnv0->distance(*geomEnv1) > minDistance) {
        return;
    }

    // brute force approach!
    for(const auto& arc0 : cs0->getArcs()) {
        const Envelope arcEnv0 = arc0.getEnvelope();

        if (arcEnv0.distanceSquared(*geomEnv1) > minDistance*minDistance) {
            continue;
        }

        for (const auto& arc1 : cs1->getArcs()) {
            const Envelope arcEnv1 = arc1.getEnvelope();

            if (arcEnv0.distanceSquared(arcEnv1) > minDistance*minDistance) {
                continue;
            }

            const auto closestPoints = arc0.closestPoints(arc1);

            const double dist = closestPoints[0].distance(closestPoints[1]);
            if(dist < minDistance) {
                minDistance = dist;
                locGeom[0] = GeometryLocation(cs0, arc0.getCoordinatePosition(), closestPoints[0]);
                locGeom[1] = GeometryLocation(cs1, arc1.getCoordinatePosition(), closestPoints[1]);
            }

            if(minDistance <= terminateDistance) {
                return;
            }
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistance(const LineString* line,
                               const Point* pt,
                               std::array<GeometryLocation, 2> & locGeom)
{
    using geos::algorithm::Distance;

    const Envelope* env0 = line->getEnvelopeInternal();
    const Envelope* env1 = pt->getEnvelopeInternal();
    if(env0->distance(*env1) > minDistance) {
        return;
    }
    const CoordinateSequence* coord0 = line->getCoordinatesRO();
    const CoordinateXY* coord = pt->getCoordinate();

    // brute force approach!
    std::size_t npts0 = coord0->getSize();
    for(std::size_t i = 0; i < npts0 - 1; ++i) {
        double dist = Distance::pointToSegment(*coord, coord0->getAt<CoordinateXY>(i), coord0->getAt<CoordinateXY>(i + 1));
        if(dist < minDistance) {
            minDistance = dist;

            const auto& p0 = coord0->getAt<CoordinateXY>(i);
            const auto& p1 = coord0->getAt<CoordinateXY>(i+1);

            CoordinateXY segClosestPoint = LineSegment::closestPoint(p0, p1, *coord);

            locGeom[0] = GeometryLocation(line, i, segClosestPoint);
            locGeom[1] = GeometryLocation(pt, 0, *coord);
        }
        if(minDistance <= terminateDistance) {
            return;
        }
    }
}

/*private*/
void
DistanceOp::computeMinDistance(const CircularString* cs,
                               const Point* pt,
                               std::array<GeometryLocation, 2> & locGeom)
{
    const Envelope* env0 = cs->getEnvelopeInternal();
    const Envelope* env1 = pt->getEnvelopeInternal();
    if(env0->distance(*env1) > minDistance) {
        return;
    }

    // brute force approach!
    const CoordinateXY& coord = *pt->getCoordinate();
    for (const auto& arc : cs->getArcs()) {
        CoordinateXY closestPt = arc.closestPoint(coord);
        const double dist = coord.distance(closestPt);

        if(dist < minDistance) {
            minDistance = dist;
            locGeom[0] = GeometryLocation(cs, arc.getCoordinatePosition(), closestPt);
            locGeom[1] = GeometryLocation(pt, 0, coord);
        }

        if(minDistance <= terminateDistance) {
            return;
        }
    }
}

/* public static */
bool
DistanceOp::isWithinDistance(const geom::Geometry& g0,
                             const geom::Geometry& g1,
                             double distance)
{
    // check envelope distance for a short-circuit negative result
    if ( g0.isEmpty() || g1.isEmpty() ) return false;
    const Envelope* env0 = g0.getEnvelopeInternal();
    const Envelope* env1 = g1.getEnvelopeInternal();
    double envDist = env0->distance(*env1);
    if (envDist > distance)
      return false;

    // MD - could improve this further with a positive short-circuit based
    // on envelope MinMaxDist

    DistanceOp distOp(g0, g1, distance);
    return distOp.distance() <= distance;
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos
