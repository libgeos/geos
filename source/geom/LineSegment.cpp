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
 * Revision 1.11  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.9  2003/10/11 01:56:08  strk
 * Code base padded with 'const' keywords ;)
 *
 **********************************************************************/



#include "stdio.h"
#include "../headers/geom.h"
#include "../headers/geosAlgorithm.h"

namespace geos {

/**
 *  Constructs an empty <code>LineSegment</code>.
 */
LineSegment::LineSegment(void){}

/**
 *  Constructs a <code>LineSegment</code> with the given start and end coordinates.
 *
 *@param  c0      start of the <code>LineSegment</code>.
 *@param  c1      end of the <code>LineSegment</code>.
 */
LineSegment::LineSegment(const Coordinate& c0, const Coordinate& c1){
	p0=c0;
	p1=c1;
}

/// Default destructor
LineSegment::~LineSegment(void){}

LineSegment::LineSegment(const LineSegment &ls):p0(ls.p0),p1(ls.p1) {}

/**
 *  Sets the parameters of the <code>LineSegment</code> to the given start and end coordinates.
 *
 *@param  c0      new start of the <code>LineSegment</code>.
 *@param  c1      new end of the <code>LineSegment</code>.
 */
void LineSegment::setCoordinates(const Coordinate& c0, const Coordinate& c1) {
	p0.x = c0.x;
	p0.y = c0.y;
	p1.x = c1.x;
	p1.y = c1.y;
}

const Coordinate& LineSegment::getCoordinate(int i) const {
	if (i==0) return p0;
	return p1;
}

void LineSegment::setCoordinates(const LineSegment ls) {
	setCoordinates(ls.p0,ls.p1);
}

/**
* Computes the length of the line segment.
* @return the length of the line segment
*/
double LineSegment::getLength() const {
	return p0.distance(p1);
}

/**
* Reverses the direction of the line segment.
*/
void LineSegment::reverse() {
	Coordinate& temp=p0;
	p0.setCoordinate(p1);
	p1.setCoordinate(temp);
}

/**
* Puts the line segment into a normalized form.
* This is useful for using line segments in maps and indexes when
* topological equality rather than exact equality is desired.
*/
void LineSegment::normalize(){
	if (p1.compareTo(p0)<0) reverse();
}

/**
* @return the angle this segment makes with the x-axis (in radians)
*/
double LineSegment::angle() const {
	return atan2(p1.y-p0.y,p1.x-p0.x);
}

/**
* Computes the distance between this line segment and another one.
*/
double LineSegment::distance(const LineSegment ls) const {
	return CGAlgorithms::distanceLineLine(p0,p1,ls.p0,ls.p1);
}

/**
* Computes the distance between this line segment and another one.
*/
double LineSegment::distance(const Coordinate& p) const {
	return CGAlgorithms::distancePointLine(p,p0,p1);
}

/**
* Compute the projection factor for the projection of the point p
* onto this LineSegment.  The projection factor is the constant k
* by which the vector for this segment must be multiplied to
* equal the vector for the projection of p.
*/
double LineSegment::projectionFactor(const Coordinate& p) const {
	if (p==p0) return 0.0;
	if (p==p1) return 1.0;
    // Otherwise, use comp.graphics.algorithms Frequently Asked Questions method
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
	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	double len2=dx*dx+dy*dy;
	double r=((p.x-p0.x)*dx+(p.y-p0.y)*dy)/len2;
	return r;
}

/**
* Compute the projection of a point onto the line determined
* by this line segment.
* <p>
* Note that the projected point
* may lie outside the line segment.  If this is the case,
* the projection factor will lie outside the range [0.0, 1.0].
*/
Coordinate* LineSegment::project(const Coordinate& p) const {
	if (p==p0 || p==p1) return new Coordinate(p);
	double r=projectionFactor(p);
	return new Coordinate(p0.x+r*(p1.x-p0.x),p0.y+r*(p1.y-p0.y));
}

/**
* Project a line segment onto this line segment and return the resulting
* line segment.  The returned line segment will be a subset of
* the target line line segment.  This subset may be null, if
* the segments are oriented in such a way that there is no projection.
* <p>
* Note that the returned line may have zero length (i.e. the same endpoints).
* This can happen for instance if the lines are perpendicular to one another.
*
* @param seg the line segment to project
* @return the projected line segment, or <code>null</code> if there is no overlap
*/
LineSegment* LineSegment::project(const LineSegment *seg) const {
	double pf0=projectionFactor(seg->p0);
	double pf1=projectionFactor(seg->p1);
	// check if segment projects at all
	if (pf0>=1.0 && pf1>=1.0) return NULL;
	if (pf0<=0.0 && pf1<=0.0) return NULL;
	Coordinate *newp0=project(seg->p0);
	Coordinate *newp1=project(seg->p1);
	LineSegment *ret = new LineSegment(*newp0,*newp1);
	delete newp0;
	delete newp1;
	return ret;
}

/**
* Computes the closest point on this line segment to another point.
* @param p the point to find the closest point to
* @return a Coordinate which is the closest point on the line segment to the point p
*/
Coordinate* LineSegment::closestPoint(const Coordinate& p) const {
	double factor=projectionFactor(p);
	if (factor>0 && factor<1) {
		return project(p);
	}
	double dist0=p0.distance(p);
	double dist1=p1.distance(p);
	if (dist0<dist1)
		return new Coordinate(p0);
	return new Coordinate(p1);
}

/**
*  Compares this object with the specified object for order.
*  Uses the standard lexicographic ordering for the points in the LineSegment.
*
*@param  o  the <code>LineSegment</code> with which this <code>LineSegment</code>
*      is being compared
*@return    a negative integer, zero, or a positive integer as this <code>LineSegment</code>
*      is less than, equal to, or greater than the specified <code>LineSegment</code>
*/
int LineSegment::compareTo(LineSegment other) const {
	int comp0=p0.compareTo(other.p0);
	if (comp0!=0) return comp0;
	return p1.compareTo(other.p1);
}

/**
*  Returns <code>true</code> if <code>other</code> is
*  topologically equal to this LineSegment (e.g. irrespective
*  of orientation).
*
*@param  other  a <code>LineSegment</code> with which to do the comparison.
*@return        <code>true</code> if <code>other</code> is a <code>LineSegment</code>
*      with the same values for the x and y ordinates.
*/
bool LineSegment::equalsTopo(const LineSegment other) const {
	return (p0==other.p0 && p1==other.p1) || (p0==other.p1 && p1==other.p0);
}

string LineSegment::toString() const {
	string out="LINESTRING( ";
	char buf[256];
	sprintf(buf, "%f %f, %f %f", p0.x, p0.y, p1.x, p1.y);
	out += buf;
	out+=")";
	return out;
}


/**
* Tests whether the segment is horizontal.
*
* @return <code>true</code> if the segment is horizontal
*/
bool LineSegment::isHorizontal() const { 
	return p0.y == p1.y;
}

/**
* Tests whether the segment is vertical.
*
* @return <code>true</code> if the segment is vertical
*/
bool LineSegment::isVertical() const { 
	return p0.x == p1.x;
}

/**
* Determines the orientation of a LineSegment relative to this segment.
* The concept of orientation is specified as follows:
* Given two line segments A and L,
* <ul
* <li>A is to the left of a segment L if A lies wholly in the
* closed half-plane lying to the left of L
* <li>A is to the right of a segment L if A lies wholly in the
* closed half-plane lying to the right of L
* <li>otherwise, A has indeterminate orientation relative to L. This
* happens if A is collinear with L or if A crosses the line determined by L.
* </ul>
*
* @param seg the LineSegment to compare
*
* @return 1 if <code>seg</code> is to the left of this segment
* @return -1 if <code>seg</code> is to the right of this segment
* @return 0 if <code>seg</code> has indeterminate orientation relative to this segment
*/
int LineSegment::orientationIndex(LineSegment *seg) const {
	int orient0 = CGAlgorithms::orientationIndex(p0, p1, seg->p0);
	int orient1 = CGAlgorithms::orientationIndex(p0, p1, seg->p1);
	// this handles the case where the points are L or collinear
	if (orient0 >= 0 && orient1 >= 0)
		return max(orient0, orient1);
	// this handles the case where the points are R or collinear
	if (orient0 <= 0 && orient1 <= 0)
		return max(orient0, orient1);
	// points lie on opposite sides ==> indeterminate orientation
	return 0;
}

/**
* Computes the perpendicular distance between the (infinite) line defined
* by this line segment and a point.
*/
double LineSegment::distancePerpendicular(const Coordinate& p) const {
	return CGAlgorithms::distancePointLinePerpendicular(p, p0, p1);
}

/**
* Computes the closest points on two line segments.
* @param p the point to find the closest point to
* @return a pair of Coordinates which are the closest points on the line segments
*/
CoordinateList* LineSegment::closestPoints(LineSegment *line){
	// test for intersection
	Coordinate *intPt = intersection(line);
	if (intPt!=NULL) {
		CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
		cl->add(*intPt);
		cl->add(*intPt);
		return cl;
	}

	/**
	*  if no intersection closest pair contains at least one endpoint.
	* Test each endpoint in turn.
	*/
	CoordinateList *closestPt=CoordinateListFactory::internalFactory->createCoordinateList(2);
	double minDistance=DoubleInfinity;
	double dist;
	Coordinate *close00 = closestPoint(line->p0);
	minDistance = close00->distance(line->p0);
	closestPt->setAt(*close00,0);
	closestPt->setAt(line->p0,1);
	Coordinate *close01 = closestPoint(line->p1);
	dist = close01->distance(line->p1);
	if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(*close01,0);
		closestPt->setAt(line->p1,1);
	}
	Coordinate *close10 = line->closestPoint(p0);
	dist = close10->distance(p0);
		if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(p0,0);
		closestPt->setAt(*close10,1);
	}
	Coordinate *close11 = line->closestPoint(p1);
	dist = close11->distance(p1);
	if (dist < minDistance) {
		minDistance = dist;
		closestPt->setAt(p1,0);
		closestPt->setAt(*close11,1);
	}

	return closestPt;
}

/**
* Computes an intersection point between two segments, if there is one.
* There may be 0, 1 or many intersection points between two segments.
* If there are 0, null is returned. If there is 1 or more, a single one
* is returned (chosen at the discretion of the algorithm).  If
* more information is required about the details of the intersection,
* the {@link RobustLineIntersector} class should be used.
*
* @param line
* @return an intersection point, or <code>null</code> if there is none
*/
Coordinate* LineSegment::intersection(LineSegment *line){
	LineIntersector *li = new RobustLineIntersector();
	li->computeIntersection(p0, p1, line->p0, line->p1);
	if (li->hasIntersection()) {
		const Coordinate &c=li->getIntersection(0);
		delete li;
		return (Coordinate*)&c;
	}
	delete li;
	return NULL;
}


/**
*  Returns <code>true</code> if <code>other</code> has the same values for
*  its points.
*
*@param  other  a <code>LineSegment</code> with which to do the comparison.
*@return        <code>true</code> if <code>other</code> is a <code>LineSegment</code>
*      with the same values for the x and y ordinates.
*/
bool operator==(const LineSegment a, const LineSegment b) {
	return a.p0==b.p0 && a.p1==b.p1;
}
}

