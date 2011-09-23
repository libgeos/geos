/**********************************************************************
 * $Id: SimpleNoder.cpp 2319 2009-04-07 19:00:36Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SimpleNoder.java rev. 1.7 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/SimpleNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/geom/CoordinateSequence.h>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*private*/
void
SimpleNoder::computeIntersects(SegmentString* e0, SegmentString* e1)
{
	assert(segInt); // must provide a segment intersector!

	const CoordinateSequence* pts0 = e0->getCoordinates();
	const CoordinateSequence* pts1 = e1->getCoordinates();
	for (unsigned int i0=0, n0=pts0->getSize()-1; i0<n0; i0++) {
		for (unsigned int i1=0, n1=pts1->getSize()-1; i1<n1; i1++) {
			segInt->processIntersections(e0, i0, e1, i1);
		}
	}
 
}

/*public*/
void
SimpleNoder::computeNodes(SegmentString::NonConstVect* inputSegmentStrings)
{
	nodedSegStrings=inputSegmentStrings;

	for (SegmentString::NonConstVect::const_iterator
			i0=inputSegmentStrings->begin(), i0End=inputSegmentStrings->end();
			i0!=i0End; ++i0)
	{
		SegmentString* edge0 = *i0;
		for (SegmentString::NonConstVect::iterator
			i1=inputSegmentStrings->begin(), i1End=inputSegmentStrings->end();
			i1!=i1End; ++i1)
		{
		        SegmentString* edge1 = *i1;
			computeIntersects(edge0, edge1);
		}
	}
}


} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.9  2006/02/23 20:05:21  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.6  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.5  2006/01/31 19:07:34  strk
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
 * Revision 1.4  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

