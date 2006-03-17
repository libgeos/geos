/**********************************************************************
 * $Id$
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
 * Last port: operation/valid/ConnectedInteriorTester.java rev. 1.14
 *
 **********************************************************************
 * 
 * TODO:
 *
 *  Remove heap allocation of GeometryFactory (might use a singleton)
 *
 **********************************************************************/

#include <vector>
#include <cassert>
#include <typeinfo>

#include <geos/opValid.h> // FIXME: split this

#include <geos/operation/overlay/MaximalEdgeRing.h>
#include <geos/operation/overlay/MinimalEdgeRing.h>
#include <geos/operation/overlay/OverlayNodeFactory.h>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>

#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geomgraph/PlanarGraph.h>
#include <geos/geomgraph/EdgeRing.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/Position.h>
#include <geos/geomgraph/Label.h>

using namespace std;
using namespace geos::geom;
using namespace geos::geomgraph;
using namespace geos::operation::overlay;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

ConnectedInteriorTester::ConnectedInteriorTester(GeometryGraph &newGeomGraph):
	geometryFactory(new GeometryFactory()),
	geomGraph(newGeomGraph)
{
}

ConnectedInteriorTester::~ConnectedInteriorTester()
{
	delete geometryFactory;
}

Coordinate&
ConnectedInteriorTester::getCoordinate()
{
	return disconnectedRingcoord;
}

const Coordinate&
ConnectedInteriorTester::findDifferentPoint(const CoordinateSequence *coord, const Coordinate& pt)
{
	unsigned int npts=coord->getSize();
	for(unsigned int i=0; i<npts; ++i)
	{
		if(!(coord->getAt(i)==pt))
			return coord->getAt(i);
	}
	return Coordinate::getNull();
}

/*public*/
bool
ConnectedInteriorTester::isInteriorsConnected()
{
	// node the edges, in case holes touch the shell
	std::vector<Edge*> splitEdges;
	geomGraph.computeSplitEdges(&splitEdges);

	// form the edges into rings
	PlanarGraph graph(operation::overlay::OverlayNodeFactory::instance());

	graph.addEdges(splitEdges);
	setInteriorEdgesInResult(graph);
	//graph.linkAllDirectedEdges();
	graph.linkResultDirectedEdges();
	std::vector<EdgeRing*> *edgeRings=buildEdgeRings(graph.getEdgeEnds());

	/*
	 * Mark all the edges for the edgeRings corresponding to the shells
	 * of the input polygons. 
	 * 
	 * Only ONE ring gets marked for each shell - if there are others which remain unmarked
	 * this indicates a disconnected interior.
	 */
	visitShellInteriors(geomGraph.getGeometry(), graph);

	/**
	 * If there are any unvisited shell edges
	 * (i.e. a ring which is not a hole and which has the interior
	 * of the parent area on the RHS)
	 * this means that one or more holes must have split the interior of the
	 * polygon into at least two pieces.  The polygon is thus invalid.
	 */
	bool res=!hasUnvisitedShellEdge(edgeRings);

	for(unsigned int i=0; i<edgeRings->size(); ++i)
	{
		delete (*edgeRings)[i];
	}
	delete edgeRings;
	return res;
}

void
ConnectedInteriorTester::setInteriorEdgesInResult(PlanarGraph &graph)
{
	std::vector<EdgeEnd*> *ee=graph.getEdgeEnds();
	for(unsigned int i=0, n=ee->size(); i<n; ++i)
	{
		// Unexpected non DirectedEdge in graphEdgeEnds
		assert(dynamic_cast<DirectedEdge*>((*ee)[i]));
		DirectedEdge *de=static_cast<DirectedEdge*>((*ee)[i]);
		if ( de->getLabel()->getLocation(0, Position::RIGHT) == Location::INTERIOR)
		{
			de->setInResult(true);
		}
	}
}

/*private*/
std::vector<EdgeRing*>*
ConnectedInteriorTester::buildEdgeRings(std::vector<EdgeEnd*> *dirEdges)
{
	std::vector<MinimalEdgeRing*> minEdgeRings;
	for(unsigned int i=0, n=dirEdges->size(); i<n; ++i)
	{
		assert(dynamic_cast<DirectedEdge*>((*dirEdges)[i]));
		DirectedEdge *de=static_cast<DirectedEdge*>((*dirEdges)[i]);
		// if this edge has not yet been processed
		if(de->isInResult() && de->getEdgeRing()==NULL)
		{
			//EdgeRing *er=new MaximalEdgeRing(de,geometryFactory);
			//edgeRings->push_back(er);

			MaximalEdgeRing* er = new MaximalEdgeRing(de, geometryFactory);
			er->linkDirectedEdgesForMinimalEdgeRings();
			er->buildMinimalRings(minEdgeRings);
		}
	}
	std::vector<EdgeRing*> *edgeRings=new std::vector<EdgeRing*>();
	edgeRings->assign(minEdgeRings.begin(), minEdgeRings.end());
	return edgeRings;
}

