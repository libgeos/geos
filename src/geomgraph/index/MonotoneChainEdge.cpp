/**********************************************************************
 * $Id: MonotoneChainEdge.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <cassert>
#include <vector>

#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/index/MonotoneChainEdge.h>
#include <geos/geomgraph/index/MonotoneChainIndexer.h>
#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

/**
 * MonotoneChains are a way of partitioning the segments of an edge to
 * allow for fast searching of intersections.
 * They have the following properties:
 * 
 *  -  the segments within a monotone chain will never intersect each other
 *  -  the envelope of any contiguous subset of the segments in a monotone
 *     chain is simply the envelope of the endpoints of the subset.
 * 
 * Property 1 means that there is no need to test pairs of segments from
 * within the same monotone chain for intersection.
 * Property 2 allows binary search to be used to find the intersection
 * points of two monotone chains.
 * For many types of real-world data, these properties eliminate a large
 * number of segment comparisons, producing substantial speed gains.
 * @version 1.1
 */

MonotoneChainEdge::~MonotoneChainEdge()
{
}

MonotoneChainEdge::MonotoneChainEdge(Edge *newE):
	e(newE),
	pts(newE->getCoordinates())
{
	assert(e);
	MonotoneChainIndexer mcb;
	mcb.getChainStartIndices(pts, startIndex);
	assert(e);
}

const CoordinateSequence*
MonotoneChainEdge::getCoordinates()
{
	assert(pts);
	return pts;
}

vector<int> &
MonotoneChainEdge::getStartIndexes()
{
	return startIndex;
}

double
MonotoneChainEdge::getMinX(int chainIndex)
{
	double x1=pts->getAt(startIndex[chainIndex]).x;
	double x2=pts->getAt(startIndex[chainIndex+1]).x;
	return x1<x2?x1:x2;
}

double
MonotoneChainEdge::getMaxX(int chainIndex)
{
	double x1=pts->getAt(startIndex[chainIndex]).x;
	double x2=pts->getAt(startIndex[chainIndex+1]).x;
	return x1>x2?x1:x2;
}

void
MonotoneChainEdge::computeIntersects(const MonotoneChainEdge &mce,
	SegmentIntersector &si)
{
	size_t I=startIndex.size()-1;
	size_t J=mce.startIndex.size()-1;
	for(size_t i=0; i<I; ++i)
	{
		for(size_t j=0; j<J; ++j)
		{
			computeIntersectsForChain(i,mce,j,si);
		}
	}
}

void
MonotoneChainEdge::computeIntersectsForChain(int chainIndex0,
	const MonotoneChainEdge &mce, int chainIndex1,
	SegmentIntersector &si)
{
	computeIntersectsForChain(startIndex[chainIndex0],
		startIndex[chainIndex0+1], mce,
		mce.startIndex[chainIndex1],
		mce.startIndex[chainIndex1+1],
		si);
}

void
MonotoneChainEdge::computeIntersectsForChain(int start0, int end0,
	const MonotoneChainEdge &mce, int start1, int end1,
	SegmentIntersector &ei)
{
	// terminating condition for the recursion
	if (end0-start0==1 && end1-start1==1) {
		ei.addIntersections(e, start0, mce.e, start1);
		return;
	}

	const Coordinate& p00=pts->getAt(start0);
	const Coordinate& p01=pts->getAt(end0);
	const Coordinate& p10=mce.pts->getAt(start1);
	const Coordinate& p11=mce.pts->getAt(end1);

	// nothing to do if the envelopes of these chains don't overlap
	env1.init(p00,p01);
	env2.init(p10,p11);

	if (!env1.intersects(&env2)) return;
	// the chains overlap, so split each in half and iterate 
	// (binary search)
	int mid0=(start0+end0)/2;
	int mid1=(start1+end1)/2;

	// Assert: mid != start or end
	// (since we checked above for end - start <= 1)
	// check terminating conditions before recursing
	if (start0<mid0) {
		if (start1<mid1)
			computeIntersectsForChain(start0, mid0, mce,
				start1, mid1, ei);
		if (mid1<end1)
			computeIntersectsForChain(start0, mid0, mce,
				mid1, end1, ei);
	}
	if (mid0<end0) {
		if (start1<mid1)
			computeIntersectsForChain(mid0, end0, mce,
				start1, mid1, ei);
		if (mid1<end1)
			computeIntersectsForChain(mid0, end0, mce,
				mid1, end1, ei);
	}
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.12  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.11  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.10  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.9  2006/02/28 14:34:04  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2006/01/31 19:07:34  strk
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
 * Revision 1.6  2005/11/15 10:04:37  strk
 *
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.5  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.4  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.17  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.16  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.15  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/

