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
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/util.h>
#include <geos/geom.h>
#include <cmath>

#ifndef DEBUG_INTERSECT
#define DEBUG_INTERSECT 0
#endif
#define DEBUG 0

namespace geos {

/**
 * Computes the "edge distance" of an intersection point p in an edge.
 * The edge distance is a metric of the point along the edge.
 * The metric used is a robust and easy to compute metric function.
 * It is <b>not</b> equivalent to the usual Euclidean metric.
 * It relies on the fact that either the x or the y ordinates of the
 * points in the edge are unique, depending on whether the edge is longer in
 * the horizontal or vertical direction.
 * 
 * NOTE: This function may produce incorrect distances
 *  for inputs where p is not precisely on p1-p2
 * (E.g. p = (139,9) p1 = (139,10), p2 = (280,1) produces distanct
 * 0.0, which is incorrect.
 * 
 * My hypothesis is that the function is safe to use for points which are the
 * result of <b>rounding</b> points which lie on the line,
 * but not safe to use for <b>truncated</b> points.
 */
double
LineIntersector::computeEdgeDistance(const Coordinate& p,const Coordinate& p0,const Coordinate& p1)
{
	double dx=fabs(p1.x-p0.x);
	double dy=fabs(p1.y-p0.y);
	double dist=-1.0;	// sentinel value
	if (p==p0) {
		dist=0.0;
	} else if (p==p1) {
		if (dx>dy)
			dist=dx;
		else
			dist=dy;
	} else {
		double pdx=fabs(p.x - p0.x);
		double pdy=fabs(p.y - p0.y);
		if (dx > dy)
			dist = pdx;
		else
			dist = pdy;
		// <FIX>
		// hack to ensure that non-endpoints always have a non-zero distance
		if (dist == 0.0 && !(p==p0)) {
			dist=max(pdx,pdy);
		}
	}
	Assert::isTrue(!(dist == 0.0 && !(p==p0)), "Bad distance calculation");
	return dist;
}

/*
 * This function is non-robust, since it may compute the square
 * of large numbers.
 * Currently not sure how to improve this.
 */
double
LineIntersector::nonRobustComputeEdgeDistance(const Coordinate& p,const Coordinate& p1,const Coordinate& p2)
{
	double dx=p.x-p1.x;
	double dy=p.y-p1.y;
	double dist=sqrt(dx*dx+dy*dy);   // dummy value
	Assert::isTrue(!(dist==0.0 && !(p==p1)), "Invalid distance calculation");
	return dist;
}

LineIntersector::LineIntersector(): pa(intPt[0]), pb(intPt[1])
{
	precisionModel=NULL;
	// alias the intersection points for ease of reference
	//pa=intPt[0];
	//pb=intPt[1];
	result=0;
}

LineIntersector::~LineIntersector()
{
}

/**
 * Force computed intersection to be rounded to a given precision model
 * @param newPM
 * @deprecated use <code>setPrecisionModel</code> instead
 */
void
LineIntersector::setMakePrecise(const PrecisionModel *newPM)
{
	precisionModel=newPM;
}

/**
 * Force computed intersection to be rounded to a given precision model.
 * No getter is provided, because the precision model is not required
 * to be specified.
 * @param precisionModel
 */
void
LineIntersector::setPrecisionModel(const PrecisionModel *newPM)
{
	precisionModel=newPM;
}

bool
LineIntersector::isCollinear() const
{
	return result==COLLINEAR;
}

/*
 * Computes the intersection of the lines p1-p2 and p3-p4
 */
void
LineIntersector::computeIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3,const Coordinate& p4)
{
	inputLines[0][0]=&p1;
	inputLines[0][1]=&p2;
	inputLines[1][0]=&p3;
	inputLines[1][1]=&p4;
	result=computeIntersect(p1,p2,p3,p4);
	//numIntersects++;
}

string
LineIntersector::toString() const
{
	string str=inputLines[0][0]->toString()+"_"
			  +inputLines[0][1]->toString()+" "
			  +inputLines[1][0]->toString()+"_"
			  +inputLines[1][1]->toString()+" : ";
	if (isEndPoint()) {
		str+=" endpoint";
	}
	if (isProperVar) {
		str+=" proper";
	}
	if (isCollinear()) {
		str+=" collinear";
	}
	return str;
}

