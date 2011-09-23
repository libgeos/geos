/**********************************************************************
 * $Id: SubgraphDepthLocater.cpp 3245 2011-02-23 16:46:10Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/SubgraphDepthLocater.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <vector>
#include <cassert>
#include <algorithm>

#include <geos/operation/buffer/BufferSubgraph.h>
#include <geos/operation/buffer/SubgraphDepthLocater.h>

#include <geos/algorithm/CGAlgorithms.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>

#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/Position.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/*
 * A segment from a directed edge which has been assigned a depth value
 * for its sides.
 */
class DepthSegment {

private:

	geom::LineSegment upwardSeg;

	/*
	 * Compare two collinear segments for left-most ordering.
	 * If segs are vertical, use vertical ordering for comparison.
	 * If segs are equal, return 0.
	 * Segments are assumed to be directed so that the second
	 * coordinate is >= to the first
	 * (e.g. up and to the right).
	 *
	 * @param seg0 a segment to compare
	 * @param seg1 a segment to compare
	 * @return
	 */
	static int compareX(const geom::LineSegment *seg0, const geom::LineSegment *seg1) 
	{
		int compare0=seg0->p0.compareTo(seg1->p0);
		if (compare0!=0) return compare0;
		return seg0->p1.compareTo(seg1->p1);
	}

public:

	int leftDepth;

	/// @param seg will be copied to private space
	DepthSegment(const geom::LineSegment &seg, int depth)
		:
		upwardSeg(seg),
		leftDepth(depth)
	{
		// input seg is assumed to be normalized
		//upwardSeg.normalize();
	}

	/**
	 * Defines a comparision operation on DepthSegments
	 * which orders them left to right
	 *
	 * <pre>
	 * DS1 < DS2   if   DS1.seg is left of DS2.seg
	 * DS1 > DS2   if   DS1.seg is right of DS2.seg
	 * </pre>
	 *
	 * @param obj
	 * @return
	 */
	int compareTo(const DepthSegment& other) const
	{
		/**
		 * try and compute a determinate orientation for the segments.
		 * Test returns 1 if other is left of this (i.e. this > other)
		 */
		int orientIndex=upwardSeg.orientationIndex(&(other.upwardSeg));

		/**
		 * If comparison between this and other is indeterminate,
		 * try the opposite call order.
		 * orientationIndex value is 1 if this is left of other,
		 * so have to flip sign to get proper comparison value of
		 * -1 if this is leftmost
		 */
		if (orientIndex==0)
			orientIndex=-1 * other.upwardSeg.orientationIndex(&upwardSeg);

		// if orientation is determinate, return it
		if (orientIndex != 0)
			return orientIndex;

		// otherwise, segs must be collinear - sort based on minimum X value
		return compareX(&upwardSeg, &(other.upwardSeg));
	}
};

struct DepthSegmentLessThen {
	bool operator() (const DepthSegment* first, const DepthSegment* second)
	{
		assert(first);
		assert(second);
		if (first->compareTo(*second)<0) return true;
		else return false;
	}
};



