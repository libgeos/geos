/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2023 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageSimplifier.h>
#include <geos/coverage/CoverageEdge.h>
#include <geos/coverage/CoverageRingEdges.h>
#include <geos/coverage/TPVWSimplifier.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>
#include <geos/util/IllegalArgumentException.h>


using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::MultiLineString;
using geos::util::ProgressFunction;

namespace geos {     // geos
namespace coverage { // geos.coverage

/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(
    std::vector<const Geometry*>& coverage,
    double tolerance,
    ProgressFunction* progressFunction)
{
    CoverageSimplifier simplifier(coverage);
    return simplifier.simplify(tolerance, progressFunction);
}

/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(
    const std::vector<std::unique_ptr<Geometry>>& coverage,
    double tolerance,
    ProgressFunction* progressFunction)
{
    std::vector<const Geometry*> geoms;
    for (auto& geom : coverage) {
        geoms.push_back(geom.get());
    }
    return simplify(geoms, tolerance, progressFunction);
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(
    std::vector<const Geometry*>& coverage,
    double tolerance,
    ProgressFunction* progressFunction)
{
    CoverageSimplifier simplifier(coverage);
    return simplifier.simplifyInner(tolerance, progressFunction);
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(
    const std::vector<std::unique_ptr<Geometry>>& coverage,
    double tolerance,
    ProgressFunction* progressFunction)
{
    std::vector<const Geometry*> geoms;
    for (auto& geom : coverage) {
        geoms.push_back(geom.get());
    }
    return simplifyInner(geoms, tolerance, progressFunction);
}


/* public */
CoverageSimplifier::CoverageSimplifier(const std::vector<const Geometry*>& coverage)
    : m_input(coverage)
    , m_geomFactory(coverage.empty() ? nullptr : coverage[0]->getFactory())
    {
        for (const Geometry* g: m_input) {
            auto typeId = g->getGeometryTypeId();
            if (typeId != geom::GEOS_POLYGON && typeId != geom::GEOS_MULTIPOLYGON)
                throw util::IllegalArgumentException("Argument is non-polygonal");
        }
    }

/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(double tolerance,
                             ProgressFunction* progressFunction)
{
    geos::util::ProgressFunction subProgress;
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0, 0.8, *progressFunction);
    }
    CoverageRingEdges cov(m_input, progressFunction ? &subProgress : nullptr);
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0.8, 0.9, *progressFunction);
    }
    simplifyEdges(cov.getEdges(), nullptr, tolerance, progressFunction ? &subProgress : nullptr);
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0.9, 1.0, *progressFunction);
    }
    return cov.buildCoverage(progressFunction ? &subProgress : nullptr);
}

/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(double tolerance,
                                  ProgressFunction* progressFunction)
{
    geos::util::ProgressFunction subProgress;
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0, 0.7, *progressFunction);
    }
    CoverageRingEdges cov(m_input, progressFunction ? &subProgress : nullptr);
    std::vector<CoverageEdge*> innerEdges = cov.selectEdges(2);
    std::vector<CoverageEdge*> outerEdges = cov.selectEdges(1);
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0.7, 0.8, *progressFunction);
    }
    std::unique_ptr<MultiLineString> constraintEdges = CoverageEdge::createLines(
        outerEdges, m_geomFactory, progressFunction ? &subProgress : nullptr);

    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0.8, 0.9, *progressFunction);
    }
    simplifyEdges(innerEdges, constraintEdges.get(), tolerance, progressFunction ? &subProgress : nullptr);
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0.9, 1.0, *progressFunction);
    }
    return cov.buildCoverage(progressFunction ? &subProgress : nullptr);
}

/* private */
void
CoverageSimplifier::simplifyEdges(
    std::vector<CoverageEdge*> edges,
    const MultiLineString* constraints,
    double tolerance,
    ProgressFunction* progressFunction)
{
    constexpr double RATIO_FIRST_PASS = 0.5;
    geos::util::ProgressFunction subProgress;
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            0, RATIO_FIRST_PASS, *progressFunction);
    }
    std::unique_ptr<MultiLineString> lines = CoverageEdge::createLines(
        edges, m_geomFactory, progressFunction ? &subProgress : nullptr);
    std::vector<bool> freeRings = getFreeRings(edges);
    if (progressFunction)
    {
        subProgress = geos::util::CreateScaledProgressFunction(
            RATIO_FIRST_PASS, 1, *progressFunction);
    }
    std::unique_ptr<MultiLineString> linesSimp = TPVWSimplifier::simplify(
        lines.get(), freeRings, constraints, tolerance,
        progressFunction ? &subProgress : nullptr);
    //Assert: mlsSimp.getNumGeometries = edges.length

    setCoordinates(edges, linesSimp.get());
}


/* private */
void
CoverageSimplifier::setCoordinates(std::vector<CoverageEdge*>& edges, const MultiLineString* lines)
{
    for (std::size_t i = 0; i < edges.size(); i++) {
        CoverageEdge* edge = edges[i];
        edge->setCoordinates(lines->getGeometryN(i)->getCoordinatesRO());
    }
}


/* private */
std::vector<bool>
CoverageSimplifier::getFreeRings(const std::vector<CoverageEdge*>& edges) const
{
    std::vector<bool> freeRings;
    for (auto edge: edges) {
        freeRings.push_back(edge->isFreeRing());
    }
    return freeRings;
}


} // geos.coverage
} // geos
