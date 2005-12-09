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


#include <geos/opPolygonize.h>
#include <geos/util.h>

namespace geos {

int
PolygonizeGraph::getDegreeNonDeleted(planarNode *node)
{
	vector<planarDirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(unsigned int i=0; i<edges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		if (!de->isMarked()) ++degree;
	}
	return degree;
}

int
PolygonizeGraph::getDegree(planarNode *node, long label)
{
	vector<planarDirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(unsigned int i=0; i<edges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		if (de->getLabel()==label) ++degree;
	}
	return degree;
}

/**
 * Deletes all edges at a node
 */
void
PolygonizeGraph::deleteAllEdges(planarNode *node)
{
	vector<planarDirectedEdge*> &edges=node->getOutEdges()->getEdges();
	for(unsigned int i=0; i<edges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		de->setMarked(true);
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		if (sym!=NULL)
			sym->setMarked(true);
	}
}

/*
 * Create a new polygonization graph.
 */
PolygonizeGraph::PolygonizeGraph(const GeometryFactory *newFactory):
	factory(newFactory)
{
}

/*
 * Destroy a PolygonizeGraph
 */
PolygonizeGraph::~PolygonizeGraph()
{
	unsigned int i;
	for (i=0; i<newEdges.size(); i++)
		delete newEdges[i];
	for (i=0; i<newDirEdges.size(); i++)
		delete newDirEdges[i];
	for (i=0; i<newNodes.size(); i++)
		delete newNodes[i];
	for (i=0; i<newEdgeRings.size(); i++)
		delete newEdgeRings[i];
	for (i=0; i<newCoords.size(); i++) delete newCoords[i];
}

/*
 * Add a LineString forming an edge of the polygon graph.
 * @param line the line to add
 */
void
PolygonizeGraph::addEdge(const LineString *line)
{
	if (line->isEmpty()) return;

	CoordinateSequence *linePts=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());

	/*
	 * This would catch invalid linestrings
	 * (containing duplicated points only)
	 */
	if ( linePts->getSize() < 2 ) return;

	const Coordinate& startPt=linePts->getAt(0);
	const Coordinate& endPt=linePts->getAt(linePts->getSize()-1);
	planarNode *nStart=getNode(startPt);
	planarNode *nEnd=getNode(endPt);
	planarDirectedEdge *de0=new PolygonizeDirectedEdge(nStart, nEnd, linePts->getAt(1), true);
	newDirEdges.push_back(de0);
	planarDirectedEdge *de1=new PolygonizeDirectedEdge(nEnd, nStart, linePts->getAt(linePts->getSize()-2), false);
	newDirEdges.push_back(de1);
	planarEdge *edge=new PolygonizeEdge(line);
	newEdges.push_back(edge);
	edge->setDirectedEdges(de0, de1);
	add(edge);

	newCoords.push_back(linePts);
}

planarNode *
PolygonizeGraph::getNode(const Coordinate& pt)
{
	planarNode *node=findNode(pt);
	if (node==NULL) {
		node=new planarNode(pt);
		newNodes.push_back(node);
		// ensure node is only added once to graph
		add(node);
	}
	return node;
}

void
PolygonizeGraph::computeNextCWEdges()
{
	vector<planarNode*> *pns=getNodes();
	// set the next pointers for the edges around each node
	for(int i=0;i<(int)pns->size();i++) {
		planarNode *node=(*pns)[i];
		computeNextCWEdges(node);
	}
	delete pns;
}

/*
 * Convert the maximal edge rings found by the initial graph traversal
 * into the minimal edge rings required by JTS polygon topology rules.
 *
 * @param ringEdges the list of start edges for the edgeRings to convert.
 */
void
PolygonizeGraph::convertMaximalToMinimalEdgeRings(vector<PolygonizeDirectedEdge*> *ringEdges)
{
	for(int i=0;i<(int)ringEdges->size();i++)
	{
		PolygonizeDirectedEdge *de=(*ringEdges)[i];
		long label=de->getLabel();
		vector<planarNode*> *intNodes=findIntersectionNodes(de, label);
		if (intNodes==NULL) continue;

		// flip the next pointers on the intersection nodes to
		// create minimal edge rings
		//vector<planarNode*> *pns=getNodes();

		// set the next pointers for the edges around each node
		for(int j=0;j<(int)intNodes->size();j++) {
			planarNode *node=(*intNodes)[j];
			computeNextCCWEdges(node, label);
		}

		delete intNodes;
	}
}

/*
 * Finds all nodes in a maximal edgering which are self-intersection nodes
 * @param startDE
 * @param label
 * @return the list of intersection nodes found,
 * or <code>NULL</code> if no intersection nodes were found.
 * Ownership of returned object goes to caller.
 */
