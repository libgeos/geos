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
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/util/UniqueCoordinateArrayFilter.h>

#include <memory> // for unique_ptr
#include <typeinfo>
#include <vector>
#include <cstdlib>

using namespace std; // Remove when we remove cout statements.
using namespace geos::geom;
using namespace geos::triangulate;
using namespace geos::util;

namespace geos {
namespace algorithm { // geos.algorithm

/*public*/
Coordinate
MaximumInscribedCircle::getCenter()
{
    compute();
    return center;
}

/*public*/
std::unique_ptr<Geometry>
MaximumInscribedCircle::getCircle()
{
    compute();
    // handle degenerate or trivial cases
    if(input->isEmpty()) {
        return std::unique_ptr<Geometry>(input->getFactory()->createPolygon());
    } else if(input->getNumPoints() == 1) {
        return std::unique_ptr<Geometry>(input->clone());
    }
}

/**
 * Temporary development helper. To be removed in final version.
 */
/*public*/
std::vector<Coordinate>*
MaximumInscribedCircle::getSites()
{
    compute();
    return computeSites();
}

/**
 * Temporary development helper. To be removed in final version.
 */
std::vector<const Point*>
MaximumInscribedCircle::getVoronoiVertices()
{
    compute();
    computeVoronoiVertices();
    return voronoiVertices;
}

/*private*/
void
MaximumInscribedCircle::compute()
{
    if(typeid(*input) == typeid(Point)) {
        cout << "Geometry is of type Point" << endl;
        center = *(input->getCoordinate());
        return;
    } else if(typeid(*input) == typeid(Polygon)) {
        cout << "Geometry is of type Polygon" << endl;
        poly = dynamic_cast<const Polygon*>(input);
        computeSites();
    } else if(typeid(*input) == typeid(MultiPolygon)) {
        cout << "Geometry is of type MultiPolygon" << endl;
        const MultiPolygon* multiPoly = dynamic_cast<const MultiPolygon*>(input);
        const Polygon* poly = polygonOfMaxArea(multiPoly);
        computeSites();
    } else {
        // degenerate/trivial cases, LineString, MultiLineString, and MultiPoint
        return;
    }
    computeVoronoiVertices();
    computeCenterAndRadius();
    return;
}

/*private*/
std::vector<Coordinate>*
MaximumInscribedCircle::computeSites()
{
    std::vector<Coordinate>* sites = new std::vector<Coordinate>;

    const LineString* exterior = poly->getExteriorRing();
    const LineString* interior;

    // Initialize bounding box
    xmin = exterior->getStartPoint()->getX();
    xmax = xmin;
    ymin = exterior->getStartPoint()->getY();
    ymax = ymin;

    addRingSites(sites, exterior);

    for(unsigned int i = 0; i < poly->getNumInteriorRing(); i++) {
        interior = poly->getInteriorRingN(i);
        addRingSites(sites, interior);
    }

    return sites;
}

/**
* This function implements the majority of GeoJSONUtils.sites from the
* nodeJS module max-inscribed-circle (npmjs.com/package/max-inscribed-circle).
* It takes a ring, and n-sects each segment of the ring, where n is the
* number of segments MaximumInscribedCircle has been called with. It then
* adds all of these n-sect coordinates into an array of coordinates that
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
MaximumInscribedCircle::addRingSites(std::vector<Coordinate>* sites, const LineString* ring)
{
    double fromX, fromY, toX, toY, segmentX, segmentY;
    std::unique_ptr<Point> fromPoint;
    std::unique_ptr<Point> toPoint;

    for(unsigned int i = 0; i < ring->getNumPoints()-1; i++) {
        fromPoint = ring->getPointN(i);
        toPoint = ring->getPointN(i+1);
        fromX = fromPoint->getX();
        fromY = fromPoint->getY();
        toX = toPoint->getX();
        toY = toPoint->getY();

        // Push segments
        for(unsigned int k = 0; k < inputNumSegments; k++) {
            segmentX = fromX + (toX - fromX) * k / inputNumSegments;
            segmentY = fromY + (toY - fromY) * k / inputNumSegments;
            sites->push_back(Coordinate(segmentX, segmentY));
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
static std::unique_ptr<MultiPoint>
extractUniquePoints(const Geometry* geom)
{

    // Code taken from GEOSGeom_extractUniquePoints_r()

    /* 1: extract points */
    std::vector<const Coordinate*> coords;
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
    const GeometryFactory& geomFact(*GeometryFactory::getDefaultInstance());
    const CoordinateSequenceFactory* coordSeqFactory = geomFact.getCoordinateSequenceFactory();

