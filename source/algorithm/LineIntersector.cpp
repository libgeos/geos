/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * A LineIntersector is an algorithm that can both test whether
 * two line segments intersect and compute the intersection point
 * if they do.
 * The intersection point may be computed in a precise or non-precise manner.
 * Computing it precisely involves rounding it to an integer.  (This assumes
 * that the input coordinates have been made precise by scaling them to
 * an integer grid.)
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/util.h>
#include <string>


#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifndef COMPUTE_Z
#define COMPUTE_Z 1
#endif // COMPUTE_Z

using namespace std;

namespace geos {
namespace algorithm { // geos.algorithm

/*public static*/
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
	assert(!(dist == 0.0 && !(p==p0))); // Bad distance calculation
	return dist;
}

/*public*/
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

/*public*/
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

/*public static*/
bool
LineIntersector::isSameSignAndNonZero(double a,double b)
{
	if (a==0 || b==0) {
		return false;
	}
	return (a<0 && b<0) || (a>0 && b>0);
}

/*private*/
void
LineIntersector::computeIntLineIndex() {
	computeIntLineIndex(0);
	computeIntLineIndex(1);
}

/*public*/
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

/*public*/
const Coordinate&
LineIntersector::getIntersectionAlongSegment(int segmentIndex,int intIndex)
{
	// lazily compute int line array
	computeIntLineIndex();
	return intPt[intLineIndex[segmentIndex][intIndex]];
}

/*public*/
int
LineIntersector::getIndexAlongSegment(int segmentIndex,int intIndex)
{
	computeIntLineIndex();
	return intLineIndex[segmentIndex][intIndex];
}

/*private*/
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

/*public*/
double
LineIntersector::getEdgeDistance(int segmentIndex,int intIndex) const
{
	double dist=computeEdgeDistance(intPt[intIndex],
		*inputLines[segmentIndex][0],
		*inputLines[segmentIndex][1]);
	return dist;
}

/*public*/
bool
LineIntersector::isInteriorIntersection()
{
	if (isInteriorIntersection(0)) return true;
	if (isInteriorIntersection(1)) return true;
	return false;
}

/*public*/
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

/*public static*/
double
LineIntersector::interpolateZ(const Coordinate &p,
	const Coordinate &p1, const Coordinate &p2)
{
#if GEOS_DEBUG
	cerr<<"LineIntersector::interpolateZ("<<p.toString()<<", "<<p1.toString()<<", "<<p2.toString()<<")"<<endl;
#endif

	if ( ISNAN(p1.z) )
	{
#if GEOS_DEBUG
		cerr<<" p1 do not have a Z"<<endl;
#endif
		return p2.z; // might be DoubleNotANumber again
	}

	if ( ISNAN(p2.z) )
	{
#if GEOS_DEBUG
		cerr<<" p2 do not have a Z"<<endl;
#endif
		return p1.z; // might be DoubleNotANumber again
	}

	if (p==p1)
	{
#if GEOS_DEBUG
		cerr<<" p==p1, returning "<<p1.z<<endl;
#endif
		return p1.z;
	}
	if (p==p2)
	{
#if GEOS_DEBUG
		cerr<<" p==p2, returning "<<p2.z<<endl;
#endif
		return p2.z;
	}

	//double zgap = fabs(p2.z - p1.z);
	double zgap = p2.z - p1.z;
	if ( ! zgap )
	{
#if GEOS_DEBUG
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
#if GEOS_DEBUG
	cerr<<" zgap:"<<zgap<<" seglen:"<<seglen<<" pdist:"<<pdist
		<<" fract:"<<fract<<" z:"<<interpolated<<endl;
#endif
	return interpolated;

}


/*public*/
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
			intPt[0]=p;
#if GEOS_DEBUG
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

/* public static */
bool
LineIntersector::hasIntersection(const Coordinate& p, const Coordinate& p1, const Coordinate& p2)
{
	if(Envelope::intersects(p1,p2,p)) {
		if ((CGAlgorithms::orientationIndex(p1,p2,p)==0)&&
			(CGAlgorithms::orientationIndex(p2,p1,p)==0)) {
			return true;
		}
	}
	return false;
}

/*private*/
int
LineIntersector::computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)
{
#if GEOS_DEBUG
	cerr<<"LineIntersector::computeIntersect called"<<endl;
	cerr<<" p1:"<<p1.toString()<<" p2:"<<p2.toString()<<" q1:"<<q1.toString()<<" q2:"<<q2.toString()<<endl;
#endif // GEOS_DEBUG

	isProperVar=false;

	// first try a fast test to see if the envelopes of the lines intersect
	if (!Envelope::intersects(p1,p2,q1,q2))
	{
#if GEOS_DEBUG
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
#if GEOS_DEBUG
		cerr<<" DONT_INTERSECT"<<endl;
#endif
		return DONT_INTERSECT;
	}

	int Qp1=CGAlgorithms::orientationIndex(q1,q2,p1);
	int Qp2=CGAlgorithms::orientationIndex(q1,q2,p2);

	if ((Qp1>0 && Qp2>0)||(Qp1<0 && Qp2<0)) {
#if GEOS_DEBUG
		cerr<<" DONT_INTERSECT"<<endl;
#endif
		return DONT_INTERSECT;
	}

	bool collinear=Pq1==0 && Pq2==0 && Qp1==0 && Qp2==0;
	if (collinear) {
#if GEOS_DEBUG
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
			intPt[0]=q1;
#if COMPUTE_Z
			if ( !ISNAN(q1.z) )
			{
				z += q1.z;
				hits++;
			}
#endif
		}
		if (Pq2==0) {
			intPt[0]=q2;
#if COMPUTE_Z
			if ( !ISNAN(q2.z) )
			{
				z += q2.z;
				hits++;
			}
#endif
		}
		if (Qp1==0) {
			intPt[0]=p1;
#if COMPUTE_Z
			if ( !ISNAN(p1.z) )
			{
				z += p1.z;
				hits++;
			}
#endif
		}
		if (Qp2==0) {
			intPt[0]=p2;
#if COMPUTE_Z
			if ( !ISNAN(p2.z) )
			{
				z += p2.z;
				hits++;
			}
#endif
		}
#if COMPUTE_Z
#if GEOS_DEBUG
		cerr<<"LineIntersector::computeIntersect: z:"<<z<<" hits:"<<hits<<endl;
#endif // GEOS_DEBUG
		if ( hits ) intPt[0].z = z/hits;
#endif // COMPUTE_Z
	} else {
		isProperVar=true;
		intersection(p1, p2, q1, q2, intPt[0]);
	}
#if GEOS_DEBUG
	cerr<<" DO_INTERSECT; intPt[0]:"<<intPt[0].toString()<<endl;
#endif // GEOS_DEBUG
	return DO_INTERSECT;
}

