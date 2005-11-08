/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <sstream>
#include <geos/geom.h>

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
bool
Envelope::intersects(const Coordinate& p1, const Coordinate& p2,
	const Coordinate& q1, const Coordinate& q2)
{
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
double
Envelope::distance(double x0,double y0,double x1,double y1)
{
	double dx=x1-x0;
	double dy=y1-y0;
	return sqrt(dx*dx+dy*dy);
}

/**
 *  Creates a null <code>Envelope</code>.
 */
Envelope::Envelope(void)
{
	init();
}

/**
 * Creates an <code>Envelope</code> for a region defined by maximum and
 * minimum values.
 *
 *@param  x1  the first x-value
 *@param  x2  the second x-value
 *@param  y1  the first y-value
 *@param  y2  the second y-value
 */
Envelope::Envelope(double x1, double x2, double y1, double y2)
{
	init(x1, x2, y1, y2);
}

/**
 *  Creates an <code>Envelope</code> for a region defined by two Coordinates.
 *
 *@param  p1  the first Coordinate
 *@param  p2  the second Coordinate
 */
Envelope::Envelope(const Coordinate& p1, const Coordinate& p2)
{
	init(p1, p2);
}

/**
 *  Creates an Envelope for a region defined by a single Coordinate.
 *
 * @param  p  the Coordinate
 */
Envelope::Envelope(const Coordinate& p)
{
	init(p);
}

/**
 *  Create an <code>Envelope</code> from an existing Envelope.
 *
 * @param  env  the Envelope to initialize from
 */
Envelope::Envelope(const Envelope &env)
{
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
 *  Initialize an <code>Envelope</code> for a region defined by
 *  maximum and minimum values.
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
 * @param  p1  the first Coordinate
 * @param  p2  the second Coordinate
 */
void
Envelope::init(const Coordinate& p1, const Coordinate& p2)
{
	init(p1.x, p2.x, p1.y, p2.y);
}

/**
 * Initialize an <code>Envelope</code> to a region defined by a single
 * Coordinate.
 *
 *@param  p  the Coordinate
 */
void
Envelope::init(const Coordinate& p)
{
	init(p.x, p.x, p.y, p.y);
}

/**
 *  Initialize an <code>Envelope</code> from an existing Envelope.
 *
 *@param  env  the Envelope to initialize from
 */
void
Envelope::init(Envelope env)
{
	init(env.minx, env.maxx, env.miny, env.maxy);
}

/**
 *  Makes this <code>Envelope</code> a "null" envelope, that is, the envelope
 *  of the empty geometry.
 */
void
Envelope::setToNull()
{
	minx=0;
	maxx=-1;
	miny=0;
	maxy=-1;
}

/*
 *  Returns <code>true</code> if this <code>Envelope</code> is a "null"
 *  envelope.
 *
 *@return    <code>true</code> if this <code>Envelope</code> is uninitialized
 *      or is the envelope of the empty geometry.
 */
//bool Envelope::isNull() const {
	//return maxx < minx;
//}

/**
 *  Returns the difference between the maximum and minimum x values.
 *
 *@return    max x - min x, or 0 if this is a null <code>Envelope</code>
 */
double
Envelope::getWidth() const
{
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
 *  Enlarges the boundary of the <code>Envelope</code> so that it contains
 *  p. Does nothing if p is already on or within the boundaries.
 *
 * @param  p  the Coordinate to include
 */
void
Envelope::expandToInclude(const Coordinate& p)
{
	expandToInclude(p.x, p.y);
}

/**
 * Enlarges the boundary of the <code>Envelope</code> so that it contains
 * (x,y). Does nothing if (x,y) is already on or within the boundaries.
 *
 * @param  x  the value to lower the minimum x to or to raise the maximum x to
 * @param  y  the value to lower the minimum y to or to raise the maximum y to
 */
void
Envelope::expandToInclude(double x, double y)
{
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
 * @param  other  the <code>Envelope</code> to merge with
 */
void
Envelope::expandToInclude(const Envelope* other)
{
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
 * @param  p  the point which this <code>Envelope</code> is
 *      being checked for containing
 * @return    <code>true</code> if the point lies in the interior or
 *      on the boundary of this <code>Envelope</code>.
 */
bool
Envelope::contains(const Coordinate& p) const
{
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
bool
Envelope::contains(double x, double y) const
{
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
bool
Envelope::contains(const Envelope* other) const
{
	if (isNull() || other->isNull()) { return false; }
	return  other->getMinX() >= minx &&
			other->getMaxX() <= maxx &&
			other->getMinY() >= miny &&
			other->getMaxY() <= maxy;
}

/*
 * Returns <code>true</code> if the <code>Envelope other</code>
 * spatially equals this <code>Envelope</code>.
 *
 * @param  other the <code>Envelope</code> which this <code>Envelope</code>
 *	  is being checked for equality
 * @return <code>true</code> if this and <code>other</code>
 *         Envelope objs are spatially equal
 */
bool
Envelope::equals(const Envelope* other) const
{
	if (isNull() || other->isNull()) { return false; }
	return  other->getMinX() == minx &&
			other->getMaxX() == maxx &&
			other->getMinY() == miny &&
			other->getMaxY() == maxy;
}

/**
 *  Returns a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I> .
 *
 * @return a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I>
 */
string
Envelope::toString() const
{
	ostringstream s;
	s<<"Env["<<minx<<":"<<maxx<<","<<miny<<":"<<maxy<<"]";
	return s.str();
}

/**
 * Computes the distance between this and another
 * <code>Envelope</code>.
 * The distance between overlapping Envelopes is 0.  Otherwise, the
 * distance is the Euclidean distance between the closest points.
 */
double
Envelope::distance(const Envelope* env) const
{
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
bool
operator==(const Envelope a, const Envelope b)
{
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

int
Envelope::hashCode() const
{
	//Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
	int result = 17;
	result = 37 * result + Coordinate::hashCode(minx);
	result = 37 * result + Coordinate::hashCode(maxx);
	result = 37 * result + Coordinate::hashCode(miny);
	result = 37 * result + Coordinate::hashCode(maxy);
	return result;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.21  2005/11/08 10:03:28  strk
 * Set library version to 2.2.0.
 * Cleaned up Doxygen warnings.
 * Inlined more Envelope methods.
 * Dropped deprecated Envelope::overlaps methods.
 *
 * Revision 1.20  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.19  2005/02/01 16:06:53  strk
 * Small optimizations.
 *
 * Revision 1.18  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.17  2004/11/08 10:58:08  strk
 * Optimized the ::intersect function to avoid nested function calls.
 *
 * Revision 1.16  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.15  2004/09/16 09:48:06  strk
 * Added Envelope::equals
 *
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

