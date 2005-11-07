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
 * Revision 1.4  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.3  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <sstream>
#include <geos/geomgraph.h>

namespace geos {

EdgeIntersection::EdgeIntersection(const Coordinate& newCoord, int newSegmentIndex, double newDist) {
	coord=newCoord;
	segmentIndex=newSegmentIndex;
	dist=newDist;
}

EdgeIntersection::~EdgeIntersection() {
	//aaa
}

int
EdgeIntersection::compare(int newSegmentIndex, double newDist) const
{
	if (segmentIndex<newSegmentIndex) return -1;
	if (segmentIndex>newSegmentIndex) return 1;
	if (dist<newDist) return -1;
	if (dist>newDist) return 1;
	return 0;
}

bool EdgeIntersection::isEndPoint(int maxSegmentIndex) {
	if (segmentIndex==0 && dist==0.0) return true;
	if (segmentIndex==maxSegmentIndex) return true;
	return false;
}

string
EdgeIntersection::print() const
{
	ostringstream s;
	s<<coord.toString()<<" seg#="<<segmentIndex<<" dist="<<dist;
	return s.str();

}

int
EdgeIntersection::compareTo(const EdgeIntersection *other) const
{
	return compare(other->segmentIndex, other->dist);
}

} // namespace geos


