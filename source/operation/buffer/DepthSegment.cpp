/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/buffer/DepthSegment.h>
#include <geos/geom/LineSegment.h>

using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

DepthSegment::DepthSegment(const LineSegment &seg, int depth):
	upwardSeg(seg), leftDepth(depth)
{
	// input seg is assumed to be normalized
	//upwardSeg.normalize();
}

DepthSegment::~DepthSegment()
{
}

/**
 * Defines a comparision operation on DepthSegments
 * which orders them left to right
 *
 * <pre>
 * DS1 < DS2   if   DS1->seg is left of DS2->seg
 * DS1 > DS2   if   DS1->seg is right of DS2->seg
 * </pre>
 *
 * @param obj
 * @return
 */
int
DepthSegment::compareTo(DepthSegment *other)
{
	/**
	 * try and compute a determinate orientation for the segments->
	 * Test returns 1 if other is left of this (i->e-> this > other)
	 */
	int orientIndex=upwardSeg.orientationIndex(&(other->upwardSeg));

	/**
	 * If comparison between this and other is indeterminate,
	 * try the opposite call order->
	 * orientationIndex value is 1 if this is left of other,
	 * so have to flip sign to get proper comparison value of
	 * -1 if this is leftmost
	 */
	if (orientIndex==0)
		orientIndex=-1 * other->upwardSeg.orientationIndex(&upwardSeg);

	// if orientation is determinate, return it
	if (orientIndex != 0)
		return orientIndex;

	// otherwise, segs must be collinear - sort based on minimum X value
	return compareX(&upwardSeg, &(other->upwardSeg));
}

/**
 * Compare two collinear segments for left-most ordering.
 * If segs are vertical, use vertical ordering for comparison.
 * If segs are equal, return 0.
 * Segments are assumed to be directed so that the second
 * coordinate is >= to the first
 * (e->g-> up and to the right)
 *
 * @param seg0 a segment to compare
 * @param seg1 a segment to compare
 * @return
 */
int
DepthSegment::compareX(LineSegment *seg0, LineSegment *seg1)
{
	int compare0=seg0->p0.compareTo(seg1->p0);
	if (compare0!=0)
		return compare0;
	return seg0->p1.compareTo(seg1->p1);

}

bool
DepthSegmentLT(DepthSegment *first, DepthSegment *second)
{
	if (first->compareTo(second)<0)
		return true;
	else
		return false;
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos
