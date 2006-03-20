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

#include <cassert>
#include <vector>

#include <geos/opRelate.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Envelope.h>
#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeIntersectionList.h>
#include <geos/geomgraph/EdgeIntersection.h>

#include <vector>
#include <cassert>

using namespace geos::geom;
using namespace geos::geomgraph;
using namespace geos::geomgraph::index;
using namespace geos::algorithm;

namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

RelateComputer::RelateComputer(std::vector<GeometryGraph*> *newArg):
	arg(newArg),
	nodes(RelateNodeFactory::instance()),
	im(new IntersectionMatrix())
{
}

RelateComputer::~RelateComputer()
{
}

IntersectionMatrix*
RelateComputer::computeIM()
{
	// since Geometries are finite and embedded in a 2-D space, the EE element must always be 2
	im->set(Location::EXTERIOR,Location::EXTERIOR,2);
	// if the Geometries don't overlap there is nothing to do
	const Envelope *e1=(*arg)[0]->getGeometry()->getEnvelopeInternal();
	const Envelope *e2=(*arg)[1]->getGeometry()->getEnvelopeInternal();
	if (!e1->intersects(e2)) {
		computeDisjointIM(im);
		//delete e1;
		//delete e2;
		return im;
	}

	SegmentIntersector *si1=(*arg)[0]->computeSelfNodes(&li,false);
	SegmentIntersector *si2=(*arg)[1]->computeSelfNodes(&li,false);

	// compute intersections between edges of the two input geometries
	SegmentIntersector *intersector=(*arg)[0]->computeEdgeIntersections((*arg)[1], &li,false);
	computeIntersectionNodes(0);
	computeIntersectionNodes(1);

	/*
	 * Copy the labelling for the nodes in the parent Geometries.
	 * These override any labels determined by intersections
	 * between the geometries.
	 */
	copyNodesAndLabels(0);
	copyNodesAndLabels(1);

	/*
	 * complete the labelling for any nodes which only have a
	 * label for a single geometry
	 */
	//Debug.addWatch(nodes.find(new Coordinate(110, 200)));
	//Debug.printWatch();
	labelIsolatedNodes();
	//Debug.printWatch();

	/*
	 * If a proper intersection was found, we can set a lower bound
	 * on the IM.
	 */
	computeProperIntersectionIM(intersector, im);

	/*
	 * Now process improper intersections
	 * (eg where one or other of the geometrys has a vertex at the
	 * intersection point)
	 * We need to compute the edge graph at all nodes to determine
	 * the IM.
	 */
	// build EdgeEnds for all intersections
	EdgeEndBuilder eeBuilder;
	std::vector<EdgeEnd*> *ee0=eeBuilder.computeEdgeEnds((*arg)[0]->getEdges());
	insertEdgeEnds(ee0);
	std::vector<EdgeEnd*> *ee1=eeBuilder.computeEdgeEnds((*arg)[1]->getEdges());
	insertEdgeEnds(ee1);
	//Debug.println("==== NodeList ===");
	//Debug.print(nodes);
	labelNodeEdges();

	/**
	 * Compute the labeling for isolated components.
	 * Isolated components are components that do not touch any
	 * other components in the graph.
	 * They can be identified by the fact that they will
	 * contain labels containing ONLY a single element, the one for
	 * their parent geometry.
	 * We only need to check components contained in the input graphs,
	 * since isolated components will not have been replaced by new
	 * components formed by intersections.
	 */
	//debugPrintln("Graph A isolated edges - ");
	labelIsolatedEdges(0,1);
	//debugPrintln("Graph B isolated edges - ");
	labelIsolatedEdges(1,0);
	// update the IM from all components
	updateIM(im);
	delete si1;
	delete si2;
	delete intersector;
	delete ee0;
	delete ee1;
	return im;
}

void
RelateComputer::insertEdgeEnds(std::vector<EdgeEnd*> *ee)
{
	for(std::vector<EdgeEnd*>::iterator i=ee->begin();i<ee->end();i++) {
		EdgeEnd *e=*i;
		nodes.add(e);
	}
}

