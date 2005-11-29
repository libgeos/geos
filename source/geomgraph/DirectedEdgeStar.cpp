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

#include <geos/geomgraph.h>
#include <geos/util.h>

#define DEBUG 0

namespace geos {

DirectedEdgeStar::DirectedEdgeStar():
	EdgeEndStar(),
	resultAreaEdgeList(NULL)
	//label(new Label())
{
}

DirectedEdgeStar::~DirectedEdgeStar()
{
	delete resultAreaEdgeList;
	//delete label;
}

/**
 * Insert a directed edge in the list
 */
void
DirectedEdgeStar::insert(EdgeEnd *ee)
{
	DirectedEdge *de=static_cast<DirectedEdge*>(ee);
	insertEdgeEnd(de);
}

Label &
DirectedEdgeStar::getLabel()
{
	return label;
}

int
DirectedEdgeStar::getOutgoingDegree()
{
	int degree = 0;
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) *it;
		if (de->isInResult()) ++degree;
	}
	return degree;
}

int
DirectedEdgeStar::getOutgoingDegree(EdgeRing *er)
{
	int degree = 0;
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) *it;
		if (de->getEdgeRing()==er) ++degree;
	}
	return degree;
}

DirectedEdge*
DirectedEdgeStar::getRightmostEdge()
{
	EdgeEndStar::iterator it=begin();
	if ( it==end() ) return NULL;

	DirectedEdge *de0=(DirectedEdge*) *it;
	++it;
	if ( it==end() ) return de0;

	it=end(); --it;
	DirectedEdge *deLast=(DirectedEdge*) *it;

	int quad0=de0->getQuadrant();
	int quad1=deLast->getQuadrant();
	if (Quadrant::isNorthern(quad0) && Quadrant::isNorthern(quad1))
		return de0;
	else if (!Quadrant::isNorthern(quad0) && !Quadrant::isNorthern(quad1))
		return deLast;
	else {
		// edges are in different hemispheres - make sure we return one that is non-horizontal
		//DirectedEdge *nonHorizontalEdge=NULL;
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
	//delete label;
	label=Label(Location::UNDEF);
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		EdgeEnd *ee=*it;
		Edge *e=ee->getEdge();
		Label *eLabel=e->getLabel();
		for (int i=0; i<2;i++) {
			int eLoc=eLabel->getLocation(i);
			if (eLoc==Location::INTERIOR || eLoc==Location::BOUNDARY)
				label.setLocation(i, Location::INTERIOR);
		}
	}
}

/**
 * For each dirEdge in the star,
 * merge the label from the sym dirEdge into the label
 */
void
DirectedEdgeStar::mergeSymLabels()
{
	EdgeEndStar::iterator endIt=end(); 
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) *it;
		Label *deLabel=de->getLabel();
		deLabel->merge(*(de->getSym()->getLabel()));
	}
}

/**
 * Update incomplete dirEdge labels from the labelling for the node
 */
void
DirectedEdgeStar::updateLabelling(Label *nodeLabel)
{
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=(DirectedEdge*) *it;
		Label *deLabel=de->getLabel();
		deLabel->setAllLocationsIfNull(0,nodeLabel->getLocation(0));
		deLabel->setAllLocationsIfNull(1,nodeLabel->getLocation(1));
	}
}

vector<DirectedEdge*>*
DirectedEdgeStar::getResultAreaEdges()
{
	if (resultAreaEdgeList!=NULL) return resultAreaEdgeList;

	resultAreaEdgeList=new vector<DirectedEdge*>();

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=static_cast<DirectedEdge*>(*it);
		if (de->isInResult() || de->getSym()->isInResult())
			resultAreaEdgeList->push_back(de);
	}
	return resultAreaEdgeList;
}

