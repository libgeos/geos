/**********************************************************************
 * $Id: EdgeList.cpp 2548 2009-06-05 13:44:48Z strk $
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
 **********************************************************************
 *
 * Last port: geomgraph/EdgeList.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#include <string>
#include <sstream>
#include <vector>

#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeList.h>
#include <geos/noding/OrientedCoordinateArray.h> 
#include <geos/profiler.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::noding;

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
	OrientedCoordinateArray* oca = new OrientedCoordinateArray(*(e->getCoordinates()));
	ocaMap[oca] = e;
}

void
EdgeList::addAll(const vector<Edge*> &edgeColl)
{
    for (std::size_t i=0, s=edgeColl.size(); i<s ; ++i)
	{
		add(edgeColl[i]);
	}
}

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

	OrientedCoordinateArray oca(*(e->getCoordinates()));

	EdgeMap::iterator it = ocaMap.find(&oca);

#if PROFILE
	prof->stop();
#endif

	if ( it != ocaMap.end() ) return it->second;
	return 0;
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
	ostringstream ss;
	ss << *this;
	return ss.str();
#if 0
	string out="EdgeList( ";
	for(unsigned int j=0, s=edges.size(); j<s; ++j)
	{
       		Edge *e=edges[j];
		if (j) out+=",";
		out += e->print();
	}
	out+=")  ";
	return out;
#endif
}

void
EdgeList::clearList()
{
        for (unsigned int pos=0; pos < edges.size(); pos++)
                delete *(&edges[pos]);

        edges.clear();
}

std::ostream&
operator<< (std::ostream&os, const EdgeList& el)
{
	os << "EdgeList: " << std::endl;
	for(std::size_t j=0, s=el.edges.size(); j<s; ++j)
	{
       		Edge *e=el.edges[j];
		os << "  " << *e << std::endl; 
	}
	return os;
}

EdgeList::~EdgeList()
{
	for (EdgeMap::iterator i=ocaMap.begin(), e=ocaMap.end(); i!=e; ++i)
	{
		delete i->first; // OrientedCoordinateArray
	}
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/03/22 12:22:49  strk
 * indexQuadtree.h split
 *
 * Revision 1.16  2006/03/14 11:03:14  strk
 * Added operator<< for Edge and EdgeList
 *
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
 **********************************************************************/
