/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009      Sandro Santilli <strk@keybit.net>
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
 * Last port: operation/IsSimpleOp.java rev. 1.18
 *
 **********************************************************************/

#include <geos/operation/IsSimpleOp.h>
#include <geos/operation/EndpointInfo.h>
#include <geos/algorithm/BoundaryNodeRule.h>
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

/*public*/
IsSimpleOp::IsSimpleOp()
	:
	isClosedEndpointsInInterior(true)
{}

/*public*/
IsSimpleOp::IsSimpleOp(const algorithm::BoundaryNodeRule& boundaryNodeRule)
	:
	isClosedEndpointsInInterior( ! boundaryNodeRule.isInBoundary(2) )
{}

/*public*/
bool
IsSimpleOp::isSimple(const LineString *geom)
{
	return isSimpleLinearGeometry(geom);
}

/*public*/
bool
IsSimpleOp::isSimple(const MultiLineString *geom)
{
	return isSimpleLinearGeometry(geom);
}

/*public*/
bool
IsSimpleOp::isSimple(const MultiPoint *mp)
{
	if (mp->isEmpty()) return true;
	set<const Coordinate*, CoordinateLessThen> points;

    for (std::size_t i=0, n=mp->getNumGeometries(); i<n; i++)
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
	std::auto_ptr<SegmentIntersector> si (graph.computeSelfNodes(&li,true));

	// if no self-intersection, must be simple
	if (!si->hasIntersection()) return true;
	
	if (si->hasProperIntersection()) return false;
	
	if (hasNonEndpointIntersection(graph)) return false;

	if ( isClosedEndpointsInInterior ) {
		if (hasClosedEndpointIntersection(graph)) return false;
	}
	
	return true;
}

/*private*/
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

/*private*/
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
	for (; it!=endPoints.end(); ++it) {
		EndpointInfo *eiInfo=it->second;
		if (eiInfo->isClosed && eiInfo->degree!=2) {
			it=endPoints.begin();
			for (; it!=endPoints.end(); ++it) {
				EndpointInfo *ep=it->second;
				delete ep;
			}
            		return true;
		}
	}

	it=endPoints.begin();
	for (; it!=endPoints.end(); ++it) {
		EndpointInfo *ep=it->second;
		delete ep;
	}
	return false;
}

/*private*/
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

