/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opPolygonize.h>
#include <geos/util.h>

namespace geos {

int PolygonizeGraph::getDegreeNonDeleted(planarNode *node) {
	vector<planarDirectedEdge*> *edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(int i=0;i<(int)edges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*) (*edges)[i];
		if (!de->isMarked()) degree++;
	}
	return degree;
}

int PolygonizeGraph::getDegree(planarNode *node, long label){
	vector<planarDirectedEdge*> *edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(int i=0;i<(int)edges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*) (*edges)[i];
		if (de->getLabel()==label) degree++;
	}
	return degree;
}

/**
* Deletes all edges at a node
*/
void PolygonizeGraph::deleteAllEdges(planarNode *node){
	vector<planarDirectedEdge*> *edges=node->getOutEdges()->getEdges();
	for(int i=0;i<(int)edges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*) (*edges)[i];
		de->setMarked(true);
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		if (sym!=NULL)
			sym->setMarked(true);
	}
}

/**
* Create a new polygonization graph.
*/
PolygonizeGraph::PolygonizeGraph(GeometryFactory *newFactory){
	factory=newFactory;
}

/**
* Add a {@link LineString} forming an edge of the polygon graph.
* @param line the line to add
*/
void PolygonizeGraph::addEdge(LineString *line){
	if (line->isEmpty()) { return;}
	CoordinateList *linePts=CoordinateList::removeRepeatedPoints(line->getCoordinates());
	Coordinate& startPt=(Coordinate)linePts->getAt(0);
	Coordinate& endPt=(Coordinate)linePts->getAt(linePts->getSize()-1);
	planarNode *nStart=getNode(startPt);
	planarNode *nEnd=getNode(endPt);
	planarDirectedEdge *de0=new PolygonizeDirectedEdge(nStart, nEnd, (Coordinate)linePts->getAt(1), true);
	planarDirectedEdge *de1=new PolygonizeDirectedEdge(nEnd, nStart, (Coordinate)linePts->getAt(linePts->getSize()-2), false);
	planarEdge *edge=new PolygonizeEdge(line);
	edge->setDirectedEdges(de0, de1);
	add(edge);
}

planarNode* PolygonizeGraph::getNode(Coordinate& pt){
	planarNode *node=findNode(pt);
	if (node==NULL) {
		node=new planarNode(pt);
		// ensure node is only added once to graph
		add(node);
	}
	return node;
}

void PolygonizeGraph::computeNextCWEdges() {
	vector<planarNode*> *pns=getNodes();
	// set the next pointers for the edges around each node
	for(int i=0;i<(int)pns->size();i++) {
		planarNode *node=(*pns)[i];
		computeNextCWEdges(node);
	}
}

/**
* Convert the maximal edge rings found by the initial graph traversal
* into the minimal edge rings required by JTS polygon topology rules.
*
* @param ringEdges the list of start edges for the edgeRings to convert.
*/
void PolygonizeGraph::convertMaximalToMinimalEdgeRings(vector<PolygonizeDirectedEdge*> *ringEdges) {
	for(int i=0;i<(int)ringEdges->size();i++) {
		PolygonizeDirectedEdge *de=(*ringEdges)[i];
		long label=de->getLabel();
		vector<planarNode*> *intNodes=findIntersectionNodes(de, label);
		if (intNodes==NULL) continue;
		// flip the next pointers on the intersection nodes to create minimal edge rings
		vector<planarNode*> *pns=getNodes();
		// set the next pointers for the edges around each node
		for(int j=0;j<(int)pns->size();j++) {
			planarNode *node=(*pns)[j];
			computeNextCCWEdges(node, label);
		}
	}
}