vector<planarNode*>*
PolygonizeGraph::findIntersectionNodes(PolygonizeDirectedEdge *startDE, long label)
{
	PolygonizeDirectedEdge *de=startDE;
	vector<planarNode*> *intNodes=NULL;
	do {
		planarNode *node=de->getFromNode();
		if (getDegree(node, label) > 1) {
			if (intNodes==NULL)
				intNodes=new vector<planarNode*>();
			intNodes->push_back(node);
		}
		de=de->getNext();
		Assert::isTrue(de!=NULL, "found NULL DE in ring");
		Assert::isTrue(de==startDE || !de->isInRing(), "found DE already in ring");
	} while (de!=startDE);
	return intNodes;
}

/**
 * Computes the EdgeRings formed by the edges in this graph.
 * @return a list of the EdgeRing found by the polygonization process.
 */
vector<polygonizeEdgeRing*>*
PolygonizeGraph::getEdgeRings()
{
	// maybe could optimize this, since most of these pointers should
	// be set correctly already
	// by deleteCutEdges()
	computeNextCWEdges();

	// clear labels of all edges in graph
	label(dirEdges, -1);
	vector<PolygonizeDirectedEdge*> *maximalRings=findLabeledEdgeRings(dirEdges);
	convertMaximalToMinimalEdgeRings(maximalRings);
	delete maximalRings;

	// find all edgerings
	vector<polygonizeEdgeRing*> *edgeRingList=new vector<polygonizeEdgeRing*>();
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		if (de->isInRing()) continue;
		polygonizeEdgeRing *er=findEdgeRing(de);
		edgeRingList->push_back(er);
	}
	return edgeRingList;
}

/**
*
* @param dirEdges a List of the DirectedEdges in the graph
* @return a List of DirectedEdges, one for each edge ring found
*/
vector<PolygonizeDirectedEdge*>*
PolygonizeGraph::findLabeledEdgeRings(vector<planarDirectedEdge*> &dirEdges)
{
	vector<PolygonizeDirectedEdge*> *edgeRingStarts=new vector<PolygonizeDirectedEdge*>();
	// label the edge rings formed
	long currLabel=1;
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		if (de->getLabel() >= 0) continue;
		edgeRingStarts->push_back(de);
		vector<planarDirectedEdge*> *edges=findDirEdgesInRing(de);
		label(*edges, currLabel);
		delete edges;
		++currLabel;
	}
	return edgeRingStarts;
}

/*
 * Finds and removes all cut edges from the graph.
 * @return a list of the LineString forming the removed cut edges
 */
vector<const LineString*> *
PolygonizeGraph::deleteCutEdges()
{
	computeNextCWEdges();

	// label the current set of edgerings
	delete findLabeledEdgeRings(dirEdges);

	/*
	 * Cut Edges are edges where both dirEdges have the same label.
	 * Delete them, and record them
	 */
	vector<const LineString*> *cutLines=new vector<const LineString*>();
	for(unsigned int i=0; i<dirEdges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		if (de->getLabel()==sym->getLabel()) {
			de->setMarked(true);
			sym->setMarked(true);
			// save the line as a cut edge
			PolygonizeEdge *e=(PolygonizeEdge*) de->getEdge();
			cutLines->push_back(e->getLine());
		}
	}
	return cutLines;
}

void
PolygonizeGraph::label(vector<planarDirectedEdge*> &dirEdges, long label)
{
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		de->setLabel(label);
	}
}

void
PolygonizeGraph::computeNextCWEdges(planarNode *node)
{
	planarDirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *startDE=NULL;
	PolygonizeDirectedEdge *prevDE=NULL;

	// the edges are stored in CCW order around the star
	vector<planarDirectedEdge*> &pde=deStar->getEdges();
	for(unsigned int i=0; i<pde.size(); ++i) {
		PolygonizeDirectedEdge *outDE=(PolygonizeDirectedEdge*)pde[i];
		if (outDE->isMarked()) continue;
		if (startDE==NULL)
			startDE=outDE;
		if (prevDE!=NULL) {
			PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) prevDE->getSym();
			sym->setNext(outDE);
		}
		prevDE=outDE;
	}
	if (prevDE!=NULL) {
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) prevDE->getSym();
		sym->setNext(startDE);
	}
}

/**
 * Computes the next edge pointers going CCW around the given node, for the
 * given edgering label.
 * This algorithm has the effect of converting maximal edgerings into
 * minimal edgerings
 */
