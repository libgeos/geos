/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Jonathan Adams <jd.adams16@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/MaximumInscribedCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>

#include <memory> // for unique_ptr
#include <typeinfo>
#include <vector>

using namespace geos::geom;
using namespace geos::triangulate;
using namespace geos::util;

namespace geos {
namespace algorithm { // geos.algorithm

/*public*/
std::unique_ptr<Geometry>
MaximumInscribedCircle::getCircle()
{
    // handle degenerate or trivial cases
    if(inputGeom->isEmpty()) {
        return std::unique_ptr<Geometry>(inputGeom->getFactory()->createPolygon());
    } else if(inputGeom->getNumPoints() == 1) {
        return std::unique_ptr<Geometry>(inputGeom->clone());
    }
    compute();
}

/*private*/
void
MaximumInscribedCircle::compute()
{
    // handle degenerate or trivial cases
    if(inputGeom->isEmpty()) {
        return;
    }
    if(inputGeom->getNumPoints() == 1) {
        return;
    }
    // computeSites();
}

/*private*/
void
MaximumInscribedCircle::computeSites(geom::Polygon& poly)
{
    const LineString* exterior = poly.getExteriorRing();
    const LineString* interior;

    // Initialize bounding box
    xmin = exterior->getStartPoint()->getX();
    xmax = xmin;
    ymin = exterior->getStartPoint()->getY();
    ymax = ymin;

    addRingSites(exterior);

    for(unsigned int i = 0; i < poly.getNumInteriorRing(); i++) {
        interior = poly.getInteriorRingN(i);
        addRingSites(interior);
    }
}

/**
* This function implements the majority of GeoJSONUtils.sites from the
* nodeJS module max-inscribed-circle (npmjs.com/package/max-inscribed-circle).
* It takes a ring, and n-sects each segment of the ring, where n is the
* number of segments MaximumInscribedCircle has been called with. It then
* adds all of these n-sect coordinates into an array of coorinates that
* will eventually be converted to a CoordinateSequence and used to call
* Voronoi.
*
* This function also updates the coordinates of a bounding box that will
* be passed on the same call to Voronoi.
*
* @param ring an internal or external ring
*/
/*private*/
void
MaximumInscribedCircle::addRingSites(const LineString* ring)
{
    double fromX, fromY, toX, toY, segmentX, segmentY;
    std::unique_ptr<Point> fromPoint;
    std::unique_ptr<Point> toPoint;

    for(unsigned int i = 0; i < ring->getNumPoints(); i++) {
        fromPoint = ring->getPointN(i);
        toPoint = ring->getPointN(i+1);
        fromX = fromPoint->getX();
        fromY = fromPoint->getY();
        toX = toPoint->getX();
        toY = toPoint->getY();

        // Push fromPoint
        sites.push_back(*(fromPoint->getCoordinate()));

        // Push segments
        for(unsigned int k = 0; k < *inputNumSegments; k++) {
            segmentX = fromX + (toX - fromX) * k / *inputNumSegments;
            segmentY = fromY + (toY - fromY) * k / *inputNumSegments;
            sites.push_back(Coordinate(segmentX, segmentY));
        }

        // Update bounding box
        if(fromX < xmin) {
            xmin = fromX;
        }
        if(fromX > xmax) {
            xmax = fromX;
        }
        if(fromY < ymin) {
            ymin = fromY;
        }
        if(fromY > ymax) {
            ymax = fromY;
        }
    }
}

// TODO: Figure out if I can use one of the extractor functions defined
//       in GEOSGeom or MakeValid.
static std::unique_ptr<geom::Geometry>
extractUniquePoints(const geom::Geometry* geom)
{

    // Code taken from GEOSGeom_extractUniquePoints_r()

    /* 1: extract points */
    std::vector<const geom::Coordinate*> coords;
    UniqueCoordinateArrayFilter filter(coords);
    geom->apply_ro(&filter);

    /* 2: for each point, create a geometry and put into a vector */
    std::vector<std::unique_ptr<Geometry>> points;
    points.reserve(coords.size());
    const GeometryFactory* factory = geom->getFactory();
    for(const Coordinate* c : coords) {
        points.emplace_back(factory->createPoint(*c));
    }

    /* 3: create a multipoint */
    return factory->createMultiPoint(std::move(points));
}

/*private*/
void
MaximumInscribedCircle::computeVoronoiVertices()
{
    VoronoiDiagramBuilder builder;
    std::unique_ptr<CoordinateSequence> coords;
    CoordinateSequenceFactory* coordSeqFactory;
    const GeometryFactory& geomFact(*GeometryFactory::getDefaultInstance());
    std::unique_ptr<Geometry> results;
    builder.setSites(*(coordSeqFactory->create(&sites)));

    results = builder.getDiagramEdges(geomFact);
    for(unsigned int i = 0; i < results->getNumGeometries(); i++) {
        coords = extractUniquePoints(results->getGeometryN(i))->getCoordinates();
        for(unsigned int j = 0; j < coords->getSize(); j++) {
            voronoiVertices.push_back(coords->getAt(j));
        }
    }
}

/*private*/
void
MaximumInscribedCircle::computeCenterAndRadius(geom::Polygon& poly)
{
    
}

} // namespace geos.algorithm
} // namespace geos
