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

