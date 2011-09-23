/**********************************************************************
 * $Id: MCIndexSnapRounder.cpp 3372 2011-05-19 19:32:06Z warmerdam $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/MCIndexSnapRounder.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/snapround/MCIndexSnapRounder.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/IntersectionFinderAdder.h>
#include <geos/noding/NodingValidator.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

#include <geos/inline.h>

#include <functional> // std::mem_fun, std::bind1st
#include <algorithm> // std::for_each 
#include <vector>


using namespace std;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

/*private*/
void
MCIndexSnapRounder::findInteriorIntersections(MCIndexNoder& noder,
		NodedSegmentString::NonConstVect* segStrings,
		vector<Coordinate>& intersections)
{
	IntersectionFinderAdder intFinderAdder(li, intersections);
	noder.setSegmentIntersector(&intFinderAdder);
	noder.computeNodes(segStrings);
}

/* private */
void
MCIndexSnapRounder::computeIntersectionSnaps(vector<Coordinate>& snapPts)
{
	for (vector<Coordinate>::iterator
			it=snapPts.begin(), itEnd=snapPts.end();
			it!=itEnd;
			++it)
	{
		Coordinate& snapPt = *it;
		HotPixel hotPixel(snapPt, scaleFactor, li);
		pointSnapper->snap(hotPixel);
	}
}

/*private*/
void
MCIndexSnapRounder::computeVertexSnaps(NodedSegmentString* e)
{
	CoordinateSequence& pts0 = *(e->getCoordinates());
	for (unsigned int i=0, n=pts0.size()-1; i<n; ++i)
	{
		HotPixel hotPixel(pts0[i], scaleFactor, li);
		bool isNodeAdded = pointSnapper->snap(hotPixel, e, i);
		// if a node is created for a vertex, that vertex must be noded too
		if (isNodeAdded) {
			e->addIntersection(pts0[i], i);
		}
	}
}

/*public*/
void
MCIndexSnapRounder::computeVertexSnaps(SegmentString::NonConstVect& edges)
{
	SegmentString::NonConstVect::iterator i=edges.begin(), e=edges.end();
	for (; i!=e; ++i)
	{
		NodedSegmentString* edge0 =
			dynamic_cast<NodedSegmentString*>(*i);
		assert(edge0);
		computeVertexSnaps(edge0);
	}
}

/*private*/
void
MCIndexSnapRounder::snapRound(MCIndexNoder& noder, 
		SegmentString::NonConstVect* segStrings)
{
	vector<Coordinate> intersections;
 	findInteriorIntersections(noder, segStrings, intersections);
	computeIntersectionSnaps(intersections);
	computeVertexSnaps(*segStrings);
	
}

/*public*/
void
MCIndexSnapRounder::computeNodes(SegmentString::NonConstVect* inputSegmentStrings)
{
	nodedSegStrings = inputSegmentStrings;
	MCIndexNoder noder;
	pointSnapper.release(); // let it leak
	pointSnapper.reset(new MCIndexPointSnapper(noder.getIndex()));
	snapRound(noder, inputSegmentStrings);

	// testing purposes only - remove in final version
	assert(nodedSegStrings == inputSegmentStrings);
	//checkCorrectness(*inputSegmentStrings);
}

/*private*/
void
MCIndexSnapRounder::checkCorrectness(
	SegmentString::NonConstVect& inputSegmentStrings)
{
	auto_ptr<SegmentString::NonConstVect> resultSegStrings(
		NodedSegmentString::getNodedSubstrings(inputSegmentStrings)
	);

	NodingValidator nv(*(resultSegStrings.get()));
	try {
		nv.checkValid();
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		throw;
	}
}


} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/05/05 15:40:39  strk
 * Had nodind validation error throw an exception for SimpleSnapRounder
 * and MCIndexSnapRounder
 *
 * Revision 1.10  2006/04/03 10:44:19  strk
 * Added missing headers
 *
 * Revision 1.9  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.8  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 * Revision 1.7  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.6  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.5  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.4  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.3  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.2  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
