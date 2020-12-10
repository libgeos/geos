/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 * Last port: planargraph/Subgraph.java rev. 1.3 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/planargraph/Subgraph.h>
#include <geos/planargraph/Edge.h>
#include <geos/planargraph/DirectedEdge.h>

#include <utility>
#include <vector>



namespace geos {
namespace planargraph {

std::pair<Edge::NonConstSet::iterator, bool>
Subgraph::add(Edge* e)
{
    std::pair<Edge::NonConstSet::iterator, bool> p = edges.insert(e);
    if(!p.second) {
        return p;
    }

    dirEdges.push_back(e->getDirEdge(0));
    dirEdges.push_back(e->getDirEdge(1));
    nodeMap.add(e->getDirEdge(0)->getFromNode());
    nodeMap.add(e->getDirEdge(1)->getFromNode());

    return p;
}

} // namespace planargraph
} // namespace geos

