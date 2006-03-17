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
 **********************************************************************
 *
 * Last port: operation/buffer/BufferSubgraph.java rev. 1.17
 *
 **********************************************************************/

#include <cassert>
#include <vector>
#include <list>

#include <geos/operation/buffer/BufferSubgraph.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>

#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/DirectedEdgeStar.h>
#include <geos/geomgraph/EdgeEndStar.h>
#include <geos/geomgraph/Position.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::operation;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

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

/*public*/
void
BufferSubgraph::create(Node *node)
{
	addReachable(node);
	finder.findEdge(&dirEdgeList);
	rightMostCoord=&(finder.getCoordinate());
}

/*private*/
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

/*private*/
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

/*private*/
void
BufferSubgraph::clearVisitedEdges()
{
	for(unsigned int i=0; i<dirEdgeList.size(); ++i)
	{
		DirectedEdge *de=dirEdgeList[i];
		de->setVisited(false);
	}
}

/*public*/
void
BufferSubgraph::computeDepth(int outsideDepth)
{
	clearVisitedEdges();
	// find an outside edge to assign depth to
	DirectedEdge *de=finder.getEdge();
#if GEOS_DEBUG
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
	assert(startEdge!=NULL); // unable to find edge to compute depths at n->getCoordinate()
	ees->computeDepths(startEdge);

	// copy depths to sym edges
	for(it=ees->begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) (*it);
		de->setVisited(true);
		copySymDepths(de);
	}
}

/*private*/
void
BufferSubgraph::copySymDepths(DirectedEdge *de)
{
#if GEOS_DEBUG
	cerr << "copySymDepths: " << de->getDepth(Position::LEFT)
	     << ", " << de->getDepth(Position::RIGHT)
	     << endl;
#endif
	DirectedEdge *sym=de->getSym();
	sym->setDepth(Position::LEFT, de->getDepth(Position::RIGHT));
	sym->setDepth(Position::RIGHT, de->getDepth(Position::LEFT));
}

/*public*/
void
BufferSubgraph::findResultEdges()
{
#if GEOS_DEBUG
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
#if GEOS_DEBUG
		cerr << " dirEdge "<<i<<": " << de->printEdge() << endl
		     << "         depth right: " << de->getDepth(Position::RIGHT) << endl
		     << "          depth left: " << de->getDepth(Position::LEFT) << endl
		     << "    interiorAreaEdge: " << de->isInteriorAreaEdge()<<endl;
#endif
		if ( de->getDepth(Position::RIGHT)>=1
			&&  de->getDepth(Position::LEFT)<=0
			&& !de->isInteriorAreaEdge()) {
					de->setInResult(true);
#if GEOS_DEBUG
					cerr<<"   IN RESULT"<<endl;
#endif
		}
	}
}

/*public*/
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

/*private*/
void
BufferSubgraph::computeDepths(DirectedEdge *startEdge)
{
	set<Node *> nodesVisited; 
	list<Node*> nodeQueue; // Used to be a vector
	Node *startNode=startEdge->getNode();
	nodeQueue.push_back(startNode);
	//nodesVisited.push_back(startNode);
	nodesVisited.insert(startNode);
	startEdge->setVisited(true);

	while (! nodeQueue.empty())
	{
		//System.out.println(nodes.size() + " queue: " + nodeQueue.size());
		Node *n=nodeQueue.front(); // [0];
		//nodeQueue.erase(nodeQueue.begin());
		nodeQueue.pop_front(); 

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

/*private*/
bool
BufferSubgraph::contains(set<Node*>&nodeSet, Node *node)
{
	//bool result=false;
	if ( nodeSet.find(node) != nodeSet.end() ) return true;
	return false;
}

/*public*/
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

std::ostream& operator<< (std::ostream& os, const BufferSubgraph& bs)
{
	os << "BufferSubgraph[" << &bs << "] "
	   << bs.nodes.size() << " nodes, "
	   << bs.dirEdgeList.size() << " directed edges" << std::endl;

	for (unsigned int i=0, n=bs.nodes.size(); i<n; i++)
		os << "  Node " << i << ": " << *(bs.nodes[i]) << std::endl;

	for (unsigned int i=0, n=bs.dirEdgeList.size(); i<n; i++)
	{
		os << "  DirEdge " << i << ": " << std::endl
		   << bs.dirEdgeList[i]->printEdge() << std::endl;
	}

	return os;
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.30  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.29  2006/03/15 17:33:39  strk
 * Changed operator<< to use operator<< for Nodes
 *
 * Revision 1.28  2006/03/15 11:39:45  strk
 * comments cleanup, changed computeDepths to use a list<> rather then a vector (performance related)
 *
 * Revision 1.27  2006/03/14 17:10:14  strk
 * cleanups
 *
 * Revision 1.26  2006/03/14 14:16:52  strk
 * operator<< for BufferSubgraph, more debugging calls
 *
 * Revision 1.25  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 * Revision 1.24  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.23  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.22  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.21  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.20  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.19  2005/11/08 20:12:44  strk
 * Memory overhead reductions in buffer operations.
 *
 **********************************************************************/

