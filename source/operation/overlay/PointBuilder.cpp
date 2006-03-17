/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <vector>
#include <cassert>

#include <geos/operation/overlay/PointBuilder.h>
#include <geos/operation/overlay/OverlayOp.h>

#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/EdgeEndStar.h>
#include <geos/geomgraph/Label.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay


/*
 * @return a list of the Points in the result of the specified
 * overlay operation
 */
vector<Point*>*
PointBuilder::build(int opCode)
{
	extractNonCoveredResultNodes(opCode);
	return resultPointList;
}

/*
 * Determines nodes which are in the result, and creates Point for them.
 *
 * This method determines nodes which are candidates for the result via their
 * labelling and their graph topology.
 *
 * @param opCode the overlay operation
 */
void
PointBuilder::extractNonCoveredResultNodes(int opCode)
{
	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap =
		op->getGraph().getNodeMap()->nodeMap;
	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (; it!=nodeMap.end(); ++it)
	{
		Node *n=it->second;

		// filter out nodes which are known to be in the result
		if (n->isInResult()) continue;

		// if an incident edge is in the result, then
		// the node coordinate is included already
		if (n->isIncidentEdgeInResult()) continue;

		if ( n->getEdges()->getDegree() == 0 ||
			opCode == OverlayOp::INTERSECTION )
		{

			/**
			 * For nodes on edges, only INTERSECTION can result 
			 * in edge nodes being included even
			 * if none of their incident edges are included
			 */
			Label *label=n->getLabel();
			if (OverlayOp::isResultOfOp(label,opCode)) 
				filterCoveredNodeToPoint(n);
		}
	}
}

void
PointBuilder::filterCoveredNodeToPoint(const Node *n)
{
	const Coordinate& coord=n->getCoordinate();
	if(!op->isCoveredByLA(coord)) {
		Point *pt=geometryFactory->createPoint(coord);
		resultPointList->push_back(pt);
	}
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.19  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.18  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.17  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.16  2005/06/28 01:07:02  strk
 * improved extraction of result points in overlay op
 *
 * Revision 1.15  2005/06/25 10:20:39  strk
 * OverlayOp speedup (JTS port)
 *
 * Revision 1.14  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 **********************************************************************/

