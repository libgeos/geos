/**********************************************************************
 * $Id: EdgeNodingValidator.cpp 2545 2009-06-05 12:07:52Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/EdgeNodingValidator.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/

#include <vector>

#include <geos/geomgraph/EdgeNodingValidator.h>
#include <geos/geomgraph/Edge.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/geom/CoordinateSequence.h>

using namespace std;
using namespace geos::noding;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

vector<SegmentString*>&
EdgeNodingValidator::toSegmentStrings(vector<Edge*>& edges)
{
	// convert Edges to SegmentStrings
	for(size_t i=0, n=edges.size(); i<n; ++i) {
		Edge *e=edges[i];
		CoordinateSequence* cs=e->getCoordinates()->clone();
		newCoordSeq.push_back(cs);
		segStr.push_back(new BasicSegmentString(cs, e));
	}
	return segStr;
}

EdgeNodingValidator::~EdgeNodingValidator()
{
	for (SegmentString::NonConstVect::iterator
			i=segStr.begin(), e=segStr.end();
			i != e;
			++i)
	{
		delete *i;
	}

	for(size_t i=0, n=newCoordSeq.size(); i<n; ++i)
		delete newCoordSeq[i];
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.7  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.4  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

