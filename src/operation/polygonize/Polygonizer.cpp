/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2010 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/Polygonizer.java 0b3c7e3eb0d3e
 *
 **********************************************************************/

#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/polygonize/PolygonizeGraph.h>
#include <geos/operation/polygonize/EdgeRing.h>
#include <geos/operation/polygonize/HoleAssigner.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/util/Interrupt.h>
// std
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif


using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

Polygonizer::LineStringAdder::LineStringAdder(Polygonizer* p):
    pol(p)
{
}

void
Polygonizer::LineStringAdder::filter_ro(const Geometry* g)
{
    auto ls = dynamic_cast<const LineString*>(g);
    if(ls) {
        pol->add(ls);
    }
}

Polygonizer::Polygonizer(bool onlyPolygonal):
    lineStringAdder(this),
    extractOnlyPolygonal(onlyPolygonal),
    computed(false),
    graph(nullptr),
    dangles(),
    cutEdges(),
    invalidRingLines(),
    holeList(),
    shellList()
{
}

/*
 * Add a collection of geometries to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param geomList a list of {@link Geometry}s with linework to be polygonized
 */
void
Polygonizer::add(std::vector<Geometry*>* geomList)
{
    for(auto& g : (*geomList)) {
        add(g);
    }
}

/*
 * Add a collection of geometries to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param geomList a list of {@link Geometry}s with linework to be polygonized
 */
void
Polygonizer::add(std::vector<const Geometry*>* geomList)
{
    for(auto& g : (*geomList)) {
        add(g);
    }
}

/*
 * Add a geometry to the linework to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param g a Geometry with linework to be polygonized
 */
void
Polygonizer::add(Geometry* g)
{
    g->apply_ro(&lineStringAdder);
}

/*
 * Add a geometry to the linework to be polygonized.
 * May be called multiple times.
 * Any dimension of Geometry may be added;
 * the constituent linework will be extracted and used
 *
 * @param g a Geometry with linework to be polygonized
 */
void
Polygonizer::add(const Geometry* g)
{
    g->apply_ro(&lineStringAdder);
}

/*
 * Add a linestring to the graph of polygon edges.
 *
 * @param line the LineString to add
 */
void
Polygonizer::add(const LineString* line)
{
    // create a new graph using the factory from the input Geometry
    if(graph == nullptr) {
        graph.reset(new PolygonizeGraph(line->getFactory()));
    }
    graph->addEdge(line);
}

/*
 * Gets the list of polygons formed by the polygonization.
 * @return a collection of Polygons
 */
std::vector<std::unique_ptr<Polygon>>
Polygonizer::getPolygons()
{
    polygonize();
    return std::move(polyList);
}

/* public */
const std::vector<const LineString*>&
Polygonizer::getDangles()
{
    polygonize();
    return dangles;
}

bool
Polygonizer::hasDangles() {
    polygonize();
    return !dangles.empty();
}

/* public */
const std::vector<const LineString*>&
Polygonizer::getCutEdges()
{
    polygonize();
    return cutEdges;
}

bool
Polygonizer::hasCutEdges()
{
    polygonize();
    return !cutEdges.empty();
}

/* public */
const std::vector<std::unique_ptr<LineString>>&
Polygonizer::getInvalidRingLines()
{
    polygonize();
    return invalidRingLines;
}

bool
Polygonizer::hasInvalidRingLines()
{
    polygonize();
    return !invalidRingLines.empty();
}

bool
Polygonizer::allInputsFormPolygons()
{
    polygonize();
    return !hasCutEdges() && !hasDangles() &&!hasInvalidRingLines();
}

