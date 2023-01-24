/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Location.h>
#include <geos/index/SpatialIndex.h>
#include <geos/index/strtree/STRtree.h>
#include <geos/operation/valid/PolygonTopologyAnalyzer.h>
#include <geos/operation/valid/IndexedNestedPolygonTester.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using namespace geos::geom;


/* public */
IndexedNestedPolygonTester::IndexedNestedPolygonTester(const MultiPolygon* p_multiPoly)
    : multiPoly(p_multiPoly)
    , nestedPt(Coordinate::getNull())
{
    loadIndex();
}


/* private */
void
IndexedNestedPolygonTester::loadIndex()
{
    for (std::size_t i = 0; i < multiPoly->getNumGeometries(); i++) {
        const Polygon* poly = multiPoly->getGeometryN(i);
        const Envelope* env = poly->getEnvelopeInternal();
        index.insert(*env, poly);
    }
}


/* private */
IndexedPointInAreaLocator&
IndexedNestedPolygonTester::getLocator(const Polygon* poly)
{
    auto search = locators.find(poly);

    // Entry not found
    if (search == locators.end())
    {
        // uses pair's piecewise constructor to emplace into
        // std::map<const Polygon*, IndexedPointInAreaLocator> locators;
        locators.emplace(std::piecewise_construct,
            std::forward_as_tuple(poly),
            std::forward_as_tuple(*poly));
        auto search2 = locators.find(poly);
        return search2->second;
    }

    IndexedPointInAreaLocator& locator = search->second;
    return locator;
}


/* public */
bool
IndexedNestedPolygonTester::isNested()
{
    for (std::size_t i = 0; i < multiPoly->getNumGeometries(); i++) {
        const Polygon* poly = multiPoly->getGeometryN(i);
        const LinearRing* shell = poly->getExteriorRing();

        std::vector<const Polygon*> results;
        index.query(*(poly->getEnvelopeInternal()), results);

        for (const Polygon* possibleOuterPoly: results) {

            if (poly == possibleOuterPoly)
                continue;
            /**
             * If polygon is not fully covered by candidate polygon it cannot be nested
             */
            if (! possibleOuterPoly->getEnvelopeInternal()->covers(poly->getEnvelopeInternal()))
                continue;

            bool gotNestedPt = findNestedPoint(shell, possibleOuterPoly, getLocator(possibleOuterPoly), nestedPt);
            if (gotNestedPt)
                return true;
        }
    }
    return false;
}


/* private */
bool
IndexedNestedPolygonTester::findNestedPoint(
    const LinearRing* shell,
    const Polygon* possibleOuterPoly,
    IndexedPointInAreaLocator& locator,
    CoordinateXY& coordNested)
{
    /**
     * Try checking two points, since checking point location is fast.
     */
    const CoordinateSequence* shellCoords = shell->getCoordinatesRO();

    const CoordinateXY& shellPt0 = shellCoords->front();
    Location loc0 = locator.locate(&shellPt0);
    if (loc0 == Location::EXTERIOR) return false;
    if (loc0 == Location::INTERIOR) {
        coordNested = shellPt0;
        return true;
    }

    const CoordinateXY& shellPt1 = shellCoords->getAt<CoordinateXY>(1);
    Location loc1 = locator.locate(&shellPt1);
    if (loc1 == Location::EXTERIOR) return false;
    if (loc1 == Location::INTERIOR) {
        coordNested = shellPt1;
        return true;
    }

    /**
     * The shell points both lie on the boundary of
     * the polygon.
     * Nesting can be checked via the topology of the incident edges.
     */
    return findIncidentSegmentNestedPoint(shell, possibleOuterPoly, coordNested);
}


/**
* Finds a point of a shell segment which lies inside a polygon, if any.
* The shell is assume to touch the polyon only at shell vertices,
* and does not cross the polygon.
*
* @param the shell to test
* @param the polygon to test against
* @return an interior segment point, or null if the shell is nested correctly
*/
/* private static */
bool
IndexedNestedPolygonTester::findIncidentSegmentNestedPoint(
    const LinearRing* shell,
    const Polygon* poly,
    CoordinateXY& coordNested)
{
    const LinearRing* polyShell = poly->getExteriorRing();
    if (polyShell->isEmpty())
        return false;

    if (! PolygonTopologyAnalyzer::isRingNested(shell, polyShell))
        return false;

    /**
     * Check if the shell is inside a hole (if there are any).
     * If so this is valid.
     */
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        const LinearRing* hole = poly->getInteriorRingN(i);
        if (hole->getEnvelopeInternal()->covers(shell->getEnvelopeInternal())
            && PolygonTopologyAnalyzer::isRingNested(shell, hole))
        {
            return false;
        }
    }

    /**
     * The shell is contained in the polygon, but is not contained in a hole.
     * This is invalid.
     */
    coordNested = shell->getCoordinatesRO()->getAt<CoordinateXY>(0);
    return true;
}



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