/*private*/
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

#if GEOS_DEBUG
	cerr<<"LineIntersector::computeCollinearIntersection called"<<endl;
	cerr<<" p1:"<<p1.toString()<<" p2:"<<p2.toString()<<" q1:"<<q1.toString()<<" q2:"<<q2.toString()<<endl;
#endif // GEOS_DEBUG

	bool p1q1p2=Envelope::intersects(p1,p2,q1);
	bool p1q2p2=Envelope::intersects(p1,p2,q2);
	bool q1p1q2=Envelope::intersects(q1,q2,p1);
	bool q1p2q2=Envelope::intersects(q1,q2,p2);

	if (p1q1p2 && p1q2p2) {
#if GEOS_DEBUG
		cerr<<" p1q1p2 && p1q2p2"<<endl;
#endif
		intPt[0]=q1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=q2;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if GEOS_DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return COLLINEAR;
	}
	if (q1p1q2 && q1p2q2) {
#if GEOS_DEBUG
		cerr<<" q1p1q2 && q1p2q2"<<endl;
#endif
		intPt[0]=p1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=p2;
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
#if GEOS_DEBUG
		cerr<<" p1q1p2 && q1p1q2"<<endl;
#endif
		intPt[0]=q1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=p1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if GEOS_DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q1==p1) && !p1q2p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q1p2 && q1p2q2) {
#if GEOS_DEBUG
		cerr<<" p1q1p2 && q1p2q2"<<endl;
#endif
		intPt[0]=q1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q1z = interpolateZ(q1, p1, p2);
		if (!ISNAN(q1z)) { ztot+=q1z; hits++; }
		if (!ISNAN(q1.z)) { ztot+=q1.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=p2;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p2z = interpolateZ(p2, q1, q2);
		if (!ISNAN(p2z)) { ztot+=p2z; hits++; }
		if (!ISNAN(p2.z)) { ztot+=p2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if GEOS_DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q1==p2) && !p1q2p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p1q2) {
#if GEOS_DEBUG
		cerr<<" p1q2p2 && q1p1q2"<<endl;
#endif
		intPt[0]=q2;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=p1;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p1z = interpolateZ(p1, q1, q2);
		if (!ISNAN(p1z)) { ztot+=p1z; hits++; }
		if (!ISNAN(p1.z)) { ztot+=p1.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if GEOS_DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q2==p1) && !p1q1p2 && !q1p2q2 ? DO_INTERSECT : COLLINEAR;
	}
	if (p1q2p2 && q1p2q2) {
#if GEOS_DEBUG
		cerr<<" p1q2p2 && q1p2q2"<<endl;
#endif
		intPt[0]=q2;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		q2z = interpolateZ(q2, p1, p2);
		if (!ISNAN(q2z)) { ztot+=q2z; hits++; }
		if (!ISNAN(q2.z)) { ztot+=q2.z; hits++; }
		if ( hits ) intPt[0].z = ztot/hits;
#endif
		intPt[1]=p2;
#if COMPUTE_Z
		ztot=0;
		hits=0;
		p2z = interpolateZ(p2, q1, q2);
		if (!ISNAN(p2z)) { ztot+=p2z; hits++; }
		if (!ISNAN(p2.z)) { ztot+=p2.z; hits++; }
		if ( hits ) intPt[1].z = ztot/hits;
#endif
#if GEOS_DEBUG
		cerr<<" intPt[0]: "<<intPt[0].toString()<<endl;
		cerr<<" intPt[1]: "<<intPt[1].toString()<<endl;
#endif
		return (q2==p2) && !p1q1p2 && !q1p1q2 ? DO_INTERSECT : COLLINEAR;
	}
	return DONT_INTERSECT;
}

