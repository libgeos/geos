#include "geosAlgorithm.h"

int RobustCGAlgorithms::orientationIndex(Coordinate p1,Coordinate p2,Coordinate q) {
	// travelling along p1->p2, turn counter clockwise to get to q return 1,
	// travelling along p1->p2, turn clockwise to get to q return -1,
	// p1, p2 and q are colinear return 0.
	double dx1=p2.x-p1.x;
	double dy1=p2.y-p1.y;
	double dx2=q.x-p2.x;
	double dy2=q.y-p2.y;
	return RobustDeterminant::signOfDet2x2(dx1,dy1,dx2,dy2);
}

RobustCGAlgorithms::RobustCGAlgorithms() {
	lineIntersector=new RobustLineIntersector();
}

bool RobustCGAlgorithms::isCCW(CoordinateList ring) {
	Coordinate hip;
	Coordinate p;
	Coordinate prev;
	Coordinate next;
	int hii;
	int i;
	int nPts=ring.getSize();
	// algorithm to check if a Ring is stored in CCW order
	// find highest point
	hip=ring.getAt(0);
	hii=0;
	for(i=1;i<nPts;i++) {
		p=ring.getAt(i);
		if (p.y>hip.y) {
			hip=p;
			hii=i;
		}
	}
	// find points on either side of highest
	int iPrev=hii-1;
	if (iPrev<0) {
		iPrev=nPts-2;
	}
	int iNext=hii+1;
	if (iNext>=nPts) {
		iNext=1;
	}
	prev=ring.getAt(iPrev);
	next=ring.getAt(iNext);
	int disc=computeOrientation(prev,hip,next);
	/*
	*  If disc is exactly 0, lines are collinear.  There are two possible cases:
	*  (1) the lines lie along the x axis in opposite directions
	*  (2) the line lie on top of one another
	*  (2) should never happen, so we're going to ignore it!
	*  (Might want to assert this)
	*  (1) is handled by checking if next is left of prev ==> CCW
	*/
	if (disc==0) {
		// poly is CCW if prev x is right of next x
		return (prev.x>next.x);
	} else {
		// if area is positive, points are ordered CCW
		return (disc>0);
	}
}

/**
* This algorithm does not attempt to first check the point against the envelope
* of the ring.
*
* @param ring assumed to have first point identical to last point
*/
bool RobustCGAlgorithms::isPointInRing(Coordinate p,CoordinateList ring) {
	int i;
	int i1;       // point index; i1 = i-1
	double xInt;  // x intersection of segment with ray
	int crossings=0;  // number of segment/ray crossings
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;
	int nPts=ring.getSize();

	/*
	*  For each segment l = (i-1, i), see if it crosses ray from test point in positive x direction.
	*/
	for (i=1;i<nPts;i++) {
		i1=i-1;
		Coordinate p1(ring.getAt(i));
		Coordinate p2(ring.getAt(i1));
		x1=p1.x-p.x;
		y1=p1.y-p.y;
		x2=p2.x-p.x;
		y2=p2.y-p.y;
		if (((y1>0) && (y2<=0)) || ((y2>0) && (y1<=0))) {
			/*
			*  segment straddles x axis, so compute intersection.
			*/
			xInt=RobustDeterminant::signOfDet2x2(x1,y1,x2,y2)/(y2-y1);
			//xsave = xInt;
			/*
			*  crosses ray if strictly positive intersection.
			*/
			if (0.0<xInt) {
				crossings++;
			}
		}
	}
	/*
	*  p is inside if number of crossings is odd.
	*/
	if ((crossings%2)==1) {
		return true;
	} else {
		return false;
	}
}

bool RobustCGAlgorithms::isOnLine(Coordinate p,CoordinateList pt) {
	for(int i=1;i<pt.getSize();i++) {
		Coordinate p0(pt.getAt(i-1));
		Coordinate p1(pt.getAt(i));
		lineIntersector->computeIntersection(p,p0,p1);
		if (lineIntersector->hasIntersection()) {
			return true;
		}
	}
	return false;
}

int RobustCGAlgorithms::computeOrientation(Coordinate p1,Coordinate p2,Coordinate q) {
	return orientationIndex(p1,p2,q);
}

bool RobustCGAlgorithms::isInEnvelope(Coordinate p,CoordinateList ring) {
	Envelope envelope;
	for(int i=0;i<ring.getSize();i++) {
		envelope.expandToInclude(ring.getAt(i));
	}
	return envelope.contains(p);
}
