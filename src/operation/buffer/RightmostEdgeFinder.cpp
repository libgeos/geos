/**********************************************************************
 * $Id: RightmostEdgeFinder.cpp 3245 2011-02-23 16:46:10Z strk $
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
 * Last port: operation/buffer/RightmostEdgeFinder.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/operation/buffer/RightmostEdgeFinder.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/DirectedEdgeStar.h>
#include <geos/geomgraph/Position.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Edge.h>

#include <vector>
#include <cassert>

using namespace geos::algorithm; // CGAlgorithms
using namespace geos::geom;
using namespace geos::geomgraph; // DirectedEdge, Position

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/*public*/
RightmostEdgeFinder::RightmostEdgeFinder()
	:
	minIndex(-1), // FIXME: don't use -1 as a sentinel, or we won't be
	              // able to use an unsigned int here
	minCoord(Coordinate::getNull()), 
	minDe(NULL),
	orientedDe(NULL)
{
}

/*public*/
void
RightmostEdgeFinder::findEdge(std::vector<DirectedEdge*>* dirEdgeList)
{

#ifndef NDEBUG
	size_t checked=0;
#endif

	/*
	 * Check all forward DirectedEdges only.  This is still general,
	 * because each edge has a forward DirectedEdge.
	 */
	size_t dirEdgeListSize=dirEdgeList->size();
	for(size_t i=0; i<dirEdgeListSize; ++i)
	{
		DirectedEdge *de=(*dirEdgeList)[i];
		assert(de);
		if (!de->isForward()) continue;
		checkForRightmostCoordinate(de);
#ifndef NDEBUG
		++checked;
#endif
	}

#ifndef NDEBUG
	assert(checked>0);
	assert(minIndex>=0);
	assert(minDe);
#endif

	/*
	 * If the rightmost point is a node, we need to identify which of
	 * the incident edges is rightmost.
	 */
	assert(minIndex!=0 || minCoord==minDe->getCoordinate());
	// inconsistency in rightmost processing

	if (minIndex==0 ) {
		findRightmostEdgeAtNode();
	} else {
		findRightmostEdgeAtVertex();
	}

	/*
	 * now check that the extreme side is the R side.
	 * If not, use the sym instead.
	 */
	orientedDe=minDe;
	int rightmostSide=getRightmostSide(minDe, minIndex);
	if (rightmostSide==Position::LEFT) {
		orientedDe=minDe->getSym();
	}
}

/*private*/
void
RightmostEdgeFinder::findRightmostEdgeAtNode()
{
	Node *node=minDe->getNode();
	assert(node);

	assert(dynamic_cast<DirectedEdgeStar*>(node->getEdges()));
	DirectedEdgeStar *star=static_cast<DirectedEdgeStar*>(node->getEdges());

	// Warning! NULL could be returned if the star is empty!
	minDe=star->getRightmostEdge();
	assert(minDe);

	// the DirectedEdge returned by the previous call is not
	// necessarily in the forward direction. Use the sym edge if it isn't.
	if (!minDe->isForward())
	{
		minDe=minDe->getSym();

		const Edge* minEdge=minDe->getEdge();
		assert(minEdge);

		const CoordinateSequence* minEdgeCoords =
				minEdge->getCoordinates();
		assert(minEdgeCoords);

		minIndex=(int)(minEdgeCoords->getSize())-1;
		assert(minIndex>=0);
	}
}

/*private*/
void
RightmostEdgeFinder::findRightmostEdgeAtVertex()
{
	/*
	 * The rightmost point is an interior vertex, so it has
	 * a segment on either side of it.
	 * If these segments are both above or below the rightmost
	 * point, we need to determine their relative orientation
	 * to decide which is rightmost.
	 */

	Edge* minEdge=minDe->getEdge();
	assert(minEdge);
	const CoordinateSequence *pts=minEdge->getCoordinates();
	assert(pts);

	// rightmost point expected to be interior vertex of edge
	assert(minIndex>0);
	assert((size_t)minIndex<pts->getSize()); 

	const Coordinate& pPrev=pts->getAt(minIndex-1);
	const Coordinate& pNext=pts->getAt(minIndex+1);
	int orientation=CGAlgorithms::computeOrientation(
			minCoord,
			pNext,
			pPrev);
	bool usePrev=false;

	// both segments are below min point
	if ( pPrev.y < minCoord.y && pNext.y < minCoord.y
		&& orientation == CGAlgorithms::COUNTERCLOCKWISE)
	{
			usePrev=true;
	}
	else if ( pPrev.y > minCoord.y && pNext.y > minCoord.y
		&& orientation == CGAlgorithms::CLOCKWISE)
	{
			usePrev=true;
	}

	// if both segments are on the same side, do nothing - either is safe
	// to select as a rightmost segment
	if (usePrev) {
		minIndex=minIndex-1;
	}
}

/*private*/
void
RightmostEdgeFinder::checkForRightmostCoordinate(DirectedEdge *de)
{
	const Edge* deEdge=de->getEdge();
	assert(deEdge);

	const CoordinateSequence *coord=deEdge->getCoordinates();
	assert(coord);

	// only check vertices which are the starting point of
	// a non-horizontal segment
	size_t n=coord->getSize()-1;
	for(size_t i=0; i<n; i++)
	{
     // only check vertices which are the start or end point
     // of a non-horizontal segment
     // <FIX> MD 19 Sep 03 - NO!  we can test all vertices,
     // since the rightmost must have a non-horiz segment adjacent to it
		if (minCoord.isNull() || 
			coord->getAt(i).x > minCoord.x )
		{
			minDe = de;
			minIndex = (int)i;
			minCoord = coord->getAt(i);
		}
	}
}

/*private*/
int
RightmostEdgeFinder::getRightmostSide(DirectedEdge *de, int index)
{
	int side = getRightmostSideOfSegment(de, index);

	if (side<0) side=getRightmostSideOfSegment(de, index-1);

	if (side<0)
	{
		// reaching here can indicate that segment is horizontal
		// Assert::shouldNeverReachHere(
		//	"problem with finding rightmost side of segment");

		minCoord=Coordinate::getNull();
		checkForRightmostCoordinate(de);
	}

	return side;
}

/*private*/
int
RightmostEdgeFinder::getRightmostSideOfSegment(DirectedEdge *de, int i)
{
	assert(de);

	const Edge *e=de->getEdge();
	assert(e);

	const CoordinateSequence *coord=e->getCoordinates();
	assert(coord);

	if ( i < 0 || i+1 >= (int)coord->getSize() ) return -1;

	// indicates edge is parallel to x-axis
	if (coord->getAt(i).y == coord->getAt(i+1).y) return -1;   

	int pos=Position::LEFT;
	if (coord->getAt(i).y < coord->getAt(i+1).y) pos=Position::RIGHT;
	return pos;
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.22  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.21  2006/05/03 15:05:36  strk
 * Assertions checking
 *
 * Revision 1.20  2006/04/05 09:20:25  strk
 * Added port informations and many assertion checking.
 * Fixed bug in getRightmostSide() method ( a "testing-only" corner case )
 *
 * Revision 1.19  2006/03/14 00:19:40  strk
 * opBuffer.h split, streamlined headers in some (not all) files in operation/buffer/
 *
 * Revision 1.18  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.17  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.16  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.15  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.14  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.13  2006/01/31 19:07:34  strk
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
 * Revision 1.12  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.11  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.10  2005/02/22 17:10:47  strk
 * Reduced CoordinateSequence::getSize() calls.
 *
 * Revision 1.9  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.5  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/

