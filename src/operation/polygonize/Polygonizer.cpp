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
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/util/Interrupt.h>
#include <geos/util/UnsupportedOperationException.h>
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

Polygonizer::SimpleCurveAdder::SimpleCurveAdder(Polygonizer* p):
    pol(p)
{
}

void
Polygonizer::SimpleCurveAdder::filter_ro(const Geometry* g)
{
    if (g->getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        const auto& cc = static_cast<const CompoundCurve&>(*g);
        for (std::size_t i = 0; i < cc.getNumCurves(); i++) {
            filter_ro(cc.getCurveN(i));
        }

        return;
    }

    if(const auto sc = dynamic_cast<const SimpleCurve*>(g)) {
        pol->add(sc);
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

void
Polygonizer::add(std::vector<Geometry*>* geomList)
{
    for(auto& g : (*geomList)) {
        add(g);
    }
}

void
Polygonizer::add(std::vector<const Geometry*>* geomList)
{
    for(auto& g : (*geomList)) {
        add(g);
    }
}

void
Polygonizer::add(const Geometry* g)
{
    g->apply_ro(&lineStringAdder);
}

void
Polygonizer::add(const SimpleCurve* line)
{
    // create a new graph using the factory from the input Geometry
    if(graph == nullptr) {
        graph.reset(new PolygonizeGraph(line->getFactory()));
    }

    if (line->getGeometryTypeId() == GEOS_CLOTHOID) {
        throw util::UnsupportedOperationException("Clothoid polygonize is not implemented");
    }
    if (line->getGeometryTypeId() == GEOS_SPIRALCURVE) {
        throw util::UnsupportedOperationException("Spiral curve polygonize is not implemented");
    }
    if (line->getGeometryTypeId() == GEOS_CIRCULARSTRING) {
        graph->addEdge(detail::down_cast<const CircularString*>(line));
    } else {
        graph->addEdge(detail::down_cast<const LineString*>(line));
    }
}

std::vector<std::unique_ptr<Polygon>>
Polygonizer::getPolygons()
{
    polygonize();

    std::vector<std::unique_ptr<Polygon>> ret;
    ret.reserve(polyList.size());
    for (auto& surf : polyList) {
        if (surf->getGeometryTypeId() == GEOS_POLYGON) {
            ret.emplace_back(detail::down_cast<Polygon*>(surf.release()));
        }
    }
    polyList.clear();

    return ret;
}

std::vector<std::unique_ptr<Surface>>
Polygonizer::getSurfaces()
{
    polygonize();

    return std::move(polyList);
}


/* public */
const std::vector<const SimpleCurve*>&
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
const std::vector<const SimpleCurve*>&
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
const std::vector<std::unique_ptr<Curve>>&
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

std::vector<std::unique_ptr<geom::Curve>>
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
    std::vector<std::unique_ptr<Curve>> invalidLines;
    for (EdgeRing* er : invalidRings) {
        if (isIncludedInvalid(er)) {
            auto ringGeom = er->getRingOwnership();
            if (ringGeom->getGeometryTypeId() == GEOS_LINEARRING) {
                ringGeom = ringGeom->getFactory()->createLineString(ringGeom->getCoordinates());
            }

            invalidLines.push_back(std::move(ringGeom));
        }
        er->setProcessed(true);
    }

    return invalidLines;
}

bool
Polygonizer::isIncludedInvalid(const EdgeRing* invalidRing)
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

std::vector<std::unique_ptr<Surface>>
Polygonizer::extractPolygons(std::vector<EdgeRing*> & shells, bool includeAll)
{
    std::vector<std::unique_ptr<Surface>> polys;
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

