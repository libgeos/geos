#include "graphindex.h"

SweepLineSegment::SweepLineSegment(Edge *newEdge,int newPtIndex,int newGeomIndex){
	pts=&(newEdge->getCoordinates());
	edge=newEdge;
	ptIndex=newPtIndex;
	geomIndex=newGeomIndex;
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
