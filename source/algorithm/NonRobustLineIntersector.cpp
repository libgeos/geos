#include "../headers/geosAlgorithm.h"
#include "../headers/util.h"
#include "stdio.h"
#include "math.h"

namespace geos {

NonRobustLineIntersector::NonRobustLineIntersector(){}

void NonRobustLineIntersector::computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2) {
	double a1;
	double b1;
	double c1;
	/*
	*  Coefficients of line eqns.
	*/
	double r;
	/*
	*  'Sign' values
	*/
	isProperVar=false;
	/*
	*  Compute a1,b1,c1,where line joining points 1 and 2
	*  is "a1 x + b1 y + c1 = 0".
	*/
	a1=p2.y-p1.y;
	b1=p1.x-p2.x;
	c1=p2.x*p1.y-p1.x*p2.y;
	/*
	*  Compute r3 and r4.
	*/
	r=a1*p.x+b1*p.y+c1;

	// if r !=0 the point does not lie on the line
	if (r!=0) {
		result=DONT_INTERSECT;
		return;
	}

	// Point lies on line-check to see whether it lies in line segment.

	double dist=rParameter(p1,p2,p);
	if (dist<0.0 || dist>1.0) {
		result=DONT_INTERSECT;
		return;
	}

	isProperVar=true;
	if (p==p1 || p==p2) {
		isProperVar=false;
	}
	result=DO_INTERSECT;
}

int NonRobustLineIntersector::computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3,const Coordinate& p4){
	double a1;
	double b1;
	double c1;
	/*
	*  Coefficients of line eqns.
	*/
	double a2;
	/*
	*  Coefficients of line eqns.
	*/
	double b2;
	/*
	*  Coefficients of line eqns.
	*/
	double c2;
	double r1;
	double r2;
	double r3;
	double r4;
	/*
	*  'Sign' values
	*/
	//double denom,offset,num;    /* Intermediate values */

	isProperVar=false;

	/*
	*  Compute a1,b1,c1,where line joining points 1 and 2
	*  is "a1 x + b1 y + c1 = 0".
	*/
	a1=p2.y-p1.y;
	b1=p1.x-p2.x;
	c1=p2.x*p1.y-p1.x*p2.y;

	/*
	*  Compute r3 and r4.
	*/
	r3=a1*p3.x+b1*p3.y+c1;
	r4=a1*p4.x+b1*p4.y+c1;

	/*
	*  Check signs of r3 and r4.  If both point 3 and point 4 lie on
	*  same side of line 1,the line segments do not intersect.
	*/
	if (r3!=0 && r4!=0 && isSameSignAndNonZero(r3,r4)) {
		return DONT_INTERSECT;
	}

	/*
	*  Compute a2,b2,c2
	*/
	a2=p4.y-p3.y;
	b2=p3.x-p4.x;
	c2=p4.x*p3.y-p3.x*p4.y;

	/*
	*  Compute r1 and r2
	*/
	r1=a2*p1.x+b2*p1.y+c2;
	r2=a2*p2.x+b2*p2.y+c2;

	/*
	*  Check signs of r1 and r2.  If both point 1 and point 2 lie
	*  on same side of second line segment,the line segments do
	*  not intersect.
	*/
	if (r1!=0 && r2 !=0 && isSameSignAndNonZero(r1,r2)) {
		return DONT_INTERSECT;
	}

	/**
	*  Line segments intersect: compute intersection point.
	*/
	double denom=a1*b2-a2*b1;
	if (denom==0) {
		return computeCollinearIntersection(p1,p2,p3,p4);
	}
	double numX=b1*c2-b2*c1;
	pa.x=numX/denom;
	/*
	*  TESTING ONLY
	*  double valX=(( num<0 ? num-offset : num+offset ) / denom);
	*  double valXInt=(int) (( num<0 ? num-offset : num+offset ) / denom);
	*  if (valXInt !=pa.x)     // TESTING ONLY
	*  System.out.println(val+"-int: "+valInt+",floor: "+pa.x);
	*/
	double numY=a2*c1-a1*c2;
	pa.y=numY/denom;

	// check if this is a proper intersection BEFORE truncating values,
	// to avoid spurious equality comparisons with endpoints
	isProperVar=true;
	if (pa==p1 || pa==p2 || pa==p3 || pa==p4) {
		isProperVar=false;
	}

	// truncate computed point to precision grid
	// TESTING-don't force coord to be precise
	if (precisionModel!=NULL) {
		precisionModel->makePrecise(&pa);
	}
	return DO_INTERSECT;
}


/*
*  p1-p2  and p3-p4 are assumed to be collinear (although
*  not necessarily intersecting). Returns:
*  DONT_INTERSECT	: the two segments do not intersect
*  COLLINEAR		: the segments intersect,in the
*  line segment pa-pb.  pa-pb is in
*  the same direction as p1-p2
*  DO_INTERSECT		: the inputLines intersect in a single point
*  only,pa
*/
int NonRobustLineIntersector::computeCollinearIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3,const Coordinate& p4){
	double r1;
	double r2;
	double r3;
	double r4;
	Coordinate q3;
	Coordinate q4;
	double t3;
	double t4;
	r1=0;
	r2=1;
	r3=rParameter(p1,p2,p3);
	r4=rParameter(p1,p2,p4);
	// make sure p3-p4 is in same direction as p1-p2
	if (r3<r4) {
		q3=p3;
		t3=r3;
		q4=p4;
		t4=r4;
	} else {
		q3=p4;
		t3=r4;
		q4=p3;
		t4=r3;
	}
	// check for no intersection
	if (t3>r2 || t4<r1) {
		return DONT_INTERSECT;
	}

	// check for single point intersection
	if (q4==p1) {
		pa.setCoordinate(p1);
		return DO_INTERSECT;
	}
	if (q3==p2) {
		pa.setCoordinate(p2);
		return DO_INTERSECT;
	}

	// intersection MUST be a segment-compute endpoints
	pa.setCoordinate(p1);
	if (t3>r1) {
		pa.setCoordinate(q3);
	}
	pb.setCoordinate(p2);
	if (t4<r2) {
		pb.setCoordinate(q4);
	}
	return COLLINEAR;
}

/*
*  RParameter computes the parameter for the point p
*  in the parameterized equation
*  of the line from p1 to p2.  The 'distance' of p along p1-p2
*/
double NonRobustLineIntersector::rParameter(const Coordinate& p1,const Coordinate& p2,const Coordinate& p) const {
	double r;
	// compute maximum delta,for numerical stability
	// also handle case of p1-p2 being vertical or horizontal
	double dx=fabs(p2.x-p1.x);
	double dy=fabs(p2.y-p1.y);
	if (dx>dy) {
		r=(p.x-p1.x)/(p2.x-p1.x);
	} else {
		r=(p.y-p1.y)/(p2.y-p1.y);
	}
	return r;
}
}

