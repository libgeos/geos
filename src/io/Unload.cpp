/**********************************************************************
 * $Id: Unload.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/unload.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace io { // geos.io

/*public static*/
void Unload::Release()
{
	//delete geom::Geometry::INTERNAL_GEOMETRY_FACTORY;
}

} // namespace geos.io
} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/04/10 13:09:49  strk
 * Added GeometryFactory::defaultInstance()
 * Made Geometry::INTERNAL_GEOMETRY_FACTORY an alias for it
 * removed last deletion from Unload::Release class
 *
 * Revision 1.25  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.24  2006/03/17 16:48:55  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 * Revision 1.23  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.22  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.21  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.20  2006/03/02 14:34:43  strk
 * GeometryGraphOperation::li made a non-static member, and not more a pointer
 *
 * Revision 1.19  2006/03/01 10:39:59  strk
 * ctor and dtor made private and inlined
 *
 * Revision 1.18  2006/02/23 11:54:20  strk
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
 * Revision 1.17  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.16  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.15  2006/01/31 19:07:34  strk
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
 * Revision 1.14  2004/12/08 14:30:53  strk
 * Added default profiler instance cleanup
 *
 * Revision 1.13  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.12  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.9  2004/04/16 08:52:52  strk
 * Unload::Release final delete (static heap allocations should be gone now)
 *
 * Revision 1.8  2004/04/16 07:42:06  strk
 * PrecisionModel::Type made an enum instead of a Type.
 *
 * Revision 1.7  2004/04/15 15:11:38  strk
 * Commented out deletion that seems to cause segfaults
 *
 * Revision 1.6  2004/04/15 14:00:31  strk
 * Added new cleanup to Unload::Release
 *
 * Revision 1.5  2004/04/01 10:44:34  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.4  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.3  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

