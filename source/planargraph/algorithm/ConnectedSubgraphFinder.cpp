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
//namespace planargraph {

void
ConnectedSubgraphFinder::getConnectedSubgraphs(vector<planarSubgraph *>& subgraphs)
{
	planarGraphComponent::setVisited(graph.nodeBegin(),
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


//} // namespace planargraph 
} // namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/02/05 17:14:43  strk
 * - New ConnectedSubgraphFinder class.
 * - More iterators returning methods, inlining and cleanups
 *   in planargraph.
 *
 *
 **********************************************************************/
