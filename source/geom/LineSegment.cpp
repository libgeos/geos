#include "geom.h"
#include "geosAlgorithm.h"

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
LineSegment::LineSegment(Coordinate c0, Coordinate c1):p0(c0),p1(c1){}

/// Default destructor
LineSegment::~LineSegment(void){}

LineSegment::LineSegment(const LineSegment &ls):p0(ls.p0),p1(ls.p1) {}

/**
 *  Sets the parameters of the <code>LineSegment</code> to the given start and end coordinates.
 *
 *@param  c0      new start of the <code>LineSegment</code>.
 *@param  c1      new end of the <code>LineSegment</code>.
 */
void LineSegment::setCoordinates(Coordinate c0, Coordinate c1) {
	p0.x = c0.x;
	p0.y = c0.y;
	p1.x = c1.x;
	p1.y = c1.y;
}

Coordinate& LineSegment::getCoordinate(int i) {
	if (i==0) return p0;
	return p1;
}

void LineSegment::setCoordinates(LineSegment ls) {
	setCoordinates(ls.p0,ls.p1);
}

void LineSegment::reverse() {
	Coordinate temp(p0);
	p0.setCoordinate(p1);
	p1.setCoordinate(temp);
}

void LineSegment::normalize(){
	if (p1.compareTo(p0)<0) reverse();
}

/**
* @return the angle this segment makes with the x-axis (in radians)
*/
double LineSegment::angle() {
	return atan2(p1.y-p0.y,p1.x-p0.x);
}

/**
* Computes the distance between this line segment and another one.
*/
double LineSegment::distance(LineSegment ls) {
	return CGAlgorithms::distanceLineLine(p0,p1,ls.p0,ls.p1);
}

/**
* Computes the distance between this line segment and another one.
*/
double LineSegment::distance(Coordinate& p) {
	return CGAlgorithms::distancePointLine(p,p0,p1);
}

/**
* Compute the projection factor for the projection of the point p
* onto this LineSegment.  The projection factor is the constant k
* by which the vector for this segment must be multiplied to
* equal the vector for the projection of p.
*/
double LineSegment::projectionFactor(Coordinate& p) {
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

Coordinate LineSegment::project(Coordinate& p) {
	if (p==p0 || p==p1) return Coordinate(p);
	double r=projectionFactor(p);
	return Coordinate(p0.x+r*(p1.x-p0.x),p0.y+r*(p1.y-p0.y));
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
int LineSegment::compareTo(LineSegment other) {
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
bool LineSegment::equalsTopo(LineSegment other) {
	return (p0==other.p0 && p1==other.p1) || (p0==other.p1 && p1==other.p0);
}

string LineSegment::toString() {
	string out="LINESTRING( ";
	out+=p0.x;
	out+=" ";
	out+=p0.y;
	out+=", ";
	out+=p1.x;
	out+=" ";
	out+=p1.y;
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
bool operator==(LineSegment a,LineSegment b) {
	return a.p0==b.p0 && a.p1==b.p1;
}
