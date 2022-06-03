/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/simplify/PolygonHullSimplifier.h>
#include <geos/simplify/RingHull.h>
#include <geos/simplify/RingHullIndex.h>

#include <geos/algorithm/Area.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/util/math.h>
#include <geos/util/IllegalArgumentException.h>


using geos::algorithm::Area;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::geom::MultiPolygon;


namespace geos {
namespace simplify { // geos.simplify


/* public static */
std::unique_ptr<Geometry>
PolygonHullSimplifier::hull(const Geometry* geom, bool bOuter, double vertexNumFraction)
{
    PolygonHullSimplifier hull(geom, bOuter);
    hull.setVertexNumFraction(std::abs(vertexNumFraction));
    return hull.getResult();
}


/* public static */
std::unique_ptr<Geometry>
PolygonHullSimplifier::hullByAreaDelta(const Geometry* geom, bool bOuter, double areaDeltaRatio)
{
    PolygonHullSimplifier hull(geom, bOuter);
    hull.setAreaDeltaRatio(std::abs(areaDeltaRatio));
    return hull.getResult();
}


/* public */
void
PolygonHullSimplifier::setVertexNumFraction(double p_vertexNumFraction)
{
    vertexNumFraction = util::clamp(p_vertexNumFraction, 0.0, 1.0);
}


/* public */
void
PolygonHullSimplifier::setAreaDeltaRatio(double p_areaDeltaRatio)
{
    areaDeltaRatio = p_areaDeltaRatio;
}


/* public */
std::unique_ptr<Geometry>
PolygonHullSimplifier::getResult()
{
    //-- handle trivial parameter values
    if (vertexNumFraction == 1 || areaDeltaRatio == 0) {
        return inputGeom->clone();
    }

    if (inputGeom->getGeometryTypeId() == geom::GEOS_MULTIPOLYGON) {
        /**
        * Only outer hulls where there is more than one polygon
        * can potentially overlap.
        * Shell outer hulls could overlap adjacent shell hulls
        * or hole hulls surrounding them;
        * hole outer hulls could overlap contained shell hulls.
        */
        bool isOverlapPossible = isOuter && (inputGeom->getNumGeometries() > 1);
        if (isOverlapPossible) {
            return computeMultiPolygonAll(static_cast<const MultiPolygon*>(inputGeom));
        }
        else {
            return computeMultiPolygonEach(static_cast<const MultiPolygon*>(inputGeom));
        }
    }
    else if (inputGeom->getGeometryTypeId() == geom::GEOS_POLYGON) {
        return computePolygon(static_cast<const Polygon*>(inputGeom));
    }
    throw util::IllegalArgumentException("Input geometry must be polygonal");
}


/* private */
std::unique_ptr<Geometry>
PolygonHullSimplifier::computeMultiPolygonAll(const MultiPolygon* multiPoly)
{
    RingHullIndex hullIndex;
    std::size_t nPoly = multiPoly->getNumGeometries();
    std::vector<std::vector<RingHull*>> polyHulls;

    //TODO: investigate if reordering input elements improves result

    //-- prepare element polygon hulls and index
    for (std::size_t i = 0 ; i < nPoly; i++) {
        const Polygon* poly = multiPoly->getGeometryN(i);
        std::vector<RingHull*> ringHulls = initPolygon(poly, hullIndex);
        polyHulls.push_back(ringHulls);
    }

    //-- compute hull polygons
    std::vector<std::unique_ptr<Polygon>> polys;
    for (std::size_t i = 0; i < nPoly; i++) {
        const Polygon* poly = multiPoly->getGeometryN(i);
        std::unique_ptr<Polygon> polyHull = polygonHull(poly, polyHulls[i], hullIndex);
        polys.emplace_back(polyHull.release());
    }
    return geomFactory->createMultiPolygon(std::move(polys));
}


/* private */
std::unique_ptr<Geometry>
PolygonHullSimplifier::computeMultiPolygonEach(const MultiPolygon* multiPoly)
{
    std::vector<std::unique_ptr<Polygon>> polys;
    for (std::size_t i = 0 ; i < multiPoly->getNumGeometries(); i++) {
        const Polygon* poly = multiPoly->getGeometryN(i);
        std::unique_ptr<Polygon> polyHull = computePolygon(poly);
        polys.emplace_back(polyHull.release());
    }
    return geomFactory->createMultiPolygon(std::move(polys));
}


/* private */
std::unique_ptr<Polygon>
PolygonHullSimplifier::computePolygon(const Polygon* poly)
{
    RingHullIndex hullIndex;
    /**
     * For a single polygon overlaps are only possible for inner hulls
     * and where holes are present.
     */
    bool isOverlapPossible = ! isOuter && (poly->getNumInteriorRing() > 0);
    hullIndex.enabled(isOverlapPossible);

    std::vector<RingHull*> inHulls = initPolygon(poly, hullIndex);
    std::unique_ptr<Polygon> polyHull = polygonHull(poly, inHulls, hullIndex);
    return polyHull;
}


/* private */
std::vector<RingHull*>
PolygonHullSimplifier::initPolygon(const Polygon* poly, RingHullIndex& hullIndex)
{
    std::vector<RingHull*> hulls;
    if (poly->isEmpty())
        return hulls;

    double areaTotal = 0.0;
    if (areaDeltaRatio >= 0) {
        areaTotal = ringArea(poly);
    }
    hulls.push_back(createRingHull(poly->getExteriorRing(), isOuter, areaTotal, hullIndex));
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        //Assert: interior ring is not empty
        RingHull* ringHull = createRingHull(poly->getInteriorRingN(i), ! isOuter, areaTotal, hullIndex);
        hulls.push_back(ringHull);
    }
    return hulls;
}


/* private */
double
PolygonHullSimplifier::ringArea(const Polygon* poly) const
{
    double area = Area::ofRing(poly->getExteriorRing()->getCoordinatesRO());
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        area += Area::ofRing(poly->getInteriorRingN(i)->getCoordinatesRO());
    }
    return area;
}


