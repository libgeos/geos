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

#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/valid/IndexedNestedHoleTester.h>
#include <geos/operation/valid/IndexedNestedPolygonTester.h>
#include <geos/util/UnsupportedOperationException.h>
#include <geos/util/IllegalArgumentException.h>

#include <cmath>

using namespace geos::geom;
using geos::algorithm::locate::IndexedPointInAreaLocator;

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

/* public */
bool
IsValidOp::isValid()
{
    return isValidGeometry(inputGeometry);
}


/* public static */
bool
IsValidOp::isValid(const CoordinateXY* coord)
{
    if (std::isfinite(coord->x) && std::isfinite(coord->y)) {
        return true;
    }
    else {
        return false;
    }
}


/* public */
const TopologyValidationError *
IsValidOp::getValidationError()
{
    isValidGeometry(inputGeometry);
    return validErr.get();
}

void
IsValidOp::logInvalid(int code, const CoordinateXY& pt)
{
    validErr = detail::make_unique<TopologyValidationError>(code, pt);
}

/* private */
bool
IsValidOp::isValidGeometry(const Geometry* g)
{
    validErr.reset(nullptr);

    if (!g)
        throw util::IllegalArgumentException("Null geometry argument to IsValidOp");

    // empty geometries are always valid
    if (g->isEmpty()) return true;
    switch (g->getGeometryTypeId()) {
        case GEOS_POINT:
            return isValid(static_cast<const Point*>(g));
        case GEOS_MULTIPOINT:
            return isValid(static_cast<const MultiPoint*>(g));
        case GEOS_LINEARRING:
            return isValid(static_cast<const LinearRing*>(g));
        case GEOS_LINESTRING:
            return isValid(static_cast<const LineString*>(g));
        case GEOS_POLYGON:
            return isValid(static_cast<const Polygon*>(g));
        case GEOS_MULTIPOLYGON:
            return isValid(static_cast<const MultiPolygon*>(g));
        case GEOS_MULTILINESTRING:
            return isValid(static_cast<const GeometryCollection*>(g));
        case GEOS_GEOMETRYCOLLECTION:
            return isValid(static_cast<const GeometryCollection*>(g));
    }

    // geometry type not known
    throw util::UnsupportedOperationException(g->getGeometryType());
}


/* private */
bool
IsValidOp::isValid(const Point* g)
{
    checkCoordinatesValid(g->getCoordinatesRO());
    if (hasInvalidError()) return false;
    return true;
}


/* private */
bool
IsValidOp::isValid(const MultiPoint* g)
{
    for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
        const Point* p = g->getGeometryN(i);
        if (p->isEmpty()) continue;
        if (!isValid(p->getCoordinate())) {
            logInvalid(TopologyValidationError::eInvalidCoordinate,
                       *(p->getCoordinate()));
            return false;;
        }
    }
    return true;
}


/* private */
bool
IsValidOp::isValid(const LineString* g)
{
    checkCoordinatesValid(g->getCoordinatesRO());
    if (hasInvalidError()) return false;

    checkTooFewPoints(g, MIN_SIZE_LINESTRING);
    if (hasInvalidError()) return false;

    return true;
}


/* private */
bool
IsValidOp::isValid(const LinearRing* g)
{
    checkCoordinatesValid(g->getCoordinatesRO());
    if (hasInvalidError()) return false;

    checkRingClosed(g);
    if (hasInvalidError()) return false;

    checkRingPointSize(g);
    if (hasInvalidError()) return false;

    checkRingSimple(g);
    if (hasInvalidError()) return false;

    return true;
}


