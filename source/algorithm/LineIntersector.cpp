#include "geosAlgorithm.h"
#include "util.h"
#include "geom.h"
#include "math.h"

/**
* Computes the "edge distance" of an intersection point p in an edge.
* The edge distance is a metric of the point along the edge.
* The metric used is a robust and easy to compute metric function.
* It is <b>not</b> equivalent to the usual Euclidean metric.
* It relies on the fact that either the x or the y ordinates of the
* points in the edge are unique, depending on whether the edge is longer in
* the horizontal or vertical direction.
* <p>
* NOTE: This function may produce incorrect distances
*  for inputs where p is not precisely on p1-p2
* (E.g. p = (139,9) p1 = (139,10), p2 = (280,1) produces distanct 0.0, which is incorrect.
* <p>
* My hypothesis is that the function is safe to use for points which are the
* result of <b>rounding</b> points which lie on the line,
* but not safe to use for <b>truncated</b> points.
*/
double LineIntersector::computeEdgeDistance(Coordinate p,Coordinate p0,Coordinate p1) {
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
		if (dx>dy)
			dist=fabs(p.x-p0.x);
		else
			dist=fabs(p.y-p0.y);
	}
//	Assert::isTrue(!(dist==0.0 && !p.equals2D(p1)), "Invalid distance calculation");
	return dist;
}

/**
* This function is non-robust, since it may compute the square of large numbers.
* Currently not sure how to improve this.
*/
double LineIntersector::nonRobustComputeEdgeDistance(Coordinate p,Coordinate p1,Coordinate p2) {
	double dx=p.x-p1.x;
	double dy=p.y-p1.y;
	double dist=sqrt(dx*dx+dy*dy);   // dummy value
	Assert::isTrue(!(dist==0.0 && !(p==p1)), "Invalid distance calculation");
	return dist;
}

LineIntersector::LineIntersector() {
	makePrecise=false;
	intPt[0].setCoordinate(Coordinate());
	intPt[1].setCoordinate(Coordinate());
	// alias the intersection points for ease of reference
	pa.setCoordinate(intPt[0]);
	pb.setCoordinate(intPt[1]);
	result=0;
}

void LineIntersector::setMakePrecise(bool newMakePrecise){
	makePrecise=newMakePrecise;
}

bool LineIntersector::isCollinear() {
	return result==COLLINEAR;
}

/**
* Computes the intersection of the lines p1-p2 and p3-p4
*/
void LineIntersector::computeIntersection(Coordinate p1,Coordinate p2,Coordinate p3,Coordinate p4) {
	inputLines[0][0]=p1;
	inputLines[0][1]=p2;
	inputLines[1][0]=p3;
	inputLines[1][1]=p4;
	result=computeIntersect(p1,p2,p3,p4);
	//numIntersects++;
}

string LineIntersector::toString() {
	string str=inputLines[0][0].toString()+" "
			  +inputLines[0][1].toString()+" "
			  +inputLines[1][0].toString()+" "
			  +inputLines[1][1].toString()+" : ";
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

bool LineIntersector::isEndPoint() {
	return hasIntersection()&&!isProperVar;
}

/**
* Tests whether the input geometries intersect.
*
* @return true if the input geometries intersect
*/
bool LineIntersector::hasIntersection() {
	return result!=DONT_INTERSECT;
}

/**
* Returns the number of intersection points found.  This will be either 0, 1 or 2.
*/
int LineIntersector::getIntersectionNum() {
	return result;
}

/**
* Returns the intIndex'th intersection point
*
* @param intIndex is 0 or 1
*
* @return the intIndex'th intersection point
*/
Coordinate LineIntersector::getIntersection(int intIndex){
	return intPt[intIndex];
}

/**
* @return true if both numbers are positive or if both numbers are negative.
* Returns false if both numbers are zero.
*/
bool LineIntersector::isSameSignAndNonZero(double a,double b) {
	if (a==0 || b==0) {
		return false;
	}
	return (a<0 && b<0) || (a>0 && b>0);
}

void LineIntersector::computeIntLineIndex() {
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
bool LineIntersector::isIntersection(Coordinate pt) {
	for (int i=0;i<result;i++) {
		if (intPt[i].equals2D(pt)) {
			return true;
		}
	}
	return false;
}

/**
* Tests whether an intersection is proper.
* <br>
* The intersection between two line segments is considered proper if
* they intersect in a single point in the interior of both segments
* (e.g. the intersection is a single point and is not equal to any of the
* endpoints).
* <p>
* The intersection between a point and a line segment is considered proper
* if the point lies in the interior of the segment (e.g. is not equal to
* either of the endpoints).
*
* @return true if the intersection is proper
*/
bool LineIntersector::isProper() {
	return hasIntersection()&&isProperVar;
}

/**
* Computes the intIndex'th intersection point in the direction of
* a specified input line segment
*
* @param segmentIndex is 0 or 1
* @param intIndex is 0 or 1
*
* @return the intIndex'th intersection point in the direction of the specified input line segment
*/
Coordinate LineIntersector::getIntersectionAlongSegment(int segmentIndex,int intIndex) {
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
int LineIntersector::getIndexAlongSegment(int segmentIndex,int intIndex) {
	computeIntLineIndex();
	return intLineIndex[segmentIndex][intIndex];
}

void LineIntersector::computeIntLineIndex(int segmentIndex) {
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
* Computes the "edge distance" of an intersection point along the specified input line segment.
*
* @param segmentIndex is 0 or 1
* @param intIndex is 0 or 1
*
* @return the edge distance of the intersection point
*/
double LineIntersector::getEdgeDistance(int segmentIndex,int intIndex) {
	double dist=computeEdgeDistance(intPt[intIndex],inputLines[segmentIndex][0],inputLines[segmentIndex][1]);
	return dist;
}

