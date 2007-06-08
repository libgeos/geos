/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithms.java rev. 1.34 (JTS-1.7.1)
 *
 **********************************************************************/

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/RobustDeterminant.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>

#include <algorithm>
//#include <cstdio>
#include <cmath>

using namespace std;

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/*public static*/
int
CGAlgorithms::orientationIndex(const Coordinate& p1,const Coordinate& p2,const Coordinate& q)
{
	// travelling along p1->p2, turn counter clockwise to get to q return 1,
	// travelling along p1->p2, turn clockwise to get to q return -1,
	// p1, p2 and q are colinear return 0.
	double dx1=p2.x-p1.x;
	double dy1=p2.y-p1.y;
	double dx2=q.x-p2.x;
	double dy2=q.y-p2.y;
	return RobustDeterminant::signOfDet2x2(dx1,dy1,dx2,dy2);
}

/*public static*/
bool
CGAlgorithms::isPointInRing(const Coordinate& p, const CoordinateSequence* ring)
{
	double xInt;  // x intersection of segment with ray
	int crossings = 0;  // number of segment/ray crossings
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;

	/*
	 * For each segment l = (i-1, i), see if it crosses ray from
	 * test point in positive x direction.
	 */
	size_t nPts=ring->getSize();
	for(size_t i=1; i<nPts; i++)
	{
		const Coordinate &p1=ring->getAt(i);
		const Coordinate &p2=ring->getAt(i-1);
		x1 = p1.x - p.x;
		y1 = p1.y - p.y;
		x2 = p2.x - p.x;
		y2 = p2.y - p.y;

		if (((y1 > 0) && (y2 <= 0)) ||
			((y2 > 0) && (y1 <= 0)))
		{
			/*
			 *  segment straddles x axis, so compute intersection.
			 */
			xInt = RobustDeterminant::signOfDet2x2(x1, y1, x2, y2)
				/ (y2 - y1);

			/*
			 *  crosses ray if strictly positive intersection.
			 */
			if (0.0 < xInt) crossings++;
		}
	}

	/*
	 *  p is inside if number of crossings is odd.
	 */
	if ((crossings % 2) == 1) return true;
	return false;
}

/*public static*/
bool
CGAlgorithms::isPointInRing(const Coordinate& p,
		const Coordinate::ConstVect& ring)
{
	double xInt;  // x intersection of segment with ray
	int crossings = 0;  // number of segment/ray crossings
	double x1;    // translated coordinates
	double y1;
	double x2;
	double y2;

	/*
	 * For each segment l = (i-1, i), see if it crosses ray from
	 * test point in positive x direction.
	 */
	for(size_t i=1, nPts=ring.size(); i<nPts; ++i)
	{
		const Coordinate *p1=ring[i];
		const Coordinate *p2=ring[i-1];
		x1 = p1->x - p.x;
		y1 = p1->y - p.y;
		x2 = p2->x - p.x;
		y2 = p2->y - p.y;

		if (((y1 > 0) && (y2 <= 0)) ||
			((y2 > 0) && (y1 <= 0)))
		{
			/*
			 *  segment straddles x axis, so compute intersection.
			 */
			xInt = RobustDeterminant::signOfDet2x2(x1, y1, x2, y2)
				/ (y2 - y1);

			/*
			 *  crosses ray if strictly positive intersection.
			 */
			if (0.0 < xInt) crossings++;
		}
	}

	/*
	 *  p is inside if number of crossings is odd.
	 */
	if ((crossings % 2) == 1) return true;
	return false;
}

/*public static*/
bool
CGAlgorithms::isOnLine(const Coordinate& p, const CoordinateSequence* pt)
{
	//LineIntersector lineIntersector;
	size_t ptsize = pt->getSize();
	if ( ptsize == 0 ) return false;

	const Coordinate *pp=&(pt->getAt(0));
	for(size_t i=1; i<ptsize; ++i)
	{
		const Coordinate &p1=pt->getAt(i);	
		if ( LineIntersector::hasIntersection(p, *pp, p1) )
			return true;
		pp=&p1;
	}
	return false;
}