    std::vector<Coordinate>* sites = computeSites();
    builder.setSites(*(coordSeqFactory->create(sites)));
    std::unique_ptr<Geometry> diagramEdges = builder.getDiagramEdges(geomFact);
    std::shared_ptr<Geometry> edges = std::move(diagramEdges);
    std::unique_ptr<CoordinateSequence> coords = extractUniquePoints(edges.get())->getCoordinates();
    for(unsigned int i = 0; i < coords->getSize(); i++) {
        const Point* point = geomFact.createPoint(coords->getAt(i));
        if(poly->intersects(point)) {
            voronoiVertices.push_back(point);
        }
    }

    // // TODO: Figure out if I can get this to work without buffer overflows so I can just use points returned by extractUniquePoints.
    // std::unique_ptr<MultiPoint> uniquePoints = extractUniquePoints(edges.get());
    // for(unsigned int i = 0; i < uniquePoints->getNumGeometries(); i++) {
    //     const Point* point  = uniquePoints->getGeometryN(i);
    //     if(poly->intersects(point)) {
    //         voronoiVertices.push_back(point);
    //     }
    // }
}

/**
 * This function takes the voronoi vertices and begins computing the distances to the polygon's exterior
 * and interior rings, keeping track of the greatest distance. Whichever vertex is the greatest distance
 * from the rings is considered the center and the distance is the radius.
 */
/*private*/
void
MaximumInscribedCircle::computeCenterAndRadius()
{
    double vertexDistance = 0, interiorRingDistance = 0, maxDistance = 0;
    const Point* bestCenterCandidate;
    const LineString* exteriorRing = poly->getExteriorRing();
    std::vector<const LineString*> interiorRings;
    for(auto vertex : voronoiVertices) {
        // Initialize the vertex distance
        vertexDistance = exteriorRing->distance(vertex);
        for(auto interiorRing : interiorRings) {
            // If any interior rings are closer, update the vertex distance
            interiorRingDistance = interiorRing->distance(vertex);
            if(interiorRingDistance < vertexDistance) {
                vertexDistance = interiorRingDistance;
            }
        }

        if(vertexDistance > maxDistance) {
            maxDistance = vertexDistance;
            bestCenterCandidate = vertex;
        }
    }

    radius = maxDistance;
    center = *(bestCenterCandidate->getCoordinate());
}

/**
* This function takes a multipolygon and returns the polygon of largest area
* in the collection.
*
* @param multiPoly
* @return the Polygon of largest area
*/
/*private*/
const Polygon*
MaximumInscribedCircle::polygonOfMaxArea(const MultiPolygon* multiPoly)
{
    double currentArea = 0, maxArea = 0;
    unsigned int maxAreaPolyIndex = 0;

    for(unsigned int i = 0; i < multiPoly->getNumGeometries(); i++) {
        currentArea = multiPoly->getGeometryN(i)->getArea();
        if(currentArea > maxArea) {
            maxArea = currentArea;
            maxAreaPolyIndex = i;
        }
    }

    return multiPoly->getGeometryN(maxAreaPolyIndex);
}

} // namespace geos.algorithm
} // namespace geos
