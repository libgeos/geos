/**********************************************************************
 * $Id$
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
 * Last port: geomgraph/EdgeIntersectionList.java rev. 1.3
 *
 **********************************************************************/

#include <geos/geomgraph.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

EdgeIntersectionList::EdgeIntersectionList(Edge *newEdge):
	edge(newEdge)
{
}

EdgeIntersectionList::~EdgeIntersectionList()
{
	for (EdgeIntersectionList::iterator it=nodeMap.begin(),
		endIt=nodeMap.end();
		it!=endIt; ++it)
	{
		delete *it;
	}
}

EdgeIntersection*
EdgeIntersectionList::add(const Coordinate& coord,
	int segmentIndex, double dist)
{
	EdgeIntersection *eiNew=new EdgeIntersection(coord, segmentIndex, dist);

	pair<EdgeIntersectionList::iterator, bool> p = nodeMap.insert(eiNew);
	if ( p.second ) { // new EdgeIntersection inserted
		return eiNew;
	} else {
		delete eiNew;
		return *(p.first);
	}
}

bool
EdgeIntersectionList::isEmpty() const
{
	return nodeMap.empty();
}

bool
EdgeIntersectionList::isIntersection(const Coordinate& pt) const
{
	EdgeIntersectionList::const_iterator
		it=nodeMap.begin(),
		endIt=nodeMap.end();

	for (; it!=endIt; ++it)
	{
		EdgeIntersection *ei=*it;
		if (ei->coord==pt) return true;
	}
	return false;
}

void
EdgeIntersectionList::addEndpoints()
{
	int maxSegIndex=edge->getNumPoints()-1;
	add(edge->pts->getAt(0), 0, 0.0);
	add(edge->pts->getAt(maxSegIndex), maxSegIndex, 0.0);
}

void
EdgeIntersectionList::addSplitEdges(vector<Edge*> *edgeList)
{
	// ensure that the list has entries for the first and last point
	// of the edge
	addEndpoints();

	EdgeIntersectionList::iterator it=nodeMap.begin();

	// there should always be at least two entries in the list
	EdgeIntersection *eiPrev=*it;
	++it;

	while (it!=nodeMap.end()) {
		EdgeIntersection *ei=*it;
		Edge *newEdge=createSplitEdge(eiPrev,ei);
		edgeList->push_back(newEdge);
		eiPrev=ei;
		it++;
	}
}

Edge *
EdgeIntersectionList::createSplitEdge(EdgeIntersection *ei0,
	EdgeIntersection *ei1)
{
#if DEBUG
	cerr<<"["<<this<<"] EdgeIntersectionList::createSplitEdge()"<<endl;
#endif // DEBUG
	int npts=ei1->segmentIndex-ei0->segmentIndex+2;

	const Coordinate& lastSegStartPt=edge->pts->getAt(ei1->segmentIndex);

	// if the last intersection point is not equal to the its segment
	// start pt, add it to the points list as well.
	// (This check is needed because the distance metric is not totally
	// reliable!). The check for point equality is 2D only - Z values
	// are ignored
	bool useIntPt1=ei1->dist>0.0 || !ei1->coord.equals2D(lastSegStartPt);

	if (!useIntPt1) --npts;

#if DEBUG
	cerr<<"    npts:"<<npts<<endl;
#endif // DEBUG

	vector<Coordinate> *vc=new vector<Coordinate>();
	vc->reserve(npts);

	vc->push_back(ei0->coord);
	for(int i=ei0->segmentIndex+1; i<=ei1->segmentIndex;i++)
	{
		if ( ! useIntPt1 && ei1->segmentIndex == i )
		{
			vc->push_back(ei1->coord);
		}
		else
		{
			vc->push_back(edge->pts->getAt(i));
		}
	}

	if (useIntPt1)
	{
		vc->push_back(ei1->coord);
	}

	CoordinateSequence* pts=new CoordinateArraySequence(vc);

	return new Edge(pts, new Label(*(edge->getLabel())));
}

string
EdgeIntersectionList::print() const
{
	string out="Intersections: ";
	EdgeIntersectionList::const_iterator it=begin(), endIt=end();
	for (; it!=endIt; ++it) {
		EdgeIntersection *ei=*it;
		out+=ei->print();
	}
	return out;
}

} // namespace

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/01/31 19:07:34  strk
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
 * Revision 1.17  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.16  2005/12/08 01:11:29  strk
 * minor optimizations in loops
 *
 * Revision 1.15  2005/12/08 00:24:41  strk
 * Reduced coordinate assignments in ::createSplitEdge
 *
 * Revision 1.14  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.13  2005/11/07 18:05:28  strk
 * Reduced set<> lookups
 *
 * Revision 1.12  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.11  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.10  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.9  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.8  2004/11/22 13:02:12  strk
 * Forced use if computed intersection point in ::createSplitEdge (for Z computation)
 *
 * Revision 1.7  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.6  2004/11/04 19:08:06  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.5  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
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
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.18  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.17  2003/11/06 19:04:28  strk
 * removed useless Coordinate copy in ::createSplitEdge()
 *
 **********************************************************************/

