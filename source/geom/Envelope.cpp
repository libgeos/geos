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
 * Revision 1.14  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.13  2004/07/19 13:19:30  strk
 * Documentation fixes
 *
 * Revision 1.12  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geom.h>
#include <stdio.h>

namespace geos {

/**
* Test the point q to see whether it intersects the Envelope defined by p1-p2
* @param p1 one extremal point of the envelope
* @param p2 another extremal point of the envelope
* @param q the point to test for intersection
* @return <code>true</code> if q intersects the envelope p1-p2
*/
bool
Envelope::intersects(const Coordinate& p1, const Coordinate& p2,
		const Coordinate& q)
{
	//OptimizeIt shows that Math#min and Math#max here are a bottleneck.
    //Replace with direct comparisons. [Jon Aquino]
    if (((q.x >= (p1.x < p2.x ? p1.x : p2.x)) && (q.x <= (p1.x > p2.x ? p1.x : p2.x))) &&
        ((q.y >= (p1.y < p2.y ? p1.y : p2.y)) && (q.y <= (p1.y > p2.y ? p1.y : p2.y)))) {
			return true;
	}
	return false;
}
/**
* Test the envelope defined by p1-p2 for intersection
* with the envelope defined by q1-q2
* @param p1 one extremal point of the envelope P
* @param p2 another extremal point of the envelope P
* @param q1 one extremal point of the envelope Q
* @param q2 another extremal point of the envelope Q
* @return <code>true</code> if Q intersects P
*/
bool Envelope::intersects(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2) {
	double minq=min(q1.x,q2.x);
	double maxq=max(q1.x,q2.x);
	double minp=min(p1.x,p2.x);
	double maxp=max(p1.x,p2.x);
	if(minp>maxq)
		return false;
	if(maxp<minq)
		return false;
	minq=min(q1.y,q2.y);
	maxq=max(q1.y,q2.y);
	minp=min(p1.y,p2.y);
	maxp=max(p1.y,p2.y);
	if(minp>maxq)
		return false;
	if(maxp<minq)
		return false;
	return true;
}

/**
* Compute the distance between two points specified by their ordinate values
*/
double Envelope::distance(double x0,double y0,double x1,double y1) {
	double dx=x1-x0;
	double dy=y1-y0;
	return sqrt(dx*dx+dy*dy);
}

