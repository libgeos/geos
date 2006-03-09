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

#include <geos/opOverlay.h>
//#include <geos/geomgraph/EdgeRing.h>
//#include <geos/geomgraph/DirectedEdge.h>
//#include <geos/geomgraphindex.h>

using namespace std;
using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geomgraph::index;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

// CGAlgorithms obsoleted
MaximalEdgeRing::MaximalEdgeRing(DirectedEdge *start,
		const GeometryFactory *geometryFactory)
	:
	EdgeRing(start, geometryFactory)
{
	computePoints(start);
	computeRing();
}

MaximalEdgeRing::~MaximalEdgeRing()
{
}

DirectedEdge*
MaximalEdgeRing::getNext(DirectedEdge *de)
{
	return de->getNext();
}

void
MaximalEdgeRing::setEdgeRing(DirectedEdge *de,EdgeRing *er)
{
	de->setEdgeRing(er);
}

/**
 * For all nodes in this EdgeRing,
 * link the DirectedEdges at the node to form minimalEdgeRings
 */
void
MaximalEdgeRing::linkDirectedEdgesForMinimalEdgeRings()
{
	DirectedEdge* de=startDe;
	do {
		Node* node=de->getNode();
		((DirectedEdgeStar*) node->getEdges())->linkMinimalDirectedEdges(this);
		de=de->getNext();
	} while (de!=startDe);
}

/*public*/
vector<MinimalEdgeRing*>*
MaximalEdgeRing::buildMinimalRings()
{
	vector<MinimalEdgeRing*> *minEdgeRings=new vector<MinimalEdgeRing*>;
	buildMinimalRings(*minEdgeRings);
	return minEdgeRings;
}

/*public*/
void
MaximalEdgeRing::buildMinimalRings(vector<MinimalEdgeRing*>& minEdgeRings)
{
	DirectedEdge *de=startDe;
	do {
		if(de->getMinEdgeRing()==NULL) {
			MinimalEdgeRing *minEr=new MinimalEdgeRing(de,geometryFactory);
			minEdgeRings.push_back(minEr);
		}
		de=de->getNext();
	} while(de!=startDe);
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/03/09 18:18:39  strk
 * Added memory-friendly MaximalEdgeRing::buildMinimalRings() implementation.
 * Applied patch to IsValid operation from JTS-1.7.1
 *
 * Revision 1.15  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.14  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.13  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.12  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.11  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/13 17:47:49  strk
 * delete statement removed
 *
 **********************************************************************/

