/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geomgraph/EdgeIntersectionList.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/geomgraph/EdgeIntersectionList.h>
#include <geos/geomgraph/EdgeIntersection.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/Label.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/util.h>

#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <utility> // std::pair

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG || GEOS_DEBUG_INTERSECT
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

EdgeIntersectionList::EdgeIntersectionList(const Edge* newEdge):
    sorted(false),
    edge(newEdge)
{
}

void
EdgeIntersectionList::add(const Coordinate& coord,
                          std::size_t segmentIndex, double dist)
{
    if (nodeMap.empty()) {
        nodeMap.emplace_back(coord, segmentIndex, dist);
        return;
    }

    if (nodeMap.back().segmentIndex == segmentIndex && nodeMap.back().dist == dist) {
        return; // don't add duplicate
    }

    nodeMap.emplace_back(coord, segmentIndex, dist);
    // Did our addition break the sortedness of the vector? If so, we'll have to
    // sort before we iterate over the intersections again.
    if (sorted && (!(nodeMap[nodeMap.size() - 2] < nodeMap[nodeMap.size() - 1]))) {
        sorted = false;
    }
}

bool
EdgeIntersectionList::isEmpty() const
{
    return nodeMap.empty();
}

bool
EdgeIntersectionList::isIntersection(const Coordinate& pt) const
{
    for(const EdgeIntersection& ei : nodeMap) {
        if(ei.coord == pt) {
            return true;
        }
    }
    return false;
}

void
EdgeIntersectionList::addEndpoints()
{
    auto maxSegIndex = edge->getNumPoints() - 1;
    add(edge->pts->getAt(0), 0, 0.0);
    add(edge->pts->getAt(maxSegIndex), maxSegIndex, 0.0);
}

void
EdgeIntersectionList::addSplitEdges(std::vector<Edge*>* edgeList)
{
    // ensure that the list has entries for the first and last point
    // of the edge
    addEndpoints();

    EdgeIntersectionList::const_iterator it = begin();

    // there should always be at least two entries in the list
    const EdgeIntersection* eiPrev = &*it;
    ++it;

    while(it != end()) {
        const EdgeIntersection* ei = &*it;
        Edge* newEdge = createSplitEdge(eiPrev, ei);
        edgeList->push_back(newEdge);
        eiPrev = ei;
        ++it;
    }
}

Edge*
EdgeIntersectionList::createSplitEdge(const EdgeIntersection* ei0,
                                      const EdgeIntersection* ei1)
{
#if GEOS_DEBUG
    std::cerr << "[" << this << "] EdgeIntersectionList::createSplitEdge()" << std::endl;
#endif // GEOS_DEBUG
    auto npts = 2ul + ei1->segmentIndex - ei0->segmentIndex;

    const Coordinate& lastSegStartPt = edge->pts->getAt(ei1->segmentIndex);

    // if the last intersection point is not equal to the its segment
    // start pt, add it to the points list as well.
    // (This check is needed because the distance metric is not totally
    // reliable!). The check for point equality is 2D only - Z values
    // are ignored
    bool useIntPt1 = ei1->dist > 0.0 || !ei1->coord.equals2D(lastSegStartPt);

    if(!useIntPt1) {
        --npts;
    }

#if GEOS_DEBUG
    std::cerr << "    npts:" << npts << std::endl;
#endif // GEOS_DEBUG

    auto vc = detail::make_unique<CoordinateSequence>();
    vc->reserve(npts);

    vc->add(ei0->coord);
    for(auto i = ei0->segmentIndex + 1; i <= ei1->segmentIndex; ++i) {
        if(! useIntPt1 && ei1->segmentIndex == i) {
            vc->add(ei1->coord);
        }
        else {
            vc->add(edge->pts->getAt(i));
        }
    }

    if(useIntPt1) {
        vc->add(ei1->coord);
    }

    return new Edge(vc.release(), edge->getLabel());
}

std::string
EdgeIntersectionList::print() const
{
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream&
operator<< (std::ostream& os, const EdgeIntersectionList& e)
{
    os << "Intersections:" << std::endl;
    for(const auto & ei : e) {
        os << ei << std::endl;
    }
    return os;
}

} // namespace geos.geomgraph
} // namespace

