/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/DirectedEdgeStar.java rev. 1.4 (JTS-1.7)
 *
 **********************************************************************/

#include <cassert>
#include <string>
#include <vector>

#include <geos/geomgraph.h>
#include <geos/util.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//using namespace std;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
void
DirectedEdgeStar::insert(EdgeEnd *ee)
{
	DirectedEdge *de=dynamic_cast<DirectedEdge*>(ee);
	assert(de);
	insertEdgeEnd(de);
}

/*public*/
int
DirectedEdgeStar::getOutgoingDegree()
{
	int degree = 0;
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		if (de->isInResult()) ++degree;
	}
	return degree;
}

/*public*/
int
DirectedEdgeStar::getOutgoingDegree(EdgeRing *er)
{
	int degree = 0;
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		if (de->getEdgeRing()==er) ++degree;
	}
	return degree;
}

/*public*/
DirectedEdge*
DirectedEdgeStar::getRightmostEdge()
{
	EdgeEndStar::iterator it=begin();
	if ( it==end() ) return NULL;

	DirectedEdge *de0=dynamic_cast<DirectedEdge*>(*it);
	assert(de0);
	++it;
	if ( it==end() ) return de0;

	it=end(); --it;
	DirectedEdge *deLast=dynamic_cast<DirectedEdge*>(*it);
	assert(deLast);

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
	assert(0); // found two horizontal edges incident on node
	return NULL;
}

/*public*/
void
DirectedEdgeStar::computeLabelling(std::vector<GeometryGraph*> *geom)
	//throw(TopologyException *)
{
	// this call can throw a TopologyException 
	// we don't have any cleanup to do...
	EdgeEndStar::computeLabelling(geom);

	// determine the overall labelling for this DirectedEdgeStar
	// (i.e. for the node it is based at)
	label=Label(Location::UNDEF);
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		EdgeEnd *ee=*it;
		Edge *e=ee->getEdge();
		Label *eLabel=e->getLabel();
		for (int i=0; i<2; ++i) {
			int eLoc=eLabel->getLocation(i);
			if (eLoc==Location::INTERIOR || eLoc==Location::BOUNDARY)
				label.setLocation(i, Location::INTERIOR);
		}
	}
}

/*public*/
void
DirectedEdgeStar::mergeSymLabels()
{
	EdgeEndStar::iterator endIt=end(); 
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		Label *deLabel=de->getLabel();
		deLabel->merge(*(de->getSym()->getLabel()));
	}
}

/*public*/
void
DirectedEdgeStar::updateLabelling(Label *nodeLabel)
{
	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		Label *deLabel=de->getLabel();
		deLabel->setAllLocationsIfNull(0,nodeLabel->getLocation(0));
		deLabel->setAllLocationsIfNull(1,nodeLabel->getLocation(1));
	}
}

/*private*/
std::vector<DirectedEdge*>*
DirectedEdgeStar::getResultAreaEdges()
{
	if (resultAreaEdgeList!=NULL) return resultAreaEdgeList;

	resultAreaEdgeList=new std::vector<DirectedEdge*>();

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
		DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		if (de->isInResult() || de->getSym()->isInResult())
			resultAreaEdgeList->push_back(de);
	}
	return resultAreaEdgeList;
}

/*public*/
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
	for (std::vector<DirectedEdge*>::iterator
		i=resultAreaEdgeList->begin(), iEnd=resultAreaEdgeList->end();
		i != iEnd;
		++i)
	{
        	DirectedEdge *nextOut=*i;

		// skip de's that we're not interested in
		if (!nextOut->getLabel()->isArea()) continue;

		DirectedEdge *nextIn=nextOut->getSym();

		// record first outgoing edge, in order to link the last incoming edge
		if (firstOut==NULL && nextOut->isInResult()) firstOut=nextOut;

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
			throw util::TopologyException("no outgoing dirEdge found",&(getCoordinate()));
		assert(firstOut->isInResult()); // unable to link last incoming dirEdge
		incoming->setNext(firstOut);
	}
}

/*public*/
void
DirectedEdgeStar::linkMinimalDirectedEdges(EdgeRing *er)
{
	// find first area edge (if any) to start linking at
	DirectedEdge *firstOut=NULL;
	DirectedEdge *incoming=NULL;
	int state=SCANNING_FOR_INCOMING;

	// link edges in CW order
	for (std::vector<DirectedEdge*>::reverse_iterator
		i=resultAreaEdgeList->rbegin(), iEnd=resultAreaEdgeList->rend();
		i != iEnd;
		++i)
	{
		//DirectedEdge *nextOut=(*resultAreaEdgeList)[i];
		DirectedEdge *nextOut=*i;

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
		assert(firstOut!=NULL); // found null for first outgoing dirEdge
		assert(firstOut->getEdgeRing()==er); // unable to link last incoming dirEdge
		incoming->setNextMin(firstOut);
	}
}

/*public*/
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
        	DirectedEdge *nextOut=dynamic_cast<DirectedEdge*>(*it);
		assert(nextOut);

		DirectedEdge *nextIn=nextOut->getSym();
		if (firstIn==NULL) firstIn=nextIn;
		if (prevOut!=NULL) nextIn->setNext(prevOut);
		// record outgoing edge, in order to link the last incoming edge
		prevOut=nextOut;
	}
	firstIn->setNext(prevOut);
}

/*public*/
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
        	DirectedEdge *nextOut=dynamic_cast<DirectedEdge*>(*it);
		assert(nextOut);

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
        	DirectedEdge *nextOut=dynamic_cast<DirectedEdge*>(*it);
		assert(nextOut);
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

/*public*/
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
		throw util::TopologyException("depth mismatch at ",&(de->getCoordinate()));
}

/*public*/
int
DirectedEdgeStar::computeDepths(EdgeEndStar::iterator startIt,
	EdgeEndStar::iterator endIt, int startDepth)
{
	int currDepth=startDepth;
	for (EdgeEndStar::iterator it=startIt; it!=endIt; ++it)
	{
        	DirectedEdge *nextDe=dynamic_cast<DirectedEdge*>(*it);
		assert(nextDe);

		nextDe->setEdgeDepths(Position::RIGHT, currDepth);
		currDepth=nextDe->getDepth(Position::LEFT);
	}
	return currDepth;
}

/*public*/
std::string
DirectedEdgeStar::print()
{
	std::string out="DirectedEdgeStar: " + getCoordinate().toString();

	EdgeEndStar::iterator endIt=end();
	for (EdgeEndStar::iterator it=begin(); it!=endIt; ++it)
	{
        	DirectedEdge *de=dynamic_cast<DirectedEdge*>(*it);
		assert(de);
		out+="out ";
		out+=de->print();
		out+="\n";
		out+="in ";
		out+=de->getSym()->print();
		out+="\n";
	}
	return out;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.15  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.14  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.13  2006/02/27 11:53:17  strk
 * DirectedEdgeStar made more safe trough assert(), use of standard iterator and
 * dynamic casts substituting static ones.
 *
 * Revision 1.12  2006/02/27 09:05:32  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.11  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.10  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.9  2005/12/07 20:51:20  strk
 * minor cleanups
 *
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

