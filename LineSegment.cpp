#include "geom.h"

/**
 *  Constructs an empty <code>LineSegment</code>.
 */
LineSegment::LineSegment(void) {
	Coordinate p0;
	Coordinate p1;
}

/**
 *  Constructs a <code>LineSegment</code> with the given start and end coordinates.
 *
 *@param  c0      start of the <code>LineSegment</code>.
 *@param  c1      end of the <code>LineSegment</code>.
 */
LineSegment::LineSegment(Coordinate c0, Coordinate c1) {
	p0=c0;
	p1=c1;
}

/// Default destructor
LineSegment::~LineSegment(void){}

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