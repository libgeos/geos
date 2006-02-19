/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/opValid.h>
#include <geos/opRelate.h>
#include <geos/opOverlay.h>
#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>

using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geomgraph::index;
using namespace geos::operation::overlay;
using namespace geos::operation::relate;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

ConsistentAreaTester::ConsistentAreaTester(GeometryGraph *newGeomGraph){
	geomGraph=newGeomGraph;
	nodeGraph=new RelateNodeGraph();
	li=new LineIntersector();
}

ConsistentAreaTester::~ConsistentAreaTester(){
	delete nodeGraph;
	delete li;
}

Coordinate& ConsistentAreaTester::getInvalidPoint(){
	return invalidPoint;
}

bool ConsistentAreaTester::isNodeConsistentArea() {
    /**
     * To fully check validity, it is necessary to
     * compute ALL intersections, including self-intersections within a single edge.
     */
	auto_ptr<SegmentIntersector> intersector(geomGraph->computeSelfNodes(li,true));
	if (intersector->hasProperIntersection()) {
		invalidPoint=intersector->getProperIntersectionPoint();
		return false;
	}
	nodeGraph->build(geomGraph);
	return isNodeEdgeAreaLabelsConsistent();
}

/**
* Check all nodes to see if their labels are consistent.
* If any are not, return false
*/
bool ConsistentAreaTester::isNodeEdgeAreaLabelsConsistent() {
	map<Coordinate*,Node*,CoordinateLessThen> &nMap=nodeGraph->getNodeMap();
	map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin();nodeIt!=nMap.end();nodeIt++) {
		RelateNode *node=(RelateNode*) nodeIt->second;
		if (!node->getEdges()->isAreaLabelsConsistent()) {
			Coordinate *c=new Coordinate(node->getCoordinate());
			invalidPoint=*c;
			delete c;
			return false;
		}
	}
	return true;
}

/**
* Checks for two duplicate rings in an area.
* Duplicate rings are rings that are topologically equal
* (that is, which have the same sequence of points up to point order).
* If the area is topologically consistent (determined by calling the
* <code>isNodeConsistentArea</code>,
* duplicate rings can be found by checking for EdgeBundles which contain
* more than one EdgeEnd.
* (This is because topologically consistent areas cannot have two rings sharing
* the same line segment, unless the rings are equal).
* The start point of one of the equal rings will be placed in
* invalidPoint.
*
* @return true if this area Geometry is topologically consistent but has two duplicate rings
*/
bool ConsistentAreaTester::hasDuplicateRings() {
	map<Coordinate*,Node*,CoordinateLessThen> &nMap=nodeGraph->getNodeMap();
	map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin(); nodeIt!=nMap.end(); ++nodeIt)
	{
		RelateNode *node=(RelateNode*) nodeIt->second;
		EdgeEndStar *ees=node->getEdges();
		EdgeEndStar::iterator endIt=ees->end();
		for(EdgeEndStar::iterator it=ees->begin(); it!=endIt; ++it)
		{
			EdgeEndBundle *eeb=(EdgeEndBundle*) (*it);
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
 * Revision 1.11  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.10  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.9  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.8  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.7  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

