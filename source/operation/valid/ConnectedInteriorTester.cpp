#include "opValid.h"
#include "opOverlay.h"
#include "stdio.h"
#include "util.h"
#include <typeinfo>

ConnectedInteriorTester::ConnectedInteriorTester(GeometryGraph *newGeomGraph) {
	geomGraph=newGeomGraph;
	geometryFactory=new GeometryFactory();
	cga=new RobustCGAlgorithms();
}

ConnectedInteriorTester::~ConnectedInteriorTester() {
	delete geometryFactory;
	delete cga;
}

Coordinate& ConnectedInteriorTester::getCoordinate() {
	return disconnectedRingcoord;
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
	return !hasUnvisitedShellEdge(edgeRings);
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
void ConnectedInteriorTester::visitShellInteriors(Geometry *g, PlanarGraph *graph) {
	if (typeid(*g)==typeid(Polygon)) {
		Polygon *p=(Polygon*) g;
		visitInteriorRing(p->getExteriorRing(),graph);
	}
	if (typeid(*g)==typeid(MultiPolygon)) {
		MultiPolygon *mp=(MultiPolygon*) g;
		for(int i=0; i<(int)mp->getNumGeometries();i++) {
			Polygon *p=(Polygon*)mp->getGeometryN(i);
			visitInteriorRing(p->getExteriorRing(),graph);
		}
	}
}

void ConnectedInteriorTester::visitInteriorRing(LineString *ring, PlanarGraph *graph) {
	CoordinateList *pts=ring->getCoordinates();
	Edge *e=graph->findEdgeInSameDirection(pts->getAt(0),pts->getAt(1));
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
		Assert::isTrue(de!=NULL, "found null Directed Edge");
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