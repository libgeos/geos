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

#include <geos/opRelate.h>
#include <geos/geomgraph.h>

using namespace geos::geomgraph;


namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

RelateNodeGraph::RelateNodeGraph()
{
	nodes=new NodeMap(RelateNodeFactory::instance());
}

RelateNodeGraph::~RelateNodeGraph() {
	delete nodes;
}

map<Coordinate*,Node*,CoordinateLessThen>&
RelateNodeGraph::getNodeMap()
{
	return nodes->nodeMap;
}

void
RelateNodeGraph::build(GeometryGraph *geomGraph)
{
	// compute nodes for intersections between previously noded edges
	computeIntersectionNodes(geomGraph,0);
	/**
	* Copy the labelling for the nodes in the parent Geometry.  These override
	* any labels determined by intersections.
	*/
	copyNodesAndLabels(geomGraph,0);
	/**
	* Build EdgeEnds for all intersections.
	*/
	EdgeEndBuilder *eeBuilder=new EdgeEndBuilder();
	vector<EdgeEnd*> *eeList=eeBuilder->computeEdgeEnds(geomGraph->getEdges());
	insertEdgeEnds(eeList);
	delete eeBuilder;
	delete eeList;
	//Debug.println("==== NodeList ===");
	//Debug.print(nodes);
}

/**
 * Insert nodes for all intersections on the edges of a Geometry.
 * Label the created nodes the same as the edge label if they do not
 * already have a label.
 * This allows nodes created by either self-intersections or
 * mutual intersections to be labelled.
 * Endpoint nodes will already be labelled from when they were inserted.
 * 
 * Precondition: edge intersections have been computed.
 */
void
RelateNodeGraph::computeIntersectionNodes(GeometryGraph *geomGraph,
	int argIndex)
{
	vector<Edge*> *edges=geomGraph->getEdges();
	vector<Edge*>::iterator edgeIt=edges->begin();
	for( ; edgeIt<edges->end(); ++edgeIt)
	{
		Edge *e=*edgeIt;
		int eLoc=e->getLabel()->getLocation(argIndex);
		EdgeIntersectionList &eiL=e->getEdgeIntersectionList();
		EdgeIntersectionList::iterator eiIt=eiL.begin();
		EdgeIntersectionList::iterator eiEnd=eiL.end();
		for( ; eiIt!=eiEnd; ++eiIt) {
			EdgeIntersection *ei=*eiIt;
			RelateNode *n=(RelateNode*) nodes->addNode(ei->coord);
			if (eLoc==Location::BOUNDARY)
				n->setLabelBoundary(argIndex);
			else {
				if (n->getLabel()->isNull(argIndex))
				  n->setLabel(argIndex,Location::INTERIOR);
			}
		}
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
RelateNodeGraph::copyNodesAndLabels(GeometryGraph *geomGraph,int argIndex)
{
	map<Coordinate*,Node*,CoordinateLessThen> &nMap=geomGraph->getNodeMap()->nodeMap;
	map<Coordinate*,Node*,CoordinateLessThen>::iterator nodeIt;
	for(nodeIt=nMap.begin();nodeIt!=nMap.end();nodeIt++) {
		Node *graphNode=nodeIt->second;
		Node *newNode=nodes->addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,graphNode->getLabel()->getLocation(argIndex));
		//node.print(System.out);
	}
}

void
RelateNodeGraph::insertEdgeEnds(vector<EdgeEnd*> *ee)
{
	for(vector<EdgeEnd*>::iterator i=ee->begin();i<ee->end();i++) {
		EdgeEnd *e=*i;
		nodes->add(e);
	}
}

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.15  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.14  2005/11/21 16:03:20  strk
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
 * Revision 1.13  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.12  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.11  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.10  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