/*public static*/
bool
CGAlgorithms::isCCW(const CoordinateSequence* ring)
{
	// # of points without closing endpoint
    const std::size_t nPts=ring->getSize()-1;

	// find highest point
	const Coordinate *hiPt=&ring->getAt(0);
	int hiIndex=0;
    for (std::size_t i=1; i<=nPts; ++i)
	{
		const Coordinate *p=&ring->getAt(i);
		if (p->y > hiPt->y) {
			hiPt = p;
			hiIndex = static_cast<int>(i);
		}
	}

	// find distinct point before highest point
	int iPrev = hiIndex;
	do {
		iPrev = iPrev - 1;
		if (iPrev < 0)
            iPrev = static_cast<int>(nPts);
	} while (ring->getAt(iPrev)==*hiPt && iPrev!=hiIndex);

	// find distinct point after highest point
	int iNext = hiIndex;
	do {
		iNext = (iNext + 1) % static_cast<int>(nPts);
	} while (ring->getAt(iNext)==*hiPt && iNext != hiIndex);

	const Coordinate *prev=&ring->getAt(iPrev);
	const Coordinate *next=&ring->getAt(iNext);

	/*
	 * This check catches cases where the ring contains an A-B-A
	 * configuration of points.
	 * This can happen if the ring does not contain 3 distinct points
	 * (including the case where the input array has fewer than 4 elements),
	 * or it contains coincident line segments.
	 */
	if ( prev->equals2D(*hiPt) || next->equals2D(*hiPt) ||
		prev->equals2D(*next) )
	{
		return false;
		// MD - don't bother throwing exception,
		// since this isn't a complete check for ring validity
		//throw  IllegalArgumentException("degenerate ring (does not contain 3 distinct points)");
	}

	int disc = computeOrientation(*prev, *hiPt, *next);

	/**
	 *  If disc is exactly 0, lines are collinear. 
	 * There are two possible cases:
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
		isCCW = (prev->x > next->x);
	} else {
		// if area is positive, points are ordered CCW
		isCCW = (disc > 0);
	}

	return isCCW;
}

/*public static*/
int
CGAlgorithms::computeOrientation(const Coordinate& p1, const Coordinate& p2,
		const Coordinate& q)
{
	return orientationIndex(p1,p2,q);
}

/*public static*/
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

/*public static*/
double
CGAlgorithms::distancePointLinePerpendicular(const Coordinate& p,const Coordinate& A,const Coordinate& B)
{
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

/*public static*/
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
		return std::min(distancePointLine(A,C,D),
						std::min(distancePointLine(B,C,D),
						std::min(distancePointLine(C,A,B), distancePointLine(D,A,B))));
	}
	double s=s_top/s_bot;
	double r=r_top/r_bot;
	if ((r<0)||( r>1)||(s<0)||(s>1)) {
		//no intersection
		return std::min(distancePointLine(A,C,D),
						std::min(distancePointLine(B,C,D),
						std::min(distancePointLine(C,A,B), distancePointLine(D,A,B))));
	}
	return 0.0; //intersection exists
}

/*public static*/
double
CGAlgorithms::signedArea(const CoordinateSequence* ring)
{
	size_t npts=ring->getSize();

	if (npts<3) return 0.0;

	double sum=0.0;
	for (size_t i=0; i<npts-1; ++i)
	{
		double bx=ring->getAt(i).x;
		double by=ring->getAt(i).y;
		double cx=ring->getAt(i+1).x;
		double cy=ring->getAt(i+1).y;
		sum+=(bx+cx)*(cy-by);
	}
	return -sum/2.0;
}

/*public static*/
double
CGAlgorithms::length(const CoordinateSequence* pts)
{
	size_t npts=pts->getSize();

	if (npts<1) return 0.0;

	double sum=0.0;

	for(size_t i=1; i<npts; ++i)
	{
		sum+=pts->getAt(i).distance(pts->getAt(i - 1));
	}
	return sum;
}


} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.33  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.32  2006/05/02 14:51:53  strk
 * Added port info and fixed doxygen comments for CGAlgorithms class
 *
 * Revision 1.31  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.30  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

