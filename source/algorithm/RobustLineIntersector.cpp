#include "geosAlgorithm.h"
#include "util.h"

RobustLineIntersector::RobustLineIntersector(){}

void RobustLineIntersector::computeIntersection(Coordinate p,Coordinate p1,Coordinate p2) {
	isProperVar=false;
	if ((RobustCGAlgorithms::orientationIndex(p1,p2,p)==0)
		&& (RobustCGAlgorithms::orientationIndex(p2,p1,p)==0)) {
		if (between(p1,p2,p)) {
			isProperVar=true;
			if ((p==p1) || (p==p2)) {
				isProperVar=false;
			}
			result=DO_INTERSECT;
			return;
		} else {
			result=DONT_INTERSECT;
			return;
		}
	} else {
		result=DONT_INTERSECT;
	}
}

int RobustLineIntersector::computeIntersect(Coordinate p1,Coordinate p2,Coordinate q1,Coordinate q2){
	isProperVar=false;

	// first try a fast test to see if the envelopes of the lines overlap
	double minq=min(q1.x,q2.x);
	double maxq=max(q1.x,q2.x);
	double minp=min(p1.x,p2.x);
	double maxp=max(p1.x,p2.x);

	if(minp>maxq)
		return DONT_INTERSECT;
	if(maxp<minq)
		return DONT_INTERSECT;

	minq=min(q1.y,q2.y);
	maxq=max(q1.y,q2.y);
	minp=min(p1.y,p2.y);
	maxp=max(p1.y,p2.y);

	if(minp>maxq)
		return DONT_INTERSECT;
	if(maxp<minq)
		return DONT_INTERSECT;

	// for each endpoint, compute which side of the other segment it lies
	// if both endpoints lie on the same side of the other segment,
	// the segments do not intersect
	int Pq1=RobustCGAlgorithms::orientationIndex(p1,p2,q1);
	int Pq2=RobustCGAlgorithms::orientationIndex(p1,p2,q2);

	if ((Pq1>0 && Pq2>0)||(Pq1<0 && Pq2<0)) {
		return DONT_INTERSECT;
	}

	int Qp1=RobustCGAlgorithms::orientationIndex(q1,q2,p1);
	int Qp2=RobustCGAlgorithms::orientationIndex(q1,q2,p2);

	if ((Qp1>0 && Qp2>0)||(Qp1<0 && Qp2<0)) {
		return DONT_INTERSECT;
	}

	/* OLD
	if (Pq1 > 0 && Pq2 > 0) {
	return DONT_INTERSECT;
	}
	if (Qp1 > 0 && Qp2 > 0) {
	return DONT_INTERSECT;
	}
	if (Pq1 < 0 && Pq2 < 0) {
	return DONT_INTERSECT;
	}
	if (Qp1 < 0 && Qp2 < 0) {
	return DONT_INTERSECT;
	}
	//*/

	bool collinear=Pq1==0 && Pq2==0 && Qp1==0 && Qp2==0;
	if (collinear) {
		return computeCollinearIntersection(p1,p2,q1,q2);
	}
	/**
	*  Check if the intersection is an endpoint. If it is, copy the endpoint as
	*  the intersection point. Copying the point rather than computing it
	*  ensures the point has the exact value, which is important for
	*  robustness. It is sufficient to simply check for an endpoint which is on
	*  the other line, since at this point we know that the inputLines must
	*  intersect.
	*/
	if (Pq1==0 || Pq2==0 || Qp1==0 || Qp2==0) {
		isProperVar=false;
		if (Pq1==0) {
			intPt[0].setCoordinate(q1);
		}
		if (Pq2==0) {
			intPt[0].setCoordinate(q2);
		}
		if (Qp1==0) {
			intPt[0].setCoordinate(p1);
		}
		if (Qp2==0) {
			intPt[0].setCoordinate(p2);
		}
	} else {
		isProperVar=true;
		intPt[0].setCoordinate(intersection(p1, p2, q1, q2));
	}
	return DO_INTERSECT;
}

bool RobustLineIntersector::between(Coordinate p1,Coordinate p2,Coordinate q) {
	if (((q.x>=min(p1.x,p2.x)) && (q.x<=max(p1.x,p2.x))) &&
		((q.y>=min(p1.y,p2.y)) && (q.y<=max(p1.y,p2.y)))) {
			return true;
	} else {
		return false;
	}
}