/**
 * Mark all the edges for the edgeRings corresponding to the shells
 * of the input polygons.  Note only ONE ring gets marked for each shell.
 */
void
ConnectedInteriorTester::visitShellInteriors(const Geometry *g, PlanarGraph &graph)
{
	if (const Polygon* p=dynamic_cast<const Polygon*>(g))
	{
		visitInteriorRing(p->getExteriorRing(), graph);
	}

	if (const MultiPolygon* mp=dynamic_cast<const MultiPolygon*>(g))
	{
		for (unsigned int i=0, n=mp->getNumGeometries(); i<n; i++) {
			const Polygon *p=static_cast<const Polygon*>(mp->getGeometryN(i));
			visitInteriorRing(p->getExteriorRing(), graph);
		}
	}
}

void
ConnectedInteriorTester::visitInteriorRing(const LineString *ring, PlanarGraph &graph)
{
	const CoordinateSequence *pts=ring->getCoordinatesRO();
	const Coordinate& pt0=pts->getAt(0);

	/**
	 * Find first point in coord list different to initial point.
	 * Need special check since the first point may be repeated.
	 */
    	const Coordinate& pt1=findDifferentPoint(pts, pt0);
	Edge *e=graph.findEdgeInSameDirection(pt0, pt1);
	DirectedEdge *de=static_cast<DirectedEdge*>(graph.findEdgeEnd(e));
	DirectedEdge *intDe=NULL;
	if (de->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de;
	} else if (de->getSym()->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de->getSym();
	}
	assert(intDe!=NULL); // unable to find dirEdge with Interior on RHS
	visitLinkedDirectedEdges(intDe);
}


void
ConnectedInteriorTester::visitLinkedDirectedEdges(DirectedEdge *start)
{
	DirectedEdge *startDe=start;
	DirectedEdge *de=start;
	//Debug.println(de);
	do {
		// found null Directed Edge
		assert(de!=NULL);

		de->setVisited(true);
		de=de->getNext();
		//Debug.println(de);
	} while (de!=startDe);
}

/**
 * Check if any shell ring has an unvisited edge.
 * A shell ring is a ring which is not a hole and which has the interior
 * of the parent area on the RHS.
 * (Note that there may be non-hole rings with the interior on the LHS,
 * since the interior of holes will also be polygonized into CW rings
 * by the linkAllDirectedEdges() step)
 *
 * @return true if there is an unvisited edge in a non-hole ring
 */
bool
ConnectedInteriorTester::hasUnvisitedShellEdge(std::vector<EdgeRing*> *edgeRings)
{
	for(std::vector<EdgeRing*>::iterator it=edgeRings->begin(), itEnd=edgeRings->end();
		it != itEnd;
		++it)
	{
		EdgeRing *er=*it;

		// don't check hole rings
		if (er->isHole()) continue;

		std::vector<DirectedEdge*> *edges=er->getEdges();
		DirectedEdge *de=(*edges)[0];

		// don't check CW rings which are holes
		// (MD - this check may now be irrelevant - 2006-03-09)
		if (de->getLabel()->getLocation(0,Position::RIGHT)!=Location::INTERIOR) continue;

		/*
		 * the edgeRing is CW ring which surrounds the INT of the area, so check all
		 * edges have been visited.  If any are unvisited, this is a disconnected part
		 * of the interior
		 */
		for(std::vector<DirectedEdge*>::iterator jt=edges->begin(), jtEnd=edges->end();
			jt != jtEnd;
			++jt)
		{
			de=*jt;
			//Debug.print("visted? "); Debug.println(de);
			if (!de->isVisited()) {
				//Debug.print("not visited "); Debug.println(de);
				disconnectedRingcoord=de->getCoordinate();
				return true;
			}
		}
	}
	return false;
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.23  2006/03/17 16:48:55  strk
 * LineIntersector and PointLocator made complete components of RelateComputer
 * (were statics const pointers before). Reduced inclusions from opRelate.h
 * and opValid.h, updated .cpp files to allow build.
 *
 **********************************************************************/
