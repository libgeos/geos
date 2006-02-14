/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <sstream>
#include <geos/noding.h>

namespace geos {

SegmentNode::SegmentNode(const Coordinate *newCoord, int nSegmentIndex, double newDist)
		:
		coord(*newCoord),
		segmentIndex(nSegmentIndex),
		dist(newDist)
{
	//coord=*newCoord; 
	//coord=new Coordinate(*newCoord);
	//segmentIndex=nSegmentIndex;
	//dist=newDist;
}

//SegmentNode::~SegmentNode()
//{
	//delete coord;
//}

/**
 * @return -1 this EdgeIntersection is located before the argument location
 * @return 0 this EdgeIntersection is at the argument location
 * @return 1 this EdgeIntersection is located after the argument location
 */
int
SegmentNode::compare(int cSegmentIndex, double cDist)
{
	if (segmentIndex < cSegmentIndex) return -1;
	if (segmentIndex > cSegmentIndex) return 1;
	if (dist<cDist) return -1;
	if (dist>cDist) return 1;
	return 0;
}

bool
SegmentNode::isEndPoint(int maxSegmentIndex)
{
	if (segmentIndex == 0 && dist == 0.0) return true;
	if (segmentIndex == maxSegmentIndex) return true;
	return false;
}

int
SegmentNode::compareTo(void* obj)
{
	SegmentNode *other=(SegmentNode*) obj;
	return compare(other->segmentIndex, other->dist);
}

string
SegmentNode::print()
{
	ostringstream s;
	s<<coord.toString()<<" seg#="<<segmentIndex<<" dist="<<dist;
	return s.str();
}

} // namespace geos

/**********************************************************************
 * $Log$
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