/**
* Finds all nodes in a maximal edgering which are self-intersection nodes
* @param startDE
* @param label
* @return the list of intersection nodes found,
* or <code>NULL</code> if no intersection nodes were found
*/
vector<planarNode*>* PolygonizeGraph::findIntersectionNodes(PolygonizeDirectedEdge *startDE, long label){
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
* @return a list of the {@link EdgeRing}s found by the polygonization process.
*/
vector<polygonizeEdgeRing*>* PolygonizeGraph::getEdgeRings() {
	// maybe could optimize this, since most of these pointers should be set correctly already
	// by deleteCutEdges()
	computeNextCWEdges();
	// clear labels of all edges in graph
	label(dirEdges,-1);
	vector<PolygonizeDirectedEdge*> *maximalRings=findLabeledEdgeRings(dirEdges);
	convertMaximalToMinimalEdgeRings(maximalRings);
	// find all edgerings
	vector<polygonizeEdgeRing*> *edgeRingList=new vector<polygonizeEdgeRing*>();
	for(int i=0;i<(int)dirEdges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)(*dirEdges)[i];
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
vector<PolygonizeDirectedEdge*>* PolygonizeGraph::findLabeledEdgeRings(vector<planarDirectedEdge*> *dirEdges){
	vector<PolygonizeDirectedEdge*> *edgeRingStarts=new vector<PolygonizeDirectedEdge*>();
	// label the edge rings formed
	long currLabel=1;
	for(int i=0;i<(int)dirEdges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)(*dirEdges)[i];
		if (de->isMarked()) continue;
		if (de->getLabel() >= 0) continue;
		edgeRingStarts->push_back(de);
		vector<planarDirectedEdge*> *edges=findDirEdgesInRing(de);
		label(edges, currLabel);
		currLabel++;
	}
	return edgeRingStarts;
}

/**
* Finds and removes all cut edges from the graph.
* @return a list of the {@link LineString}s forming the removed cut edges
*/
vector<LineString*>* PolygonizeGraph::deleteCutEdges(){
	computeNextCWEdges();
	// label the current set of edgerings
	findLabeledEdgeRings(dirEdges);
	/**
	* Cut Edges are edges where both dirEdges have the same label.
	* Delete them, and record them
	*/
	vector<LineString*> *cutLines=new vector<LineString*>();
	for(int i=0;i<(int)dirEdges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)(*dirEdges)[i];
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

void PolygonizeGraph::label(vector<planarDirectedEdge*> *dirEdges, long label){
	for(int i=0;i<(int)dirEdges->size();i++) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)(*dirEdges)[i];
		de->setLabel(label);
	}
}

void PolygonizeGraph::computeNextCWEdges(planarNode *node){
	planarDirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *startDE=NULL;
	PolygonizeDirectedEdge *prevDE=NULL;

	// the edges are stored in CCW order around the star
	vector<planarDirectedEdge*> *pde=deStar->getEdges();
	for(int i=0;i<(int)pde->size();i++) {
		PolygonizeDirectedEdge *outDE=(PolygonizeDirectedEdge*)(*pde)[i];
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
* This algorithm has the effect of converting maximal edgerings into minimal edgerings
*/
void PolygonizeGraph::computeNextCCWEdges(planarNode *node, long label) {
	planarDirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *firstOutDE=NULL;
	PolygonizeDirectedEdge *prevInDE=NULL;

	// the edges are stored in CCW order around the star
	vector<planarDirectedEdge*> *edges=deStar->getEdges();
	for(int i=(int)edges->size()-1;i>=0;i--) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)(*edges)[i];
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

/**
* Traverse a ring of DirectedEdges, accumulating them into a list.
* This assumes that all dangling directed edges have been removed
* from the graph, so that there is always a next dirEdge.
*
* @param startDE the DirectedEdge to start traversing at
* @return a List of DirectedEdges that form a ring
*/
vector<planarDirectedEdge*>* PolygonizeGraph::findDirEdgesInRing(PolygonizeDirectedEdge *startDE){
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

polygonizeEdgeRing* PolygonizeGraph::findEdgeRing(PolygonizeDirectedEdge *startDE){
	PolygonizeDirectedEdge *de=startDE;
	polygonizeEdgeRing *er=new polygonizeEdgeRing(factory);
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
* @return a List containing the {@link LineStrings} that formed dangles
*/
vector<LineString*>* PolygonizeGraph::deleteDangles() {
	vector<planarNode*> *nodesToRemove=findNodesOfDegree(1);
	vector<LineString*> *dangleLines=new vector<LineString*>();
	vector<planarNode*> *nodeStack=new vector<planarNode*>();
	for(int i=0;i<(int)nodesToRemove->size();i++) {
		nodeStack->push_back((*nodesToRemove)[i]);
	}
	while (!nodeStack->empty()) {
		planarNode *node=(*nodeStack)[nodeStack->size()-1];
		nodeStack->pop_back();
		deleteAllEdges(node);
		vector<planarDirectedEdge*> *nodeOutEdges=node->getOutEdges()->getEdges();
		for(int j=0;j<(int)nodeOutEdges->size();j++) {
			PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*) (*nodeOutEdges)[j];
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
				nodeStack->push_back(toNode);
		}
	}
	return dangleLines;
}
}
