/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CleanCoverage.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/index/quadtree/Quadtree.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/relateng/IntersectionMatrixPattern.h>
#include <geos/operation/relateng/RelateNG.h>


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::index::quadtree::Quadtree;
using geos::operation::overlayng::OverlayNG;
using geos::operation::overlayng::OverlayNGRobust;
using geos::operation::relateng::IntersectionMatrixPattern;
using geos::operation::relateng::RelateNG;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public */
CleanCoverage::CleanCoverage(std::size_t size)
{
    cov.resize(size);
}


/* public */
void
CleanCoverage::add(std::size_t i, const Polygon* poly)
{
    if (cov[i] == nullptr) {
        cov[i] = std::make_unique<CleanArea>();
    }
    cov[i]->add(poly);
}


/* public */
void
CleanCoverage::mergeOverlap(const Polygon* overlap,
    MergeStrategy& mergeStrategy,
    std::vector<std::size_t>& parentIndexes)
{
    std::size_t mergeTarget = findMergeTarget(overlap, mergeStrategy, parentIndexes, cov);
    add(mergeTarget, overlap);
}


/* public static */
std::size_t
CleanCoverage::findMergeTarget(const Polygon* poly,
    MergeStrategy& strat,
    std::vector<std::size_t>& parentIndexes,
    std::vector<std::unique_ptr<CleanArea>>& cov)
{
    //-- sort parent indexes ascending, so that overlaps merge to first parent by default
    std::vector<size_t> indexesAsc;
    indexesAsc.copy(parentIndexes.begin(), parentIndexes.end(), back_inserter(indexesAsc));
    std::sort(indexesAsc.begin(), indexesAsc.end());

    for (std::size_t index : indexesAsc) {
        strat.checkMergeTarget(index, cov[index].get(), poly);
    }
    return strat.getTarget();
}


/* public */
void
CleanCoverage::mergeGaps(std::vector<const Polygon*>& gaps)
{
    createIndex();
    for (const Polygon* gap : gaps) {
        mergeGap(gap);
    }
}


/* private */
void
CleanCoverage::mergeGap(const Polygon* gap)
{
    std::vector<CleanArea*> adjacents = findAdjacentAreas(gap);

    /**
     * No adjacent means this is likely an artifact
     * of an invalid input polygon.
     * Discard polygon.
     */
    if (adjacents.empty())
        return;

    CleanArea* mergeTarget = findMaxBorderLength(gap, adjacents);
    covIndex->remove(mergeTarget->getEnvelope(), mergeTarget);
    mergeTarget->add(gap);
    covIndex->insert(mergeTarget->getEnvelope(), mergeTarget);
}


/* private */
CleanArea*
CleanCoverage::findMaxBorderLength(const Polygon* poly,
    std::vector<CleanArea*>& areas)
{
    double maxLen = 0;
    CleanArea* maxLenArea = nullptr;
    for (CleanArea* a : areas) {
        double len = a->getBorderLength(poly);
        if (maxLenArea == nullptr || len > maxLen) {
            maxLen = len;
            maxLenArea = a;
        }
    }
    return maxLenArea;
}


/* private */
std::vector<CleanArea*>
CleanCoverage::findAdjacentAreas(const Geometry* poly)
{
    std::vector<CleanArea*> adjacents;
    auto rel = RelateNG::prepare(poly);
    const Envelope* queryEnv = poly->getEnvelopeInternal();

    std::vector<CleanArea*> candidateAdjIndex = covIndex->query(queryEnv);

    std::vector<void*> queryResult;
    query(queryEnv, queryResult);

    for (void* ptr : queryResult) {
        CleanArea* area = static_cast<CleanArea*>(ptr);
        if (area != nullptr && area->isAdjacent(*rel)) {
            adjacents.push_back(area);
        }
    }
    return adjacents;
}


/* private */
void
CleanCoverage::createIndex()
{
    covIndex = std::make_unique<Quadtree>();
    for (std::size_t i = 0; i < cov.size(); i++) {
        //-- null areas are never merged to
        if (cov[i] != nullptr) {
            covIndex->insert(cov[i]->getEnvelope(), static_cast<void*>(cov[i].get()));
        }
    }
}


/* public */
std::vector<std::unique_ptr<Geometry>>
CleanCoverage::toCoverage(const GeometryFactory* geomFactory)
{
    std::vector<std::unique_ptr<Geometry>> cleanCov;
    cleanCov.resize(cov.size());
    for (std::size_t i = 0; i < cov.size(); i++) {
        std::unique_ptr<Geometry> merged;
        if (cov[i] == nullptr) {
            merged = geomFactory->createEmpty(2);
        }
        else {
            merged = cov[i]->union();
        }
        cleanCov[i] = std::move(merged);
    }
    return cleanCov;
}


///// CleanCoverage::CleanArea ////////////////////////////////////////


/* public */
void
CleanCoverage::CleanArea::add(const Polygon* poly)
{
    polys.push_back(poly);
}


/* public */
const Envelope*
CleanCoverage::CleanArea::getEnvelope()
{
    env.init();
    for (const Polygon* poly : polys) {
        env.expandToInclude(poly->getEnvelopeInternal());
    }
    return &env;
}


/* public */
double
CleanCoverage::CleanArea::getBorderLength(const Polygon* adjPoly)
{
    //TODO: find optimal way of computing border len given a coverage
    double len = 0.0;
    for (const Polygon* poly : polys) {
        //TODO: find longest connected border len
        auto border = OverlayNGRobust::overlay(
            static_cast<const Geometry*>(poly),
            static_cast<const Geometry*>(adjPoly),
            OverlayNG::INTERSECTION);
        double borderLen = border->getLength();
        len += borderLen;
    }
    return len;
}


/* public */
double
CleanCoverage::CleanArea::getArea()
{
    //TODO: cache area?
    double area = 0.0;
    for (const Polygon* poly : polys) {
        area += poly->getArea();
    }
    return area;
}


/* public */
bool
CleanCoverage::CleanArea::isAdjacent(RelateNG& rel)
{
    for (const Polygon* poly : polys) {
        //TODO: is there a faster way to check adjacency in coverage?
        auto geom = static_cast<const Geometry*>(poly);
        bool isAdjacent = rel.evaluate(geom, IntersectionMatrixPattern::ADJACENT);
        if (isAdjacent)
            return true;
    }
    return false;
}


/* public */
std::unique_ptr<Geometry>
CleanCoverage::CleanArea::union()
{
    std::vector<const Geometry*> geoms;
    for (const Polygon* poly : polys) {
        geoms.push_back(static_cast<const Geometry*>(poly));
    }
    return CoverageUnion::union(geoms);
}



} // namespace geos.coverage
} // namespace geos


