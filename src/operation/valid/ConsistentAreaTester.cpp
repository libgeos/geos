/**********************************************************************
 * $Id: ConsistentAreaTester.cpp 2547 2009-06-05 13:41:50Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
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
 * Last port: operation/valid/ConsistentAreaTester.java rev. 1.14 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/valid/ConsistentAreaTester.h> 
#include <geos/algorithm/LineIntersector.h>
#include <geos/geomgraph/GeometryGraph.h> 
#include <geos/geomgraph/EdgeEnd.h> 
#include <geos/geomgraph/EdgeEndStar.h> 
#include <geos/geomgraph/Edge.h> 
#include <geos/geomgraph/index/SegmentIntersector.h> 
#include <geos/geom/Coordinate.h> 
#include <geos/operation/relate/RelateNodeGraph.h> 
#include <geos/operation/relate/RelateNode.h> 
#include <geos/operation/relate/EdgeEndBundle.h> 

#include <memory> // auto_ptr
#include <cassert> 

using namespace std;
using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

ConsistentAreaTester::ConsistentAreaTester(GeometryGraph *newGeomGraph)
	:
	li(),
	geomGraph(newGeomGraph),
	nodeGraph(),
	invalidPoint()
{
}

ConsistentAreaTester::~ConsistentAreaTester()
{
}

Coordinate&
ConsistentAreaTester::getInvalidPoint()
{
	return invalidPoint;
}

bool
ConsistentAreaTester::isNodeConsistentArea()
{
	using geomgraph::index::SegmentIntersector;

	/**
	 * To fully check validity, it is necessary to
	 * compute ALL intersections, including self-intersections within a single edge.
	 */
	auto_ptr<SegmentIntersector> intersector(geomGraph->computeSelfNodes(&li, true));
	if (intersector->hasProperIntersection()) {
		invalidPoint=intersector->getProperIntersectionPoint();
		return false;
	}
	nodeGraph.build(geomGraph);
	return isNodeEdgeAreaLabelsConsistent();
}

/*private*/
bool
ConsistentAreaTester::isNodeEdgeAreaLabelsConsistent()
{
	assert(geomGraph);

	map<Coordinate*,Node*,CoordinateLessThen>& nMap=nodeGraph.getNodeMap();
	map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin();nodeIt!=nMap.end();nodeIt++) {
		relate::RelateNode *node=static_cast<relate::RelateNode*>(nodeIt->second);
		if (!node->getEdges()->isAreaLabelsConsistent(*geomGraph)) {
			invalidPoint=node->getCoordinate();
			return false;
		}
	}
	return true;
}

/*public*/
bool
ConsistentAreaTester::hasDuplicateRings()
{
	map<Coordinate*,Node*,CoordinateLessThen>& nMap=nodeGraph.getNodeMap();
	map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin(); nodeIt!=nMap.end(); ++nodeIt)
	{
		assert(dynamic_cast<relate::RelateNode*>(nodeIt->second));
		relate::RelateNode *node=static_cast<relate::RelateNode*>(nodeIt->second);
		EdgeEndStar *ees=node->getEdges();
		EdgeEndStar::iterator endIt=ees->end();
		for(EdgeEndStar::iterator it=ees->begin(); it!=endIt; ++it)
		{
			assert(dynamic_cast<relate::EdgeEndBundle*>(*it));
			relate::EdgeEndBundle *eeb=static_cast<relate::EdgeEndBundle*>(*it);
			if (eeb->getEdgeEnds()->size()>1) {
				invalidPoint=eeb->getEdge()->getCoordinate(0);
				return true;
			}
		}
	}
	return false;
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/03/27 10:37:59  strk
 * Reduced heap allocations and probability of error by making LineIntersector
 * and RelateNodeGraph part of ConsistentAreaTester class .
 *
 * Revision 1.16  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 * Revision 1.15  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.14  2006/03/17 16:48:55  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 * Revision 1.13  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

