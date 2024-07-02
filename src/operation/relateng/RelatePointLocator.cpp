/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Location.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/relateng/AdjacentEdgeLocator.h>
#include <geos/operation/relateng/DimensionLocation.h>
#include <geos/operation/relateng/LinearBoundary.h>
#include <geos/operation/relateng/RelatePointLocator.h>
#include <geos/constants.h>


using geos::algorithm::PointLocation;
using geos::algorithm::locate::IndexedPointInAreaLocator;
using geos::algorithm::locate::PointOnGeometryLocator;
using geos::algorithm::locate::SimplePointInAreaLocator;
using namespace geos::geom;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* private */
void
RelatePointLocator::init(const Geometry* p_geom)
{
    //-- cache empty status, since may be checked many times
    isEmpty = p_geom->isEmpty();
    extractElements(p_geom);

    if (!lines.empty()) {
        lineBoundary.reset(new LinearBoundary(lines, boundaryRule));
    }

    if (!polygons.empty()) {
        polyLocator.resize(polygons.size());
    }
        // polyLocator = isPrepared
        //     ? new IndexedPointInAreaLocator[polygons.size()]
        //         : new SimplePointInAreaLocator[polygons.size()];
}


/* public */
bool
RelatePointLocator::hasBoundary() const
{
    return lineBoundary->hasBoundary();
}


/* private */
void
RelatePointLocator::extractElements(const Geometry* p_geom)
{
    if (p_geom->isEmpty())
        return;

    if (p_geom->getGeometryTypeId() == GEOS_POINT) {
        addPoint(static_cast<const Point*>(p_geom));
    }
    else if (p_geom->getGeometryTypeId() == GEOS_LINESTRING) {
        addLine(static_cast<const LineString*>(p_geom));
    }
    else if (p_geom->getGeometryTypeId() == GEOS_POLYGON ||
             p_geom->getGeometryTypeId() == GEOS_MULTIPOLYGON)
    {
        addPolygonal(p_geom);
    }
    else if (p_geom->isCollection()) {
        for (std::size_t i = 0; i < p_geom->getNumGeometries(); i++) {
            const Geometry* g = p_geom->getGeometryN(i);
            extractElements(g);
        }
    }
}


/* private */
void
RelatePointLocator::addPoint(const Point* pt)
{
    points.insert(pt->getCoordinate());
}


/* private */
void
RelatePointLocator::addLine(const LineString* line)
{
    lines.push_back(line);
}


/* private */
void
RelatePointLocator::addPolygonal(const Geometry* polygonal)
{
    polygons.push_back(polygonal);
}


/* public */
Location
RelatePointLocator::locate(const CoordinateXY* p)
{
    return DimensionLocation::location(locateWithDim(p));
}


/* public */
Location
RelatePointLocator::locateLineEnd(const CoordinateXY* p) const
{
    return lineBoundary->isBoundary(p) ? Location::BOUNDARY : Location::INTERIOR;
}


/* public */
Location
RelatePointLocator::locateNode(const CoordinateXY* p, const Geometry* parentPolygonal)
{
    return DimensionLocation::location(locateNodeWithDim(p, parentPolygonal));
}


/* public */
int
RelatePointLocator::locateNodeWithDim(const CoordinateXY* p, const Geometry* parentPolygonal)
{
    return locateWithDim(p, true, parentPolygonal);
}


/* public */
int
RelatePointLocator::locateWithDim(const CoordinateXY* p)
{
    return locateWithDim(p, false, nullptr);
}


/* private */
int
RelatePointLocator::locateWithDim(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal)
{
    if (isEmpty) return DimensionLocation::EXTERIOR;

    /**
     * In a polygonal geometry a node must be on the boundary.
     * (This is not the case for a mixed collection, since
     * the node may be in the interior of a polygon.)
     */
    GeometryTypeId geomType = geom->getGeometryTypeId();
    if (isNode && (geomType == GEOS_POLYGON || geomType == GEOS_MULTIPOLYGON))
        return DimensionLocation::AREA_BOUNDARY;

    int dimLoc = computeDimLocation(p, isNode, parentPolygonal);
    return dimLoc;
}


