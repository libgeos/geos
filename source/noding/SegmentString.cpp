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

#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentNodeList.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/noding/Octant.h>
//#include <geos/profiler.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifndef GEOS_INLINE
# include "geos/noding/SegmentString.inl"
#endif

#include <iostream>
#include <sstream>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*public*/
void
SegmentString::addIntersections(LineIntersector *li, unsigned int segmentIndex,
		int geomIndex)
{
	for (int i=0, n=li->getIntersectionNum(); i<n; ++i) {
		addIntersection(li, segmentIndex, geomIndex, i);
	}
}

/*public*/
void
SegmentString::addIntersection(LineIntersector *li, unsigned int segmentIndex,
		int geomIndex, int intIndex)
{
    UNREFERENCED_PARAMETER(geomIndex);

	const Coordinate &intPt=li->getIntersection(intIndex);
	addIntersection(intPt, segmentIndex);
}

/*public*/
void
SegmentString::addIntersection(const Coordinate& intPt,
	unsigned int segmentIndex)
{
	unsigned int normalizedSegmentIndex = segmentIndex;

	if ( segmentIndex > size()-2 )
	{
		throw util::IllegalArgumentException("SegmentString::addIntersection: SegmentIndex out of range");
	}

	// normalize the intersection point location
	unsigned int nextSegIndex = normalizedSegmentIndex + 1;
	if (nextSegIndex < size())
	{
		const Coordinate& nextPt = pts->getAt(nextSegIndex);

		// Normalize segment index if intPt falls on vertex
		// The check for point equality is 2D only -
		// Z values are ignored
		if (intPt.equals2D(nextPt)) {
			normalizedSegmentIndex = nextSegIndex;
		}
	}

	/*
	 * Add the intersection point to edge intersection list
	 * (unless the node is already known)
	 */
	//SegmentNode *ei=
	nodeList.add(intPt, normalizedSegmentIndex);


	testInvariant();
}

/* public static */
void
SegmentString::getNodedSubstrings(const SegmentString::NonConstVect& segStrings,
	SegmentString::NonConstVect *resultEdgeList)
{
	assert(resultEdgeList);
	for ( SegmentString::NonConstVect::const_iterator
		i=segStrings.begin(), iEnd=segStrings.end();
		i != iEnd; ++i )
	{
		SegmentString* ss = *i;
		assert(ss);
		ss->getNodeList().addSplitEdges(resultEdgeList);
	}
}

/* public static */
SegmentString::NonConstVect*
SegmentString::getNodedSubstrings(const SegmentString::NonConstVect& segStrings)
{
	SegmentString::NonConstVect* resultEdgelist = \
		new SegmentString::NonConstVect();
	getNodedSubstrings(segStrings, resultEdgelist);
	return resultEdgelist;
}

/*public*/
int
SegmentString::getSegmentOctant(unsigned int index) const
{
	testInvariant();
	if (index >= size() - 1) return -1;
	return Octant::octant(getCoordinate(index), getCoordinate(index+1));
}

/* public */
void
SegmentString::notifyCoordinatesChange() const
{
	npts = pts->size();

	// What about SegmentNodes ? should
	// we invalidate them as well ?

	testInvariant();
}


std::ostream& operator<< (std::ostream& os, const SegmentString& ss)
{
	os << "SegmentString: " << std::endl;
	os << " LINESTRING" << *(ss.pts) << ";" << std::endl;
	os << " Nodes: " << ss.nodeList.size() << std::endl;

	return os;
}


} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.30  2006/05/05 14:25:05  strk
 * moved getSegmentOctant out of .inl into .cpp, renamed private eiList to nodeList as in JTS, added more assertion checking and fixed doxygen comments
 *
 * Revision 1.29  2006/05/04 07:43:44  strk
 * output operator for SegmentString class
 *
 * Revision 1.28  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.27  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.26  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.25  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.24  2006/02/28 17:44:27  strk
 * Added a check in SegmentNode::addSplitEdge to prevent attempts
 * to build SegmentString with less then 2 points.
 * This is a temporary fix for the buffer.xml assertion failure, temporary
 * as Martin Davis review would really be needed there.
 *
 * Revision 1.23  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.22  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.21  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.20  2006/02/15 14:59:08  strk
 * JTS-1.7 sync for:
 * noding/SegmentNode.cpp
 * noding/SegmentNodeList.cpp
 * noding/SegmentString.cpp
 *
 * Revision 1.19  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.18  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.17  2005/12/08 01:39:28  strk
 * SegmentString::eiList made a real object rather then a pointer.
 * Adde getter for const and non-const references of it (dropping get by pointer)
 *
 * Revision 1.16  2005/02/22 18:21:46  strk
 * Changed SegmentNode to contain a *real* Coordinate (not a pointer) to reduce
 * construction costs.
 *
 * Revision 1.15  2005/02/22 16:23:28  strk
 * Cached number of points in CoordinateSequence.
 *
 * Revision 1.14  2005/02/22 15:49:20  strk
 * Reduced calls to CoordinateArraySequence->getSize().
 *
 * Revision 1.13  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.12  2005/02/01 14:18:36  strk
 * More profiler labels
 *
 * Revision 1.11  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.10  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.9  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.6  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.5  2004/05/27 10:27:03  strk
 * Memory leaks fixed.
 *
 * Revision 1.4  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.3  2004/05/06 15:54:15  strk
 * SegmentNodeList keeps track of created splitEdges for later destruction.
 * SegmentString constructor copies given Label.
 * Buffer operation does no more leaks for doc/example.cpp
 *
 * Revision 1.2  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

