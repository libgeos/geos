#include "geosAlgorithm.h"
#include "stdio.h"
#include "math.h"

/**
* Computes the distance from a point p to a line segment AB
*
* Note: NON-ROBUST!
*/
double CGAlgorithms::distancePointLine(Coordinate p,Coordinate A,Coordinate B){
	//if start==end, then use pt distance
	if (A==B) return A.distance(B);

    // otherwise use comp.graphics.algorithms Frequently Asked Questions method
    /*(1)     	      AC dot AB
                   r = ---------
                         ||AB||^2
		r has the following meaning:
		r=0 P = A
		r=1 P = B
		r<0 P is on the backward extension of AB
		r>1 P is on the forward extension of AB
		0<r<1 P is interior to AB
	*/
	double r=((p.x-A.x)*(B.x-A.x)+(p.y-A.y)*(B.y-A.y))/
			 ((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y));
	if (r<0) return p.distance(A);
	if (r>1) return p.distance(B);
    /*(2)
		     (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
		s = -----------------------------
		             	L^2

		Then the distance from C to P = |s|*L.
	*/
	double s=((A.y-p.y)*(B.x-A.x)-(A.x-p.x)*(B.y-A.y))/
			 ((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y));
	return fabs(s)*sqrt(((B.x-A.x)*(B.x-A.x)+(B.y-A.y)*(B.y-A.y)));
}

/**
* Computes the distance from a line segment AB to a line segment CD
*
* Note: NON-ROBUST!
*/
double CGAlgorithms::distanceLineLine(Coordinate A,Coordinate B,Coordinate C,Coordinate D){
	// check for zero-length segments
	if (A==B) return distancePointLine(A,C,D);
	if (C==D) return distancePointLine(D,A,B);

    // AB and CD are line segments
    /* from comp.graphics.algo

	Solving the above for r and s yields
				(Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
	           r = ----------------------------- (eqn 1)
				(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)

		 	(Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
		s = ----------------------------- (eqn 2)
			(Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
	Let P be the position vector of the intersection point, then
		P=A+r(B-A) or
		Px=Ax+r(Bx-Ax)
		Py=Ay+r(By-Ay)
	By examining the values of r & s, you can also determine some other
limiting conditions:
		If 0<=r<=1 & 0<=s<=1, intersection exists
		r<0 or r>1 or s<0 or s>1 line segments do not intersect
		If the denominator in eqn 1 is zero, AB & CD are parallel
		If the numerator in eqn 1 is also zero, AB & CD are collinear.

	*/
	double r_top=(A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
	double r_bot=(B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
	double s_top=(A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);
	double s_bot=(B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);
	if ((r_bot==0)||(s_bot==0)) {
		return min(distancePointLine(A,C,D),min(distancePointLine(B,C,D),min(distancePointLine(C,A,B),distancePointLine(D,A,B))));
	}
	double s=s_top/s_bot;
	double r=r_top/r_bot;
	if ((r<0)||( r>1)||(s<0)||(s>1)) {
		//no intersection
		return min(distancePointLine(A,C,D),min(distancePointLine(B,C,D),min(distancePointLine(C,A,B),distancePointLine(D,A,B))));
	}
	return 0.0; //intersection exists
}

/**
* Returns the signed area for a ring.  The area is positive if
* the ring is oriented CW.
*/
double CGAlgorithms::signedArea(CoordinateList* ring) {
	if (ring->getSize()<3) return 0.0;
	double sum=0.0;
	double ax=ring->getAt(0).x;
	double ay=ring->getAt(0).y;
	for (int i=1;i<ring->getSize()-1;i++) {
		double bx=ring->getAt(i).x;
		double by=ring->getAt(i).y;
		double cx=ring->getAt(i+1).x;
		double cy=ring->getAt(i+1).y;
		sum+=ax*by-ay*bx+ay*cx-ax*cy+bx*cy-cx*by;
	}
	return -sum/2.0;
}

/**
* Returns the length of a list of line segments.
*/
double CGAlgorithms::length(CoordinateList* pts) {
	if (pts->getSize()<1) return 0.0;
	double sum=0.0;
	for(int i=1;i<pts->getSize();i++) {
		sum+=pts->getAt(i).distance(pts->getAt(i - 1));
	}
	return sum;
}
