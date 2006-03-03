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
#include <utility>

using namespace std;

namespace geos {
namespace planargraph {

pair<planarEdge::NonConstSet::iterator, bool>
planarSubgraph::add(planarEdge *e)
{
	pair<planarEdge::NonConstSet::iterator,bool> p = edges.insert(e);
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
 * Revision 1.4  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.3  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.2  2006/02/08 12:59:56  strk
 * - NEW Geometry::applyComponentFilter() templated method
 * - Changed Geometry::getGeometryN() to take unsigned int and getNumGeometries
 *   to return unsigned int.
 * - Changed planarNode::getDegree() to return unsigned int.
 * - Added Geometry::NonConstVect typedef
 * - NEW LineSequencer class
 * - Changed planarDirectedEdgeStar::outEdges from protected to private
 * - added static templated setVisitedMap to change Visited flag
 *   for all values in a map
 * - Added const versions of some planarDirectedEdgeStar methods.
 * - Added containers typedefs for planarDirectedEdgeStar
 *
 * Revision 1.1  2006/02/04 00:54:57  strk
 * - Doxygen dox updated
 * - LineStringLT struct moved from geomgraph.h to geom.h
 * - New planarSubgraph class
 * - Fixed ruby Makefiles to avoid running tests when disabled
 * - Renamed TESTS variable to XMLTESTS to not confuse 'make check' rule
 *
 *
 **********************************************************************/

