/**********************************************************************
 * $Id: SimpleSnapRounder.cpp 3239 2011-02-23 14:37:29Z strk $
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
 * Last port: noding/snapround/SimpleSnapRounder.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/noding/snapround/SimpleSnapRounder.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/NodingValidator.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/IntersectionFinderAdder.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/LineIntersector.h>

#include <vector>
#include <exception>
#include <iostream>
#include <cassert>

using namespace std;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

/*public*/
SimpleSnapRounder::SimpleSnapRounder(const geom::PrecisionModel& newPm)
	:
	pm(newPm),
	li(&newPm),
	scaleFactor(newPm.getScale())
{
}

/*public*/
std::vector<SegmentString*>*
SimpleSnapRounder::getNodedSubstrings() const
{
	return nodedSegStrings;
}

/*public*/
void
SimpleSnapRounder::computeNodes(
		std::vector<SegmentString*>* inputSegmentStrings)
{
	nodedSegStrings = inputSegmentStrings;
	snapRound(inputSegmentStrings, li);

	// testing purposes only - remove in final version
	assert(nodedSegStrings == inputSegmentStrings);
	checkCorrectness(*inputSegmentStrings);
}


/*private*/
void
SimpleSnapRounder::checkCorrectness(
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

/*private*/
void
SimpleSnapRounder::computeSnaps(const SegmentString::NonConstVect& segStrings,
		vector<Coordinate>& snapPts)
{
	for (SegmentString::NonConstVect::const_iterator
			i=segStrings.begin(), iEnd=segStrings.end();
			i!=iEnd; ++i)
	{
		NodedSegmentString* ss = dynamic_cast<NodedSegmentString*>(*i);

		computeSnaps(ss, snapPts);
	}
}

/*private*/
void
SimpleSnapRounder::computeSnaps(NodedSegmentString* ss, vector<Coordinate>& snapPts)
{
	for (vector<Coordinate>::iterator
		it=snapPts.begin(), itEnd=snapPts.end();
		it!=itEnd; ++it)
	{
		const Coordinate& snapPt = *it;
		HotPixel hotPixel(snapPt, scaleFactor, li);
		for (int i=0, n=ss->size()-1; i<n; ++i) {
			hotPixel.addSnappedNode(*ss, i);
		}
	}
}

/*private*/
void
SimpleSnapRounder::computeVertexSnaps(NodedSegmentString* e0, NodedSegmentString* e1)
{
	const CoordinateSequence* pts0 = e0->getCoordinates();
	const CoordinateSequence* pts1 = e1->getCoordinates();

	for (unsigned int i0=0, n0=pts0->getSize()-1; i0<n0; i0++)
	{
		const Coordinate& p0 = pts0->getAt(i0);

		HotPixel hotPixel(p0, scaleFactor, li);
		for (unsigned int i1=1, n1=pts1->getSize()-1; i1<n1; i1++)
		{
        		// don't snap a vertex to itself
			if (i0 == i1 && e0 == e1) {
				continue;
			}
//cerr<<"trying "<<p0<<" against "<<pts1->getAt(i1)<<" "<<pts1->getAt(i1 + 1)<<endl;
			bool isNodeAdded = hotPixel.addSnappedNode(*e1, i1);
			// if a node is created for a vertex, that vertex must be noded too
			if (isNodeAdded) {
				e0->addIntersection(p0, i0);
			}
		}
	}
 
}

/*public*/
void
SimpleSnapRounder::computeVertexSnaps(const SegmentString::NonConstVect& edges)
{
	for (SegmentString::NonConstVect::const_iterator
			i0=edges.begin(), i0End=edges.end();
			i0!=i0End; ++i0)
	{
		NodedSegmentString* edge0 = dynamic_cast<NodedSegmentString*>(*i0);
		assert(edge0);
		for (SegmentString::NonConstVect::const_iterator
				i1=edges.begin(), i1End=edges.end();
				i1!=i1End; ++i1)
		{
			NodedSegmentString* edge1 = dynamic_cast<NodedSegmentString*>(*i1);
			assert(edge1);
			computeVertexSnaps(edge0, edge1);
		}
	}
}


/*private*/
void
SimpleSnapRounder::snapRound(SegmentString::NonConstVect* segStrings,
		LineIntersector& li)
{
	assert(segStrings);

	vector<Coordinate> intersections;
	findInteriorIntersections(*segStrings, li, intersections);
	computeSnaps(*segStrings, intersections);
	computeVertexSnaps(*segStrings);
}

/*private*/
void
SimpleSnapRounder::findInteriorIntersections(
	SegmentString::NonConstVect& segStrings,
	LineIntersector& li, vector<Coordinate>& ret)
{
	IntersectionFinderAdder intFinderAdder(li, ret);
	MCIndexNoder noder;
	noder.setSegmentIntersector(&intFinderAdder);
	noder.computeNodes(&segStrings);
	//intFinderAdder.getInteriorIntersections();
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
 * Revision 1.10  2006/05/03 17:54:58  strk
 * Uncommented correctness checker
 *
 * Revision 1.9  2006/05/03 15:49:16  strk
 * fixed bug in SimpleSnapRounder::computeSnaps, trying to access out-of-bound vector element.
 *
 * Revision 1.8  2006/05/03 15:02:49  strk
 * moved some implementations from header to .cpp file (taken out of inline)
 *
 * Revision 1.7  2006/03/15 09:51:13  strk
 * streamlined headers usage
 *
 * Revision 1.6  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.5  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.4  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.3  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.2  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
