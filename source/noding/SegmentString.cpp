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

#include <geos/noding.h>
#include <geos/profiler.h>

namespace geos {

#if PROFILE
static Profiler *profiler=Profiler::instance();
#endif

/**
 * Adds EdgeIntersections for one or both
 * intersections found for a segment of an edge to the edge intersection list.
 */
void
SegmentString::addIntersections(LineIntersector *li, unsigned int segmentIndex,
		int geomIndex)
{
	for (int i=0; i<li->getIntersectionNum(); i++) {
		addIntersection(li,segmentIndex, geomIndex, i);
	}
}

/**
 * Add an SegmentNode for intersection intIndex.
 * An intersection that falls exactly on a vertex
 * of the SegmentString is normalized
 * to use the higher of the two possible segmentIndexes
 */
void
SegmentString::addIntersection(LineIntersector *li, unsigned int segmentIndex,
		int geomIndex, int intIndex)
{
	const Coordinate &intPt=li->getIntersection(intIndex);
	addIntersection(intPt, segmentIndex);
}

void
SegmentString::addIntersection(const Coordinate& intPt,
	unsigned int segmentIndex)
{
	unsigned int normalizedSegmentIndex = segmentIndex;

	// normalize the intersection point location
	unsigned int nextSegIndex = normalizedSegmentIndex + 1;
	if (nextSegIndex < npts)
	{
		const Coordinate& nextPt = pts->getAt(nextSegIndex);

		// Normalize segment index if intPt falls on vertex
		// The check for point equality is 2D only -
		// Z values are ignored
		if (intPt.equals2D(nextPt)) {
			normalizedSegmentIndex = nextSegIndex;
		}
	}

	/**
	 * Add the intersection point to edge intersection list.
	 */
	//SegmentNode *ei=
	eiList.add(intPt, normalizedSegmentIndex);

}

/* public static */
void
SegmentString::getNodedSubstrings(const SegmentString::NonConstVect& segStrings,
	SegmentString::NonConstVect *resultEdgeList)
{
	for ( SegmentString::NonConstVect::const_iterator
		i=segStrings.begin(), iEnd=segStrings.end();
		i != iEnd; ++i )
	{
		SegmentString* ss = *i;
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

} // namespace geos

/**********************************************************************
 * $Log$
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

