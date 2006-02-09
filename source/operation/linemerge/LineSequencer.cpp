/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/linemerge/LineSequencer.java rev. 1.5 (JTS-1.7)
 *
 **********************************************************************/

#include <cassert>
#include <typeinfo>
#include <limits>
#include <geos/opLinemerge.h>
#include <geos/util.h>
#include <geos/geom.h>

namespace geos {

namespace planargraph {

	typedef planarNode Node;
	typedef planarNodeMap NodeMap;
	typedef planarPlanarGraph PlanarGraph;
	typedef planarDirectedEdge DirectedEdge;
	typedef planarDirectedEdgeStar DirectedEdgeStar;
	typedef planarGraphComponent GraphComponent;
	typedef planarSubgraph Subgraph;

} // namespace planargraph

using namespace planargraph;


/* static */
bool
LineSequencer::isSequenced(const Geometry* geom)
{
	const MultiLineString *mls;

	if ( ! (mls=dynamic_cast<const MultiLineString *>(geom)) )
	{
		return true;
	}

	// the nodes in all subgraphs which have been completely scanned
	Coordinate::ConstSet prevSubgraphNodes;
	Coordinate::ConstVect currNodes;

	const Coordinate* lastNode = NULL;

	for (unsigned int i=0, n=mls->getNumGeometries(); i<n; ++i)
	{
		assert(dynamic_cast<const LineString*>(mls->getGeometryN(i)));

		const LineString& line = \
			static_cast<const LineString&>(*(mls->getGeometryN(i)));

		const Coordinate* startNode = &(line.getCoordinateN(0));
		const Coordinate* endNode = &(line.getCoordinateN(line.getNumPoints() - 1));

		/**
		 * If this linestring is connected to a previous subgraph,
		 * geom is not sequenced
		 */
		if (prevSubgraphNodes.find(startNode) != prevSubgraphNodes.end())
		{
			return false;
		}
		if (prevSubgraphNodes.find(endNode) != prevSubgraphNodes.end())
		{
			return false;
		}

		if (lastNode != NULL)
		{
			if (! startNode->equals2D(*lastNode))
			{
				// start new connected sequence
				prevSubgraphNodes.insert(currNodes.begin(),
						currNodes.end());
				currNodes.clear();
			}
		}
		currNodes.push_back(startNode);
		currNodes.push_back(endNode);
		lastNode = endNode;
	}
	return true;
} 

/* private */
bool
LineSequencer::hasSequence(Subgraph& graph)
{
	int oddDegreeCount = 0;
	for (NodeMap::container::const_iterator
		it=graph.nodeBegin(), endIt=graph.nodeEnd();
		it!=endIt;
		++it)
	{
		Node* node = it->second;
		if (node->getDegree() % 2 == 1)
		oddDegreeCount++;
	}
	return oddDegreeCount <= 2;
}


/*private*/
LineSequencer::Sequences*
LineSequencer::findSequences()
{
	Sequences *sequences = new Sequences();
	ConnectedSubgraphFinder csFinder(graph);
	vector<Subgraph*>subgraphs;
	csFinder.getConnectedSubgraphs(subgraphs);
	for (vector<Subgraph*>::const_iterator
		it=subgraphs.begin(), endIt=subgraphs.end();
		it!=endIt;
		++it )
	{
		Subgraph* subgraph = *it;
		if (hasSequence(*subgraph)) {
			DirectedEdge::NonConstList* seq=findSequence(*subgraph);
			sequences->push_back(seq);
		}
		else {
			// if any subgraph cannot be sequenced, abort
			return NULL;
		}
	}
	return sequences;
}

/*private*/
void
LineSequencer::addLine(const LineString *lineString)
{
	if (factory == NULL) {
		factory = lineString->getFactory();
	}
	graph.addEdge(lineString);
	++lineCount;
}

/* private */
void
LineSequencer::computeSequence()
{
	if (isRun) return;
	isRun = true;

	Sequences* sequences(findSequences());
	if (sequences == NULL) return;

	sequencedGeometry = auto_ptr<Geometry>(buildSequencedGeometry(*sequences));
	isSequenceableVar = true;

	unsigned int finalLineCount = sequencedGeometry->getNumGeometries();

	Assert::isTrue(lineCount == finalLineCount,
			"Lines were missing from result");

	Assert::isTrue(dynamic_cast<LineString *>(sequencedGeometry.get())
		|| dynamic_cast<MultiLineString *>(sequencedGeometry.get()),
		"Result is not lineal");
}

/*private*/
Geometry*
LineSequencer::buildSequencedGeometry(const Sequences& sequences)
{
	auto_ptr<Geometry::NonConstVect> lines(new Geometry::NonConstVect);

	for (Sequences::const_iterator
		i1=sequences.begin(), i1End=sequences.end();
		i1 != i1End;
		++i1)
	{
		planarDirectedEdge::NonConstList& seq = *(*i1);
		for(planarDirectedEdge::NonConstList::iterator i2=seq.begin(),
			i2End=seq.end(); i2 != i2End; ++i2)
		{
			const planarDirectedEdge* de = *i2;
			LineMergeEdge* e = dynamic_cast<LineMergeEdge* >(de->getEdge());
			assert(e);
			const LineString* line = e->getLine();

			// lineToAdd will be a *copy* of input things
			LineString* lineToAdd;

			if ( ! de->getEdgeDirection() && ! line->isClosed() ) {
				lineToAdd = reverse(line);
			} else {
				Geometry* lineClone = line->clone();
				assert(dynamic_cast<LineString *>(lineClone));
				lineToAdd = static_cast<LineString *>(lineClone); 
			}

			lines->push_back(lineToAdd);
		}
	}

	if ( lines->size() == 0 ) {
		return NULL;
	} else {
		Geometry::NonConstVect *l=lines.get();
		lines.release();
		return factory->buildGeometry(l);
	}
}

/*static private*/
LineString *
LineSequencer::reverse(const LineString *line)
{
	CoordinateSequence* cs=line->getCoordinates();
	CoordinateSequence::reverse(cs);
	return line->getFactory()->createLineString(cs);
}

/*private static*/
const planarNode*
LineSequencer::findLowestDegreeNode(const planarSubgraph& graph)
{
	int minDegree = numeric_limits<int>::max(); 
	const planarNode* minDegreeNode = NULL;
	for (planarNodeMap::container::const_iterator
		it = graph.nodeBegin(), itEnd = graph.nodeEnd();
		it != itEnd;
		++it )
	{
		const planarNode* node = (*it).second;
		if (minDegreeNode == NULL || node->getDegree() < minDegree)
		{
			minDegree = node->getDegree();
			minDegreeNode = node;
		}
	}
	return minDegreeNode;
}

/*private static*/
const DirectedEdge*
LineSequencer::findUnvisitedBestOrientedDE(const Node* node)
{
	const DirectedEdge* wellOrientedDE = NULL;
	const DirectedEdge* unvisitedDE = NULL;
	const DirectedEdgeStar* des=node->getOutEdges();
	for (DirectedEdge::NonConstVect::const_iterator i=des->begin(),
		e=des->end();
		i!=e;
		++i)
	{
		DirectedEdge* de = *i;
		if (! de->getEdge()->isVisited()) {
			unvisitedDE = de;
			if (de->getEdgeDirection()) wellOrientedDE = de;
		}
	}
	if (wellOrientedDE != NULL)
		return wellOrientedDE;
	return unvisitedDE;
}


/*private*/
void
LineSequencer::addReverseSubpath(const DirectedEdge *de,
		DirectedEdge::NonConstList& deList,
		DirectedEdge::NonConstList::iterator lit,
		bool expectedClosed)
{
	// trace an unvisited path *backwards* from this de
	Node* endNode = de->getToNode();

	Node* fromNode = NULL;
	while (true) {
		deList.insert(lit, de->getSym());
		de->getEdge()->setVisited(true);
		fromNode = de->getFromNode();
		const DirectedEdge* unvisitedOutDE = findUnvisitedBestOrientedDE(fromNode);

		// this must terminate, since we are continually marking edges as visited
		if (unvisitedOutDE == NULL) break;
		de = unvisitedOutDE->getSym();
	}
	if ( expectedClosed ) {
		// the path should end at the toNode of this de,
		// otherwise we have an error
		assert(fromNode == endNode);
	}

}

/*private*/
DirectedEdge::NonConstList* 
LineSequencer::findSequence(Subgraph& graph)
{
	GraphComponent::setVisited(graph.edgeBegin(),
			graph.edgeEnd(), false);

	const Node* startNode = findLowestDegreeNode(graph);

	const DirectedEdge *startDE = *(startNode->getOutEdges()->begin());
	const DirectedEdge *startDESym = startDE->getSym();

	DirectedEdge::NonConstList *seq = new DirectedEdge::NonConstList();

	DirectedEdge::NonConstList::iterator lit=seq->begin();
	addReverseSubpath(startDESym, *seq, lit, false);

	lit=seq->end();
	while (lit != seq->begin()) {
		const DirectedEdge* prev = *(--lit);
		const DirectedEdge* unvisitedOutDE = findUnvisitedBestOrientedDE(prev->getFromNode());
		if (unvisitedOutDE != NULL)
			addReverseSubpath(unvisitedOutDE->getSym(), *seq, lit, true);
	}

	// At this point, we have a valid sequence of graph DirectedEdges,
	// but it is not necessarily appropriately oriented relative to
	// the underlying geometry.
	DirectedEdge::NonConstList* orientedSeq = orient(seq);

	if (orientedSeq != seq) delete seq;

	return orientedSeq;
}

/* private */
DirectedEdge::NonConstList* 
LineSequencer::orient(DirectedEdge::NonConstList* seq) 
{
	const DirectedEdge* startEdge = seq->front();
	const DirectedEdge* endEdge = seq->back();
	Node* startNode = startEdge->getFromNode();
	Node* endNode = endEdge->getToNode();

	bool flipSeq = false;
	bool hasDegree1Node = \
		startNode->getDegree() == 1 || endNode->getDegree() == 1;

	if (hasDegree1Node)
	{
		bool hasObviousStartNode = false;

		// test end edge before start edge, to make result stable
		// (ie. if both are good starts, pick the actual start
		if (endEdge->getToNode()->getDegree() == 1 &&
				endEdge->getEdgeDirection() == false)
		{
			hasObviousStartNode = true;
			flipSeq = true;
		}
		if (startEdge->getFromNode()->getDegree() == 1 && 
				startEdge->getEdgeDirection() == true)
		{
			hasObviousStartNode = true;
			flipSeq = false;
		}

		// since there is no obvious start node,
		// use any node of degree 1
		if (! hasObviousStartNode)
		{
			// check if the start node should actually
			// be the end node
			if (startEdge->getFromNode()->getDegree() == 1)
				flipSeq = true;
			// if the end node is of degree 1, it is
			// properly the end node
		}

	}


	// if there is no degree 1 node, just use the sequence as is
	// (Could insert heuristic of taking direction of majority of
	// lines as overall direction)

	if (flipSeq)
	{
		return reverse(*seq);
	}
	return seq;
}

/* private */
DirectedEdge::NonConstList* 
LineSequencer::reverse(DirectedEdge::NonConstList& seq)
{
	DirectedEdge::NonConstList* newSeq = new DirectedEdge::NonConstList();
	DirectedEdge::NonConstList::iterator it=seq.begin(), itEnd=seq.end();
	for (; it!=itEnd; ++it) {
		const DirectedEdge *de = *it;
		newSeq->push_front(de->getSym());
	}
	return newSeq;
}

 

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/02/09 01:13:24  strk
 * Added missing <limits> include
 *
 * Revision 1.1  2006/02/08 12:59:56  strk
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
 **********************************************************************/