int RobustLineIntersector::computeCollinearIntersection(Coordinate p1,Coordinate p2,Coordinate q1,Coordinate q2) {
	bool p1q1p2=between(p1,p2,q1);
	bool p1q2p2=between(p1,p2,q2);
	bool q1p1q2=between(q1,q2,p1);
	bool q1p2q2=between(q1,q2,p2);

	if (p1q1p2 && p1q2p2) {
		intPt[0].setCoordinate(q1);
		intPt[1].setCoordinate(q2);
		return COLLINEAR;
	}
	if (q1p1q2 && q1p2q2) {
		intPt[0].setCoordinate(p1);
		intPt[1].setCoordinate(p2);
		return COLLINEAR;
	}
	if (p1q1p2 && q1p1q2) {
		intPt[0].setCoordinate(q1);
		intPt[1].setCoordinate(p1);
		return (q1==p1) && !p1q2p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q1p2 && q1p2q2) {
		intPt[0].setCoordinate(q1);
		intPt[1].setCoordinate(p2);
		return (q1==p2) && !p1q2p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p1q2) {
		intPt[0].setCoordinate(q2);
		intPt[1].setCoordinate(p1);
		return (q2==p1) && !p1q1p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p2q2) {
		intPt[0].setCoordinate(q2);
		intPt[1].setCoordinate(p2);
		return (q2==p2) && !p1q1p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	return DONT_INTERSECT;
}

Coordinate RobustLineIntersector::intersection(Coordinate p1,Coordinate p2,Coordinate q1,Coordinate q2) {
	//HCoordinate *l1=new HCoordinate(HCoordinate(p1),HCoordinate(p2));
	//HCoordinate *l2=new HCoordinate(HCoordinate(q1),HCoordinate(q2));
	//HCoordinate intHCoord(*l1,*l2);
	//Coordinate intPt;
	//intPt.setNull();
	//try {
	//	intPt.setCoordinate(intHCoord.getCoordinate());
	//} catch (NotRepresentableException e) {
	//	Assert::shouldNeverReachHere("Coordinate for intersection is not calculable");
	//}
	//if (makePrecise) {
	//	intPt.makePrecise();
	//}
	//return Coordinate(intPt);

	Coordinate *n1=new Coordinate(p1);
	Coordinate *n2=new Coordinate(p2);
	Coordinate *n3=new Coordinate(q1);
	Coordinate *n4=new Coordinate(q2);
	Coordinate *normPt=new Coordinate();
	normalize(n1, n2, n3, n4, normPt);
	Coordinate intPt;
	try {
		intPt.setCoordinate(HCoordinate::intersection(*n1,*n2,*n3,*n4));
	} catch (NotRepresentableException e) {
		Assert::shouldNeverReachHere("Coordinate for intersection is not calculable");
    }
	intPt.x+=normPt->x;
	intPt.y+=normPt->y;
	if (makePrecise) {
		intPt.makePrecise();
	}
	return Coordinate(intPt);
}

void RobustLineIntersector::normalize(Coordinate *n1,Coordinate *n2,Coordinate *n3,Coordinate *n4,Coordinate *normPt){
	normPt->x=smallestInAbsValue(n1->x,n2->x,n3->x,n4->x);
	normPt->y=smallestInAbsValue(n1->y,n2->y,n3->y,n4->y);
	n1->x-=normPt->x;
	n1->y-=normPt->y;
	n2->x-=normPt->x;
	n2->y-=normPt->y;
	n3->x-=normPt->x;
	n3->y-=normPt->y;
	n4->x-=normPt->x;
	n4->y-=normPt->y;
}

double RobustLineIntersector::smallestInAbsValue(double x1,double x2,double x3,double x4){
	double x=x1;
	double xabs=fabs(x);
	if(fabs(x2)<xabs) {
		x=x2;
		xabs=fabs(x2);
	}
	if(fabs(x3)<xabs) {
		x=x3;
		xabs=fabs(x3);
	}
	if(fabs(x4)<xabs) {
		x=x4;
	}
	return x;
}
