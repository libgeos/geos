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
 **********************************************************************
 * $Log$
 * Revision 1.13  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
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


#include <geos/opValid.h>
#include <geos/opOverlay.h>
#include <stdio.h>
#include <geos/util.h>
#include <typeinfo>

namespace geos {

ConnectedInteriorTester::ConnectedInteriorTester(GeometryGraph *newGeomGraph) {
	geomGraph=newGeomGraph;
	geometryFactory=new GeometryFactory();
	cga=new CGAlgorithms();
}

ConnectedInteriorTester::~ConnectedInteriorTester() {
	delete geometryFactory;
	delete cga;
}

Coordinate& ConnectedInteriorTester::getCoordinate() {
	return disconnectedRingcoord;
}

const Coordinate& ConnectedInteriorTester::findDifferentPoint(const CoordinateSequence *coord, const Coordinate& pt){
	for(int i=0;i<coord->getSize();i++) {
		if(!(coord->getAt(i)==pt))
			return coord->getAt(i);
	}
	return Coordinate::getNull();
}

bool ConnectedInteriorTester::isInteriorsConnected() {
	// node the edges, in case holes touch the shell
	vector<Edge*> *splitEdges=new vector<Edge*>();
	geomGraph->computeSplitEdges(splitEdges);
	// polygonize the edges
	PlanarGraph *graph=new PlanarGraph(new OverlayNodeFactory());
	graph->addEdges(splitEdges);
	setAllEdgesInResult(graph);
	graph->linkAllDirectedEdges();
	vector<EdgeRing*> *edgeRings=buildEdgeRings(graph->getEdgeEnds());
	/**
	* Mark all the edges for the edgeRings corresponding to the shells
	* of the input polygons.  Note only ONE ring gets marked for each shell.
	*/
	visitShellInteriors(geomGraph->getGeometry(),graph);
	/**
	* If there are any unvisited shell edges
	* (i.e. a ring which is not a hole and which has the interior
	* of the parent area on the RHS)
	* this means that one or more holes must have split the interior of the
	* polygon into at least two pieces.  The polygon is thus invalid.
	*/
	bool res=!hasUnvisitedShellEdge(edgeRings);
	delete graph;
	delete splitEdges;
	for(int i=0;i<(int)edgeRings->size();i++) {
		delete (*edgeRings)[i];
	}
	delete edgeRings;
	return res;
}

void ConnectedInteriorTester::setAllEdgesInResult(PlanarGraph *graph) {
	vector<EdgeEnd*> *ee=graph->getEdgeEnds();
	for(int i=0; i<(int) ee->size(); i++) {
		DirectedEdge *de=(DirectedEdge*)(*ee)[i];
		de->setInResult(true);
	}
}

/**
* for all DirectedEdges in result, form them into EdgeRings
*/
vector<EdgeRing*>* ConnectedInteriorTester::buildEdgeRings(vector<EdgeEnd*> *dirEdges) {
	vector<EdgeRing*> *edgeRings=new vector<EdgeRing*>();
	for(int i=0; i<(int) dirEdges->size(); i++) {
		DirectedEdge *de=(DirectedEdge*)(*dirEdges)[i];
		// if this edge has not yet been processed
		if(de->getEdgeRing()==NULL) {
			EdgeRing *er=new MaximalEdgeRing(de,geometryFactory,cga);
			edgeRings->push_back(er);
		}
	}
	return edgeRings;
}

/**
* Mark all the edges for the edgeRings corresponding to the shells
* of the input polygons.  Note only ONE ring gets marked for each shell.
*/
void ConnectedInteriorTester::visitShellInteriors(const Geometry *g, PlanarGraph *graph) {
	if (typeid(*g)==typeid(Polygon)) {
		const Polygon *p=(Polygon*) g;
		visitInteriorRing(p->getExteriorRing(),graph);
	}
	if (typeid(*g)==typeid(MultiPolygon)) {
		const MultiPolygon *mp=(MultiPolygon*) g;
		for(int i=0; i<(int)mp->getNumGeometries();i++) {
			const Polygon *p=(Polygon*)mp->getGeometryN(i);
			visitInteriorRing(p->getExteriorRing(),graph);
		}
	}
}

void ConnectedInteriorTester::visitInteriorRing(const LineString *ring, PlanarGraph *graph) {
	const CoordinateSequence *pts=ring->getCoordinatesRO();
	const Coordinate& pt0=pts->getAt(0);
    /**
     * Find first point in coord list different to initial point.
     * Need special check since the first point may be repeated.
     */
    	const Coordinate& pt1=findDifferentPoint(pts,pt0);
	Edge *e=graph->findEdgeInSameDirection(pt0,pt1);
	DirectedEdge *de=(DirectedEdge*) graph->findEdgeEnd(e);
	DirectedEdge *intDe=NULL;
	if (de->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de;
	} else if (de->getSym()->getLabel()->getLocation(0,Position::RIGHT)==Location::INTERIOR) {
		intDe=de->getSym();
	}
	Assert::isTrue(intDe!=NULL, "unable to find dirEdge with Interior on RHS");
	visitLinkedDirectedEdges(intDe);
}


void ConnectedInteriorTester::visitLinkedDirectedEdges(DirectedEdge *start){
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
bool ConnectedInteriorTester::hasUnvisitedShellEdge(vector<EdgeRing*> *edgeRings) {
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
}

