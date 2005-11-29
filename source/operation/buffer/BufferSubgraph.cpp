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

#include <geos/opBuffer.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

// Argument is unused
BufferSubgraph::BufferSubgraph(CGAlgorithms *cga):
	rightMostCoord(NULL),
	env(NULL)
{
}

BufferSubgraph::~BufferSubgraph()
{
	delete env;
}

/**
 * Creates the subgraph consisting of all edges reachable from this node.
 * Finds the edges in the graph and the rightmost coordinate.
 *
 * @param node a node to start the graph traversal from
 */
void
BufferSubgraph::create(Node *node)
{
	addReachable(node);
	finder.findEdge(&dirEdgeList);
	rightMostCoord=&(finder.getCoordinate());
}

/**
 * Adds all nodes and edges reachable from this node to the subgraph.
 * Uses an explicit stack to avoid a large depth of recursion.
 *
 * @param node a node known to be in the subgraph
 */
void
BufferSubgraph::addReachable(Node *startNode)
{
	vector<Node*> nodeStack;
	nodeStack.push_back(startNode);
	while (!nodeStack.empty()) {
		Node *node=nodeStack.back();
		nodeStack.pop_back();
		add(node, &nodeStack);
	}
}

/**
 * Adds the argument node and all its out edges to the subgraph
 * @param node the node to add
 * @param nodeStack the current set of nodes being traversed
 */
void
BufferSubgraph::add(Node *node, vector<Node*> *nodeStack)
{
	node->setVisited(true);
	nodes.push_back(node);
	EdgeEndStar *ees=node->getEdges();
	EdgeEndStar::iterator it=ees->begin();
	EdgeEndStar::iterator endIt=ees->end();
	for( ; it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) (*it);
		dirEdgeList.push_back(de);
		DirectedEdge *sym=de->getSym();
		Node *symNode=sym->getNode();
		/**
		 * NOTE: this is a depth-first traversal of the graph.
		 * This will cause a large depth of recursion.
		 * It might be better to do a breadth-first traversal.
		 */
		if (! symNode->isVisited()) nodeStack->push_back(symNode);
	}
}

void
BufferSubgraph::clearVisitedEdges()
{
	for(unsigned int i=0; i<dirEdgeList.size(); ++i)
	{
		DirectedEdge *de=dirEdgeList[i];
		de->setVisited(false);
	}
}

void
BufferSubgraph::computeDepth(int outsideDepth)
{
	clearVisitedEdges();
	// find an outside edge to assign depth to
	DirectedEdge *de=finder.getEdge();
#if DEBUG
cerr<<"outside depth: "<<outsideDepth<<endl;
#endif
	//Node *n=de->getNode();
	//Label *label=de->getLabel();
	// right side of line returned by finder is on the outside
	de->setEdgeDepths(Position::RIGHT, outsideDepth);
	copySymDepths(de);
	//computeNodeDepth(n, de);
	computeDepths(de);
}

void
BufferSubgraph::computeNodeDepth(Node *n)
	// throw(TopologyException *)
{
	// find a visited dirEdge to start at
	DirectedEdge *startEdge=NULL;

	DirectedEdgeStar *ees=(DirectedEdgeStar *)n->getEdges();

	EdgeEndStar::iterator endIt=ees->end();

	EdgeEndStar::iterator it=ees->begin();
	for(; it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*)*it;
		if (de->isVisited() || de->getSym()->isVisited()) {
			startEdge=de;
			break;
		}
	}
	// MD - testing  Result: breaks algorithm
	//if (startEdge==null) return;
	Assert::isTrue(startEdge!=NULL, "unable to find edge to compute depths at " + n->getCoordinate().toString());
	ees->computeDepths(startEdge);

	// copy depths to sym edges
	for(it=ees->begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) (*it);
		de->setVisited(true);
		copySymDepths(de);
	}
}

void
BufferSubgraph::copySymDepths(DirectedEdge *de)
{
	DirectedEdge *sym=de->getSym();
	sym->setDepth(Position::LEFT, de->getDepth(Position::RIGHT));
	sym->setDepth(Position::RIGHT, de->getDepth(Position::LEFT));
#if DEBUG
cerr<<"copySymDepths: "<<de->getDepth(Position::LEFT)<<", "<<de->getDepth(Position::RIGHT)<<endl;
#endif
}

/**
 * Find all edges whose depths indicates that they are in the result area(s).
 * Since we want polygon shells to be
 * oriented CW, choose dirEdges with the interior of the result on the RHS.
 * Mark them as being in the result.
 * Interior Area edges are the result of dimensional collapses.
 * They do not form part of the result area boundary.
 */
