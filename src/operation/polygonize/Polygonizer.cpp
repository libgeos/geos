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
 * Last port: operation/polygonize/Polygonizer.java rev. 974
 *
 **********************************************************************/

#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/polygonize/PolygonizeGraph.h>
#include <geos/operation/polygonize/EdgeRing.h>
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

using namespace std;
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
    graph(nullptr),
    dangles(),
    cutEdges(),
    invalidRingLines(),
    holeList(),
    shellList(),
    polyList(nullptr)
{
}

Polygonizer::~Polygonizer()
{
    for(auto& r : invalidRingLines) {
        delete r;
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
Polygonizer::add(vector<Geometry*>* geomList)
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
Polygonizer::add(vector<const Geometry*>* geomList)
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
unique_ptr<vector<unique_ptr<Polygon>>>
Polygonizer::getPolygons()
{
    polygonize();
    return std::move(polyList);
}

/* public */
const vector<const LineString*>&
Polygonizer::getDangles()
{
    polygonize();
    return dangles;
}

/* public */
const vector<const LineString*>&
Polygonizer::getCutEdges()
{
    polygonize();
    return cutEdges;
}

/* public */
const vector<LineString*>&
Polygonizer::getInvalidRingLines()
{
    polygonize();
    return invalidRingLines;
}

/* public */
void
Polygonizer::polygonize()
{
    // check if already computed
    if(polyList != nullptr) {
        return;
    }

    // if no geometries were supplied it's possible graph could be null
    if(graph == nullptr) {
        polyList.reset(new std::vector<std::unique_ptr<Polygon>>());
        return;
    }

    graph->deleteDangles(dangles);

    graph->deleteCutEdges(cutEdges);

    vector<EdgeRing*> edgeRingList;
    graph->getEdgeRings(edgeRingList);
#if GEOS_DEBUG
    cerr << "Polygonizer::polygonize(): " << edgeRingList.size() << " edgeRings in graph" << endl;
#endif
    vector<EdgeRing*> validEdgeRingList;
    invalidRingLines.clear(); /* what if it was populated already ? we should clean ! */
    findValidRings(edgeRingList, validEdgeRingList, invalidRingLines);
#if GEOS_DEBUG
    cerr << "                           " << validEdgeRingList.size() << " valid" << endl;
    cerr << "                           " << invalidRingLines.size() << " invalid" << endl;
#endif

    findShellsAndHoles(validEdgeRingList);
#if GEOS_DEBUG
    cerr << "                           " << holeList.size() << " holes" << endl;
    cerr << "                           " << shellList.size() << " shells" << endl;
#endif

    assignHolesToShells(holeList, shellList);

    bool includeAll = true;
    if (extractOnlyPolygonal) {
        findDisjointShells(shellList);
        includeAll = false;
    }
    polyList = extractPolygons(shellList, includeAll);
}

/* private */
void
Polygonizer::findValidRings(const vector<EdgeRing*>& edgeRingList,
                            vector<EdgeRing*>& validEdgeRingList,
                            vector<LineString*>& invalidRingList)
{
    for(const auto& er : edgeRingList) {
        if(er->isValid()) {
            validEdgeRingList.push_back(er);
        }
        else {
            invalidRingList.push_back(er->getLineString().release());
        }
        GEOS_CHECK_FOR_INTERRUPTS();
    }
}

/* private */
void
Polygonizer::findShellsAndHoles(const vector<EdgeRing*>& edgeRingList)
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

/* private */
void
Polygonizer::assignHolesToShells(const vector<EdgeRing*>& holeList, vector<EdgeRing*>& shellList)
{
    for(const auto& holeER : holeList) {
        assignHoleToShell(holeER, shellList);
        GEOS_CHECK_FOR_INTERRUPTS();
    }
}

/* private */
void
Polygonizer::assignHoleToShell(EdgeRing* holeER,
                               vector<EdgeRing*>& shellList)
{
    EdgeRing* shell = EdgeRing::findEdgeRingContaining(holeER, &shellList);

    if(shell != nullptr) {
        shell->addHole(holeER);
    }
}

void
Polygonizer::findDisjointShells(vector<EdgeRing*> & shells)
{
    findOuterShells(shells);

    bool isMoreToScan;
    do {
        isMoreToScan = false;
        for (EdgeRing* er : shells) {
            if (er->isIncludedSet()) {
                continue;
            }
            er->updateIncluded();
            if (!er->isIncludedSet()) {
                isMoreToScan = true;
            }
        }
    } while (isMoreToScan);
}

void
Polygonizer::findOuterShells(vector<EdgeRing*> & shells)
{
    for (EdgeRing* er : shells) {
        auto outerHoleER = er->getOuterHole();
        if (outerHoleER != nullptr && !outerHoleER->isProcessed()) {
            er->setIncluded(true);
            outerHoleER->setProcessed(true);
        }
    }
}

std::unique_ptr<std::vector<std::unique_ptr<Polygon>>>
Polygonizer::extractPolygons(vector<EdgeRing*> & shells, bool includeAll)
{
    std::unique_ptr<std::vector<std::unique_ptr<Polygon>>> polys(new std::vector<std::unique_ptr<Polygon>>);
    for (EdgeRing* er : shells) {
        if (includeAll || er->isIncluded()) {
            polys->emplace_back(er->getPolygon());
        }
    }

    return polys;
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

