/**********************************************************************
 * $Id: SimpleEdgeSetIntersector.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <vector>

#include <geos/geomgraph/index/SimpleEdgeSetIntersector.h>
#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geomgraph/Edge.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

SimpleEdgeSetIntersector::SimpleEdgeSetIntersector():
	nOverlaps(0)
{
}

void
SimpleEdgeSetIntersector::computeIntersections(vector<Edge*> *edges,
		SegmentIntersector *si, bool testAllSegments)
{
	nOverlaps = 0;
	size_t nedges=edges->size();
	for(size_t i0=0; i0<nedges; ++i0)
	{
		Edge *edge0=(*edges)[i0];
		for(size_t i1=0; i1<nedges; ++i1)
		{
			Edge *edge1=(*edges)[i1];
			if (testAllSegments || edge0!=edge1)
				computeIntersects(edge0,edge1,si);
		}
	}
}


void
SimpleEdgeSetIntersector::computeIntersections(vector<Edge*> *edges0,
		vector<Edge*> *edges1, SegmentIntersector *si)
{
	nOverlaps = 0;

	size_t nedges0=edges0->size();
	size_t nedges1=edges1->size();

	for(size_t i0=0;i0<nedges0; ++i0)
	{
		Edge *edge0=(*edges0)[i0];
		for(size_t i1=0; i1<nedges1; ++i1)
		{
			Edge *edge1=(*edges1)[i1];
			computeIntersects(edge0,edge1,si);
		}
	}
}

/**
 * Performs a brute-force comparison of every segment in each Edge.
 * This has n^2 performance, and is about 100 times slower than using
 * monotone chains.
 */
void
SimpleEdgeSetIntersector::computeIntersects(Edge *e0, Edge *e1,
		SegmentIntersector *si)
{
	const CoordinateSequence *pts0=e0->getCoordinates();
	const CoordinateSequence *pts1=e1->getCoordinates();

	size_t npts0=pts0->getSize();
	size_t npts1=pts1->getSize();

	for(size_t i0=0; i0<npts0-1; ++i0)
	{
		for(size_t i1=0; i1<npts1-1; ++i1)
		{
			si->addIntersections(e0, i0, e1, i1);
		}
	}
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.9  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.8  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.7  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
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
 * Revision 1.4  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
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
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.6  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.4  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/