 /**
 *  Creates a null <code>Envelope</code>.
 */
Envelope::Envelope(void) {
	init();
}

/**
 *  Creates an <code>Envelope</code> for a region defined by maximum and minimum values.
 *
 *@param  x1  the first x-value
 *@param  x2  the second x-value
 *@param  y1  the first y-value
 *@param  y2  the second y-value
 */
Envelope::Envelope(double x1, double x2, double y1, double y2){
	init(x1, x2, y1, y2);
}

/**
 *  Creates an <code>Envelope</code> for a region defined by two Coordinates.
 *
 *@param  p1  the first Coordinate
 *@param  p2  the second Coordinate
 */
Envelope::Envelope(const Coordinate& p1, const Coordinate& p2){
	init(p1, p2);
}

/**
 *  Creates an Envelope for a region defined by a single Coordinate.
 *
 * @param  p  the Coordinate
 */
Envelope::Envelope(const Coordinate& p){
	init(p);
}

/**
 *  Create an <code>Envelope</code> from an existing Envelope.
 *
 *@param  env  the Envelope to initialize from
 */
Envelope::Envelope(const Envelope &env){
	init(env.minx, env.maxx, env.miny, env.maxy);
}

///Default destructor
Envelope::~Envelope(void) {}

/**
 *  Initialize to a null <code>Envelope</code>.
 */
void Envelope::init(){
	setToNull();
}

/**
 *  Initialize an <code>Envelope</code> for a region defined by maximum and minimum values.
 *
 *@param  x1  the first x-value
 *@param  x2  the second x-value
 *@param  y1  the first y-value
 *@param  y2  the second y-value
 */
void Envelope::init(double x1, double x2, double y1, double y2){
	if (x1 < x2) {
		minx = x1;
		maxx = x2;
	} else {
		minx = x2;
		maxx = x1;
	}
	if (y1 < y2) {
		miny = y1;
		maxy = y2;
	} else {
		miny = y2;
		maxy = y1;
	}
}

/**
 *  Initialize an <code>Envelope</code> to a region defined by two Coordinates.
 *
 *@param  p1  the first Coordinate
 *@param  p2  the second Coordinate
 */
void Envelope::init(const Coordinate& p1, const Coordinate& p2){
	init(p1.x, p2.x, p1.y, p2.y);
}

/**
 *  Initialize an <code>Envelope</code> to a region defined by a single Coordinate.
 *
 *@param  p  the Coordinate
 */
void Envelope::init(const Coordinate& p){
	init(p.x, p.x, p.y, p.y);
}

/**
 *  Initialize an <code>Envelope</code> from an existing Envelope.
 *
 *@param  env  the Envelope to initialize from
 */
void Envelope::init(Envelope env){
	init(env.minx, env.maxx, env.miny, env.maxy);
}

/**
 *  Makes this <code>Envelope</code> a "null" envelope, that is, the envelope
 *  of the empty geometry.
 */
void Envelope::setToNull() {
	minx=0;
	maxx=-1;
	miny=0;
	maxy=-1;
}

/**
 *  Returns <code>true</code> if this <code>Envelope</code> is a "null"
 *  envelope.
 *
 *@return    <code>true</code> if this <code>Envelope</code> is uninitialized
 *      or is the envelope of the empty geometry.
 */
bool Envelope::isNull() const {
	return maxx < minx;
}

/**
 *  Returns the difference between the maximum and minimum x values.
 *
 *@return    max x - min x, or 0 if this is a null <code>Envelope</code>
 */
double Envelope::getWidth() const {
	if (isNull()) {
		return 0;
	}
	return maxx - minx;
}

/**
 *  Returns the difference between the maximum and minimum y values.
 *
 *@return    max y - min y, or 0 if this is a null <code>Envelope</code>
 */
double Envelope::getHeight() const {
	if (isNull()) {
		return 0;
	}
	return maxy - miny;
}

/**
 *  Returns the <code>Envelope</code>s maximum y-value. min y > max y
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the maximum y-coordinate
 */
double Envelope::getMaxY() const {
	return maxy;
}

/**
 *  Returns the <code>Envelope</code>s maximum x-value. min x > max x
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the maximum x-coordinate
 */
double Envelope::getMaxX() const {
	return maxx;
}

/**
 *  Returns the <code>Envelope</code>s minimum y-value. min y > max y
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the minimum y-coordinate
 */
double Envelope::getMinY() const {
	return miny;
}

/**
 *  Returns the <code>Envelope</code>s minimum x-value. min x > max x
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the minimum x-coordinate
 */
double Envelope::getMinX() const {
	return minx;
}

/**
 *  Enlarges the boundary of the <code>Envelope</code> so that it contains
 *  p. Does nothing if p is already on or within the boundaries.
 *
 *@param  p  the Coordinate to include
 */
void Envelope::expandToInclude(const Coordinate& p) {
	expandToInclude(p.x, p.y);
}

/**
 *  Enlarges the boundary of the <code>Envelope</code> so that it contains
 *  (x,y). Does nothing if (x,y) is already on or within the boundaries.
 *
 *@param  x  the value to lower the minimum x to or to raise the maximum x to
 *@param  y  the value to lower the minimum y to or to raise the maximum y to
 */
void Envelope::expandToInclude(double x, double y) {
	if (isNull()) {
		minx = x;
		maxx = x;
		miny = y;
		maxy = y;
	} else {
		if (x < minx) {
			minx = x;
		}
		if (x > maxx) {
			maxx = x;
		}
		if (y < miny) {
			miny = y;
		}
		if (y > maxy) {
			maxy = y;
		}
	}
}

/**
 *  Enlarges the boundary of the <code>Envelope</code> so that it contains
 *  <code>other</code>. Does nothing if <code>other</code> is wholly on or
 *  within the boundaries.
 *
 *@param  other  the <code>Envelope</code> to merge with
 */
void Envelope::expandToInclude(const Envelope* other) {
	if (other->isNull()) {
		return;
	}
	if (isNull()) {
		minx = other->getMinX();
		maxx = other->getMaxX();
		miny = other->getMinY();
		maxy = other->getMaxY();
	} else {
		if (other->minx < minx) {
			minx = other->minx;
		}
		if (other->maxx > maxx) {
			maxx = other->maxx;
		}
		if (other->miny < miny) {
			miny = other->miny;
		}
		if (other->maxy > maxy) {
			maxy = other->maxy;
		}
	}
}

/**
 *  Returns <code>true</code> if the given point lies in or on the envelope.
 *
 *@param  p  the point which this <code>Envelope</code> is
 *      being checked for containing
 *@return    <code>true</code> if the point lies in the interior or
 *      on the boundary of this <code>Envelope</code>.
 */
bool Envelope::contains(const Coordinate& p) const {
	return contains(p.x, p.y);
}

/**
 *  Returns <code>true</code> if the given point lies in or on the envelope.
 *
 *@param  x  the x-coordinate of the point which this <code>Envelope</code> is
 *      being checked for containing
 *@param  y  the y-coordinate of the point which this <code>Envelope</code> is
 *      being checked for containing
 *@return    <code>true</code> if <code>(x, y)</code> lies in the interior or
 *      on the boundary of this <code>Envelope</code>.
 */
bool Envelope::contains(double x, double y) const {
	return  x >= minx &&
			x <= maxx &&
			y >= miny &&
			y <= maxy;
}

/**
 *  Returns <code>true</code> if the <code>Envelope other</code>
 *  lies wholely inside this <code>Envelope</code> (inclusive of the boundary).
 *
 *@param  other  the <code>Envelope</code> which this <code>Envelope</code> is
 *        being checked for containing
 *@return        <code>true</code> if <code>other</code>
 *              is contained in this <code>Envelope</code>
 */
bool Envelope::contains(const Envelope* other) const {
	if (isNull() || other->isNull()) { return false; }
	return  other->getMinX() >= minx &&
			other->getMaxX() <= maxx &&
			other->getMinY() >= miny &&
			other->getMaxY() <= maxy;
}

/**
 * Check if the point <code>other</code>
 * overlaps (lies inside) the region of this <code>Envelope</code>.
 *
 * @param  other  the Coordinate to be tested
 * @return  <code>true</code> if the point overlaps this Envelope
 */
bool Envelope::intersects(const Coordinate& other) const {
	return intersects(other.x, other.y);
}

/**
* @deprecated Use #intersects instead.
*/
bool Envelope::overlaps(const Coordinate& p) const {
	return intersects(p);
}

/**
 *  Check if the point <code>(x, y)</code>
 *  overlaps (lies inside) the region of this <code>Envelope</code>.
 *
 *@param  x  the x-ordinate of the point
 *@param  y  the y-ordinate of the point
 *@return        <code>true</code> if the point overlaps this <code>Envelope</code>
 */
bool Envelope::intersects(double x, double y) const {
	return !(x > maxx ||
			 x < minx ||
			 y > maxy ||
			 y < miny);
}

/**
* @deprecated Use #intersects instead.
*/
bool Envelope::overlaps(double x, double y) const {
	return intersects(x,y);
}


/**
 *  Check if the region defined by <code>other</code>
 *  overlaps (intersects) the region of this <code>Envelope</code>.
 *
 *@param  other  the <code>Envelope</code> which this <code>Envelope</code> is
 *          being checked for overlapping
 *@return        <code>true</code> if the <code>Envelope</code>s overlap
 */
bool Envelope::intersects(const Envelope* other) const {
	if (isNull() || other->isNull()) { return false; }
	return !(other->getMinX() > maxx ||
			 other->getMaxX() < minx ||
			 other->getMinY() > maxy ||
			 other->getMaxY() < miny);
}

/**
* @deprecated Use #intersects instead. In the future, #overlaps may be
* changed to be a true overlap check; that is, whether the intersection is
* two-dimensional.
*/
bool Envelope::overlaps(const Envelope *other) const {
	return intersects(other);
}

/**
 *  Returns a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I> .
 *
 *@return    a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I>
 */
string Envelope::toString() const {
	string result("");
	char buffer[255];
	sprintf(buffer,"Env[%g:%g,%g:%g]",minx,maxx,miny,maxy);
	result.append(buffer);
	result.append("");
	return result;
}

/**
* Computes the distance between this and another
* <code>Envelope</code>.
* The distance between overlapping Envelopes is 0.  Otherwise, the
* distance is the Euclidean distance between the closest points.
*/
double Envelope::distance(const Envelope* env) const {
	if (intersects(env)) return 0;
	double dx=0.0;
	if(maxx<env->minx) dx=env->minx-maxx;
	if(minx>env->maxx) dx=minx-env->maxx;
	double dy=0.0;
	if(maxy<env->miny) dy=env->miny-maxy;
	if(miny>env->maxy) dy=miny-env->maxy;
	// if either is zero, the envelopes overlap either vertically or horizontally
	if (dx==0.0) return dy;
	if (dy==0.0) return dx;
	return sqrt(dx*dx+dy*dy);
}

// Checks if two Envelopes are equal
bool operator==(const Envelope a, const Envelope b) {
	if (a.isNull()) {
		return b.isNull();
	}
	if (b.isNull()) {
		return a.isNull();
	}
	return a.getMaxX() == b.getMaxX() &&
		   a.getMaxY() == b.getMaxY() &&
		   a.getMinX() == b.getMinX() &&
		   a.getMinY() == b.getMinY();
}

int Envelope::hashCode() const{
	//Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
	int result = 17;
	result = 37 * result + Coordinate::hashCode(minx);
	result = 37 * result + Coordinate::hashCode(maxx);
	result = 37 * result + Coordinate::hashCode(miny);
	result = 37 * result + Coordinate::hashCode(maxy);
	return result;
}

}

