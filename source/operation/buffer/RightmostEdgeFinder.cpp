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
 * Revision 1.8  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.5  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/


#include <geos/opBuffer.h>
#include <geos/util.h>

namespace geos {

RightmostEdgeFinder::RightmostEdgeFinder(CGAlgorithms *newCga){
	cga=newCga;
	minIndex=-1;
	minDe=NULL;
	orientedDe=NULL;
	minCoord.setNull();
}
DirectedEdge* RightmostEdgeFinder::getEdge() {
	return orientedDe;
}

Coordinate& RightmostEdgeFinder::getCoordinate() {
	return minCoord;
}

void RightmostEdgeFinder::findEdge(vector<DirectedEdge*>* dirEdgeList){
	/**
	* Check all forward DirectedEdges only.  This is still general,
	* because each edge has a forward DirectedEdge.
	*/
	for(int i=0;i<(int)dirEdgeList->size();i++) {
		DirectedEdge *de=(*dirEdgeList)[i];
		if (!de->isForward())
			continue;
		checkForRightmostCoordinate(de);
	}

	/**
	* If the rightmost point is a node, we need to identify which of
	* the incident edges is rightmost.
	*/
	Assert::isTrue(minIndex!=0 || minCoord==minDe->getCoordinate(), "inconsistency in rightmost processing");
	if (minIndex==0 ) {
		findRightmostEdgeAtNode();
	} else {
		findRightmostEdgeAtVertex();
	}
	/**
	* now check that the extreme side is the R side.
	* If not, use the sym instead.
	*/
	orientedDe=minDe;
	int rightmostSide=getRightmostSide(minDe,minIndex);
	if (rightmostSide==Position::LEFT) {
		orientedDe=minDe->getSym();
	}
}

void RightmostEdgeFinder::findRightmostEdgeAtNode(){
	Node *node=minDe->getNode();
	DirectedEdgeStar *star=(DirectedEdgeStar*) node->getEdges();
	minDe=star->getRightmostEdge();
	// the DirectedEdge returned by the previous call is not
	// necessarily in the forward direction. Use the sym edge if it isn't.
	if (!minDe->isForward()) {
		minDe=minDe->getSym();
		minIndex=minDe->getEdge()->getCoordinates()->getSize()-1;
	}
}

void RightmostEdgeFinder::findRightmostEdgeAtVertex() {
	/**
	* The rightmost point is an interior vertex, so it has a segment on either side of it.
	* If these segments are both above or below the rightmost point, we need to
	* determine their relative orientation to decide which is rightmost.
	*/

	const CoordinateList *pts=minDe->getEdge()->getCoordinates();
	Assert::isTrue(minIndex>0 && minIndex<pts->getSize(), "rightmost point expected to be interior vertex of edge");
	const Coordinate& pPrev=pts->getAt(minIndex-1);
	const Coordinate& pNext=pts->getAt(minIndex+1);
	int orientation=cga->computeOrientation(minCoord,pNext,pPrev);
	bool usePrev=false;
	// both segments are below min point
	if (pPrev.y<minCoord.y && pNext.y<minCoord.y
		&& orientation==CGAlgorithms::COUNTERCLOCKWISE) {
			usePrev=true;
	} else if (pPrev.y>minCoord.y && pNext.y>minCoord.y
		&& orientation==CGAlgorithms::CLOCKWISE) {
			usePrev=true;
	}
	// if both segments are on the same side, do nothing - either is safe
	// to select as a rightmost segment
	if (usePrev) {
		minIndex=minIndex-1;
	}
}

void RightmostEdgeFinder::checkForRightmostCoordinate(DirectedEdge *de) {
	const CoordinateList *coord=de->getEdge()->getCoordinates();
	// only check vertices which are the starting point of a non-horizontal segment
	for(int i=0;i<coord->getSize()-1;i++) {
     // only check vertices which are the start or end point of a non-horizontal segment
     // <FIX> MD 19 Sep 03 - NO!  we can test all vertices, since the rightmost must have a non-horiz segment adjacent to it
		if (minCoord==Coordinate::getNull() || coord->getAt(i).x>minCoord.x ) {
			minDe=de;
			minIndex=i;
			minCoord=coord->getAt(i);
		}
	}
}

int RightmostEdgeFinder::getRightmostSide(DirectedEdge *de, int index){
	int side=getRightmostSideOfSegment(de,index);
	if (side<0)
		side=getRightmostSideOfSegment(de,index-1);
	if (side<0)
		// reaching here can indicate that segment is horizontal
		// Assert::shouldNeverReachHere("problem with finding rightmost side of segment");
		minCoord=Coordinate::nullCoord;
	checkForRightmostCoordinate(de);
	return side;
}

int RightmostEdgeFinder::getRightmostSideOfSegment(DirectedEdge *de, int i){
	Edge *e=de->getEdge();
	const CoordinateList *coord=e->getCoordinates();
	if (i<0 || i+1>=coord->getSize()) return -1;
	if (coord->getAt(i).y==coord->getAt(i+1).y) return -1;    // indicates edge is parallel to x-axis
	int pos=Position::LEFT;
	if (coord->getAt(i).y<coord->getAt(i+1).y) pos=Position::RIGHT;
	return pos;
}
}