/* private */
bool
IsValidOp::isValid(const Polygon* g)
{
    checkCoordinatesValid(g);
    if (hasInvalidError()) return false;

    checkRingsClosed(g);
    if (hasInvalidError()) return false;

    checkRingsPointSize(g);
    if (hasInvalidError()) return false;

    PolygonTopologyAnalyzer areaAnalyzer(g, isInvertedRingValid);

    checkAreaIntersections(areaAnalyzer);
    if (hasInvalidError()) return false;

    checkHolesInShell(g);
    if (hasInvalidError()) return false;

    checkHolesNotNested(g);
    if (hasInvalidError()) return false;

    checkInteriorConnected(areaAnalyzer);
    if (hasInvalidError()) return false;

    return true;
}


/* private */
bool
IsValidOp::isValid(const MultiPolygon* g)
{
    for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
        const Polygon* p = g->getGeometryN(i);
        checkCoordinatesValid(p);
        if (hasInvalidError()) return false;

        checkRingsClosed(p);
        if (hasInvalidError()) return false;

        checkRingsPointSize(p);
        if (hasInvalidError()) return false;
    }

    PolygonTopologyAnalyzer areaAnalyzer(g, isInvertedRingValid);

    checkAreaIntersections(areaAnalyzer);
    if (hasInvalidError()) return false;

    for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
        const Polygon* p = g->getGeometryN(i);
        checkHolesInShell(p);
        if (hasInvalidError()) return false;
    }

    for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
        const Polygon* p = g->getGeometryN(i);
        checkHolesNotNested(p);
        if (hasInvalidError()) return false;
    }

    checkShellsNotNested(g);
    if (hasInvalidError()) return false;

    checkInteriorConnected(areaAnalyzer);
    if (hasInvalidError()) return false;

    return true;
}


/* private */
bool
IsValidOp::isValid(const GeometryCollection* gc)
{
    for (std::size_t i = 0; i < gc->getNumGeometries(); i++) {
        if (! isValidGeometry(gc->getGeometryN(i)))
            return false;
    }
    return true;
}


/* private */
void
IsValidOp::checkCoordinatesValid(const CoordinateSequence* coords)
{
    for (std::size_t i = 0; i < coords->size(); i++) {
        if (! isValid(coords->getAt<CoordinateXY>(i))) {
            logInvalid(TopologyValidationError::eInvalidCoordinate,
                       coords->getAt<CoordinateXY>(i));
            return;
        }
    }
}


/* private */
void
IsValidOp::checkCoordinatesValid(const Polygon* poly)
{
    checkCoordinatesValid(poly->getExteriorRing()->getCoordinatesRO());
    if (hasInvalidError()) return;
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        checkCoordinatesValid(poly->getInteriorRingN(i)->getCoordinatesRO());
        if (hasInvalidError()) return;
    }
}


/* private */
void
IsValidOp::checkRingClosed(const LinearRing* ring)
{
    if (ring->isEmpty()) return;
    if (! ring->isClosed()) {
        Coordinate pt = ring->getNumPoints() >= 1
                        ? ring->getCoordinateN(0)
                        : Coordinate();
        logInvalid(TopologyValidationError::eRingNotClosed, pt);
        return;
    }
}


/* private */
void
IsValidOp::checkRingsClosed(const Polygon* poly)
{
    checkRingClosed(poly->getExteriorRing());
    if (hasInvalidError()) return;
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        checkRingClosed(poly->getInteriorRingN(i));
        if (hasInvalidError()) return;
    }
}


/* private */
void
IsValidOp::checkRingsPointSize(const Polygon* poly)
{
    checkRingPointSize(poly->getExteriorRing());
    if (hasInvalidError()) return;
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        checkRingPointSize(poly->getInteriorRingN(i));
        if (hasInvalidError()) return;
    }
}


/* private */
void
IsValidOp::checkRingPointSize(const LinearRing* ring)
{
    if (ring->isEmpty()) return;
    checkTooFewPoints(ring, MIN_SIZE_RING);
}


/* private */
void
IsValidOp::checkTooFewPoints(const LineString* line, std::size_t minSize)
{
    if (! isNonRepeatedSizeAtLeast(line, minSize) ) {
        CoordinateXY pt = line->getNumPoints() >= 1
                        ? line->getCoordinatesRO()->getAt<CoordinateXY>(0)
                        : Coordinate();
        logInvalid(TopologyValidationError::eTooFewPoints, pt);
    }
}


