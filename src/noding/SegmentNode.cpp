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
 * Last port: noding/SegmentNode.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#include <iostream>
#include <sstream>
#include <iomanip>

#include <geos/noding/SegmentNode.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/geom/Coordinate.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding


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
		assert(0); // invalid octant value
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

/*public*/
SegmentNode::SegmentNode(const NodedSegmentString& ss, const Coordinate& nCoord,
		unsigned int nSegmentIndex, int nSegmentOctant)
	:
	segString(ss),
	segmentOctant(nSegmentOctant),
	coord(nCoord),
	segmentIndex(nSegmentIndex)
{
	// Number of points in NodedSegmentString is one-more number of segments
	assert(segmentIndex < segString.size() );

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

#if GEOS_DEBUG
	cerr << setprecision(17) << "compareTo: " << *this << ", " << other <<endl;
#endif

	if (coord.equals2D(other.coord)) {

#if GEOS_DEBUG
		cerr << " Coordinates equal!"<<endl;
#endif

		return 0;
	}

#if GEOS_DEBUG
	cerr << " Coordinates do not equal!"<<endl;
#endif

	return SegmentPointComparator::compare(segmentOctant, coord,
			other.coord);
}

ostream& operator<< (ostream& os, const SegmentNode& n)
{
	return os<<n.coord<<" seg#="<<n.segmentIndex<<" octant#="<<n.segmentOctant<<endl;
}

} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.14  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.13  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.12  2006/02/28 17:44:27  strk
 * Added a check in SegmentNode::addSplitEdge to prevent attempts
 * to build SegmentString with less then 2 points.
 * This is a temporary fix for the buffer.xml assertion failure, temporary
 * as Martin Davis review would really be needed there.
 *
 * Revision 1.11  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.10  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
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