/*private*/
void
LineIntersector::intersection(const Coordinate& p1, const Coordinate& p2,
	const Coordinate& q1, const Coordinate& q2, Coordinate &intPt) const
{
	Coordinate n1=p1;
	Coordinate n2=p2;
	Coordinate n3=q1;
	Coordinate n4=q2;
	Coordinate normPt;

	//normalize(&n1, &n2, &n3, &n4, &normPt);
	normalizeToEnvCentre(n1, n2, n3, n4, normPt);

#if GEOS_DEBUG
	cerr<<"RobustIntersector::intersection(p1,p2,q1,q2,intPt) called:"<<endl;
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
		HCoordinate::intersection(n1,n2,n3,n4,intPt);
#if GEOS_DEBUG
		cerr<<" HCoordinate found intersection h:"<<intPt.toString()<<endl;
#endif

	} catch (const NotRepresentableException& e) {
		util::Assert::shouldNeverReachHere("Coordinate for intersection is not calculable"+e.toString());
    	}

	intPt.x+=normPt.x;
	intPt.y+=normPt.y;

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

#if GEOS_DEBUG
	if (!((LineIntersector *)this)->isInSegmentEnvelopes(intPt))
	{
		cerr<<"Intersection outside segment envelopes: "<<
			intPt.toString();
	}
#endif
 
	if (precisionModel!=NULL) precisionModel->makePrecise(&intPt);


#if COMPUTE_Z
	double ztot = 0;
	double zvals = 0;
	double zp = interpolateZ(intPt, p1, p2);
	double zq = interpolateZ(intPt, q1, q2);
	if ( !ISNAN(zp)) { ztot += zp; zvals++; }
	if ( !ISNAN(zq)) { ztot += zq; zvals++; }
	if ( zvals ) intPt.z = ztot/zvals;
#endif // COMPUTE_Z

}


/*private*/
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

/*private*/
bool
LineIntersector::isInSegmentEnvelopes(const Coordinate& intPt)
{
	Envelope env0(*inputLines[0][0], *inputLines[0][1]);
	Envelope env1(*inputLines[1][0], *inputLines[1][1]);
	return env0.contains(intPt) && env1.contains(intPt);
}

/*private*/
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

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.35  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.34  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.33  2006/03/02 12:11:58  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.32  2006/02/27 09:05:32  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.31  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.30  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.29  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.28  2006/01/30 21:59:18  frank
 * yikes!  comment out debug define again
 *
 * Revision 1.27  2006/01/30 21:58:38  frank
 * fixed up some debug messages
 *
 * Revision 1.26  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
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
