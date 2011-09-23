/**********************************************************************
 * $Id: Subgraph.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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

using namespace std;

namespace geos {
namespace planargraph {

pair<Edge::NonConstSet::iterator, bool>
Subgraph::add(Edge *e)
{
	pair<Edge::NonConstSet::iterator,bool> p = edges.insert(e);
	if (!p.second) return p;

	dirEdges.push_back(e->getDirEdge(0));
	dirEdges.push_back(e->getDirEdge(1));
	nodeMap.add(e->getDirEdge(0)->getFromNode());
	nodeMap.add(e->getDirEdge(1)->getFromNode());

	return p;
}

} // namespace planargraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 **********************************************************************/

