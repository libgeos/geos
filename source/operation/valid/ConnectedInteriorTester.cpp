/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * TODO:
 *
 *  Remove heap allocation of GeometryFactory (might use a singleton)
 *
 **********************************************************************/


#include <geos/opValid.h>
#include <geos/opOverlay.h>
#include <geos/util.h>
#include <typeinfo>

namespace geos {

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

bool
ConnectedInteriorTester::isInteriorsConnected()
{
	// node the edges, in case holes touch the shell
	vector<Edge*> splitEdges;
	geomGraph.computeSplitEdges(&splitEdges);

	// polygonize the edges
	PlanarGraph graph(OverlayNodeFactory::instance());
	graph.addEdges(splitEdges);
	setAllEdgesInResult(graph);
	graph.linkAllDirectedEdges();
	vector<EdgeRing*> *edgeRings=buildEdgeRings(graph.getEdgeEnds());

	/**
	 * Mark all the edges for the edgeRings corresponding to the shells
	 * of the input polygons.  Note only ONE ring gets marked for each shell.
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
ConnectedInteriorTester::setAllEdgesInResult(PlanarGraph &graph)
{
	vector<EdgeEnd*> *ee=graph.getEdgeEnds();
	for(unsigned int i=0; i<ee->size(); ++i) {
		DirectedEdge *de=dynamic_cast<DirectedEdge*>((*ee)[i]);
		Assert::isTrue( de != NULL, "Unexpected non DirectedEdge in graphEdgeEnds");
		de->setInResult(true);
	}
}

/**
 * for all DirectedEdges in result, form them into EdgeRings
 */
vector<EdgeRing*>*
ConnectedInteriorTester::buildEdgeRings(vector<EdgeEnd*> *dirEdges)
{
	vector<EdgeRing*> *edgeRings=new vector<EdgeRing*>();
	for(unsigned int i=0; i<dirEdges->size(); ++i)
	{
		DirectedEdge *de=(DirectedEdge*)(*dirEdges)[i];
		// if this edge has not yet been processed
		if(de->getEdgeRing()==NULL) {
			EdgeRing *er=new MaximalEdgeRing(de,geometryFactory);
			edgeRings->push_back(er);
		}
	}
	return edgeRings;
}

/**
 * Mark all the edges for the edgeRings corresponding to the shells
 * of the input polygons.  Note only ONE ring gets marked for each shell.
 */
void
ConnectedInteriorTester::visitShellInteriors(const Geometry *g, PlanarGraph &graph)
{
	if (typeid(*g)==typeid(Polygon)) {
		const Polygon *p=(Polygon*) g;
		visitInteriorRing(p->getExteriorRing(), graph);
	}
	if (typeid(*g)==typeid(MultiPolygon)) {
		const MultiPolygon *mp=(MultiPolygon*) g;
		for(int i=0; i<(int)mp->getNumGeometries();i++) {
			const Polygon *p=(Polygon*)mp->getGeometryN(i);
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
	DirectedEdge *de=(DirectedEdge*) graph.findEdgeEnd(e);
	DirectedEdge *intDe=NULL;
	if (de->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de;
	} else if (de->getSym()->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de->getSym();
	}
	Assert::isTrue(intDe!=NULL, "unable to find dirEdge with Interior on RHS");
	visitLinkedDirectedEdges(intDe);
}


void
ConnectedInteriorTester::visitLinkedDirectedEdges(DirectedEdge *start)
{
	DirectedEdge *startDe=start;
	DirectedEdge *de=start;
	//Debug.println(de);
	do {
		Assert::isTrue(de!=NULL, "ConnectedInteriorTester::visitLinkedDirectedEdges() found null Directed Edge");
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
ConnectedInteriorTester::hasUnvisitedShellEdge(vector<EdgeRing*> *edgeRings)
{
	for(int i=0;i<(int)edgeRings->size();i++) {
		EdgeRing *er=(*edgeRings)[i];
		if (er->isHole()) continue;
		vector<DirectedEdge*> *edges=er->getEdges();
		DirectedEdge *de=(*edges)[0];
		// don't check CW rings which are holes
		if (de->getLabel()->getLocation(0,Position::RIGHT)!=Location::INTERIOR) continue;
		// must have a CW ring which surrounds the INT of the area, so check all
		// edges have been visited
		for(int j=0; j<(int)edges->size();j++) {
			de=(*edges)[j];
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/01/31 19:07:34  strk
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
 * Revision 1.16  2005/12/07 19:18:23  strk
 * Changed PlanarGraph::addEdges and EdgeList::addAll to take
 * a const vector by reference rather then a non-const vector by
 * pointer.
 * Optimized polygon vector allocations in OverlayOp::computeOverlay.
 *
 * Revision 1.15  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.14  2005/11/21 16:03:20  strk
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
 * Revision 1.13  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.12  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.9  2003/10/20 14:02:14  strk
 * more explicit exception thrown on null Directed Edge detection
 *
 * Revision 1.8  2003/10/15 11:24:28  strk
 * Use getCoordinatesRO() introduced.
 *
 **********************************************************************/
