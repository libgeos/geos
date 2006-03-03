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

#include <geos/geomgraph.h>
#include <geos/indexQuadtree.h>
#include <geos/profiler.h>
#include <string>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::index::quadtree;

namespace geos {
namespace geomgraph { // geos.geomgraph

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

/*public*/
void
EdgeList::add(Edge *e)
{
	edges.push_back(e);
	index->insert(e->getEnvelope(), e);
}

void
EdgeList::addAll(const vector<Edge*> &edgeColl)
{
	for (unsigned int i=0, s=edgeColl.size(); i<s ; ++i)
	{
		add(edgeColl[i]);
	}
}

// <FIX> fast lookup for edges
/**
 * If there is an edge equal to e already in the list, return it.
 * Otherwise return null.
 * @return  equal edge, if there is one already in the list
 *          null otherwise
 */
Edge *
EdgeList::findEqualEdge(Edge *e)
{
#if PROFILE
	static Profile *prof = profiler->get("EdgeList::findEqualEdge(Edge *e)");
	prof->start();
#endif
	vector<void*> testEdges;
	index->query(e->getEnvelope(), testEdges);
#if PROFILE
	prof->stop();
#endif

#if GEOS_DEBUG
	cerr << "EdgeList::findEqualEdge found " << testEdges.size() <<
			" overlapping edges" << endl;
#endif

	for (unsigned int i=0, s=testEdges.size(); i<s; ++i)
	{
		Edge* testEdge=static_cast<Edge*>(testEdges[i]);
		if (testEdge->equals(e))
		{
			return testEdge;
		}
	}
	return NULL;
}

Edge*
EdgeList::get(int i)
{
	return edges[i];
}

/**
 * If the edge e is already in the list, return its index.
 * @return  index, if e is already in the list
 *          -1 otherwise
 */
int
EdgeList::findEdgeIndex(Edge *e)
{
	for (int i=0, s=(int)edges.size(); i<s; ++i)
	{
		if ( edges[i]->equals(e) ) return i;
	}
	return -1;
}

string
EdgeList::print()
{
	string out="EdgeList( ";
	for(unsigned int j=0, s=edges.size(); j<s; ++j)
	{
       		Edge *e=edges[j];
		if (j) out+=",";
		out += e->print();
	}
	out+=")  ";
	return out;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.14  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.13  2006/02/28 14:34:04  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.12  2006/02/23 11:54:20  strk
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
 * Revision 1.11  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.10  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.9  2006/01/31 19:07:34  strk
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
 * Revision 1.8  2005/12/07 19:18:23  strk
 * Changed PlanarGraph::addEdges and EdgeList::addAll to take
 * a const vector by reference rather then a non-const vector by
 * pointer.
 * Optimized polygon vector allocations in OverlayOp::computeOverlay.
 *
 * Revision 1.7  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
 * Revision 1.6  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.5  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
