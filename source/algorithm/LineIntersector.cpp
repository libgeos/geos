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

//#define DEBUG 1

#ifndef COMPUTE_Z
#define COMPUTE_Z 1
#endif // COMPUTE_Z

namespace geos {

LineIntersector::LineIntersector(): pa(intPt[0]), pb(intPt[1])
{
	precisionModel=NULL;
	result=0;
}

LineIntersector::~LineIntersector(){}

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



void
LineIntersector::computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2)
{
	isProperVar=false;

	// do between check first, since it is faster than the orientation test
	if(Envelope::intersects(p1,p2,p)) {
		if ((CGAlgorithms::orientationIndex(p1,p2,p)==0)&&
			(CGAlgorithms::orientationIndex(p2,p1,p)==0)) {
			isProperVar=true;
			if ((p==p1)||(p==p2)) // 2d only test
			{
				isProperVar=false;
			}
#if COMPUTE_Z
			intPt[0].setCoordinate(p);
#if DEBUG
			cerr<<"RobustIntersector::computeIntersection(Coordinate,Coordinate,Coordinate) calling interpolateZ"<<endl;
#endif
			double z = interpolateZ(p, p1, p2);
			if ( !ISNAN(z) )
			{
				if ( ISNAN(intPt[0].z) )
					intPt[0].z = z;
				else
					intPt[0].z = (intPt[0].z+z)/2;
			}
#endif // COMPUTE_Z
			result=DO_INTERSECT;
			return;
		}
	}
	result = DONT_INTERSECT;
}

bool
LineIntersector::hasIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2)
{
	if(Envelope::intersects(p1,p2,p)) {
		if ((CGAlgorithms::orientationIndex(p1,p2,p)==0)&&
			(CGAlgorithms::orientationIndex(p2,p1,p)==0)) {
			return true;
		}
	}
	return false;
}

int
LineIntersector::computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)
{
#if DEBUG
	cerr<<"LineIntersector::computeIntersect called"<<endl;
	cerr<<" p1:"<<p1.toString()<<" p2:"<<p2.toString()<<" q1:"<<q1.toString()<<" q2:"<<q2.toString()<<endl;
#endif // DEBUG

	isProperVar=false;

	// first try a fast test to see if the envelopes of the lines intersect
	if (!Envelope::intersects(p1,p2,q1,q2))
	{
#if DEBUG
		cerr<<" DONT_INTERSECT"<<endl;
#endif
		return DONT_INTERSECT;
	}

	// for each endpoint, compute which side of the other segment it lies
	// if both endpoints lie on the same side of the other segment,
	// the segments do not intersect
	int Pq1=CGAlgorithms::orientationIndex(p1,p2,q1);
	int Pq2=CGAlgorithms::orientationIndex(p1,p2,q2);

	if ((Pq1>0 && Pq2>0) || (Pq1<0 && Pq2<0)) 
	{
#if DEBUG
		cerr<<" DONT_INTERSECT"<<endl;
#endif
		return DONT_INTERSECT;
	}

	int Qp1=CGAlgorithms::orientationIndex(q1,q2,p1);
	int Qp2=CGAlgorithms::orientationIndex(q1,q2,p2);

	if ((Qp1>0 && Qp2>0)||(Qp1<0 && Qp2<0)) {
#if DEBUG
		cerr<<" DONT_INTERSECT"<<endl;
#endif
		return DONT_INTERSECT;
	}

	bool collinear=Pq1==0 && Pq2==0 && Qp1==0 && Qp2==0;
	if (collinear) {
#if DEBUG
		cerr<<" computingCollinearIntersection"<<endl;
#endif
		return computeCollinearIntersection(p1,p2,q1,q2);
	}

	/*
	 * Check if the intersection is an endpoint.
	 * If it is, copy the endpoint as
	 * the intersection point. Copying the point rather than
	 * computing it ensures the point has the exact value,
	 * which is important for robustness. It is sufficient to
	 * simply check for an endpoint which is on the other line,
	 * since at this point we know that the inputLines must
	 *  intersect.
	 */
	if (Pq1==0 || Pq2==0 || Qp1==0 || Qp2==0) {
#if COMPUTE_Z
		int hits=0;
		double z=0.0;
#endif
		isProperVar=false;
		if (Pq1==0) {
			intPt[0].setCoordinate(q1);
#if COMPUTE_Z
			if ( !ISNAN(q1.z) )
			{
				z += q1.z;
				hits++;
			}
#endif
		}
		if (Pq2==0) {
			intPt[0].setCoordinate(q2);
#if COMPUTE_Z
			if ( !ISNAN(q2.z) )
			{
				z += q2.z;
				hits++;
			}
#endif
		}
		if (Qp1==0) {
			intPt[0].setCoordinate(p1);
#if COMPUTE_Z
			if ( !ISNAN(p1.z) )
			{
				z += p1.z;
				hits++;
			}
#endif
		}
		if (Qp2==0) {
			intPt[0].setCoordinate(p2);
#if COMPUTE_Z
			if ( !ISNAN(p2.z) )
			{
				z += p2.z;
				hits++;
			}
#endif
		}
#if COMPUTE_Z
#if DEBUG
		cerr<<"LineIntersector::computeIntersect: z:"<<z<<" hits:"<<hits<<endl;
#endif // DEBUG
		if ( hits ) intPt[0].z = z/hits;
#endif // COMPUTE_Z
	} else {
		isProperVar=true;
		Coordinate *c=intersection(p1, p2, q1, q2);
		intPt[0].setCoordinate(*c);
		delete c;
	}
#if DEBUG
	cerr<<" DO_INTERSECT; intPt[0]:"<<intPt[0].toString()<<endl;
#endif // DEBUG
	return DO_INTERSECT;
}

