#include "geom.h"
#include "stdio.h"

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
Envelope::Envelope(Coordinate& p1, Coordinate& p2){
	init(p1, p2);
}

/**
 *  Creates an <code>Envelope</code> for a region defined by a single Coordinate.
 *
 *@param  p1  the Coordinate
 */
Envelope::Envelope(Coordinate& p){
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
void Envelope::init(Coordinate& p1, Coordinate& p2){
	init(p1.x, p2.x, p1.y, p2.y);
}

/**
 *  Initialize an <code>Envelope</code> to a region defined by a single Coordinate.
 *
 *@param  p  the Coordinate
 */
void Envelope::init(Coordinate& p){
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
bool Envelope::isNull() {
	return maxx < minx;
}

/**
 *  Returns the difference between the maximum and minimum x values.
 *
 *@return    max x - min x, or 0 if this is a null <code>Envelope</code>
 */
double Envelope::getWidth() {
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
double Envelope::getHeight() {
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
double Envelope::getMaxY() {
	return maxy;
}

/**
 *  Returns the <code>Envelope</code>s maximum x-value. min x > max x
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the maximum x-coordinate
 */
double Envelope::getMaxX() {
	return maxx;
}

/**
 *  Returns the <code>Envelope</code>s minimum y-value. min y > max y
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the minimum y-coordinate
 */
double Envelope::getMinY() {
	return miny;
}

/**
 *  Returns the <code>Envelope</code>s minimum x-value. min x > max x
 *  indicates that this is a null <code>Envelope</code>.
 *
 *@return    the minimum x-coordinate
 */
double Envelope::getMinX() {
	return minx;
}

/**
 *  Enlarges the boundary of the <code>Envelope</code> so that it contains
 *  p. Does nothing if p is already on or within the boundaries.
 *
 *@param  p  the Coordinate to include
 */
void Envelope::expandToInclude(Coordinate& p) {
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
void Envelope::expandToInclude(Envelope other) {
	if (other.isNull()) {
		return;
	}
	if (isNull()) {
		minx = other.getMinX();
		maxx = other.getMaxX();
		miny = other.getMinY();
		maxy = other.getMaxY();
	} else {
		if (other.minx < minx) {
			minx = other.minx;
		}
		if (other.maxx > maxx) {
			maxx = other.maxx;
		}
		if (other.miny < miny) {
			miny = other.miny;
		}
		if (other.maxy > maxy) {
			maxy = other.maxy;
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
bool Envelope::contains(Coordinate& p) {
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
bool Envelope::contains(double x, double y) {
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
bool Envelope::contains(Envelope other) {
	return  other.getMinX() >= minx &&
			other.getMaxX() <= maxx &&
			other.getMinY() >= miny &&
			other.getMaxY() <= maxy;
}

/**
 *  Check if the point <code>p</code>
 *  overlaps (lies inside) the region of this <code>Envelope</code>.
 *
 *@param  other  the <code>Coordinate</code> to be tested
 *@return        <code>true</code> if the point overlaps this <code>Envelope</code>
 */
bool Envelope::overlaps(Coordinate& p) {
	return overlaps(p.x, p.y);
}

/**
 *  Check if the point <code>(x, y)</code>
 *  overlaps (lies inside) the region of this <code>Envelope</code>.
 *
 *@param  x  the x-ordinate of the point
 *@param  y  the y-ordinate of the point
 *@return        <code>true</code> if the point overlaps this <code>Envelope</code>
 */
bool Envelope::overlaps(double x, double y) {
	return !(x > maxx ||
			 x < minx ||
			 y > maxy ||
			 y < miny);
}

/**
 *  Check if the region defined by <code>other</code>
 *  overlaps (intersects) the region of this <code>Envelope</code>.
 *
 *@param  other  the <code>Envelope</code> which this <code>Envelope</code> is
 *          being checked for overlapping
 *@return        <code>true</code> if the <code>Envelope</code>s overlap
 */
bool Envelope::overlaps(Envelope other) {
	return !(other.getMinX() > maxx ||
			 other.getMaxX() < minx ||
			 other.getMinY() > maxy ||
			 other.getMaxY() < miny);
}

/**
 *  Returns a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I> .
 *
 *@return    a <code>string</code> of the form <I>Env[minx:maxx,miny:maxy]</I>
 */
string Envelope::toString() {
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
double Envelope::distance(Envelope env) {
	if (overlaps(env)) return 0;
	if (maxx<env.minx) {
		// this is left of env
		if (maxy<env.miny) {
			// this is below left of env
			return distance(maxx,maxy,env.minx,env.miny);
		} else if (miny>env.maxy) {
			// this is above left of env
			return distance(maxx,miny,env.minx,env.maxy);
		} else {
			// this is directly left of env
			return env.minx-maxx;
		}
	} else {
		// this is right of env
		if (maxy<env.miny) {
			// this is below right of env
			return distance(minx,maxy,env.maxx,env.miny);
		} else if (miny>env.maxy) {
			// this is above right of env
			return distance(minx,miny,env.maxx,env.maxy);
		} else {
			// this is directly right of env
			return minx-env.maxx;
		}
	}
}

/// Checks if two Envelopes are equal
bool operator==(Envelope a, Envelope b) {
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