bool
LineIntersector::isEndPoint() const
{
	return hasIntersection()&&!isProperVar;
}

/**
 * Tests whether the input geometries intersect.
 *
 * @return true if the input geometries intersect
 */
bool
LineIntersector::hasIntersection() const
{
	return result!=DONT_INTERSECT;
}

/**
 * Returns the number of intersection points found.
 * This will be either 0, 1 or 2.
 */
int
LineIntersector::getIntersectionNum() const
{
	return result;
}

/**
 * Returns the intIndex'th intersection point
 *
 * @param intIndex is 0 or 1
 *
 * @return the intIndex'th intersection point
 */
const Coordinate&
LineIntersector::getIntersection(int intIndex) const
{
#if DEBUG_INTERSECT
	cerr<<"LineIntersector::getIntersection("<<intIndex<<"): "<<intPt[intIndex].toString()<<endl;
#endif
	return intPt[intIndex];
}

/**
 * @return true if both numbers are positive or if both numbers are negative.
 * Returns false if both numbers are zero.
 */
bool
LineIntersector::isSameSignAndNonZero(double a,double b)
{
	if (a==0 || b==0) {
		return false;
	}
	return (a<0 && b<0) || (a>0 && b>0);
}

void
LineIntersector::computeIntLineIndex() {
//	if (intLineIndex==null) {
//	intLineIndex=new int[2][2];
	computeIntLineIndex(0);
	computeIntLineIndex(1);
//}
}

/**
 * Test whether a point is a intersection point of two line segments.
 * Note that if the intersection is a line segment, this method only tests for
 * equality with the endpoints of the intersection segment.
 * It does <b>not</b> return true if
 * the input point is internal to the intersection segment.
 *
 * @return true if the input point is one of the intersection points.
 */
bool
LineIntersector::isIntersection(const Coordinate& pt) const
{
	for (int i=0;i<result;i++) {
		if (intPt[i].equals2D(pt)) {
			return true;
		}
	}
	return false;
}

/**
 * Tests whether an intersection is proper.
 * 
 * The intersection between two line segments is considered proper if
 * they intersect in a single point in the interior of both segments
 * (e.g. the intersection is a single point and is not equal to any of the
 * endpoints).
 * 
 * The intersection between a point and a line segment is considered proper
 * if the point lies in the interior of the segment (e.g. is not equal to
 * either of the endpoints).
 *
 * @return true if the intersection is proper
 */
bool
LineIntersector::isProper() const
{
	return hasIntersection()&&isProperVar;
}

/**
 * Computes the intIndex'th intersection point in the direction of
 * a specified input line segment
 *
 * @param segmentIndex is 0 or 1
 * @param intIndex is 0 or 1
 *
 * @return the intIndex'th intersection point in the direction of the
 *         specified input line segment
 */
const Coordinate&
LineIntersector::getIntersectionAlongSegment(int segmentIndex,int intIndex)
{
	// lazily compute int line array
	computeIntLineIndex();
	return intPt[intLineIndex[segmentIndex][intIndex]];
}

/**
 * Computes the index of the intIndex'th intersection point in the direction of
 * a specified input line segment
 *
 * @param segmentIndex is 0 or 1
 * @param intIndex is 0 or 1
 *
 * @return the index of the intersection point along the segment (0 or 1)
 */
int
LineIntersector::getIndexAlongSegment(int segmentIndex,int intIndex)
{
	computeIntLineIndex();
	return intLineIndex[segmentIndex][intIndex];
}

void
LineIntersector::computeIntLineIndex(int segmentIndex)
{
	double dist0=getEdgeDistance(segmentIndex,0);
	double dist1=getEdgeDistance(segmentIndex,1);
	if (dist0>dist1) {
		intLineIndex[segmentIndex][0]=0;
		intLineIndex[segmentIndex][1]=1;
	} else {
		intLineIndex[segmentIndex][0]=1;
		intLineIndex[segmentIndex][1]=0;
	}
}