/*public*/
int
SubgraphDepthLocater::getDepth(const Coordinate& p)
{
	vector<DepthSegment*> stabbedSegments;
	findStabbedSegments(p, stabbedSegments);

	// if no segments on stabbing line subgraph must be outside all others
	if (stabbedSegments.size()==0) return 0;

	sort(stabbedSegments.begin(), stabbedSegments.end(), DepthSegmentLessThen());

	DepthSegment *ds=stabbedSegments[0];
	int ret = ds->leftDepth;

#if GEOS_DEBUG
	cerr<<"SubgraphDepthLocater::getDepth("<<p.toString()<<"): "<<ret<<endl;
#endif

	for (vector<DepthSegment *>::iterator
		it=stabbedSegments.begin(), itEnd=stabbedSegments.end();
		it != itEnd;
		++it)
	{
		delete *it;
	}

	return ret;
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(const Coordinate &stabbingRayLeftPt,
			std::vector<DepthSegment*>& stabbedSegments)
{
	size_t size = subgraphs->size();
	for (size_t i=0; i<size; ++i)
	{
		BufferSubgraph *bsg=(*subgraphs)[i];

		// optimization - don't bother checking subgraphs
		// which the ray does not intersect
		Envelope *env = bsg->getEnvelope();
		if ( stabbingRayLeftPt.y < env->getMinY()
			|| stabbingRayLeftPt.y > env->getMaxY()
                        || stabbingRayLeftPt.x < env->getMinX()
                        || stabbingRayLeftPt.x > env->getMaxX())
		{
				continue;
		}

		findStabbedSegments(stabbingRayLeftPt, bsg->getDirectedEdges(),
			stabbedSegments);
	}
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(
	const Coordinate &stabbingRayLeftPt,
	vector<DirectedEdge*>* dirEdges,
	vector<DepthSegment*>& stabbedSegments)
{

	/**
	 * Check all forward DirectedEdges only. This is still general,
	 * because each Edge has a forward DirectedEdge.
	 */
	for (size_t i=0, n=dirEdges->size(); i<n; ++i)
	{
		DirectedEdge *de=(*dirEdges)[i];
		if (!de->isForward()) continue;
		findStabbedSegments(stabbingRayLeftPt, de, stabbedSegments);
	}
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(
	const Coordinate &stabbingRayLeftPt,
	DirectedEdge *dirEdge,
	vector<DepthSegment*>& stabbedSegments)
{
	const CoordinateSequence *pts=dirEdge->getEdge()->getCoordinates();

// It seems that LineSegment is *very* slow... undef this
// to see yourself
// LineSegment has been refactored to be mostly inline, still
// it makes copies of the given coordinates, while the 'non-LineSemgent'
// based code below uses pointers instead. I'll kip the SKIP_LS
// defined until LineSegment switches to Coordinate pointers instead.
//
#define SKIP_LS 1

	int n = pts->getSize()-1;
	for (int i=0; i<n; ++i) {
#ifndef SKIP_LS
		seg.p0=pts->getAt(i);
		seg.p1=pts->getAt(i + 1);
#if GEOS_DEBUG
		cerr << " SubgraphDepthLocater::findStabbedSegments: segment " << i
			<< " (" << seg << ") ";
#endif

#else
		const Coordinate *low=&(pts->getAt(i));
		const Coordinate *high=&(pts->getAt(i+1));
		const Coordinate *swap=NULL;

#endif

#ifndef SKIP_LS
		// ensure segment always points upwards
		//if (seg.p0.y > seg.p1.y)
		{
			seg.reverse();
#if GEOS_DEBUG
			cerr << " reverse (" << seg << ") ";
#endif
		}
#else
		if (low->y > high->y)
		{
			swap=low;
			low=high;
			high=swap;
		}
#endif

		// skip segment if it is left of the stabbing line
		// skip if segment is above or below stabbing line
#ifndef SKIP_LS
		double maxx=max(seg.p0.x, seg.p1.x);
#else
		double maxx=max(low->x, high->x);
#endif
		if (maxx < stabbingRayLeftPt.x)
		{
#if GEOS_DEBUG
			cerr<<" segment is left to stabbing line, skipping "<<endl;
#endif
			continue;
		}

		// skip horizontal segments (there will be a non-horizontal
		// one carrying the same depth info
#ifndef SKIP_LS
		if (seg.isHorizontal())
#else
		if (low->y == high->y)
#endif
		{
#if GEOS_DEBUG
			cerr<<" segment is horizontal, skipping "<<endl;
#endif
			continue;
		}

		// skip if segment is above or below stabbing line
#ifndef SKIP_LS
		if (stabbingRayLeftPt.y < seg.p0.y ||
			stabbingRayLeftPt.y > seg.p1.y)
#else
		if (stabbingRayLeftPt.y < low->y ||
			stabbingRayLeftPt.y > high->y)
#endif
		{
#if GEOS_DEBUG 
			cerr<<" segment above or below stabbing line, skipping "<<endl;
#endif
			continue;
		}

		// skip if stabbing ray is right of the segment
#ifndef SKIP_LS
		if (CGAlgorithms::computeOrientation(seg.p0, seg.p1,
#else
		if (CGAlgorithms::computeOrientation(*low, *high,
#endif
				stabbingRayLeftPt)==CGAlgorithms::RIGHT)
		{
#if GEOS_DEBUG
			cerr<<" stabbing ray right of segment, skipping"<<endl;
#endif
			continue;
		}

#ifndef SKIP_LS
		// stabbing line cuts this segment, so record it
		int depth=dirEdge->getDepth(Position::LEFT);
		// if segment direction was flipped, use RHS depth instead
		if (! (seg.p0==pts->getAt(i)))
			depth=dirEdge->getDepth(Position::RIGHT);
#else
		int depth = swap ?
			dirEdge->getDepth(Position::RIGHT)
			:
			dirEdge->getDepth(Position::LEFT);
#endif

#if GEOS_DEBUG
	cerr<<" depth: "<<depth<<endl;
#endif

#ifdef SKIP_LS
		seg.p0 = *low;
		seg.p1 = *high;
#endif

		DepthSegment *ds=new DepthSegment(seg, depth);
		stabbedSegments.push_back(ds);
	}
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.29  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.28  2006/03/20 10:13:15  strk
 * Bug #71 - Missing <algorithm>
 *
 * Revision 1.27  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.26  2006/03/15 18:57:39  strk
 * small cleanup
 *
 * Revision 1.25  2006/03/15 15:50:12  strk
 * const correctness, cleanups
 *
 * Revision 1.24  2006/03/15 12:52:56  strk
 * DepthSegment class moved inside SubgraphDepthLocator implementaion
 * as it was private to this file in JTS. Also, changed to reduce
 * copies of LineSegment copies.
 *
 * Revision 1.23  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 * Revision 1.22  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.21  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.20  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.19  2006/03/02 09:46:04  strk
 * cleaned up debugging lines
 *
 * Revision 1.18  2006/03/01 17:16:39  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.17  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.16  2006/01/31 19:07:34  strk
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
 * Revision 1.15  2005/11/08 20:12:44  strk
 * Memory overhead reductions in buffer operations.
 *
 * Revision 1.14  2005/07/11 10:27:14  strk
 * Fixed initializzazion lists
 *
 * Revision 1.13  2005/06/30 18:31:48  strk
 * Ported SubgraphDepthLocator optimizations from JTS code
 *
 * Revision 1.12  2005/06/28 21:13:43  strk
 * Fixed a bug introduced by LineSegment skip - made LineSegment skip a compile-time optione
 *
 * Revision 1.11  2005/06/27 21:58:31  strk
 * Bugfix in DepthSegmentLT as suggested by Graeme Hiebert
 *
 * Revision 1.10  2005/06/27 21:24:54  strk
 * Fixed bug just-introduced with optimization.
 *
 * Revision 1.9  2005/06/27 21:21:21  strk
 * Reduced Coordinate copies due to LineSegment overuse
 *
 * Revision 1.8  2005/05/23 15:13:00  strk
 * Added debugging output
 *
 * Revision 1.7  2005/05/20 16:15:41  strk
 * Code cleanups
 *
 * Revision 1.6  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.5  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.4  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/05/05 12:29:44  strk
 * memleak fixed in ::getDepth
 *
 * Revision 1.2  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.1  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