/* public */
void
Polygonizer::polygonize()
{
    // check if already computed
    if(computed) {
        return;
    }

    // if no geometries were supplied it's possible graph could be null
    if(graph == nullptr) {
        polyList.clear();
        return;
    }

    graph->deleteDangles(dangles);

    graph->deleteCutEdges(cutEdges);

    std::vector<EdgeRing*> edgeRingList;
    graph->getEdgeRings(edgeRingList);
#if GEOS_DEBUG
    std::cerr << "Polygonizer::polygonize(): " << edgeRingList.size() << " edgeRings in graph" << std::endl;
#endif
    std::vector<EdgeRing*> validEdgeRingList;
    std::vector<EdgeRing*> invalidRings;
    invalidRingLines.clear(); /* what if it was populated already ? we should clean ! */
    findValidRings(edgeRingList, validEdgeRingList, invalidRings);
    invalidRingLines = extractInvalidLines(invalidRings);
#if GEOS_DEBUG
    std::cerr << "                           " << validEdgeRingList.size() << " valid" << std::endl;
    std::cerr << "                           " << invalidRingLines.size() << " invalid" << std::endl;
#endif

    findShellsAndHoles(validEdgeRingList);
#if GEOS_DEBUG
    std::cerr << "                           " << holeList.size() << " holes" << std::endl;
    std::cerr << "                           " << shellList.size() << " shells" << std::endl;
#endif

    HoleAssigner::assignHolesToShells(holeList, shellList);

    bool includeAll = true;
    if (extractOnlyPolygonal) {
        findDisjointShells();
        includeAll = false;
    }
    polyList = extractPolygons(shellList, includeAll);

    computed = true;
}

/* private */
void
Polygonizer::findValidRings(const std::vector<EdgeRing*>& edgeRingList,
                            std::vector<EdgeRing*>& validEdgeRingList,
                            std::vector<EdgeRing*>& invalidRingList)
{
    for(const auto& er : edgeRingList) {
        er->computeValid();
        if(er->isValid()) {
            validEdgeRingList.push_back(er);
        }
        else {
            invalidRingList.push_back(er);
        }
        GEOS_CHECK_FOR_INTERRUPTS();
    }
}

std::vector<std::unique_ptr<geom::LineString>>
Polygonizer::extractInvalidLines(std::vector<EdgeRing*>& invalidRings)
{
    /**
     * Sort rings by increasing envelope area.
     * This causes inner rings to be processed before the outer rings
     * containing them, which allows outer invalid rings to be discarded
     * since their linework is already reported in the inner rings.
     */
    std::sort(invalidRings.begin(),
              invalidRings.end(),
              [](EdgeRing* a, EdgeRing* b) {
                return a->getRingInternal()->getEnvelope()->getArea() <
                       b->getRingInternal()->getEnvelope()->getArea();
    });

    /**
     * Scan through rings.  Keep only rings which have an adjacent EdgeRing
     * which is either valid or marked as not processed.
     * This avoids including outer rings which have linework which is duplicated.
     */
    std::vector<std::unique_ptr<LineString>> invalidLines;
    for (EdgeRing* er : invalidRings) {
        if (isIncludedInvalid(er)) {
            invalidLines.push_back(er->getLineString());
        }
        er->setProcessed(true);
    }

    return invalidLines;
}

bool
Polygonizer::isIncludedInvalid(EdgeRing* invalidRing)
{
    for (const PolygonizeDirectedEdge* de: invalidRing->getEdges()) {
        const PolygonizeDirectedEdge* deAdj = static_cast<PolygonizeDirectedEdge*>(de->getSym());
        const EdgeRing* erAdj = deAdj->getRing();

        bool isEdgeIncluded = erAdj->isValid() || erAdj->isProcessed();
        if (!isEdgeIncluded) {
            return true;
        }
    }

    return false;
}

/* private */
void
Polygonizer::findShellsAndHoles(const std::vector<EdgeRing*>& edgeRingList)
{
    holeList.clear();
    shellList.clear();
    for(auto& er : edgeRingList) {
        er->computeHole();
        if(er->isHole()) {
            holeList.push_back(er);
        }
        else {
            shellList.push_back(er);
        }

        GEOS_CHECK_FOR_INTERRUPTS();
    }
}


void
Polygonizer::findDisjointShells() {
    findOuterShells(shellList);

    for (EdgeRing *er : shellList) {
        if (!er->isIncludedSet()) {
            er->updateIncludedRecursive();
        }
    }

    return;
}

void
Polygonizer::findOuterShells(std::vector<EdgeRing*> & shells)
{
    for (EdgeRing* er : shells) {
        auto outerHoleER = er->getOuterHole();
        if (outerHoleER != nullptr && !outerHoleER->isProcessed()) {
            er->setIncluded(true);
            outerHoleER->setProcessed(true);
        }
    }
}

std::vector<std::unique_ptr<Polygon>>
Polygonizer::extractPolygons(std::vector<EdgeRing*> & shells, bool includeAll)
{
    std::vector<std::unique_ptr<Polygon>> polys;
    for (EdgeRing* er : shells) {
        if (includeAll || er->isIncluded()) {
            polys.emplace_back(er->getPolygon());
        }
    }

    return polys;
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