void
BufferSubgraph::findResultEdges()
{
#if DEBUG
	cerr<<"BufferSubgraph::findResultEdges got "<<dirEdgeList.size()<<" edges"<<endl;
#endif
	for(unsigned int i=0; i<dirEdgeList.size(); ++i)
	{
		DirectedEdge *de=dirEdgeList[i];

		/**
		 * Select edges which have an interior depth on the RHS
		 * and an exterior depth on the LHS.
		 * Note that because of weird rounding effects there may be
		 * edges which have negative depths!  Negative depths
		 * count as "outside".
		 */
		// <FIX> - handle negative depths
#if DEBUG
		cerr<<" dirEdge "<<i<<": depth right:"<<de->getDepth(Position::RIGHT)<<endl;
#endif
		if ( de->getDepth(Position::RIGHT)>=1
			&&  de->getDepth(Position::LEFT)<=0
			&& !de->isInteriorAreaEdge()) {
					de->setInResult(true);
#if DEBUG
					cerr<<"   in result"<<endl;
#endif
		}
	}
}

/**
 * BufferSubgraphs are compared on the x-value of their rightmost Coordinate.
 * This defines a partial ordering on the graphs such that:
 * 
 * g1 >= g2 <==> Ring(g2) does not contain Ring(g1)
 * 
 * where Polygon(g) is the buffer polygon that is built from g.
 * 
 * This relationship is used to sort the BufferSubgraphs so that shells are
 * guaranteed to be built before holes.
 */
int
BufferSubgraph::compareTo(BufferSubgraph *graph)
{
	if (rightMostCoord->x<graph->rightMostCoord->x) {
		return -1;
	}
	if (rightMostCoord->x>graph->rightMostCoord->x) {
		return 1;
	}
	return 0;
}

/**
 * Compute depths for all dirEdges via breadth-first traversal of
 * nodes in graph.
 *
 * @param startEdge edge to start processing with
 */
// <FIX> MD - use iteration & queue rather than recursion, for speed and robustness
void
BufferSubgraph::computeDepths(DirectedEdge *startEdge)
{
	//vector<Node*> nodesVisited; //Used to be a HashSet
	set<Node *>nodesVisited;
	vector<Node*> nodeQueue;
	Node *startNode=startEdge->getNode();
	nodeQueue.push_back(startNode);
	//nodesVisited.push_back(startNode);
	nodesVisited.insert(startNode);
	startEdge->setVisited(true);

	while (! nodeQueue.empty())
	{
		//System.out.println(nodes.size() + " queue: " + nodeQueue.size());
		Node *n=nodeQueue[0];
		nodeQueue.erase(nodeQueue.begin());
		nodesVisited.insert(n);

		// compute depths around node, starting at this edge since it has depths assigned
		computeNodeDepth(n);

		// add all adjacent nodes to process queue,
		// unless the node has been visited already
		EdgeEndStar *ees=n->getEdges();
		EdgeEndStar::iterator endIt=ees->end();
		EdgeEndStar::iterator it=ees->begin();
		for(; it!=endIt; ++it)
		{
			DirectedEdge *de=(DirectedEdge*) (*it);
			DirectedEdge *sym=de->getSym();
			if (sym->isVisited()) continue;
			Node *adjNode=sym->getNode();

			//if (! contains(nodesVisited,adjNode))
			if(nodesVisited.insert(adjNode).second)
			{
				nodeQueue.push_back(adjNode);
				//nodesVisited.insert(adjNode);
			}
		}
	}
}

bool
BufferSubgraph::contains(set<Node*>&nodeSet, Node *node)
{
	//bool result=false;
	if ( nodeSet.find(node) != nodeSet.end() ) return true;
	return false;
}

Envelope *
BufferSubgraph::getEnvelope()
{
	if (env == NULL) {
		env = new Envelope();
		unsigned int size = dirEdgeList.size();
		for(unsigned int i=0; i<size; ++i)
		{
			DirectedEdge *dirEdge=dirEdgeList[i];
			const CoordinateSequence *pts = dirEdge->getEdge()->getCoordinates();
			int n = pts->getSize()-1;
			for (int j=0; j<n; ++j) {
				env->expandToInclude(pts->getAt(j));
			}
		}
	}
	return env;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.19  2005/11/08 20:12:44  strk
 * Memory overhead reductions in buffer operations.
 *
 * Revision 1.18  2005/06/30 18:31:48  strk
 * Ported SubgraphDepthLocator optimizations from JTS code
 *
 * Revision 1.17  2005/05/23 15:13:00  strk
 * Added debugging output
 *
 * Revision 1.16  2005/05/22 17:45:27  strk
 * Fixed initialization list order
 *
 * Revision 1.15  2005/05/20 16:15:41  strk
 * Code cleanups
 *
 * Revision 1.14  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.13  2005/02/04 18:49:48  strk
 * Changed ::computeDepths to use a set instead of a vector for checking
 * visited Edges.
 *
 * Revision 1.12  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.11  2004/07/07 07:52:13  strk
 * Removed note about required speedup in BufferSubgraph.
 * I've made tests with 'sets' and there is actually a big slow down..
 *
 * Revision 1.10  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2004/05/19 12:50:53  strk
 * Removed all try/catch blocks transforming stack allocated-vectors to auto-heap-allocations
 *
 * Revision 1.8  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.7  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.6  2004/04/16 12:48:07  strk
 * Leak fixes.
 *
 * Revision 1.5  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

