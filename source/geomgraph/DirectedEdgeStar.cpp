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
 * Revision 1.3  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.2  2004/04/21 14:14:28  strk
 * Fixed bug in computeDepths
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.17  2003/11/12 15:43:38  strk
 * Added some more throw specifications
 *
 * Revision 1.16  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geomgraph.h"
#include "../headers/util.h"

namespace geos {

DirectedEdgeStar::DirectedEdgeStar(){
//	resultAreaEdgeList=new vector<DirectedEdge*>();
	resultAreaEdgeList=NULL;
	label=new Label();
}

DirectedEdgeStar::~DirectedEdgeStar(){
	delete resultAreaEdgeList;
	delete label;

}

/**
 * Insert a directed edge in the list
 */
void DirectedEdgeStar::insert(EdgeEnd *ee){
	DirectedEdge *de=(DirectedEdge*) ee;
	insertEdgeEnd(de,de);
}

Label* DirectedEdgeStar::getLabel() {
	return label;
}

int DirectedEdgeStar::getOutgoingDegree(){
	int degree = 0;
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		if (de->isInResult()) degree++;
	}
	return degree;
}

int DirectedEdgeStar::getOutgoingDegree(EdgeRing *er){
	int degree = 0;
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		if (de->getEdgeRing()==er) degree++;
	}
	return degree;
}

DirectedEdge* DirectedEdgeStar::getRightmostEdge() {
	vector<EdgeEnd*> *edges=getEdges();
	int size=(int)edges->size();
	if (size<1) return NULL;
//    EdgeEnd* e0=(EdgeEnd*) edges;
    DirectedEdge *de0=(DirectedEdge*) (*edges)[0];
	if (size==1) return de0;
    DirectedEdge *deLast=(DirectedEdge*)(*edges)[size-1];
	int quad0=de0->getQuadrant();
	int quad1=deLast->getQuadrant();
	if (Quadrant::isNorthern(quad0) && Quadrant::isNorthern(quad1))
		return de0;
	else if (!Quadrant::isNorthern(quad0) && !Quadrant::isNorthern(quad1))
		return deLast;
	else {
		// edges are in different hemispheres - make sure we return one that is non-horizontal
		DirectedEdge *nonHorizontalEdge=NULL;
		if (de0->getDy()!=0)
			return de0;
		else if (deLast->getDy()!=0)
			return deLast;
	}
	Assert::shouldNeverReachHere("found two horizontal edges incident on node");
	return NULL;
}

/**
 * Compute the labelling for all dirEdges in this star, as well
 * as the overall labelling
 */
void
DirectedEdgeStar::computeLabelling(vector<GeometryGraph*> *geom)
	//throw(TopologyException *)
{
	// this call can throw a TopologyException 
	// we don't have any cleanup to do...
	EdgeEndStar::computeLabelling(geom);

	// determine the overall labelling for this DirectedEdgeStar
	// (i.e. for the node it is based at)
	delete label;
	label=new Label(Location::UNDEF);
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		EdgeEnd *ee=*it;
		Edge *e=ee->getEdge();
		Label *eLabel=e->getLabel();
		for (int i=0; i<2;i++) {
			int eLoc=eLabel->getLocation(i);
			if (eLoc==Location::INTERIOR || eLoc==Location::BOUNDARY)
				label->setLocation(i,Location::INTERIOR);
		}
	}
}

/**
 * For each dirEdge in the star,
 * merge the label from the sym dirEdge into the label
 */
void DirectedEdgeStar::mergeSymLabels(){
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		Label *deLabel=de->getLabel();
		deLabel->merge(de->getSym()->getLabel());
	}
}

/**
 * Update incomplete dirEdge labels from the labelling for the node
 */
void DirectedEdgeStar::updateLabelling(Label *nodeLabel){
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		Label *deLabel=de->getLabel();
		deLabel->setAllLocationsIfNull(0,nodeLabel->getLocation(0));
		deLabel->setAllLocationsIfNull(1,nodeLabel->getLocation(1));
	}
}

vector<DirectedEdge*>* DirectedEdgeStar::getResultAreaEdges() {
	if (resultAreaEdgeList!=NULL) return resultAreaEdgeList;
	resultAreaEdgeList=new vector<DirectedEdge*>();
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		if (de->isInResult() || de->getSym()->isInResult())
		resultAreaEdgeList->push_back(de);
	}
	return resultAreaEdgeList;
}

