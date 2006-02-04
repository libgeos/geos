/**********************************************************************
 * $Id$
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

#include <geos/planargraph.h>

namespace geos {
//namespace planargraph {

pair<planarEdge::ConstSet::iterator, bool>
planarSubgraph::add(planarEdge *e)
{
	pair<planarEdge::ConstSet::iterator,bool> p = edges.insert(e);
	if (!p.second) return p;

	dirEdges.push_back(e->getDirEdge(0));
	dirEdges.push_back(e->getDirEdge(1));
	nodeMap.add(e->getDirEdge(0)->getFromNode());
	nodeMap.add(e->getDirEdge(1)->getFromNode());

	return p;
}

//} // namespace planargraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/02/04 00:54:57  strk
 * - Doxygen dox updated
 * - LineStringLT struct moved from geomgraph.h to geom.h
 * - New planarSubgraph class
 * - Fixed ruby Makefiles to avoid running tests when disabled
 * - Renamed TESTS variable to XMLTESTS to not confuse 'make check' rule
 *
 *
 **********************************************************************/

