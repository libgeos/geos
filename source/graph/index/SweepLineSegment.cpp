/*
* $Log$
* Revision 1.8  2003/10/15 15:30:32  strk
* Declared a SweepLineEventOBJ from which MonotoneChain and SweepLineSegment
* derive to abstract SweepLineEvent object previously done on void * pointers.
* No more compiler warnings...
*
*/
#include "../../headers/graphindex.h"

namespace geos {

SweepLineSegment::SweepLineSegment(Edge *newEdge,int newPtIndex){
	pts=newEdge->getCoordinates();
	edge=newEdge;
	ptIndex=newPtIndex;
}

SweepLineSegment::~SweepLineSegment() {
}

double SweepLineSegment::getMinX(){
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1<x2?x1:x2;
}

double SweepLineSegment::getMaxX() {
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1>x2?x1:x2;
}

void SweepLineSegment::computeIntersections(SweepLineSegment *ss,SegmentIntersector *si) {
	si->addIntersections(edge,ptIndex,ss->edge,ss->ptIndex);
}
}