//bool LineIntersector::intersectsEnvelope(Coordinate& p1,Coordinate& p2,Coordinate& q) {
//	if (((q.x>=min(p1.x,p2.x)) && (q.x<=max(p1.x,p2.x))) &&
//		((q.y>=min(p1.y,p2.y)) && (q.y<=max(p1.y,p2.y)))) {
//			return true;
//	} else {
//		return false;
//	}
//}

int
LineIntersector::computeCollinearIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)
{
#if COMPUTE_Z
	double ztot;
	int hits;
	double p2z;
	double p1z;
	double q1z;
	double q2z;
#endif // COMPUTE_Z

#if DEBUG
	cerr<<"LineIntersector::computeCollinearIntersection called"<<endl;
	cerr<<" p1:"<<p1.toString()<<" p2:"<<p2.toString()<<" q1:"<<q1.toString()<<" q2:"<<q2.toString()<<endl;
#endif // DEBUG

	bool p1q1p2=Envelope::intersects(p1,p2,q1);
	bool p1q2p2=Envelope::intersects(p1,p2,q2);
	bool q1p1q2=Envelope::intersects(q1,q2,p1);
	bool q1p2q2=Envelope::intersects(q1,q2,p2);

	if (p1q1p2 && p1q2p2) {
#if DEBUG
		cerr<<" p1q1p2 && p1q2p2"<<endl;
#endif
		intPt[0].setCoordinate(q1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(q2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return COLLINEAR;
	}
	if (q1p1q2 && q1p2q2) {
#if DEBUG
		cerr<<" q1p1q2 && q1p2q2"<<endl;
#endif
		intPt[0].setCoordinate(p1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(p2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p2z = interpolateZ(p2, q1, q2);
		if (!ISNAN(p2z)) { ztot+=p2z; hits++; }
		if (!ISNAN(p2.z)) { ztot+=p2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
		return COLLINEAR;
	}
	if (p1q1p2 && q1p1q2) {
#if DEBUG
		cerr<<" p1q1p2 && q1p1q2"<<endl;
#endif
		intPt[0].setCoordinate(q1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(p1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q1==p1) && !p1q2p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q1p2 && q1p2q2) {
#if DEBUG
		cerr<<" p1q1p2 && q1p2q2"<<endl;
#endif
		intPt[0].setCoordinate(q1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(p2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p2z = interpolateZ(p2, q1, q2);
		if (!ISNAN(p2z)) { ztot+=p2z; hits++; }
		if (!ISNAN(p2.z)) { ztot+=p2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q1==p2) && !p1q2p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p1q2) {
#if DEBUG
		cerr<<" p1q2p2 && q1p1q2"<<endl;
#endif
		intPt[0].setCoordinate(q2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(p1);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q2==p1) && !p1q1p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p2q2) {
#if DEBUG
		cerr<<" p1q2p2 && q1p2q2"<<endl;
#endif
		intPt[0].setCoordinate(q2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1].setCoordinate(p2);
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p2z = interpolateZ(p2, q1, q2);
		if (!ISNAN(p2z)) { ztot+=p2z; hits++; }
		if (!ISNAN(p2.z)) { ztot+=p2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q2==p2) && !p1q1p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	return DONT_INTERSECT;
}

Coordinate*
LineIntersector::intersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2) const
{
	Coordinate n1=p1;
	Coordinate n2=p2;
	Coordinate n3=q1;
	Coordinate n4=q2;
	Coordinate normPt;

	//normalize(&n1, &n2, &n3, &n4, &normPt);
	normalizeToEnvCentre(n1, n2, n3, n4, normPt);

	Coordinate *intPt=NULL;

#if DEBUG
	cerr<<"RobustIntersector::intersection(p1,p2,q1,q2) called:"<<endl;
	cerr<<" p1"<<p1.toString()<<endl;
	cerr<<" p2"<<p2.toString()<<endl;
	cerr<<" q1"<<q1.toString()<<endl;
	cerr<<" q2"<<q2.toString()<<endl;

	cerr<<" n1"<<n1.toString()<<endl;
	cerr<<" n2"<<n2.toString()<<endl;
	cerr<<" n3"<<n3.toString()<<endl;
	cerr<<" n4"<<n4.toString()<<endl;
#endif

	try {
		intPt=HCoordinate::intersection(n1,n2,n3,n4);
#if DEBUG
		cerr<<" HCoordinate found intersection h:"<<h->toString()<<endl;
#endif

	} catch (NotRepresentableException *e) {
		delete intPt;
		Assert::shouldNeverReachHere("Coordinate for intersection is not calculable"+e->toString());
    	}

	intPt->x+=normPt.x;
	intPt->y+=normPt.y;

/**
 *
 * MD - May 4 2005 - This is still a problem.  Here is a failure case:
 *
 * LINESTRING (2089426.5233462777 1180182.3877339689,
 *             2085646.6891757075 1195618.7333999649)
 * LINESTRING (1889281.8148903656 1997547.0560044837,
 *             2259977.3672235999 483675.17050843034)
 * int point = (2097408.2633752143,1144595.8008114607)
 */

#if DEBUG
	if (! isInSegmentEnvelopes(intPt)) 
	{
		cerr<<"Intersection outside segment envelopes: "<<
			intPt->toString();
	}
#endif
 
	if (precisionModel!=NULL) precisionModel->makePrecise(intPt);


#if COMPUTE_Z
	double ztot = 0;
	double zvals = 0;
	double zp = interpolateZ(*intPt, p1, p2);
	double zq = interpolateZ(*intPt, q1, q2);
	if ( !ISNAN(zp)) { ztot += zp; zvals++; }
	if ( !ISNAN(zq)) { ztot += zq; zvals++; }
	if ( zvals ) intPt->z = ztot/zvals;
#endif // COMPUTE_Z

	return intPt;
}


double
LineIntersector::smallestInAbsValue(double x1,double x2,double x3,double x4) const
{
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

/*
 * Test whether a point lies in the envelopes of both input segments.
 * A correctly computed intersection point should return <code>true</code>
 * for this test.
 * Since this test is for debugging purposes only, no attempt is
 * made to optimize the envelope test.
 *
 * @return <code>true</code> if the input point lies within both input
 *	segment envelopes
 */
bool
LineIntersector::isInSegmentEnvelopes(const Coordinate& intPt)
{
	Envelope *env0=new Envelope(*inputLines[0][0], *inputLines[0][1]);
	Envelope *env1=new Envelope(*inputLines[1][0], *inputLines[1][1]);
	return env0->contains(intPt) && env1->contains(intPt);
}

void
LineIntersector::normalizeToEnvCentre(Coordinate &n00, Coordinate &n01,
		Coordinate &n10, Coordinate &n11, Coordinate &normPt) const
{
	double minX0 = n00.x < n01.x ? n00.x : n01.x;
	double minY0 = n00.y < n01.y ? n00.y : n01.y;
	double maxX0 = n00.x > n01.x ? n00.x : n01.x;
	double maxY0 = n00.y > n01.y ? n00.y : n01.y;
	
	double minX1 = n10.x < n11.x ? n10.x : n11.x;
	double minY1 = n10.y < n11.y ? n10.y : n11.y;
	double maxX1 = n10.x > n11.x ? n10.x : n11.x;
	double maxY1 = n10.y > n11.y ? n10.y : n11.y;
	
	double intMinX = minX0 > minX1 ? minX0 : minX1;
	double intMaxX = maxX0 < maxX1 ? maxX0 : maxX1;
	double intMinY = minY0 > minY1 ? minY0 : minY1;
	double intMaxY = maxY0 < maxY1 ? maxY0 : maxY1;
	
	double intMidX = (intMinX + intMaxX) / 2.0;
	double intMidY = (intMinY + intMaxY) / 2.0;

	normPt.x = intMidX;
	normPt.y = intMidY;

	n00.x -= normPt.x;    n00.y -= normPt.y;
	n01.x -= normPt.x;    n01.y -= normPt.y;
	n10.x -= normPt.x;    n10.y -= normPt.y;
	n11.x -= normPt.x;    n11.y -= normPt.y;

#if COMPUTE_Z
	double minZ0 = n00.z < n01.z ? n00.z : n01.z;
	double minZ1 = n10.z < n11.z ? n10.z : n11.z;
	double maxZ0 = n00.z > n01.z ? n00.z : n01.z;
	double maxZ1 = n10.z > n11.z ? n10.z : n11.z;
	double intMinZ = minZ0 > minZ1 ? minZ0 : minZ1;
	double intMaxZ = maxZ0 < maxZ1 ? maxZ0 : maxZ1;
	double intMidZ = (intMinZ + intMaxZ) / 2.0;
	normPt.z = intMidZ;
	n00.z -= normPt.z;
	n01.z -= normPt.z;
	n10.z -= normPt.z;
	n11.z -= normPt.z;
#endif
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.25  2005/11/15 18:30:59  strk
 * Removed dead code
 *
 * Revision 1.24  2005/06/24 11:09:42  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.33  2005/05/09 10:35:20  strk
 * Ported JTS robustness patches made by Martin on suggestions by Kevin.
 *
 * Revision 1.32  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.31  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.30  2005/01/18 17:09:53  strk
 * Fixed interpolateZ call using final intersection point instead of HCoordinate.
 *
 * Revision 1.29  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.28  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.27  2004/11/26 09:53:48  strk
 * Added more FINITE calls, and added inf and -inf to FINITE checks
 *
 * Revision 1.26  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.25  2004/11/22 13:02:40  strk
 * Fixed a bug in Collinear intersection Z computation
 *
 * Revision 1.24  2004/11/22 11:34:16  strk
 * Added Z computation for CollinearIntersections
 *
 * Revision 1.23  2004/11/20 15:40:49  strk
 * Added Z computation in point-segment intersection.
 *
 * Revision 1.22  2004/11/17 15:09:08  strk
 * Changed COMPUTE_Z defaults to be more conservative
 *
 * Revision 1.21  2004/11/17 08:41:42  strk
 * Fixed a bug in Z computation and removed debugging output by default.
 *
 * Revision 1.20  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.19  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.18  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.17  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.16  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.15  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
