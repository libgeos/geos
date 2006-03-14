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
 **********************************************************************/

#include <sstream>
#include <string>

#include <geos/geom/Coordinate.h>

#ifndef USE_INLINE
# include <geos/geom/Coordinate.inl>
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

Coordinate Coordinate::nullCoord=Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber);

string Coordinate::toString() const
{
	ostringstream s;
	s<<*this;
	return s.str();
}

std::ostream& operator<< (std::ostream& os, const Coordinate& c)
{
	if (ISNAN(c.z)) {
		os << c.x << " " << c.y;
	} else {
		os << c.x << " " << c.y << " " << c.z;
	}
	return os;
}


} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.31  2006/03/14 15:32:24  strk
 * Cleaned up toString funx (more WKT friendly)
 *
 * Revision 1.30  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.29  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.28  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.27  2006/02/28 17:44:26  strk
 * Added a check in SegmentNode::addSplitEdge to prevent attempts
 * to build SegmentString with less then 2 points.
 * This is a temporary fix for the buffer.xml assertion failure, temporary
 * as Martin Davis review would really be needed there.
 *
 * Revision 1.26  2006/02/28 14:34:03  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.25  2006/02/24 15:39:06  strk
 * - operator>> for Coordinate, planarNode and planarEdge
 * - Fixed bug in planarGraphComponent::setMarked
 * - Added linemerge.xml test (single test, should grow a bit)
 *
 * Revision 1.24  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.23  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.22  2006/01/31 19:07:33  strk
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
 * Revision 1.21  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.20  2005/01/28 08:47:06  strk
 * Removed sprintf usage, replaced with sstream
 *
 * Revision 1.19  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.18  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.17  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.16  2004/07/22 07:04:49  strk
 * Documented missing geometry functions.
 *
 * Revision 1.15  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.14  2004/07/14 21:17:10  strk
 * added inequality operator for Coordinate
 *
 * Revision 1.13  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.12  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

