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
 **********************************************************************/

#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

namespace geos {
namespace operation { 

/** \brief
 * Contains classes that perform a topological overlay to compute boolean spatial functions.
 * 
 * The Overlay Algorithm is used in spatial analysis methods for computing set-theoretic
 * operations (boolean combinations) of input {@link Geometry}s. The algorithm for
 * computing the overlay uses the intersection operations supported by topology graphs.
 * To compute an overlay it is necessary to explicitly compute the resultant graph formed
 * by the computed intersections.
 * 
 * The algorithm to compute a set-theoretic spatial analysis method has the following steps:
 * <UL>
 *   <LI>Build topology graphs of the two input geometries.  For each geometry all
 *       self-intersection nodes are computed and added to the graph.
 *   <LI>Compute nodes for all intersections between edges and nodes of the graphs.
 *   <LI>Compute the labeling for the computed nodes by merging the labels from the input graphs.
 *   <LI>Compute new edges between the compute intersection nodes.  Label the edges appropriately.
 *   <LI>Build the resultant graph from the new nodes and edges.
 *   <LI>Compute the labeling for isolated components of the graph.  Add the
 *       isolated components to the resultant graph.
 *   <LI>Compute the result of the boolean combination by selecting the node and edges
 *       with the appropriate labels. Polygonize areas and sew linear geometries together.
 * </UL>
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
namespace overlay { // geos.operation.overlay

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/PolygonBuilder.h>
#include <geos/operation/overlay/PointBuilder.h>
#include <geos/operation/overlay/LineBuilder.h>
#include <geos/operation/overlay/MinimalEdgeRing.h>
#include <geos/operation/overlay/MaximalEdgeRing.h>
#include <geos/operation/overlay/OverlayNodeFactory.h>
#include <geos/operation/overlay/EdgeSetNoder.h>
#include <geos/operation/overlay/ElevationMatrix.h>

#endif

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.25  2006/03/14 17:08:04  strk
 * comments cleanup, integrity checks
 *
 * Revision 1.24  2006/03/14 16:55:01  strk
 * comments cleanup
 *
 * Revision 1.23  2006/03/09 18:18:32  strk
 * Added memory-friendly MaximalEdgeRing::buildMinimalRings() implementation.
 * Applied patch to IsValid operation from JTS-1.7.1
 *
 * Revision 1.22  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.21  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.20  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.19  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.18  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.17  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.16  2005/12/08 00:03:51  strk
 * LineBuilder::lineEdgesList made a real vector, rather then pointer (private member).
 * Small optimizations in LineBuilder loops, cleanups in LineBuilder class dox.
 *
 * Revision 1.15  2005/11/21 16:03:20  strk
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
 * Revision 1.14  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.13  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.12  2005/07/11 12:17:47  strk
 * Cleaned up syntax
 *
 * Revision 1.11  2005/06/28 01:07:02  strk
 * improved extraction of result points in overlay op
 *
 * Revision 1.10  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.9  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.8  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.7  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.6  2004/11/22 15:51:52  strk
 * Added interpolation of containing geometry's average Z for point_in_poly case.
 *
 * Revision 1.5  2004/11/20 18:17:26  strk
 * Added Z propagation for overlay lines output.
 *
 * Revision 1.4  2004/11/20 17:16:10  strk
 * Handled Z merging for point on polygon boundary case.
 *
 * Revision 1.3  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.18  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.17  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.16  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.15  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.14  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.13  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.11  2003/11/12 16:14:56  strk
 * Added some more throw specifications and cleanup on exception (leaks removed).
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