/**
 * Computes the "edge distance" of an intersection point along the specified
 * input line segment.
 *
 * @param segmentIndex is 0 or 1
 * @param intIndex is 0 or 1
 *
 * @return the edge distance of the intersection point
 */
double
LineIntersector::getEdgeDistance(int segmentIndex,int intIndex) const
{
	double dist=computeEdgeDistance(intPt[intIndex],
		*inputLines[segmentIndex][0],
		*inputLines[segmentIndex][1]);
	return dist;
}

/**
 * Tests whether either intersection point is an interior point of one of
 * the input segments.
 *
 * @return <code>true</code> if either intersection point is in the interior
 * of one of the input segments
 */
bool
LineIntersector::isInteriorIntersection()
{
	if (isInteriorIntersection(0)) return true;
	if (isInteriorIntersection(1)) return true;
	return false;
}

/**
 * Tests whether either intersection point is an interior point of the
 * specified input segment.
 *
 * @return <code>true</code> if either intersection point is in the interior
 * of the input segment
 */
bool
LineIntersector::isInteriorIntersection(int inputLineIndex)
{
	for (int i=0; i<result; i++)
	{
		if (!(intPt[i].equals2D(*inputLines[inputLineIndex][0])
            		|| intPt[i].equals2D(*inputLines[inputLineIndex][1])))
	    	{
			return true;
		}
	}
	return false;
}

double
LineIntersector::interpolateZ(const Coordinate &p,
	const Coordinate &p1, const Coordinate &p2)
{
#if DEBUG
	cerr<<"LineIntersector::interpolateZ("<<p.toString()<<", "<<p1.toString()<<", "<<p2.toString()<<")"<<endl;
#endif

	if ( ISNAN(p1.z) )
	{
#if DEBUG
		cerr<<" p1 do not have a Z"<<endl;
#endif
		return p2.z; // might be DoubleNotANumber again
	}

	if ( ISNAN(p2.z) )
	{
#if DEBUG
		cerr<<" p2 do not have a Z"<<endl;
#endif
		return p1.z; // might be DoubleNotANumber again
	}

	if (p==p1)
	{
#if DEBUG
		cerr<<" p==p1, returning "<<p1.z<<endl;
#endif
		return p1.z;
	}
	if (p==p2)
	{
#if DEBUG
		cerr<<" p==p2, returning "<<p2.z<<endl;
#endif
		return p2.z;
	}

	//double zgap = fabs(p2.z - p1.z);
	double zgap = p2.z - p1.z;
	if ( ! zgap )
	{
#if DEBUG
		cerr<<" no zgap, returning "<<p2.z<<endl;
#endif
		return p2.z;
	}
	double xoff = (p2.x-p1.x);
	double yoff = (p2.y-p1.y);
	double seglen = (xoff*xoff+yoff*yoff);
	xoff = (p.x-p1.x);
	yoff = (p.y-p1.y);
	double pdist = (xoff*xoff+yoff*yoff);
	double fract = sqrt(pdist/seglen);
	double zoff = zgap*fract;
	//double interpolated = p1.z < p2.z ? p1.z+zoff : p1.z-zoff;
	double interpolated = p1.z+zoff;
#if DEBUG
	cerr<<" zgap:"<<zgap<<" seglen:"<<seglen<<" pdist:"<<pdist
		<<" fract:"<<fract<<" z:"<<interpolated<<endl;
#endif
	return interpolated;

}


} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.23  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.22  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.21  2005/01/18 17:22:03  strk
 * reverted previous change, sign was actually stored in zgap
 *
 * Revision 1.20  2005/01/14 08:04:38  strk
 * Fixed Z interpolation to use the correct sign
 *
 * Revision 1.19  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.18  2004/11/26 09:53:48  strk
 * Added more FINITE calls, and added inf and -inf to FINITE checks
 *
 * Revision 1.17  2004/11/24 18:10:23  strk
 * Cleanup of interpolateZ
 *
 * Revision 1.16  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.15  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.14  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.13  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.12  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