/* private */
bool
IsValidOp::isNonRepeatedSizeAtLeast(const LineString* line, std::size_t minSize)
{
    std::size_t numPts = 0;
    const CoordinateXY* prevPt = nullptr;
    const CoordinateSequence& seq = *line->getCoordinatesRO();
    for (std::size_t i = 0; i < seq.size(); i++) {
        if (numPts >= minSize) return true;
        const CoordinateXY& pt = seq.getAt<CoordinateXY>(i);
        if (prevPt == nullptr || ! pt.equals2D(*prevPt))
            numPts++;
        prevPt = &pt;
    }
    return numPts >= minSize;
}


/* private */
void
IsValidOp::checkAreaIntersections(PolygonTopologyAnalyzer& areaAnalyzer)
{
    if (areaAnalyzer.hasInvalidIntersection()) {
        logInvalid(areaAnalyzer.getInvalidCode(),
                   areaAnalyzer.getInvalidLocation());
    }
}


/* private */
void
IsValidOp::checkRingSimple(const LinearRing* ring)
{
    CoordinateXY intPt = PolygonTopologyAnalyzer::findSelfIntersection(ring);
    if (! intPt.isNull()) {
        logInvalid(TopologyValidationError::eRingSelfIntersection,
            intPt);
    }
}


/* private */
void
IsValidOp::checkHolesInShell(const Polygon* poly)
{
    // skip test if no holes are present
    if (poly->getNumInteriorRing() <= 0) return;

    const LinearRing* shell = poly->getExteriorRing();
    bool isShellEmpty = shell->isEmpty();

    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        const LinearRing* hole = poly->getInteriorRingN(i);
        if (hole->isEmpty()) continue;

        const CoordinateXY* invalidPt = nullptr;
        if (isShellEmpty) {
            invalidPt = hole->getCoordinate();
        }
        else {
            invalidPt = findHoleOutsideShellPoint(hole, shell);
        }
        if (invalidPt != nullptr) {
            logInvalid(
                TopologyValidationError::eHoleOutsideShell,
                *invalidPt);
            return;
        }
    }
}


/* private */
const CoordinateXY*
IsValidOp::findHoleOutsideShellPoint(const LinearRing* hole, const LinearRing* shell)
{
    const CoordinateXY& holePt0 = hole->getCoordinatesRO()->getAt<CoordinateXY>(0);
    /**
     * If hole envelope is not covered by shell, it must be outside
     */
    if (! shell->getEnvelopeInternal()->covers(hole->getEnvelopeInternal()))
        return &holePt0;

    if (PolygonTopologyAnalyzer::isRingNested(hole, shell))
        return nullptr;
    return &holePt0;
}


/* private */
void
IsValidOp::checkHolesNotNested(const Polygon* poly)
{
    // skip test if no holes are present
    if (poly->getNumInteriorRing() <= 0) return;

    IndexedNestedHoleTester nestedTester(poly);
    if (nestedTester.isNested()) {
        logInvalid(TopologyValidationError::eNestedHoles,
                   nestedTester.getNestedPoint());
    }
}


/* private */
void
IsValidOp::checkShellsNotNested(const MultiPolygon* mp)
{
    // skip test if only one shell present
    if (mp->getNumGeometries() <= 1)
        return;

    IndexedNestedPolygonTester nestedTester(mp);
    if (nestedTester.isNested()) {
        logInvalid(TopologyValidationError::eNestedShells,
                   nestedTester.getNestedPoint());
    }
}


/* private */
void
IsValidOp::checkInteriorConnected(PolygonTopologyAnalyzer& analyzer)
{
    if (analyzer.isInteriorDisconnected())
        logInvalid(TopologyValidationError::eDisconnectedInterior,
                   analyzer.getDisconnectionLocation());
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