/**
 * Traverse the star of DirectedEdges, linking the included edges together.
 * To link two dirEdges, the <next> pointer for an incoming dirEdge
 * is set to the next outgoing edge.
 * 
 * DirEdges are only linked if:
 * 
 * - they belong to an area (i.e. they have sides)
 * - they are marked as being in the result
 * 
 * Edges are linked in CCW order (the order they are stored).
 * This means that rings have their face on the Right
 * (in other words,
 * the topological location of the face is given by the RHS label of the DirectedEdge)
 * 
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
	for (unsigned int i=0; i<resultAreaEdgeList->size(); ++i)
	{
        	DirectedEdge *nextOut=(*resultAreaEdgeList)[i];

		// skip de's that we're not interested in
		if (!nextOut->getLabel()->isArea()) continue;

		DirectedEdge *nextIn=nextOut->getSym();

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
		Assert::isTrue(firstOut->isInResult(), "unable to link last incoming dirEdge");
		incoming->setNext(firstOut);
	}
}

void
DirectedEdgeStar::linkMinimalDirectedEdges(EdgeRing *er)
{
	// find first area edge (if any) to start linking at
	DirectedEdge *firstOut=NULL;
	DirectedEdge *incoming=NULL;
	int state=SCANNING_FOR_INCOMING;
	// link edges in CW order

	/*
	 * We must use a SIGNED integer here to be able to check for i<0
	 * to end the loop.
	 */
	for (int i=resultAreaEdgeList->size()-1; i>=0; --i)
	{
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

void
DirectedEdgeStar::linkAllDirectedEdges()
{
	//getEdges();

	// find first area edge (if any) to start linking at
	DirectedEdge *prevOut=NULL;
	DirectedEdge *firstIn=NULL;

	// link edges in CW order
	EdgeEndStar::reverse_iterator rbeginIt=rbegin(); 
	EdgeEndStar::reverse_iterator rendIt=rend(); 
	for(EdgeEndStar::reverse_iterator it=rbeginIt; it!=rendIt; ++it)
	{
        	DirectedEdge *nextOut=static_cast<DirectedEdge*>(*it);
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
void
DirectedEdgeStar::findCoveredLineEdges()
{
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

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
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
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
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

void
DirectedEdgeStar::computeDepths(DirectedEdge *de)
{
	EdgeEndStar::iterator edgeIterator=find(de);

	//Label *deLabel=de->getLabel();
	int startDepth=de->getDepth(Position::LEFT);
	int targetLastDepth=de->getDepth(Position::RIGHT);

	// compute the depths from this edge up to the end of the edge array
	EdgeEndStar::iterator nextEdgeIterator=edgeIterator;
	++nextEdgeIterator;
	int nextDepth=computeDepths(nextEdgeIterator, end(), startDepth);

	// compute the depths for the initial part of the array
	int lastDepth=computeDepths(begin(), edgeIterator, nextDepth);

	if (lastDepth!=targetLastDepth)
		throw new TopologyException("depth mismatch at ",&(de->getCoordinate()));
//	Assert::isTrue(lastDepth==targetLastDepth, "depth mismatch at " + de->getCoordinate().toString());
}

/**
 * Compute the DirectedEdge depths for a subsequence of the edge array.
 *
 * @return the last depth assigned (from the R side of the last edge visited)
 */
int
DirectedEdgeStar::computeDepths(EdgeEndStar::iterator startIt,
	EdgeEndStar::iterator endIt, int startDepth)
{
	int currDepth=startDepth;
	for (EdgeEndStar::iterator it=startIt; it!=endIt; ++it)
	{
        	DirectedEdge *nextDe=(DirectedEdge*)(*it);
		//Label *deLabel=nextDe->getLabel();
		nextDe->setEdgeDepths(Position::RIGHT, currDepth);
		currDepth=nextDe->getDepth(Position::LEFT);
	}
	return currDepth;
}

string
DirectedEdgeStar::print()
{
	string out="DirectedEdgeStar: " + getCoordinate().toString();

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.7  2005/11/21 16:03:20  strk
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
 * Revision 1.6  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.5  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.4  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
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

