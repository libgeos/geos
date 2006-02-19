/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opLinemerge.h>
#include <geos/planargraph.h>
#include <geos/util.h>

using namespace geos::planargraph;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

LineMergeDirectedEdge::LineMergeDirectedEdge(planarNode *newFrom,
		planarNode *newTo, const Coordinate& newDirectionPt,
		bool nEdgeDirection):
	planarDirectedEdge(newFrom,newTo,newDirectionPt,nEdgeDirection)
{}

/**
 * Returns the directed edge that starts at this directed edge's end point,
 * or null if there are zero or multiple directed edges starting there.  
 * @return
 */
LineMergeDirectedEdge* LineMergeDirectedEdge::getNext()
{
	if (getToNode()->getDegree()!=2) {
		return NULL;
	}
	if (getToNode()->getOutEdges()->getEdges()[0]==getSym()) {
		return (LineMergeDirectedEdge*) getToNode()->getOutEdges()->getEdges()[1];
	}
	Assert::isTrue(getToNode()->getOutEdges()->getEdges()[1]==getSym());
	return (LineMergeDirectedEdge*) getToNode()->getOutEdges()->getEdges()[0];
}

} // namespace geos.operation.linemerge
} // namespace geos.operation
} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.3  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/