/* private */
int
RelatePointLocator::computeDimLocation(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal)
{
    //-- check dimensions in order of precedence
    if (!polygons.empty()) {
        Location locPoly = locateOnPolygons(p, isNode, parentPolygonal);
        if (locPoly != Location::EXTERIOR)
            return DimensionLocation::locationArea(locPoly);
    }
    if (!lines.empty()) {
        Location locLine = locateOnLines(p, isNode);
        if (locLine != Location::EXTERIOR)
            return DimensionLocation::locationLine(locLine);
    }
    if (!points.empty()) {
        Location locPt = locateOnPoints(p);
        if (locPt != Location::EXTERIOR)
            return DimensionLocation::locationPoint(locPt);
    }
    return DimensionLocation::EXTERIOR;
}


/* private */
Location
RelatePointLocator::locateOnPoints(const CoordinateXY* p) const
{
    auto search = points.find(p);
    if (search != points.end())
        return Location::INTERIOR;
    else
        return Location::EXTERIOR;
}


/* private */
Location
RelatePointLocator::locateOnLines(const CoordinateXY* p, bool isNode)
{
    if (lineBoundary != nullptr && lineBoundary->isBoundary(p)) {
        return Location::BOUNDARY;
    }
    //-- must be on line, in interior
    if (isNode)
        return Location::INTERIOR;

    //TODO: index the lines
    for (const LineString* line : lines) {
        //-- have to check every line, since any/all may contain point
        Location loc = locateOnLine(p, /*isNode,*/ line);
        if (loc != Location::EXTERIOR)
            return loc;
        //TODO: minor optimization - some BoundaryNodeRules can short-circuit
    }
    return Location::EXTERIOR;
}


/* private */
Location
RelatePointLocator::locateOnLine(const CoordinateXY* p, /*bool isNode,*/ const LineString* l)
{
    // bounding-box check
    if (! l->getEnvelopeInternal()->intersects(*p))
        return Location::EXTERIOR;

    const CoordinateSequence* seq = l->getCoordinatesRO();
    if (PointLocation::isOnLine(*p, seq)) {
        return Location::INTERIOR;
    }
    return Location::EXTERIOR;
}


/* private */
Location
RelatePointLocator::locateOnPolygons(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal)
{
    int numBdy = 0;
    //TODO: use a spatial index on the polygons
    for (std::size_t i = 0; i < polygons.size(); i++) {
        Location loc = locateOnPolygonal(p, isNode, parentPolygonal, i);
        if (loc == Location::INTERIOR) {
            return Location::INTERIOR;
        }
        if (loc == Location::BOUNDARY) {
            numBdy += 1;
        }
    }
    if (numBdy == 1) {
        return Location::BOUNDARY;
    }
    //-- check for point lying on adjacent boundaries
    else if (numBdy > 1) {
        if (adjEdgeLocator == nullptr) {
            adjEdgeLocator.reset(new AdjacentEdgeLocator(geom));
        }
        return adjEdgeLocator->locate(p);
    }
    return Location::EXTERIOR;
}


/* private */
Location
RelatePointLocator::locateOnPolygonal(const CoordinateXY* p,
    bool isNode,
    const Geometry* parentPolygonal,
    std::size_t index)
{
    const Geometry* polygonal = polygons[index];
    if (isNode && parentPolygonal == polygonal) {
        return Location::BOUNDARY;
    }
    PointOnGeometryLocator* locator = getLocator(index);
    return locator->locate(p);
}


/* private */
PointOnGeometryLocator *
RelatePointLocator::getLocator(std::size_t index)
{
    std::unique_ptr<PointOnGeometryLocator>& locator = polyLocator[index];
    if (locator == nullptr) {
        const Geometry* polygonal = polygons[index];
        if (isPrepared) {
            locator.reset(new IndexedPointInAreaLocator(*polygonal));
        }
        else {
            locator.reset(new SimplePointInAreaLocator(*polygonal));
        }
    }
    return locator.get();
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




