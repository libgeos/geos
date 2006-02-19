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
 **********************************************************************/


#include <geos/planargraph.h>

namespace geos {
namespace planargraph {
namespace algorithm {

void
ConnectedSubgraphFinder::getConnectedSubgraphs(vector<planarSubgraph *>& subgraphs)
{
	planarGraphComponent::setVisitedMap(graph.nodeBegin(),
			graph.nodeEnd(), false);

	for (planarPlanarGraph::EdgeIterator
			it=graph.edgeBegin(),
			itEnd=graph.edgeEnd();
			it!=itEnd; ++it)
	{
		planarEdge *e = *it;
		planarNode *node = e->getDirEdge(0)->getFromNode();
		if (! node->isVisited()) {
			subgraphs.push_back(findSubgraph(node));
		}
	}
 
}

/*private*/
planarSubgraph* 
ConnectedSubgraphFinder::findSubgraph(planarNode* node)
{
	planarSubgraph* subgraph = new planarSubgraph(graph);
	addReachable(node, subgraph);
	return subgraph;
}

/*private*/
void
ConnectedSubgraphFinder::addReachable(planarNode* startNode,
		planarSubgraph* subgraph)
{
	stack<planarNode *> nodeStack;
	nodeStack.push(startNode);
	while ( !nodeStack.empty() )
	{
		planarNode* node = nodeStack.top();
		nodeStack.pop();
		addEdges(node, nodeStack, subgraph);
	}
}

/*private*/
void
ConnectedSubgraphFinder::addEdges(planarNode* node,
		stack<planarNode *>& nodeStack, planarSubgraph* subgraph)
{
	node->setVisited(true);
	planarDirectedEdgeStar *des=node->getOutEdges();
	for (planarDirectedEdge::Vect::iterator i=des->begin(), iEnd=des->end();
			i!=iEnd; ++i)
	{
		planarDirectedEdge *de=*i;
		subgraph->add(de->getEdge());
		planarNode *toNode = de->getToNode();
		if ( ! toNode->isVisited() ) nodeStack.push(toNode);
	}
}


} // namespace geos.planargraph.algorithm
} // namespace geos.planargraph 
} // namespace geos 

/**********************************************************************
 * $Log$
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
 * Revision 1.1  2006/02/05 17:14:43  strk
 * - New ConnectedSubgraphFinder class.
 * - More iterators returning methods, inlining and cleanups
 *   in planargraph.
 *
 *
 **********************************************************************/