/**
 * Traverse the star of DirectedEdges, linking the included edges together.
 * To link two dirEdges, the <next> pointer for an incoming dirEdge
 * is set to the next outgoing edge.
 * <p>
 * DirEdges are only linked if:
 * <ul>
 * <li>they belong to an area (i.e. they have sides)
 * <li>they are marked as being in the result
 * </ul>
 * <p>
 * Edges are linked in CCW order (the order they are stored).
 * This means that rings have their face on the Right
 * (in other words,
 * the topological location of the face is given by the RHS label of the DirectedEdge)
 * <p>
 * PRECONDITION: No pair of dirEdges are both marked as being in the result
 */
void
DirectedEdgeStar::linkResultDirectedEdges() 
	// throw(TopologyException *)
{
	// make sure edges are copied to resultAreaEdges list
	getResultAreaEdges();
	// find first area edge (if any) to start linking at
	DirectedEdge *firstOut=NULL;
	DirectedEdge *incoming=NULL;
	int state=SCANNING_FOR_INCOMING;
	// link edges in CCW order
	for (unsigned int i=0; i<resultAreaEdgeList->size();i++) {
        DirectedEdge *nextOut=(DirectedEdge*) (*resultAreaEdgeList)[i];
		DirectedEdge *nextIn=nextOut->getSym();
		// skip de's that we're not interested in
		if (!nextOut->getLabel()->isArea()) continue;
		// record first outgoing edge, in order to link the last incoming edge
		if (firstOut==NULL && nextOut->isInResult()) firstOut=nextOut;
		// assert: sym.isInResult() == false, since pairs of dirEdges should have been removed already
		switch (state) {
			case SCANNING_FOR_INCOMING:
				if (!nextIn->isInResult()) continue;
				incoming=nextIn;
				state=LINKING_TO_OUTGOING;
				break;
			case LINKING_TO_OUTGOING:
				if (!nextOut->isInResult()) continue;
				incoming->setNext(nextOut);
				state=SCANNING_FOR_INCOMING;
				break;
		}
	}
	if (state==LINKING_TO_OUTGOING) {
		if (firstOut==NULL)
			throw new TopologyException("no outgoing dirEdge found",&(getCoordinate()));
//		Assert::isTrue(firstOut!=NULL, "no outgoing dirEdge found");
		Assert::isTrue(firstOut->isInResult(), "unable to link last incoming dirEdge");
		incoming->setNext(firstOut);
	}
}

void DirectedEdgeStar::linkMinimalDirectedEdges(EdgeRing *er){
	// find first area edge (if any) to start linking at
	DirectedEdge *firstOut=NULL;
	DirectedEdge *incoming=NULL;
	int state=SCANNING_FOR_INCOMING;
	// link edges in CW order
	for (int i=(int)resultAreaEdgeList->size()-1;i>=0;i--) {
        DirectedEdge *nextOut=(DirectedEdge*)(*resultAreaEdgeList)[i];
		DirectedEdge *nextIn=nextOut->getSym();
		// record first outgoing edge, in order to link the last incoming edge
		if (firstOut==NULL && nextOut->getEdgeRing()==er) firstOut=nextOut;
		switch (state) {
			case SCANNING_FOR_INCOMING:
				if (nextIn->getEdgeRing()!=er) continue;
				incoming=nextIn;
				state = LINKING_TO_OUTGOING;
				break;
			case LINKING_TO_OUTGOING:
				if (nextOut->getEdgeRing()!=er) continue;
				incoming->setNextMin(nextOut);
				state = SCANNING_FOR_INCOMING;
			break;
		}
	}
	if (state==LINKING_TO_OUTGOING) {
		Assert::isTrue(firstOut!=NULL, "found null for first outgoing dirEdge");
		Assert::isTrue(firstOut->getEdgeRing()==er, "unable to link last incoming dirEdge");
		incoming->setNextMin(firstOut);
	}
}

