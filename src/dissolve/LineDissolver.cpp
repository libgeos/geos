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

#include <geos/dissolve/LineDissolver.h>

#include <geos/dissolve/DissolveHalfEdge.h>
#include <geos/edgegraph/HalfEdge.h>
#include <geos/edgegraph/MarkHalfEdge.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>


using namespace geos::geom;
using geos::edgegraph::HalfEdge;
using geos::edgegraph::MarkHalfEdge;


namespace geos {      // geos
namespace dissolve {  // geos.dissolve


/* public */
std::unique_ptr<Geometry>
LineDissolver::dissolve(const Geometry* g)
{
    LineDissolver d;
    d.add(g);
    return d.getResult();
}


/* public */
void
LineDissolver::add(const Geometry* geom)
{
    if (factory == nullptr) {
        factory = geom->getFactory();
    }

    struct LineStringFilter : public GeometryComponentFilter {

        LineDissolver *m_ld;

        LineStringFilter(LineDissolver* ld) : m_ld(ld) {};

        void filter_ro(const Geometry* g) override {
            GeometryTypeId type = g->getGeometryTypeId();
            if (type == GEOS_LINEARRING || type == GEOS_LINESTRING) {
                m_ld->add(static_cast<const LineString*>(g));
            }
        }
    };

    LineStringFilter filter(this);
    geom->apply_ro(&filter);
}


/* public */
void
LineDissolver::add(std::vector<const Geometry*> geometries)
{
    for (const Geometry* geom : geometries) {
        add(geom);
    }
}


/* private */
void
LineDissolver::add(const LineString* lineString)
{
    const CoordinateSequence* seq = lineString->getCoordinatesRO();
    bool doneStart = false;
    for (std::size_t i = 1; i < seq->size(); i++) {
        CoordinateXYZM orig, dest;
        seq->getAt(i-1, orig);
        seq->getAt(i,   dest);
        DissolveHalfEdge* e = static_cast<DissolveHalfEdge*>(graph.addEdge(orig, dest));
        // skip zero-length edges
        if (e == nullptr) continue;
        /**
         * Record source initial segments, so that they can be reflected in output when needed
         * (i.e. during formation of isolated rings)
         */
        if (! doneStart) {
            e->setStart();
            doneStart = true;
        }
    }
}


/* public */
std::unique_ptr<Geometry>
LineDissolver::getResult()
{
    if (result == nullptr)
        computeResult();
    return std::move(result);
}


/* private */
void
LineDissolver::computeResult()
{
    std::vector<const HalfEdge*> edges;
    graph.getVertexEdges(edges);

    for (const HalfEdge* ce : edges) {
        HalfEdge* e = const_cast<HalfEdge*>(ce);
        if (MarkHalfEdge::isMarked(e)) continue;
        process(e);
    }

    result = factory->buildGeometry(std::move(lines));
}


/* private */
void
LineDissolver::process(HalfEdge* e)
{
    HalfEdge* eNode = e->prevNode();
    // if edge is in a ring, just process this edge
    if (eNode == nullptr)
        eNode = e;
    stackEdges(eNode);
    // extract lines from node edges in stack
    buildLines();
}


/* private */
void
LineDissolver::stackEdges(HalfEdge* node)
{
    HalfEdge* e = node;
    do {
        if (! MarkHalfEdge::isMarked(e))
            nodeEdgeStack.push(e);
        e = e->oNext();
    } while (e != node);
}


/* private */
void
LineDissolver::buildLines()
{
    while (! nodeEdgeStack.empty()) {
        HalfEdge* e = nodeEdgeStack.top();
        nodeEdgeStack.pop();
        if (MarkHalfEdge::isMarked(e))
            continue;
        buildLine(e);
    }
}


/* private */
void
LineDissolver::updateRingStartEdge(DissolveHalfEdge* e)
{
    if (! e->isStart()) {
        e = static_cast<DissolveHalfEdge*>(e->sym());
        if (! e->isStart()) return;
    }
    // here e is known to be a start edge
    if (ringStartEdge == nullptr) {
        ringStartEdge = e;
        return;
    }
    if (e->orig().compareTo(ringStartEdge->orig()) < 0) {
        ringStartEdge = e;
    }
}


/* private */
void
LineDissolver::buildLine(HalfEdge* eStart)
{
    std::unique_ptr<CoordinateSequence> line(new CoordinateSequence(0, 4));
    DissolveHalfEdge* e = static_cast<DissolveHalfEdge*>(eStart);
    ringStartEdge = nullptr;

    MarkHalfEdge::markBoth(e);
    line->add(e->orig(), false);

    // scan along the path until a node is found (if one exists)
    while (e->sym()->degree() == 2) {
        updateRingStartEdge(e);
        DissolveHalfEdge* eNext = static_cast<DissolveHalfEdge*>(e->next());
        // check if edges form a ring - if so, we're done
        if (eNext == eStart)  {
            buildRing(ringStartEdge);
            return;
        }

        // add point to line, and move to next edge
        line->add(eNext->orig(), false);

        e = eNext;
        MarkHalfEdge::markBoth(e);
    }

    // add final node
    line->add(e->dest(), false);

    // queue up the final node edges
    stackEdges(e->sym());
    // store the scanned line
    addLine(line);
}


/* private */
void
LineDissolver::buildRing(HalfEdge* eStartRing)
{
    std::unique_ptr<CoordinateSequence> line(new CoordinateSequence(0, 4));
    HalfEdge* e = eStartRing;

    // add first node
    line->add(e->orig(), false);

    // scan along the path until a node is found (if one exists)
    while (e->sym()->degree() == 2) {
        HalfEdge* eNext = e->next();
        // check if edges form a ring - if so, we're done
        if (eNext == eStartRing)
            break;

        // add point to line, and move to next edge
        line->add(eNext->orig(), false);
        e = eNext;
    }
    // add final node
    line->add(e->dest(), false);

    // store the scanned line
    addLine(line);
}


/* private */
void
LineDissolver::addLine(std::unique_ptr<CoordinateSequence>& cs)
{
    auto ls = factory->createLineString(std::move(cs));
    lines.emplace_back(ls.release());
}


} // namespace geos.dissolve
} // namespace geos


