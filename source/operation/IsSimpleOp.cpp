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
 **********************************************************************/

#include <geos/operation/IsSimpleOp.h>
#include <geos/operation/EndpointInfo.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeIntersection.h>
#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>

#include <set>
#include <cassert>

using namespace std;
using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geomgraph::index;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation

IsSimpleOp::IsSimpleOp()
{}

bool
IsSimpleOp::isSimple(const LineString *geom)
{
	return isSimpleLinearGeometry(geom);
}

bool
IsSimpleOp::isSimple(const MultiLineString *geom)
{
	return isSimpleLinearGeometry(geom);
}

/**
 * A MultiPoint is simple iff it has no repeated points
 */
bool
IsSimpleOp::isSimple(const MultiPoint *mp)
{
	if (mp->isEmpty()) return true;
	set<const Coordinate*, CoordinateLessThen> points;

	for (unsigned int i=0, n=mp->getNumGeometries(); i<n; i++)
	{
		assert(dynamic_cast<const Point*>(mp->getGeometryN(i)));
		const Point *pt=static_cast<const Point*>(mp->getGeometryN(i));
		const Coordinate *p=pt->getCoordinate();
		if (points.find(p) != points.end()) {
			return false;
		}
		points.insert(p);
	}
	return true;
}

bool
IsSimpleOp::isSimpleLinearGeometry(const Geometry *geom)
{
	if (geom->isEmpty()) return true;
	GeometryGraph graph(0,geom);
	LineIntersector li;
	SegmentIntersector *si=graph.computeSelfNodes(&li,true);
	// if no self-intersection, must be simple
	if (!si->hasIntersection()) {
		delete si;
		return true;
	}
	if (si->hasProperIntersection()) {
		delete si;
		return false;
	}
	if (hasNonEndpointIntersection(graph)) {
		delete si;
		return false;
	}
	if (hasClosedEndpointIntersection(graph)) {
		delete si;
		return false;
	}
	delete si;
	return true;
}

/**
 * For all edges, check if there are any intersections which are NOT at an endpoint.
 * The Geometry is not simple if there are intersections not at endpoints.
 */
bool
IsSimpleOp::hasNonEndpointIntersection(GeometryGraph &graph)
{
	vector<Edge*> *edges=graph.getEdges();
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		int maxSegmentIndex=e->getMaximumSegmentIndex();
		EdgeIntersectionList &eiL=e->getEdgeIntersectionList();
		for ( EdgeIntersectionList::iterator eiIt=eiL.begin(),
			eiEnd=eiL.end(); eiIt!=eiEnd; ++eiIt )
		{
			EdgeIntersection *ei=*eiIt;
			if (!ei->isEndPoint(maxSegmentIndex))
				return true;
		}
	}
	return false;
}

/**
 * Test that no edge intersection is the
 * endpoint of a closed line.  To check this we compute the
 * degree of each endpoint. The degree of endpoints of closed lines
 * must be exactly 2.
 */
bool
IsSimpleOp::hasClosedEndpointIntersection(GeometryGraph &graph)
{
	map<const Coordinate*,EndpointInfo*,CoordinateLessThen> endPoints;
	vector<Edge*> *edges=graph.getEdges();
	for (vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		//int maxSegmentIndex=e->getMaximumSegmentIndex();
		bool isClosed=e->isClosed();
		const Coordinate *p0=&e->getCoordinate(0);
		addEndpoint(endPoints,p0,isClosed);
		const Coordinate *p1=&e->getCoordinate(e->getNumPoints()-1);
		addEndpoint(endPoints,p1,isClosed);
	}

	map<const Coordinate*,EndpointInfo*,CoordinateLessThen>::iterator it=endPoints.begin();
	for (; it!=endPoints.end(); it++) {
		EndpointInfo *eiInfo=it->second;
		if (eiInfo->isClosed && eiInfo->degree!=2) {
			it=endPoints.begin();
			for (; it!=endPoints.end(); it++) {
				EndpointInfo *ep=it->second;
				delete ep;
			}
            		return true;
		}
	}

	it=endPoints.begin();
	for (; it!=endPoints.end(); it++) {
		EndpointInfo *ep=it->second;
		delete ep;
	}
	return false;
}

/**
* Add an endpoint to the map, creating an entry for it if none exists
*/
void
IsSimpleOp::addEndpoint(
	map<const Coordinate*,EndpointInfo*,CoordinateLessThen>&endPoints,
	const Coordinate *p,bool isClosed)
{
	map<const Coordinate*,EndpointInfo*,CoordinateLessThen>::iterator it=endPoints.find(p);
	EndpointInfo *eiInfo;
	if (it==endPoints.end()) {
		eiInfo=NULL;
	} else {
		eiInfo=it->second;
	}
	if (eiInfo==NULL) {
		eiInfo=new EndpointInfo(*p);
		endPoints[p]=eiInfo;
	}
	eiInfo->addEndpoint(isClosed);
}

EndpointInfo::EndpointInfo(const Coordinate& newPt)
{
	pt=newPt;
	isClosed=false;
	degree=0;
}

void
EndpointInfo::addEndpoint(bool newIsClosed)
{
	degree++;
	isClosed|=newIsClosed;
}

} // namespace geos::operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.23  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 * Revision 1.22  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.21  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.20  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.19  2005/11/21 16:03:20  strk
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
 * Revision 1.18  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.17  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.16  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.15  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.14  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.13  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

