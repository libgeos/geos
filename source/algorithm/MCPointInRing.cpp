#include "../headers/geosAlgorithm.h"
#include "stdio.h"

MCPointInRing::MCSelecter::MCSelecter(Coordinate& newP,MCPointInRing *prt) {
	p=newP;
	parent=prt;
}

void MCPointInRing::MCSelecter::select(LineSegment *ls) {
	parent->testLineSegment(p,ls);
}

MCPointInRing::MCPointInRing(LinearRing *newRing) {
	ring=newRing;
	intTree=NULL;
	crossings=0;
    buildIndex();
}

void MCPointInRing::buildIndex() {
	Envelope *env=ring->getEnvelopeInternal();
	intTree=new IntervalTree(env->getMinY(),env->getMaxY());
	CoordinateList *pts=ring->getCoordinates();
	vector<indexMonotoneChain*> *mcList=MonotoneChainBuilder::getChains(pts);
	for(int i=0;i<(int)mcList->size();i++) {
		indexMonotoneChain *mc=(*mcList)[i];
		Envelope *mcEnv=mc->getEnvelope();
		intTree->insert(mcEnv->getMinY(),mcEnv->getMaxY(),mc);
	}
}

bool MCPointInRing::isInside(Coordinate& pt) {
	crossings=0;
	// test all segments intersected by ray from pt in positive x direction
	Envelope *rayEnv=new Envelope(DoubleNegInfinity,DoubleInfinity,pt.y,pt.y);
	vector<void*> *segs=intTree->query(pt.y);
	//System.out.println("query size=" + segs.size());
	MCSelecter *mcSelecter=new MCSelecter(pt,this);
	for(int i=0;i<(int)segs->size();i++) {
		indexMonotoneChain *mc=(indexMonotoneChain*) (*segs)[i];
		testMonotoneChain(rayEnv,mcSelecter,mc);
	}
	/*
	*  p is inside if number of crossings is odd.
	*/
	if((crossings%2)==1) {
		return true;
	}
	return false;
}


void MCPointInRing::testMonotoneChain(Envelope *rayEnv,MCSelecter *mcSelecter,indexMonotoneChain *mc) {
	mc->select(rayEnv, mcSelecter);
}

void MCPointInRing::testLineSegment(Coordinate& p,LineSegment *seg) {
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
	if (((y1>0)&&(y2<=0)) || ((y2>0)&&(y1<=0))) {
		/*
		*  segment straddles x axis, so compute intersection.
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