void DirectedEdgeStar::linkAllDirectedEdges(){
	getEdges();
	// find first area edge (if any) to start linking at
	DirectedEdge *prevOut=NULL;
	DirectedEdge *firstIn=NULL;
	// link edges in CW order
	for(int i=(int)edgeList->size()-1;i>=0;i--) {
        DirectedEdge *nextOut=(DirectedEdge*)(*edgeList)[i];
		DirectedEdge *nextIn=nextOut->getSym();
		if (firstIn==NULL) firstIn=nextIn;
		if (prevOut!=NULL) nextIn->setNext(prevOut);
		// record outgoing edge, in order to link the last incoming edge
		prevOut=nextOut;
	}
	firstIn->setNext(prevOut);
}

/**
 * Traverse the star of edges, maintaing the current location in the result
 * area at this node (if any).
 * If any L edges are found in the interior of the result, mark them as covered.
 */
void DirectedEdgeStar::findCoveredLineEdges(){
	// Since edges are stored in CCW order around the node,
	// as we move around the ring we move from the right to the left side of the edge

	/**
	 * Find first DirectedEdge of result area (if any).
	 * The interior of the result is on the RHS of the edge,
	 * so the start location will be:
	 * - INTERIOR if the edge is outgoing
	 * - EXTERIOR if the edge is incoming
	 */
	int startLoc=Location::UNDEF;
    vector<EdgeEnd*>::iterator it;
	for (it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *nextOut=(DirectedEdge*) *it;
		DirectedEdge *nextIn=nextOut->getSym();
		if (!nextOut->isLineEdge()) {
			if (nextOut->isInResult()) {
				startLoc=Location::INTERIOR;
				break;
			}
			if (nextIn->isInResult()) {
				startLoc=Location::EXTERIOR;
				break;
			}
		}
	}
	// no A edges found, so can't determine if L edges are covered or not
	if (startLoc==Location::UNDEF) return;
	/**
	 * move around ring, keeping track of the current location
	 * (Interior or Exterior) for the result area.
	 * If L edges are found, mark them as covered if they are in the interior
	 */
	int currLoc=startLoc;
	for (it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *nextOut=(DirectedEdge*) *it;
		DirectedEdge *nextIn=nextOut->getSym();
		if (nextOut->isLineEdge()) {
			nextOut->getEdge()->setCovered(currLoc==Location::INTERIOR);
		} else {  // edge is an Area edge
			if (nextOut->isInResult())
				currLoc=Location::EXTERIOR;
			if (nextIn->isInResult())
				currLoc=Location::INTERIOR;
		}
	}
}

void DirectedEdgeStar::computeDepths(DirectedEdge *de){
	int edgeIndex=findIndex(de);
	Label *deLabel=de->getLabel();
	int startDepth=de->getDepth(Position::LEFT);
	int targetLastDepth=de->getDepth(Position::RIGHT);

	// compute the depths from this edge up to the end of the edge array
	int nextDepth=computeDepths(edgeIndex+1, edgeList->size(), startDepth);
	// compute the depths for the initial part of the array
	int lastDepth=computeDepths(0,edgeIndex,nextDepth);
	if (lastDepth!=targetLastDepth)
		throw new TopologyException("depth mismatch at ",&(de->getCoordinate()));
//	Assert::isTrue(lastDepth==targetLastDepth, "depth mismatch at " + de->getCoordinate().toString());
}

/**
 * Compute the DirectedEdge depths for a subsequence of the edge array.
 *
 * @return the last depth assigned (from the R side of the last edge visited)
 */
int DirectedEdgeStar::computeDepths(int startIndex,int endIndex,int startDepth){
	int currDepth=startDepth;
	for (int i=startIndex;i<endIndex;i++) {
        DirectedEdge *nextDe=(DirectedEdge*)(*edgeList)[i];
		Label *deLabel=nextDe->getLabel();
		nextDe->setEdgeDepths(Position::RIGHT, currDepth);
		currDepth=nextDe->getDepth(Position::LEFT);
	}
	return currDepth;
}

string DirectedEdgeStar::print() {
	string out="DirectedEdgeStar: " + getCoordinate().toString();
	for (vector<EdgeEnd*>::iterator it=getIterator();it<edgeList->end();it++) {
		DirectedEdge *de=(DirectedEdge*) *it;
		out+="out ";
		out+=de->print();
		out+="\n";
		out+="in ";
		out+=de->getSym()->print();
		out+="\n";
	}
	return out;
}

}

