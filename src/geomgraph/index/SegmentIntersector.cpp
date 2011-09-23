/**********************************************************************
 * $Id: SegmentIntersector.cpp 2723 2009-11-19 16:40:29Z pramsey $
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
 **********************************************************************/


#include <stdlib.h>
#include <vector>

#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/Node.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#define DEBUG_INTERSECT 0

#if GEOS_DEBUG || DEBUG_INTERSECT
#include <iostream>
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

using namespace geos::algorithm;

bool
SegmentIntersector::isAdjacentSegments(int i1,int i2)
{
	return abs(i1-i2)==1;
}

void
SegmentIntersector::setBoundaryNodes(vector<Node*> *bdyNodes0,
	vector<Node*> *bdyNodes1)
{
	bdyNodes[0]=bdyNodes0;
	bdyNodes[1]=bdyNodes1;
}

/*
 * @return the proper intersection point, or <code>null</code>
 * if none was found
 */
Coordinate&
SegmentIntersector::getProperIntersectionPoint()
{
	return properIntersectionPoint;
}

bool
SegmentIntersector::hasIntersection()
{
	return hasIntersectionVar;
}

/*
 * A proper intersection is an intersection which is interior to at least two
 * line segments.  Note that a proper intersection is not necessarily
 * in the interior of the entire Geometry, since another edge may have
 * an endpoint equal to the intersection, which according to SFS semantics
 * can result in the point being on the Boundary of the Geometry.
 */
bool
SegmentIntersector::hasProperIntersection()
{
	return hasProper;
}

/*
 * A proper interior intersection is a proper intersection which is <b>not</b>
 * contained in the set of boundary nodes set for this SegmentIntersector.
 */
bool
SegmentIntersector::hasProperInteriorIntersection()
{
	return hasProperInterior;
}

/*
 * A trivial intersection is an apparent self-intersection which in fact
 * is simply the point shared by adjacent line segments.
 * Note that closed edges require a special check for the point
 * shared by the beginning and end segments.
 */
bool
SegmentIntersector::isTrivialIntersection(Edge *e0,int segIndex0,Edge *e1,int segIndex1)
{
//	if (e0->equals(e1)) 
	if (e0==e1) {
		if (li->getIntersectionNum()==1) {
			if (isAdjacentSegments(segIndex0,segIndex1))
				return true;
			if (e0->isClosed()) {
				int maxSegIndex=e0->getNumPoints()-1;
				if ((segIndex0==0 && segIndex1==maxSegIndex)
					|| (segIndex1==0 && segIndex0==maxSegIndex)) {
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * This method is called by clients of the EdgeIntersector class to test
 * for and add intersections for two segments of the edges being intersected.
 * Note that clients (such as MonotoneChainEdges) may choose not to intersect
 * certain pairs of segments for efficiency reasons.
 */
void
SegmentIntersector::addIntersections(Edge *e0,int segIndex0,Edge *e1,int segIndex1)
{

#if GEOS_DEBUG
	cerr<<"SegmentIntersector::addIntersections() called"<<endl;
#endif

//	if (e0->equals(e1) && segIndex0==segIndex1) return;
	if (e0==e1 && segIndex0==segIndex1) return;
	numTests++;
	const CoordinateSequence* cl0=e0->getCoordinates();
	const Coordinate& p00=cl0->getAt(segIndex0);
	const Coordinate& p01=cl0->getAt(segIndex0+1);
	const CoordinateSequence* cl1=e1->getCoordinates();
	const Coordinate& p10=cl1->getAt(segIndex1);
	const Coordinate& p11=cl1->getAt(segIndex1+1);
	li->computeIntersection(p00,p01,p10,p11);

	/*
	 * Always record any non-proper intersections.
	 * If includeProper is true, record any proper intersections as well.
	 */
	if (li->hasIntersection()) {
		if (recordIsolated) {
			e0->setIsolated(false);
			e1->setIsolated(false);
		}
		//intersectionFound = true;
		numIntersections++;

		// If the segments are adjacent they have at least one trivial
		// intersection, the shared endpoint.
		// Don't bother adding it if it is the
		// only intersection.
		if (!isTrivialIntersection(e0,segIndex0,e1,segIndex1))
		{
#if GEOS_DEBUG
			cerr<<"SegmentIntersector::addIntersections(): has !TrivialIntersection"<<endl;
#endif // DEBUG_INTERSECT
			hasIntersectionVar=true;
			if (includeProper || !li->isProper()) {
				//Debug.println(li);
				e0->addIntersections(li,segIndex0,0);
				e1->addIntersections(li,segIndex1,1);
#if GEOS_DEBUG
				cerr<<"SegmentIntersector::addIntersections(): includeProper || !li->isProper()"<<endl;
#endif // DEBUG_INTERSECT
			}
			if (li->isProper())
			{
				properIntersectionPoint=li->getIntersection(0);
#if GEOS_DEBUG
				cerr<<"SegmentIntersector::addIntersections(): properIntersectionPoint: "<<properIntersectionPoint.toString()<<endl;
#endif // DEBUG_INTERSECT
				hasProper=true;
				if (!isBoundaryPoint(li,bdyNodes))
					hasProperInterior=true;
			}
			//if (li.isCollinear())
			//hasCollinear = true;
		}
	}
}

/*private*/
bool
SegmentIntersector::isBoundaryPoint(LineIntersector *li,
		vector<Node*> *tstBdyNodes)
{
	if ( ! tstBdyNodes ) return false;

	for(vector<Node*>::iterator i=tstBdyNodes->begin();i<tstBdyNodes->end();i++) {
		Node *node=*i;
		const Coordinate& pt=node->getCoordinate();
		if (li->isIntersection(pt)) return true;
	}
	return false;
}


/*private*/
bool
SegmentIntersector::isBoundaryPoint(LineIntersector *li,
		vector<vector<Node*>*>& tstBdyNodes)
{
	if (isBoundaryPoint(li, tstBdyNodes[0])) return true;
	if (isBoundaryPoint(li, tstBdyNodes[1])) return true;
	return false;
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/03/23 13:31:58  strk
 * Fixed to allow build with GEOS_DEBUG
 *
 * Revision 1.17  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.16  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.15  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.14  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.13  2006/02/23 11:54:20  strk
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
 * Revision 1.12  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.11  2006/01/31 19:07:34  strk
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
 * Revision 1.10  2005/11/21 16:03:20  strk
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
 * Revision 1.9  2005/11/15 10:04:37  strk
 *
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.8  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.7  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.6  2004/11/17 08:41:42  strk
 * Fixed a bug in Z computation and removed debugging output by default.
 *
 * Revision 1.5  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
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
 * Revision 1.18  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/