/* private */
RingHull*
PolygonHullSimplifier::createRingHull(const LinearRing* ring, bool p_isOuter, double areaTotal, RingHullIndex& hullIndex)
{
    // Create the RingHull in our memory store and
    // then grab back the raw pointer.
    RingHull* ringHull = new RingHull(ring, p_isOuter);
    ringStore.emplace_back(ringHull);
    // ringStore.emplace_back(new RingHull(ring, p_isOuter));
    // RingHull* ringHull = ringStore.back().get();

    double dNumPoints = static_cast<double>(ring->getNumPoints());
    if (vertexNumFraction >= 0) {
        std::size_t targetVertexCount = static_cast<std::size_t>(
            std::ceil(vertexNumFraction * (dNumPoints - 1)));
        ringHull->setMinVertexNum(targetVertexCount);
    }
    else if (areaDeltaRatio >= 0) {
        double linearRingArea = Area::ofRing(ring->getCoordinatesRO());
        double linearRingWeight = linearRingArea / areaTotal;
        double maxAreaDelta = linearRingWeight * areaDeltaRatio * linearRingArea;
        ringHull->setMaxAreaDelta(maxAreaDelta);
    }
    if (hullIndex.enabled()) {
        hullIndex.add(ringHull);
    }
    return ringHull;
}


/* private */
std::unique_ptr<Polygon>
PolygonHullSimplifier::polygonHull(const Polygon* poly, std::vector<RingHull*>& ringHulls, RingHullIndex& hullIndex) const
{
    if (poly->isEmpty())
        return poly->clone();

    std::size_t ringIndex = 0;
    std::unique_ptr<LinearRing> shellHull = ringHulls[ringIndex++]->getHull(hullIndex);
    std::vector<std::unique_ptr<LinearRing>> holeHulls;
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        std::unique_ptr<LinearRing> polyHull = ringHulls[ringIndex++]->getHull(hullIndex);
        //TODO: handle empty
        holeHulls.emplace_back(polyHull.release());
    }
    return geomFactory->createPolygon(
        std::move(shellHull),
        std::move(holeHulls));
}


} // namespace geos.simplify
} // namespace geos