void
PolygonizeGraph::computeNextCCWEdges(planarNode *node, long label)
{
	planarDirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *firstOutDE=NULL;
	PolygonizeDirectedEdge *prevInDE=NULL;

	// the edges are stored in CCW order around the star
	vector<planarDirectedEdge*> &edges=deStar->getEdges();

	/*
	 * Must use a SIGNED int here to allow for beak condition
	 * to be true.
	 */
	for(int i=edges.size()-1; i>=0; --i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		PolygonizeDirectedEdge *outDE=NULL;
		if (de->getLabel()==label) outDE=de;
		PolygonizeDirectedEdge *inDE=NULL;
		if (sym->getLabel()==label) inDE= sym;
		if (outDE==NULL && inDE==NULL) continue; // this edge is not in edgering
		if (inDE != NULL) {
			prevInDE=inDE;
		}
		if (outDE != NULL) {
			if (prevInDE != NULL) {
				prevInDE->setNext(outDE);
				prevInDE=NULL;
			}
			if (firstOutDE==NULL)
				firstOutDE=outDE;
		}
	}
	if (prevInDE != NULL) {
		Assert::isTrue(firstOutDE != NULL);
		prevInDE->setNext(firstOutDE);
	}
}

/*
 * Traverse a ring of DirectedEdges, accumulating them into a list.
 * This assumes that all dangling directed edges have been removed
 * from the graph, so that there is always a next dirEdge.
 *
 * @param startDE the DirectedEdge to start traversing at
 * @return a List of DirectedEdges that form a ring
 */
vector<planarDirectedEdge*>*
PolygonizeGraph::findDirEdgesInRing(PolygonizeDirectedEdge *startDE)
{
	PolygonizeDirectedEdge *de=startDE;
	vector<planarDirectedEdge*> *edges=new vector<planarDirectedEdge*>();
	do {
		edges->push_back(de);
		de=de->getNext();
		Assert::isTrue(de != NULL, "found NULL DE in ring");
		Assert::isTrue(de==startDE || !de->isInRing(), "found DE already in ring");
	} while (de != startDE);
	return edges;
}

polygonizeEdgeRing *
PolygonizeGraph::findEdgeRing(PolygonizeDirectedEdge *startDE)
{
	PolygonizeDirectedEdge *de=startDE;
	polygonizeEdgeRing *er=new polygonizeEdgeRing(factory);
	// Now, when will we delete those polygonizeEdgeRings ?
	newEdgeRings.push_back(er);
	do {
		er->add(de);
		de->setRing(er);
		de=de->getNext();
		Assert::isTrue(de != NULL, "found NULL DE in ring");
		Assert::isTrue(de==startDE || ! de->isInRing(), "found DE already in ring");
	} while (de != startDE);
	return er;
}

/**
 * Marks all edges from the graph which are "dangles".
 * Dangles are which are incident on a node with degree 1.
 * This process is recursive, since removing a dangling edge
 * may result in another edge becoming a dangle.
 * In order to handle large recursion depths efficiently,
 * an explicit recursion stack is used
 *
 * @return a List containing the LineStrings that formed dangles
 */
vector<const LineString*>*
PolygonizeGraph::deleteDangles()
{
	vector<planarNode*> *nodesToRemove=findNodesOfDegree(1);
	vector<const LineString*> *dangleLines=new vector<const LineString*>();
	vector<planarNode*> nodeStack;
	for(int i=0;i<(int)nodesToRemove->size();i++) {
		nodeStack.push_back((*nodesToRemove)[i]);
	}
	delete nodesToRemove;
	while (!nodeStack.empty()) {
		planarNode *node=nodeStack[nodeStack.size()-1];
		nodeStack.pop_back();
		deleteAllEdges(node);
		vector<planarDirectedEdge*> &nodeOutEdges=node->getOutEdges()->getEdges();
		for(unsigned int j=0; j<nodeOutEdges.size(); ++j)
		{
			PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)nodeOutEdges[j];
			// delete this edge and its sym
			de->setMarked(true);
			PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
			if (sym != NULL)
				sym->setMarked(true);
			// save the line as a dangle
			PolygonizeEdge *e=(PolygonizeEdge*) de->getEdge();
			dangleLines->push_back(e->getLine());
			planarNode *toNode=de->getToNode();
			// add the toNode to the list to be processed, if it is now a dangle
			if (getDegreeNonDeleted(toNode)==1)
				nodeStack.push_back(toNode);
		}
	}
	return dangleLines;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2005/12/09 10:03:46  strk
 * Fixed a bug making PolygonizeGraph choking on invalid LineStrings.
 * Minor optimizations in Polygonizer loops.
 *
 * Revision 1.10  2005/11/21 16:03:20  strk
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
 * Revision 1.9  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.8  2004/12/14 10:35:44  strk
 * Comments cleanup. PolygonizeGraph keeps track of generated CoordinateSequence
 * for delayed destruction.
 *
 * Revision 1.7  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.6  2004/10/26 16:09:21  strk
 * Some more intentation and envelope equality check fix.
 *
 * Revision 1.5  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/
