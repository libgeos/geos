#include "../headers/geosAlgorithm.h"
#include "stdio.h"

SIRtreePointInRing::SIRtreePointInRing(LinearRing *newRing){
	ring=newRing;
	sirTree=NULL;
	crossings=0;
	buildIndex();
}

void SIRtreePointInRing::buildIndex() {
	Envelope *env=ring->getEnvelopeInternal();
	sirTree=new SIRtree();
	CoordinateList *pts=ring->getCoordinates();
	for(int i=1;i<pts->getSize();i++) {
		if(pts->getAt(i-1)==pts->getAt(i)) {continue;} //Optimization suggested by MD. [Jon Aquino]
		LineSegment *seg=new LineSegment(pts->getAt(i-1),pts->getAt(i));
		sirTree->insert(seg->p0.y,seg->p1.y,seg);
	}
}

bool SIRtreePointInRing::isInside(Coordinate& pt) {
	crossings=0;
	// test all segments intersected by vertical ray at pt
	vector<void*> *segs=intTree->query(pt.y);
	//System.out.println("query size=" + segs.size());
	for(int i=0;i<(int)segs->size();i++) {
		LineSegment *seg=(LineSegment*) (*segs)[i];
		testLineSegment(pt,seg);
	}

	/*
	*  p is inside if number of crossings is odd.
	*/
	if ((crossings%2)==1) {
		return true;
	}
	return false;
}

void SIRtreePointInRing::testLineSegment(Coordinate& p,LineSegment *seg) {
	double xInt;  // x intersection of segment with ray
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;

	/*
	*  Test if segment crosses ray from test point in positive x direction.
	*/
	Coordinate& p1=seg->p0;
	Coordinate& p2=seg->p1;
	x1=p1.x-p.x;
	y1=p1.y-p.y;
	x2=p2.x-p.x;
	y2=p2.y-p.y;

	if (((y1>0) && (y2<=0)) ||
		((y2>0) && (y1<=0))) {
		/*
		*  segment straddles x axis,so compute intersection.
		*/
		xInt=RobustDeterminant::signOfDet2x2(x1,y1,x2,y2)/(y2-y1);
		//xsave=xInt;
		/*
		*  crosses ray if strictly positive intersection.
		*/
		if (0.0<xInt) {
			crossings++;
		}
	}
}