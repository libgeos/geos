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
 * Revision 1.16  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.15  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.14  2004/04/16 09:01:29  strk
 * Removed memory leak in CGAlgorithms::isOnline
 *
 * Revision 1.13  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.12  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.11  2004/02/27 17:42:15  strk
 * made CGAlgorithms::signedArea() and CGAlgorithms::length() arguments const-correct
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>
#include <stdio.h>
#include <math.h>

namespace geos {

/**
* Returns the index of the direction of the point <code>q</code>
* relative to a
* vector specified by <code>p1-p2</code>.
*
* @param p1 the origin point of the vector
* @param p2 the final point of the vector
* @param q the point to compute the direction to
*
* @return 1 if q is counter-clockwise (left) from p1-p2
* @return -1 if q is clockwise (right) from p1-p2
* @return 0 if q is collinear with p1-p2
*/
int CGAlgorithms::orientationIndex(const Coordinate& p1,const Coordinate& p2,const Coordinate& q) {
	// travelling along p1->p2, turn counter clockwise to get to q return 1,
	// travelling along p1->p2, turn clockwise to get to q return -1,
	// p1, p2 and q are colinear return 0.
	double dx1=p2.x-p1.x;
	double dy1=p2.y-p1.y;
	double dx2=q.x-p2.x;
	double dy2=q.y-p2.y;
	return RobustDeterminant::signOfDet2x2(dx1,dy1,dx2,dy2);
}

/**
* Test whether a point lies inside a ring.
* The ring may be oriented in either direction.
* If the point lies on the ring boundary the result of this method is unspecified.
* <p>
* This algorithm does not attempt to first check the point against the envelope
* of the ring.
*
* @param p point to check for ring inclusion
* @param ring assumed to have first point identical to last point
* @return <code>true</code> if p is inside ring
*/
bool CGAlgorithms::isPointInRing(const Coordinate& p, const CoordinateSequence* ring) {
	int i;
	int i1;       // point index; i1 = i-1
	double xInt;  // x intersection of segment with ray
	int crossings = 0;  // number of segment/ray crossings
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;
	int nPts=ring->getSize();
	/*
	*  For each segment l = (i-1, i), see if it crosses ray from test point in positive x direction.
	*/
	for(i=1;i<nPts;i++) {
		i1 = i - 1;
		Coordinate p1=ring->getAt(i);
		Coordinate p2=ring->getAt(i1);
		x1 = p1.x - p.x;
		y1 = p1.y - p.y;
		x2 = p2.x - p.x;
		y2 = p2.y - p.y;

		if (((y1 > 0) && (y2 <= 0)) ||
			((y2 > 0) && (y1 <= 0))) {
			/*
			*  segment straddles x axis, so compute intersection.
			*/
			xInt = RobustDeterminant::signOfDet2x2(x1, y1, x2, y2) / (y2 - y1);
			//xsave = xInt;
			/*
			*  crosses ray if strictly positive intersection.
			*/
			if (0.0 < xInt) {
				crossings++;
			}
		}
	}
	/*
	*  p is inside if number of crossings is odd.
	*/
	if ((crossings % 2) == 1) {
		return true;
	} else {
		return false;
	}
}

/**
* Test whether a point lies on a linestring.
*
* @return true true if
* the point is a vertex of the line or lies in the interior of a line
* segment in the linestring
*/
bool CGAlgorithms::isOnLine(const Coordinate& p, const CoordinateSequence* pt) {
	LineIntersector *lineIntersector=new RobustLineIntersector();
	for(int i=1;i<pt->getSize();i++) {
		Coordinate p0=pt->getAt(i-1);
		Coordinate p1=pt->getAt(i);	
		lineIntersector->computeIntersection(p, p0, p1);
		if (lineIntersector->hasIntersection()) {
			delete lineIntersector;
			return true;
		}
	}
	delete lineIntersector;
	return false;
}

/**
* Computes whether a ring defined by an array of {@link Coordinate} is
* oriented counter-clockwise.
* <p>
* This will handle coordinate lists which contain repeated points.
*
* @param ring an array of coordinates forming a ring
* @return <code>true</code> if the ring is oriented counter-clockwise.
* @throws IllegalArgumentException if the ring is degenerate (does not contain 3 distinct points)
*/
bool CGAlgorithms::isCCW(const CoordinateSequence* ring) {
	// # of points without closing endpoint
	int nPts=ring->getSize()-1;
	// find highest point
	Coordinate hip=ring->getAt(0);
	int hii=0;
	for (int i=1;i<=nPts;i++) {
		Coordinate p=ring->getAt(i);
		if (p.y > hip.y) {
			hip = p;
			hii = i;
		}
	}
	// find distinct point before highest point
	int iPrev = hii;
	do {
		iPrev = iPrev - 1;
		if (iPrev < 0) iPrev = nPts;
	} while (ring->getAt(iPrev)==hip && iPrev!=hii);
	// find distinct point after highest point
	int iNext = hii;
	do {
		iNext = (iNext + 1) % nPts;
	} while (ring->getAt(iNext)==hip && iNext != hii);
	Coordinate prev=ring->getAt(iPrev);
	Coordinate next=ring->getAt(iNext);
	// this will catch all cases where there are not 3 distinct points,
	// including the case where the input array has fewer than 4 elements
	if (prev==hip || next==hip || prev==next)
		throw new IllegalArgumentException("degenerate ring (does not contain 3 distinct points)");
	int disc = computeOrientation(prev, hip, next);

	/**
	*  If disc is exactly 0, lines are collinear.  There are two possible cases:
	*  (1) the lines lie along the x axis in opposite directions
	*  (2) the lines lie on top of one another
	*
	*  (1) is handled by checking if next is left of prev ==> CCW
	*  (2) should never happen, so we're going to ignore it!
	*  (Might want to assert this)
	*/
	bool isCCW=false;
	if (disc == 0) {
		// poly is CCW if prev x is right of next x
		isCCW = (prev.x > next.x);
	} else {
		// if area is positive, points are ordered CCW
		isCCW = (disc > 0);
	}
	return isCCW;
}

/**
* Computes the orientation of a point q to the directed line segment p1-p2.
* The orientation of a point relative to a directed line segment indicates
* which way you turn to get to q after travelling from p1 to p2.
*
* @return 1 if q is counter-clockwise from p1-p2
* @return -1 if q is clockwise from p1-p2
* @return 0 if q is collinear with p1-p2
*/
int CGAlgorithms::computeOrientation(const Coordinate& p1, const Coordinate& p2, const Coordinate& q) {
	return orientationIndex(p1,p2,q);
}

/**
* Computes the distance from a point p to a line segment AB
*
* Note: NON-ROBUST!
*
* @param p the point to compute the distance for
* @param A one point of the line
* @param B another point of the line (must be different to A)
* @return the distance from p to line segment AB
*/
double
CGAlgorithms::distancePointLine(const Coordinate& p, const Coordinate& A,
		const Coordinate& B)
{
	//if start==end, then use pt distance
	if (A==B) return p.distance(A);

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
	if (r<=0.0) return p.distance(A);
	if (r>=1.0) return p.distance(B);
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
* Computes the perpendicular distance from a point p
* to the (infinite) line containing the points AB
*
* @param p the point to compute the distance for
* @param A one point of the line
* @param B another point of the line (must be different to A)
* @return the distance from p to line AB
*/
double CGAlgorithms::distancePointLinePerpendicular(const Coordinate& p,const Coordinate& A,const Coordinate& B) {
    // use comp.graphics.algorithms Frequently Asked Questions method
    /*(2)
                     (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
                s = -----------------------------
                                     L^2

                Then the distance from C to P = |s|*L.
        */

	double s = ((A.y - p.y) *(B.x - A.x) - (A.x - p.x)*(B.y - A.y) )
              /
            ((B.x - A.x) * (B.x - A.x) + (B.y - A.y) * (B.y - A.y) );
    return fabs(s)*sqrt(((B.x - A.x) * (B.x - A.x) + (B.y - A.y) * (B.y - A.y)));
}

/**
* Computes the distance from a line segment AB to a line segment CD
*
* Note: NON-ROBUST!
*
* @param A a point of one line
* @param B the second point of  (must be different to A)
* @param C one point of the line
* @param D another point of the line (must be different to A)
*/
double
CGAlgorithms::distanceLineLine(const Coordinate& A, const Coordinate& B,
		const Coordinate& C, const Coordinate& D)
{
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
double CGAlgorithms::signedArea(const CoordinateSequence* ring) {
	if (ring->getSize()<3) return 0.0;
	double sum=0.0;
	for (int i=0;i<ring->getSize()-1;i++) {
		double bx=ring->getAt(i).x;
		double by=ring->getAt(i).y;
		double cx=ring->getAt(i+1).x;
		double cy=ring->getAt(i+1).y;
		sum+=(bx+cx)*(cy-by);
	}
	return -sum/2.0;
}

/**
* Returns the length of a list of line segments.
*/
double CGAlgorithms::length(const CoordinateSequence* pts) {
	if (pts->getSize()<1) return 0.0;
	double sum=0.0;
	for(int i=1;i<pts->getSize();i++) {
		sum+=pts->getAt(i).distance(pts->getAt(i - 1));
	}
	return sum;
}




}

