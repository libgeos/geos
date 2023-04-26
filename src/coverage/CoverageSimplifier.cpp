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


using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::MultiLineString;


namespace geos {     // geos
namespace coverage { // geos.coverage

/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(
    std::vector<const Geometry*>& coverage,
    double tolerance)
{
    CoverageSimplifier simplifier(coverage);
    return simplifier.simplify(tolerance);
}

/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(
    const std::vector<std::unique_ptr<Geometry>>& coverage,
    double tolerance)
{
    std::vector<const Geometry*> geoms;
    for (auto& geom : coverage) {
        geoms.push_back(geom.get());
    }
    return simplify(geoms, tolerance);
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(
    std::vector<const Geometry*>& coverage,
    double tolerance)
{
    CoverageSimplifier simplifier(coverage);
    return simplifier.simplifyInner(tolerance);
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(
    const std::vector<std::unique_ptr<Geometry>>& coverage,
    double tolerance)
{
    std::vector<const Geometry*> geoms;
    for (auto& geom : coverage) {
        geoms.push_back(geom.get());
    }
    return simplifyInner(geoms, tolerance);
}


/* public */
CoverageSimplifier::CoverageSimplifier(std::vector<const Geometry*>& coverage)
    : m_input(coverage)
    , m_geomFactory(coverage.empty() ? nullptr : coverage[0]->getFactory())
    {}

/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplify(double tolerance)
{
    CoverageRingEdges cov(m_input);
    simplifyEdges(cov.getEdges(), nullptr, tolerance);
    return cov.buildCoverage();
}

/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageSimplifier::simplifyInner(double tolerance)
{
    CoverageRingEdges cov(m_input);
    std::vector<CoverageEdge*> innerEdges = cov.selectEdges(2);
    std::vector<CoverageEdge*> outerEdges = cov.selectEdges(1);
    std::unique_ptr<MultiLineString> constraintEdges = CoverageEdge::createLines(outerEdges, m_geomFactory);

    simplifyEdges(innerEdges, constraintEdges.get(), tolerance);
    return cov.buildCoverage();
}

/* private */
void
CoverageSimplifier::simplifyEdges(
    std::vector<CoverageEdge*> edges,
    const MultiLineString* constraints,
    double tolerance)
{
    std::unique_ptr<MultiLineString> lines = CoverageEdge::createLines(edges, m_geomFactory);
    std::vector<bool> freeRings = getFreeRings(edges);
    std::unique_ptr<MultiLineString> linesSimp = TPVWSimplifier::simplify(lines.get(), freeRings, constraints, tolerance);
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
