/**********************************************************************
 * $Id$
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
 ********************************************************************** 
 *
 * This includes all headers from geos/algorithm.
 * It is reccommended you avoid to include this file, but rather
 * you include the specific headers you need. This is to reduce
 * dependency and thus build times.
 * We kept this file to provide some degree of backward compatibility.
 * This is also where Doxygen documentation for the geos::algorithm
 * namespace resides.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_H
#define GEOS_ALGORITHM_H

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/algorithm/CentroidArea.h>
#include <geos/algorithm/CentroidLine.h>
#include <geos/algorithm/CentroidPoint.h>
#include <geos/algorithm/ConvexHull.h>
#include <geos/algorithm/HCoordinate.h>
#include <geos/algorithm/InteriorPointArea.h>
#include <geos/algorithm/InteriorPointLine.h>
#include <geos/algorithm/InteriorPointPoint.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/MCPointInRing.h>
#include <geos/algorithm/MinimumDiameter.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/algorithm/PointInRing.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/algorithm/RobustDeterminant.h>
#include <geos/algorithm/SIRtreePointInRing.h>
#include <geos/algorithm/SimplePointInAreaLocator.h>
#include <geos/algorithm/SimplePointInRing.h>

namespace geos {

/** \brief
 * Contains classes and interfaces implementing fundamental computational geometry algorithms.
 * 
 * <H3>Robustness</H3>
 * 
 * Geometrical algorithms involve a combination of combinatorial and numerical computation.  As with
 * all numerical computation using finite-precision numbers, the algorithms chosen are susceptible to
 * problems of robustness.  A robustness problem occurs when a numerical calculation produces an
 * incorrect answer for some inputs due to round-off errors.  Robustness problems are especially
 * serious in geometric computation, since they can result in errors during topology building.
 * <P>
 * There are many approaches to dealing with the problem of robustness in geometrical computation.
 * Not surprisingly, most robust algorithms are substantially more complex and less performant than
 * the non-robust versions.  Fortunately, JTS is sensitive to robustness problems in only a few key
 * functions (such as line intersection and the point-in-polygon test).  There are efficient robust
 * algorithms available for these functions, and these algorithms are implemented in JTS.
 * 
 * <H3>Computational Performance</H3>
 * 
 * Runtime performance is an important consideration for a production-quality implementation of
 * geometric algorithms.  The most computationally intensive algorithm used in JTS is intersection
 * detection.  JTS methods need to determine both all intersection between the line segments in a
 * single Geometry (self-intersection) and all intersections between the line segments of two different
 * Geometries.
 * <P>
 * The obvious naive algorithm for intersection detection (comparing every segment with every other)
 * has unacceptably slow performance.  There is a large literature of faster algorithms for intersection
 * detection.  Unfortunately, many of them involve substantial code complexity.  JTS tries to balance code
 * simplicity with performance gains.  It uses some simple techniques to produce substantial performance
 * gains for common types of input data.
 * 
 * 
 * <h2>Package Specification</h2>
 * 
 * <ul>
 *   <li>Java Topology Suite Technical Specifications
 *   <li><A HREF="http://www.opengis.org/techno/specs.htm">
 *       OpenGIS Simple Features Specification for SQL</A>
 * </ul>
 * 
 */
namespace algorithm { // geos::algorithm
} // namespace geos::algorithm
} // namespace geos


// OBSOLETED, it seems
//namespace geos {
//namespace algorithm { // geos::algorithm
//class BigQuad {
//public:
//	geom::Coordinate northmost;
//	geom::Coordinate southmost;
//	geom::Coordinate westmost;
//	geom::Coordinate eastmost;
//};
//} // namespace geos::algorithm
//} // namespace geos



#endif

/**********************************************************************
 * $Log$
 * Revision 1.29  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.28  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.27  2006/03/03 13:50:16  strk
 * Cleaned up InteriorPointLine class
 *
 * Revision 1.26  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.25  2006/03/01 17:16:39  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.24  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.23  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.22  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.21  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.20  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.19  2006/01/31 19:07:34  strk
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
 * Revision 1.18  2005/11/29 15:16:44  strk
 * Fixed sign-related warnings and signatures.
 *
 * Revision 1.17  2005/11/24 23:07:00  strk
 * CentroidLine made concrete class (only destructor was virtual) - avoided heap allocation for owned Coordinate centSum
 *
 * Revision 1.16  2005/11/21 16:03:20  strk
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
 * Revision 1.15  2005/11/15 18:30:59  strk
 * Removed dead code
 *
 * Revision 1.14  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.13  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.12  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.11  2005/05/09 10:35:20  strk
 * Ported JTS robustness patches made by Martin on suggestions by Kevin.
 *
 * Revision 1.10  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.9  2004/11/23 19:53:07  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.8  2004/11/06 08:16:46  strk
 * Fixed CGAlgorithms::isCCW from JTS port.
 * Code cleanup in IsValidOp.
 *
 * Revision 1.7  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.6  2004/09/13 10:12:49  strk
 * Added invalid coordinates checks in IsValidOp.
 * Cleanups.
 *
 * Revision 1.5  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
 * Revision 1.2  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.32  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.31  2004/04/20 12:47:57  strk
 * MinimumDiameter leaks plugged.
 *
 * Revision 1.30  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.29  2004/02/27 17:42:15  strk
 * made CGAlgorithms::signedArea() and CGAlgorithms::length() arguments const-correct
 *
 * Revision 1.28  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