void
RelateComputer::computeProperIntersectionIM(SegmentIntersector *intersector,IntersectionMatrix *imX)
{
	// If a proper intersection is found, we can set a lower bound on the IM.
	int dimA=(*arg)[0]->getGeometry()->getDimension();
	int dimB=(*arg)[1]->getGeometry()->getDimension();
	bool hasProper=intersector->hasProperIntersection();
	bool hasProperInterior=intersector->hasProperInteriorIntersection();
	// For Geometry's of dim 0 there can never be proper intersections.
	/**
	* If edge segments of Areas properly intersect, the areas must properly overlap.
	*/
	if (dimA==2 && dimB==2) {
		if (hasProper) imX->setAtLeast("212101212");
	}
	/**
	* If an Line segment properly intersects an edge segment of an Area,
	* it follows that the Interior of the Line intersects the Boundary of the Area.
	* If the intersection is a proper <i>interior</i> intersection, then
	* there is an Interior-Interior intersection too.
	* Note that it does not follow that the Interior of the Line intersects the Exterior
	* of the Area, since there may be another Area component which contains the rest of the Line.
	*/
	 else if (dimA==2 && dimB==1) {
		if (hasProper) imX->setAtLeast("FFF0FFFF2");
		if (hasProperInterior) imX->setAtLeast("1FFFFF1FF");
	} else if (dimA==1 && dimB==2) {
		if (hasProper) imX->setAtLeast("F0FFFFFF2");
		if (hasProperInterior) imX->setAtLeast("1F1FFFFFF");
	}
	/* If edges of LineStrings properly intersect *in an interior point*, all
	we can deduce is that
	the interiors intersect.  (We can NOT deduce that the exteriors intersect,
	since some other segments in the geometries might cover the points in the
	neighbourhood of the intersection.)
	It is important that the point be known to be an interior point of
	both Geometries, since it is possible in a self-intersecting geometry to
	have a proper intersection on one segment that is also a boundary point of another segment.
	*/
	else if (dimA==1 && dimB==1) {
		if (hasProperInterior) imX->setAtLeast("0FFFFFFFF");
	}
}

/**
 * Copy all nodes from an arg geometry into this graph.
 * The node label in the arg geometry overrides any previously computed
 * label for that argIndex.
 * (E.g. a node may be an intersection node with
 * a computed label of BOUNDARY,
 * but in the original arg Geometry it is actually
 * in the interior due to the Boundary Determination Rule)
 */
void
RelateComputer::copyNodesAndLabels(int argIndex)
{
	const NodeMap* nm = (*arg)[argIndex]->getNodeMap();
	NodeMap::const_iterator nodeIt=nm->begin(), nodeEnd=nm->end();

	for( ; nodeIt!=nodeEnd; nodeIt++)
	{
		Node *graphNode=nodeIt->second;
		Node *newNode=nodes.addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,
				graphNode->getLabel()->getLocation(argIndex));
		//node.print(System.out);
	}
}


/**
 * Insert nodes for all intersections on the edges of a Geometry.
 * Label the created nodes the same as the edge label if they do not
 * already have a label.
 * This allows nodes created by either self-intersections or
 * mutual intersections to be labelled.
 * Endpoint nodes will already be labelled from when they were inserted.
 */
void
RelateComputer::computeIntersectionNodes(int argIndex)
{
	std::vector<Edge*> *edges=(*arg)[argIndex]->getEdges();
	for(std::vector<Edge*>::iterator i=edges->begin();i<edges->end();i++)
	{
		Edge *e=*i;
		int eLoc=e->getLabel()->getLocation(argIndex);
		EdgeIntersectionList &eiL=e->getEdgeIntersectionList();
		EdgeIntersectionList::iterator it=eiL.begin();
		EdgeIntersectionList::iterator end=eiL.end();
		for( ; it!=end; ++it)
		{
			EdgeIntersection *ei=*it;
			assert(dynamic_cast<RelateNode*>(nodes.addNode(ei->coord)));
			RelateNode *n=static_cast<RelateNode*>(nodes.addNode(ei->coord));
			if (eLoc==Location::BOUNDARY)
			{
				n->setLabelBoundary(argIndex);
			}
			else
			{
				if (n->getLabel()->isNull(argIndex))
				  n->setLabel(argIndex,Location::INTERIOR);
			}
		}
	}
}

/*
 * For all intersections on the edges of a Geometry,
 * label the corresponding node IF it doesn't already have a label.
 * This allows nodes created by either self-intersections or
 * mutual intersections to be labelled.
 * Endpoint nodes will already be labelled from when they were inserted.
 */
void
RelateComputer::labelIntersectionNodes(int argIndex)
{
	std::vector<Edge*> *edges=(*arg)[argIndex]->getEdges();
	for(std::vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int eLoc=e->getLabel()->getLocation(argIndex);
		EdgeIntersectionList &eiL=e->getEdgeIntersectionList();
		EdgeIntersectionList::iterator eiIt=eiL.begin();
		EdgeIntersectionList::iterator eiEnd=eiL.end();
		
		for( ; eiIt!=eiEnd; ++eiIt)
		{
			EdgeIntersection *ei=*eiIt;
			RelateNode *n=(RelateNode*) nodes.find(ei->coord);
			if (n->getLabel()->isNull(argIndex)) {
				if (eLoc==Location::BOUNDARY)
				  n->setLabelBoundary(argIndex);
				else
				  n->setLabel(argIndex,Location::INTERIOR);
			}
		}
	}
}

/**
 * If the Geometries are disjoint, we need to enter their dimension and
 * boundary dimension in the Ext rows in the IM
 */
