/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.10  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.9  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.6  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.5  2004/05/27 10:27:03  strk
 * Memory leaks fixed.
 *
 * Revision 1.4  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.3  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.2  2004/05/03 20:49:20  strk
 * Some more leaks fixed
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include <geos/noding.h>

namespace geos {
SegmentNodeList::SegmentNodeList(const SegmentString *newEdge)
{
	nodes=new set<SegmentNode*,SegmentNodeLT>();
	edge=newEdge;
}

SegmentNodeList::~SegmentNodeList() {
	set<SegmentNode *, SegmentNodeLT>::iterator it;
	for(it=nodes->begin(); it != nodes->end(); it++) delete *it;
	delete nodes;

	unsigned int i;
	for(i=0; i<splitEdges.size(); i++)
		delete splitEdges[i];
	for(i=0; i<splitCoordLists.size(); i++)
		delete splitCoordLists[i];
}

/**
* Adds an intersection into the list, if it isn't already there.
* The input segmentIndex and dist are expected to be normalized.
* @return the SegmentIntersection found or added
*/
SegmentNode* SegmentNodeList::add(Coordinate *intPt, int segmentIndex, double dist){
	SegmentNode *eiNew=new SegmentNode(intPt, segmentIndex, dist);
	if (nodes->find(eiNew)!=nodes->end()) {
		SegmentNode *found = *nodes->find(eiNew);
		delete eiNew;
		return found;
	}
	nodes->insert(eiNew);
	return eiNew;
}

/**
* Adds entries for the first and last points of the edge to the list
*/
void SegmentNodeList::addEndpoints(){
	int maxSegIndex = edge->size() - 1;
	add((Coordinate*)&(edge->getCoordinate(0)), 0, 0.0);
	add((Coordinate*)&(edge->getCoordinate(maxSegIndex)), maxSegIndex, 0.0);
}

/**
* Creates new edges for all the edges that the intersections in this
* list split the parent edge into.
* Adds the edges to the input list (this is so a single list
* can be used to accumulate all split edges for a Geometry).
*/
void SegmentNodeList::addSplitEdges(vector<SegmentString*> *edgeList){
	// testingOnly
	//vector<SegmentString*> *testingSplitEdges=new vector<SegmentString*>();
	// ensure that the list has entries for the first and last point of the edge
	addEndpoints();

	set<SegmentNode*,SegmentNodeLT>::iterator it=nodes->begin();
	// there should always be at least two entries in the list
	SegmentNode *eiPrev=*it;
	it++;
	for(;it!=nodes->end();it++) {
		SegmentNode *ei=*it;
		SegmentString *newEdge=createSplitEdge(eiPrev, ei);
		edgeList->push_back(newEdge);
		//testingSplitEdges->push_back(newEdge);
		eiPrev = ei;
	}
	//checkSplitEdgesCorrectness(testingSplitEdges);
}

void SegmentNodeList::checkSplitEdgesCorrectness(vector<SegmentString*> *splitEdges){
	const CoordinateSequence *edgePts=edge->getCoordinates();
	// check that first and last points of split edges are same as endpoints of edge
	SegmentString *split0=(*splitEdges)[0];
	Coordinate pt0=split0->getCoordinate(0);
	if (!(pt0==edgePts->getAt(0)))
		throw new GEOSException("bad split edge start point at " + pt0.toString());
	SegmentString *splitn=(*splitEdges)[splitEdges->size()-1];
	const CoordinateSequence *splitnPts=splitn->getCoordinates();
	Coordinate ptn=splitnPts->getAt(splitnPts->getSize()-1);
	if (!(ptn==edgePts->getAt(edgePts->getSize()-1)))
		throw new GEOSException("bad split edge end point at " + ptn.toString());
}

/**
* Create a new "split edge" with the section of points between
* (and including) the two intersections.
* The label for the new edge is the same as the label for the parent edge.
*/
SegmentString*
SegmentNodeList::createSplitEdge(SegmentNode *ei0, SegmentNode *ei1)
{
	//Debug.print("\ncreateSplitEdge"); Debug.print(ei0); Debug.print(ei1);
	int npts = ei1->segmentIndex - ei0->segmentIndex + 2;
	Coordinate lastSegStartPt=edge->getCoordinate(ei1->segmentIndex);
	// if the last intersection point is not equal to the its segment start pt,
	// add it to the points list as well.
	// (This check is needed because the distance metric is not totally reliable!)
	// The check for point equality is 2D only - Z values are ignored
	bool useIntPt1=ei1->dist > 0.0 || ! ei1->coord->equals2D(lastSegStartPt);
	if (! useIntPt1) {
		npts--;
	}
	CoordinateSequence *pts = new DefaultCoordinateSequence(npts); 
	int ipt = 0;
	pts->setAt(Coordinate(*(ei0->coord)),ipt++);
	for (int i = ei0->segmentIndex + 1; i <= ei1->segmentIndex; i++) {
		pts->setAt(edge->getCoordinate(i),ipt++);
	}
	if (useIntPt1) 	pts->setAt(*(ei1->coord),ipt++);
	SegmentString *ret = new SegmentString(pts,edge->getContext());
	splitEdges.push_back(ret);
	splitCoordLists.push_back(pts);
	//delete pts;
	return ret;
}

string SegmentNodeList::print(){
	string out="Intersections:";
	set<SegmentNode*,SegmentNodeLT>::iterator it=nodes->begin();
	for(;it!=nodes->end();it++) {
		SegmentNode *ei=*it;
		out.append(ei->print());
	}
	return out;
}
}
