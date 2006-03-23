/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_LINESEGMENT_INL
#define GEOS_LINESEGMENT_INL

#include <cassert>
#include <geos/geosAlgorithm.h>

#include <iostream>

namespace geos {
namespace geom { // geos::geom

INLINE
LineSegment::LineSegment(const LineSegment& ls)
	:
	p0(ls.p0),
	p1(ls.p1)
{
}

INLINE
LineSegment::LineSegment(const Coordinate& c0, const Coordinate& c1)
	:
	p0(c0),
	p1(c1)
{
}

INLINE
LineSegment::LineSegment()
{
}

INLINE
LineSegment::~LineSegment()
{
}

INLINE double
LineSegment::distancePerpendicular(const Coordinate& p) const
{
	return algorithm::CGAlgorithms::distancePointLinePerpendicular(p, p0, p1);
}

INLINE double
LineSegment::distance(const LineSegment& ls) const
{
	return algorithm::CGAlgorithms::distanceLineLine(p0, p1, ls.p0, ls.p1);
}

/*public*/
INLINE double
LineSegment::distance(const Coordinate& p) const
{
	return algorithm::CGAlgorithms::distancePointLine(p, p0, p1);
}

INLINE void
LineSegment::normalize()
{
	if (p1.compareTo(p0)<0) reverse();
}


INLINE void
LineSegment::setCoordinates(const Coordinate& c0, const Coordinate& c1)
{
	p0=c0; p1=c1;
}

INLINE const Coordinate&
LineSegment::operator[](unsigned int i) const
{
	if (i==0) return p0;
	assert(i==1);
	return p1;
}

INLINE Coordinate&
LineSegment::operator[](unsigned int i) 
{
	if (i==0) return p0;
	assert(i==1);
	return p1;
}

INLINE void
LineSegment::setCoordinates(const LineSegment& ls)
{
	setCoordinates(ls.p0,ls.p1);
}

INLINE double
LineSegment::getLength() const
{
	return p0.distance(p1);
}

INLINE bool
LineSegment::isHorizontal() const
{
	return p0.y == p1.y;
}

INLINE bool
LineSegment::isVertical() const
{
	return p0.x == p1.x;
}

INLINE int
LineSegment::orientationIndex(const LineSegment* seg) const
{
	assert(seg);
	return orientationIndex(*seg);
}

INLINE CoordinateSequence*
LineSegment::closestPoints(const LineSegment* line)
{
	assert(line);
	return closestPoints(*line);
}

INLINE double
LineSegment::angle() const
{
	return atan2(p1.y-p0.y,p1.x-p0.x);
}

INLINE std::ostream&
operator<< (std::ostream& o, const LineSegment& l)
{
	return o<<"LINESEGMENT("<<l.p0.x<<" "<<l.p0.y<<","<<l.p1.x<<" "<<l.p1.y<<")";
}

INLINE bool
operator==(const LineSegment& a, const LineSegment& b)
{
	return a.p0==b.p0 && a.p1==b.p1;
}


} // namespace geos::geom
} // namespace geos

#endif // GEOS_LINESEGMENT_INL