void
RelateComputer::computeDisjointIM(IntersectionMatrix *imX)
{
	const Geometry *ga=(*arg)[0]->getGeometry();
	if (!ga->isEmpty()) {
		imX->set(Location::INTERIOR,Location::EXTERIOR,ga->getDimension());
		imX->set(Location::BOUNDARY,Location::EXTERIOR,ga->getBoundaryDimension());
	}
	const Geometry *gb=(*arg)[1]->getGeometry();
	if (!gb->isEmpty()) {
		imX->set(Location::EXTERIOR,Location::INTERIOR,gb->getDimension());
		imX->set(Location::EXTERIOR,Location::BOUNDARY,gb->getBoundaryDimension());
	}
}


void
RelateComputer::labelNodeEdges()
{
	std::map<Coordinate*,Node*,CoordinateLessThen> &nMap=nodes.nodeMap;
	std::map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin();nodeIt!=nMap.end();nodeIt++)
	{
		RelateNode *node=(RelateNode*) nodeIt->second;
		node->getEdges()->computeLabelling(arg);
		//Debug.print(node.getEdges());
		//node.print(System.out);
	}
}

/**
 * update the IM with the sum of the IMs for each component
 */
void
RelateComputer::updateIM(IntersectionMatrix *imX)
{
	//Debug.println(im);
	std::vector<Edge *>::iterator ei=isolatedEdges.begin();
	for ( ; ei<isolatedEdges.end(); ++ei)
	{
		Edge *e=*ei;
		e->GraphComponent::updateIM(imX);
		//Debug.println(im);
	}
	std::map<Coordinate*,Node*,CoordinateLessThen> &nMap=nodes.nodeMap;
	std::map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin();nodeIt!=nMap.end();nodeIt++) {
		RelateNode *node=(RelateNode*) nodeIt->second;
		node->updateIM(imX);
		//Debug.println(im);
		node->updateIMFromEdges(imX);
		//Debug.println(im);
		//node.print(System.out);
	}
}

/**
* Processes isolated edges by computing their labelling and adding them
* to the isolated edges list.
* Isolated edges are guaranteed not to touch the boundary of the target (since if they
* did, they would have caused an intersection to be computed and hence would
* not be isolated)
*/
void RelateComputer::labelIsolatedEdges(int thisIndex,int targetIndex) {
	std::vector<Edge*> *edges=(*arg)[thisIndex]->getEdges();
	for(std::vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		if (e->isIsolated()) {
			labelIsolatedEdge(e,targetIndex,(*arg)[targetIndex]->getGeometry());
			isolatedEdges.push_back(e);
		}
	}
}

/**
 * Label an isolated edge of a graph with its relationship to the target geometry.
 * If the target has dim 2 or 1, the edge can either be in the interior or the exterior.
 * If the target has dim 0, the edge must be in the exterior
 */
void
RelateComputer::labelIsolatedEdge(Edge *e, int targetIndex, const Geometry *target)
{
	// this won't work for GeometryCollections with both dim 2 and 1 geoms
	if (target->getDimension()>0) {
		// since edge is not in boundary, may not need the full generality of PointLocator?
		// Possibly should use ptInArea locator instead?  We probably know here
		// that the edge does not touch the bdy of the target Geometry
		int loc=ptLocator.locate(e->getCoordinate(), target);
		e->getLabel()->setAllLocations(targetIndex,loc);
	} else {
		e->getLabel()->setAllLocations(targetIndex,Location::EXTERIOR);
	}
	//System.out.println(e.getLabel());
}

/**
 * Isolated nodes are nodes whose labels are incomplete
 * (e.g. the location for one Geometry is null).
 * This is the case because nodes in one graph which don't intersect
 * nodes in the other are not completely labelled by the initial process
 * of adding nodes to the nodeList.
 * To complete the labelling we need to check for nodes that lie in the
 * interior of edges, and in the interior of areas.
 */
void
RelateComputer::labelIsolatedNodes()
{
	NodeMap::iterator nodeIt=nodes.begin(), nodeEnd=nodes.end();
	for( ; nodeIt!=nodeEnd; nodeIt++)
	{
		Node *n=nodeIt->second;
		Label *label=n->getLabel();
		// isolated nodes should always have at least one geometry in their label
		assert(label->getGeometryCount()>0); // node with empty label found
		if (n->isIsolated()) {
			if (label->isNull(0))
				labelIsolatedNode(n,0);
			else
				labelIsolatedNode(n,1);
		}
	}
}

/**
 * Label an isolated node with its relationship to the target geometry.
 */
void
RelateComputer::labelIsolatedNode(Node *n,int targetIndex)
{
	int loc=ptLocator.locate(n->getCoordinate(),
			(*arg)[targetIndex]->getGeometry());
	n->getLabel()->setAllLocations(targetIndex,loc);
	//debugPrintln(n.getLabel());
}

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.31  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.30  2006/03/17 16:48:55  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 * Revision 1.29  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.28  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.27  2006/02/23 11:54:21  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.26  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.25  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.24  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.23  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.22  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.21  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.20  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.19  2004/08/04 08:26:02  strk
 * comments lift, stack allocation reduced
 *
 * Revision 1.18  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.17  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.16  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.15  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

