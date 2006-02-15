/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SegmentNode.java rev. 1.5 (JTS-1.7)
 *
 **********************************************************************/

#include <sstream>
#include <geos/noding.h>

namespace geos {


/**
 * Implements a robust method of comparing the relative position of two
 * points along the same segment.
 * The coordinates are assumed to lie "near" the segment.
 * This means that this algorithm will only return correct results
 * if the input coordinates
 * have the same precision and correspond to rounded values
 * of exact coordinates lying on the segment.
 *
 * Last port: noding/SegmentPointComparator.java rev. 1.2 (JTS-1.7)
 */
class SegmentPointComparator {

public:

	/**
	 * Compares two Coordinates for their relative position along a 
	 * segment lying in the specified Octant.
	 *
	 * @return -1 node0 occurs first
	 * @return 0 the two nodes are equal
	 * @return 1 node1 occurs first
	 */
	static int compare(int octant, const Coordinate& p0,
			const Coordinate& p1)
	{
		// nodes can only be equal if their coordinates are equal
		if (p0.equals2D(p1)) return 0;

		int xSign = relativeSign(p0.x, p1.x);
		int ySign = relativeSign(p0.y, p1.y);

		switch (octant) {
			case 0: return compareValue(xSign, ySign);
			case 1: return compareValue(ySign, xSign);
			case 2: return compareValue(ySign, -xSign);
			case 3: return compareValue(-xSign, ySign);
			case 4: return compareValue(-xSign, -ySign);
			case 5: return compareValue(-ySign, -xSign);
			case 6: return compareValue(-ySign, xSign);
			case 7: return compareValue(xSign, -ySign);
		}
		assert(0); // "invalid octant value"
		return 0;
	 
	}

	static int relativeSign(double x0, double x1)
	{
		if (x0 < x1) return -1;
		if (x0 > x1) return 1;
		return 0;
	}

	static int compareValue(int compareSign0, int compareSign1)
	{
		if (compareSign0 < 0) return -1;
		if (compareSign0 > 0) return 1;
		if (compareSign1 < 0) return -1;
		if (compareSign1 > 0) return 1;
		return 0;
	}
 
};

SegmentNode::SegmentNode(const SegmentString& ss, const Coordinate& nCoord,
		unsigned int nSegmentIndex, int nSegmentOctant)
	:
	segString(ss),
	segmentOctant(nSegmentOctant),
	coord(nCoord),
	segmentIndex(nSegmentIndex)
{
	isInteriorVar = \
		!coord.equals2D(segString.getCoordinate(segmentIndex));
}


bool
SegmentNode::isEndPoint(unsigned int maxSegmentIndex) const
{
	if (segmentIndex == 0 && ! isInteriorVar) return true;
	if (segmentIndex == maxSegmentIndex) return true;
	return false;
}

/**
 * @return -1 this EdgeIntersection is located before the argument location
 * @return 0 this EdgeIntersection is at the argument location
 * @return 1 this EdgeIntersection is located after the argument location
 */
int
SegmentNode::compareTo(const SegmentNode& other)
{
	if (segmentIndex < other.segmentIndex) return -1;
	if (segmentIndex > other.segmentIndex) return 1;

	if (coord.equals2D(other.coord)) return 0;

	return SegmentPointComparator::compare(segmentOctant, coord,
			other.coord);
}

string
SegmentNode::print()
{
	ostringstream s;
	s<<coord.toString()<<" seg#="<<segmentIndex;
	return s.str();
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/02/15 14:59:07  strk
 * JTS-1.7 sync for:
 * noding/SegmentNode.cpp
 * noding/SegmentNodeList.cpp
 * noding/SegmentString.cpp
 *
 * Revision 1.8  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.7  2005/02/22 18:21:46  strk
 * Changed SegmentNode to contain a *real* Coordinate (not a pointer) to reduce
 * construction costs.
 *
 * Revision 1.6  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.5  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.4  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/06/15 07:40:30  strk
 * Added missing <stdio.h> include
 *
 * Revision 1.2  2004/05/03 12:09:22  strk
 * newline added at end of file
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

