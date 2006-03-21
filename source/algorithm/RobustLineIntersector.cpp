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
namespace algorithm { // geos.algorithm

RobustLineIntersector::RobustLineIntersector(){}
RobustLineIntersector::~RobustLineIntersector(){}

void
RobustLineIntersector::computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2)
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

int
RobustLineIntersector::computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)
{
#if DEBUG
	cerr<<"RobustLineIntersector::computeIntersect called"<<endl;
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
		cerr<<"RobustLineIntersector::computeIntersect: z:"<<z<<" hits:"<<hits<<endl;
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

//bool RobustLineIntersector::intersectsEnvelope(Coordinate& p1,Coordinate& p2,Coordinate& q) {
//	if (((q.x>=min(p1.x,p2.x)) && (q.x<=max(p1.x,p2.x))) &&
//		((q.y>=min(p1.y,p2.y)) && (q.y<=max(p1.y,p2.y)))) {
//			return true;
//	} else {
//		return false;
//	}
//}

int
RobustLineIntersector::computeCollinearIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)
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
	cerr<<"RobustLineIntersector::computeCollinearIntersection called"<<endl;
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
RobustLineIntersector::intersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2) const
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

	} catch (const NotRepresentableException& e) {
		delete intPt;
		Assert::shouldNeverReachHere("Coordinate for intersection is not calculable"+e.toString());
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


void
RobustLineIntersector::normalize(Coordinate *n1,Coordinate *n2,Coordinate *n3,Coordinate *n4,Coordinate *normPt) const
{
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

#if COMPUTE_Z
	normPt->z=smallestInAbsValue(n1->z,n2->z,n3->z,n4->z);
	n1->z-=normPt->z;
	n2->z-=normPt->z;
	n3->z-=normPt->z;
	n4->z-=normPt->z;
#endif
}

double
RobustLineIntersector::smallestInAbsValue(double x1,double x2,double x3,double x4) const
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
RobustLineIntersector::isInSegmentEnvelopes(const Coordinate& intPt)
{
	Envelope *env0=new Envelope(*inputLines[0][0], *inputLines[0][1]);
	Envelope *env1=new Envelope(*inputLines[1][0], *inputLines[1][1]);
	return env0->contains(intPt) && env1->contains(intPt);
}

void
RobustLineIntersector::normalizeToEnvCentre(Coordinate &n00, Coordinate &n01,
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
 * Revision 1.36  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.35  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 **********************************************************************/
