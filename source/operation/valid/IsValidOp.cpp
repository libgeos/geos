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
 * Revision 1.22  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.21  2004/09/13 10:12:49  strk
 * Added invalid coordinates checks in IsValidOp.
 * Cleanups.
 *
 * Revision 1.20  2004/09/13 09:18:10  strk
 * Added IsValidOp::isValid(Coordinate &)
 *
 * Revision 1.19  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.18  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.17  2004/05/18 00:02:37  ybychkov
 * IsValidOp::checkShellNotNested() bugfix from JTS 1.4.1 (not released yet) has been added.
 *
 * Revision 1.16  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.15  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.14  2003/10/15 11:24:28  strk
 * Use getCoordinatesRO() introduced.
 *
 * Revision 1.13  2003/10/13 17:54:40  strk
 * IsValidOp constructor used same name for the arg and a private 
 * element. Fixed.
 *
 **********************************************************************/


#include <geos/opValid.h>
#include <stdio.h>
#include <typeinfo>
#include <set>
#include <geos/util.h>

namespace geos {

const Coordinate& IsValidOp::findPtNotNode(const CoordinateSequence *testCoords,const LinearRing *searchRing, GeometryGraph *graph) {
	// find edge corresponding to searchRing.
	Edge *searchEdge=graph->findEdge(searchRing);
	// find a point in the testCoords which is not a node of the searchRing
	EdgeIntersectionList *eiList=searchEdge->getEdgeIntersectionList();
	// somewhat inefficient - is there a better way? (Use a node map, for instance?)
	for(int i=0;i<testCoords->getSize(); i++) {
		const Coordinate& pt=testCoords->getAt(i);
		if (!eiList->isIntersection(pt)) {
			return pt;
		}
	}
	return Coordinate::getNull();
}

IsValidOp::IsValidOp(const Geometry *geom){
	isChecked=false;
	validErr=NULL;
	parentGeometry=geom;
}

IsValidOp::~IsValidOp(){
	delete validErr;
}

bool IsValidOp::isValid() {
	checkValid(parentGeometry);
	return validErr==NULL;
}

/*
 * Checks whether a coordinate is valid for processing.
 * Coordinates are valid iff their x and y coordinates are in the
 * range of the floating point representation.
 *
 * @param coord the coordinate to validate
 * @return <code>true</code> if the coordinate is valid
 */
bool IsValidOp::isValid(const Coordinate &coord) {
	if (! finite(coord.x) ) return false;
	if (! finite(coord.y) ) return false;
	return true;
}

TopologyValidationError* IsValidOp::getValidationError() {
	checkValid(parentGeometry);
	return validErr;
}

void
IsValidOp::checkValid(const Geometry *g)
{
	const GeometryCollection *gc;
	if (isChecked) return;
	validErr=NULL;
	if (g->isEmpty()) return;
	if (typeid(*g)==typeid(Point)) checkValid((Point *)g);
	else if (typeid(*g)==typeid(LinearRing)) checkValid((LinearRing*)g);
	else if (typeid(*g)==typeid(LineString)) checkValid((LineString*)g);
	else if (typeid(*g)==typeid(Polygon)) checkValid((Polygon*)g);
	else if (typeid(*g)==typeid(MultiPolygon)) checkValid((MultiPolygon*)g);
	else if ((gc=dynamic_cast<const GeometryCollection *>(g)))
		checkValid(gc);
	else throw new UnsupportedOperationException();
}

/*
 * Checks validity of a Point.
 */
void IsValidOp::checkValid(const Point *g){
	CoordinateSequence *cs = g->getCoordinates();
	checkInvalidCoordinates(cs);
	delete cs;
}

/*
 * Checks validity of a LineString.  Almost anything goes for linestrings!
 */
void IsValidOp::checkValid(const LineString *g){
	checkInvalidCoordinates(g->getCoordinatesRO());
	if (validErr != NULL) return;
	GeometryGraph *graph=new GeometryGraph(0,g);
	checkTooFewPoints(graph);
	delete graph;
}

/**
* Checks validity of a LinearRing.
*/
void IsValidOp::checkValid(const LinearRing *g){
	checkInvalidCoordinates(g->getCoordinatesRO());
	if (validErr != NULL) return;
	GeometryGraph *graph = new GeometryGraph(0, g);
	checkTooFewPoints(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	LineIntersector *li = new RobustLineIntersector();
	graph->computeSelfNodes(li, true);
	checkNoSelfIntersectingRings(graph);
	delete li;
	delete graph;
}

/**
* Checks the validity of a polygon.
* Sets the validErr flag.
*/
void IsValidOp::checkValid(const Polygon *g){
	checkInvalidCoordinates(g);
	if (validErr != NULL) return;
	GeometryGraph *graph=new GeometryGraph(0,g);
	checkTooFewPoints(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkConsistentArea(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkNoSelfIntersectingRings(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkHolesInShell(g,graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	//SLOWcheckHolesNotNested(g);
	checkHolesNotNested(g,graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkConnectedInteriors(graph);
	delete graph;
}

void
IsValidOp::checkValid(const MultiPolygon *g)
{
	for (int i=0; i<g->getNumGeometries(); i++)
	{
		checkInvalidCoordinates((const Polygon *)g->getGeometryN(i));
		if (validErr != NULL) return;
	}

	GeometryGraph *graph=new GeometryGraph(0,g);
	checkTooFewPoints(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkConsistentArea(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkNoSelfIntersectingRings(graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}

	for(int i=0;i<g->getNumGeometries();i++) {
		Polygon *p=(Polygon*)g->getGeometryN(i);
		checkHolesInShell(p,graph);
		if (validErr!=NULL) {
			delete graph;
			return;
		}
	}
	for(int i=0;i<g->getNumGeometries();i++) {
		Polygon *p=(Polygon*)g->getGeometryN(i);
		//checkDisjointHolesNotNested(p);
		checkHolesNotNested(p,graph);
		if (validErr!=NULL) {
			delete graph;
			return;
		}
	}
	checkShellsNotNested(g,graph);
	if (validErr!=NULL) {
		delete graph;
		return;
	}
	checkConnectedInteriors(graph);
	delete graph;
}

void IsValidOp::checkValid(const GeometryCollection *gc) {
	for(int i=0;i<gc->getNumGeometries();i++) {
		const Geometry *g=gc->getGeometryN(i);
		checkValid(g);
		if (validErr!=NULL) return;
	}
}

void IsValidOp::checkTooFewPoints(GeometryGraph *graph) {
	if (graph->hasTooFewPoints()) {
		validErr=new TopologyValidationError(
			TopologyValidationError::TOO_FEW_POINTS,
			graph->getInvalidPoint());
		return;
	}
}

void IsValidOp::checkConsistentArea(GeometryGraph *graph) {
	auto_ptr<ConsistentAreaTester> cat(new ConsistentAreaTester(graph));
	bool isValidArea=cat->isNodeConsistentArea();
	if (!isValidArea) {
		validErr=new TopologyValidationError(
			TopologyValidationError::SELF_INTERSECTION,
			cat->getInvalidPoint());
		return;
	}
	if (cat->hasDuplicateRings()) {
		validErr=new TopologyValidationError(
			TopologyValidationError::DUPLICATE_RINGS,
			cat->getInvalidPoint());
	}
}

void IsValidOp::checkNoSelfIntersectingRings(GeometryGraph *graph) {
	vector<Edge*> *edges=graph->getEdges();
	for(int i=0;i<(int)edges->size();i++) {
		Edge *e=(*edges)[i];
		checkSelfIntersectingRing(e->getEdgeIntersectionList());
		if(validErr!=NULL) return;
	}
}

/**
* check that a ring does not self-intersect, except at its endpoints.
* Algorithm is to count the number of times each node along edge occurs.
* If any occur more than once, that must be a self-intersection.
*/
void IsValidOp::checkSelfIntersectingRing(EdgeIntersectionList *eiList) {
	set<Coordinate,CoordLT> *nodeSet=new set<Coordinate,CoordLT>();
	bool isFirst=true;
	vector<EdgeIntersection*> *l=eiList->list;
	for(int i=0;i<(int)l->size();i++) {
		EdgeIntersection *ei=(*l)[i];
		if (isFirst) {
			isFirst=false;
			continue;
		}
		if (nodeSet->find(ei->coord)!=nodeSet->end()) {
			validErr=new TopologyValidationError(
				TopologyValidationError::RING_SELF_INTERSECTION,
				ei->coord);
			delete nodeSet;
			return;
		} else {
			nodeSet->insert(ei->coord);
		}
	}
	delete nodeSet;
}

/* NO LONGER NEEDED AS OF JTS Ver 1.2
void IsValidOp::checkNoRepeatedPoint(Geometry *g) {
	RepeatedPointTester *rpt=new RepeatedPointTester();
	if (rpt->hasRepeatedPoint(g)) {
		validErr=new TopologyValidationError(
			TopologyValidationError::REPEATED_POINT,
			rpt->getCoordinate());
	}
}
*/

/**
* Test that each hole is inside the polygon shell.
* This routine assumes that the holes have previously been tested
* to ensure that all vertices lie on the shell or inside it.
* A simple test of a single point in the hole can be used,
* provide the point is chosen such that it does not lie on the
* boundary of the shell.
* @param p the polygon to be tested for hole inclusion
* @param graph a GeometryGraph incorporating the polygon
*/
void IsValidOp::checkHolesInShell(const Polygon *p,GeometryGraph *graph) {
	LinearRing *shell=(LinearRing*) p->getExteriorRing();
	const CoordinateSequence *shellPts=shell->getCoordinatesRO();
	//PointInRing pir=new SimplePointInRing(shell);
	//PointInRing pir=new SIRtreePointInRing(shell);
//	auto_ptr<PointInRing> pir(new MCPointInRing(shell));
	auto_ptr<PointInRing> pir(new MCPointInRing(shell));
	for(int i=0;i<p->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*) p->getInteriorRingN(i);
		const Coordinate& holePt=findPtNotNode(hole->getCoordinatesRO(),shell,graph);
		Assert::isTrue(!(holePt==Coordinate::getNull()), "Unable to find a hole point not a vertex of the shell");
		bool outside=!pir->isInside(holePt);
		if (outside) {
			validErr=new TopologyValidationError(
				TopologyValidationError::HOLE_OUTSIDE_SHELL,
				holePt);
			return;
		}
	}
}

/**
* Tests that no hole is nested inside another hole.
* This routine assumes that the holes are disjoint.
* To ensure this, holes have previously been tested
* to ensure that:
* <ul>
* <li>they do not partially overlap
* (checked by <code>checkRelateConsistency</code>)
* <li>they are not identical
* (checked by <code>checkRelateConsistency</code>)
* </ul>
*/
void IsValidOp::checkHolesNotNested(const Polygon *p,GeometryGraph *graph) {
	auto_ptr<QuadtreeNestedRingTester> nestedTester(new QuadtreeNestedRingTester(graph));
	//SimpleNestedRingTester nestedTester=new SimpleNestedRingTester(arg[0]);
	//SweeplineNestedRingTester nestedTester=new SweeplineNestedRingTester(arg[0]);
	for(int i=0;i<p->getNumInteriorRing();i++) {
		LinearRing *innerHole=(LinearRing*) p->getInteriorRingN(i);
		nestedTester->add(innerHole);
	}
	bool isNonNested=nestedTester->isNonNested();
	if (!isNonNested) {
		validErr=new TopologyValidationError(
			TopologyValidationError::NESTED_HOLES,
			nestedTester->getNestedPoint());
	}
}

/*
 * Tests that no element polygon is wholly in the interior of another
 * element polygon.
 * 
 * Preconditions:
 * 
 * - shells do not partially overlap
 * - shells do not touch along an edge
 * - no duplicate rings exist
 *
 * This routine relies on the fact that while polygon shells may touch at
 * one or more vertices, they cannot touch at ALL vertices.
 */
void
IsValidOp::checkShellsNotNested(const MultiPolygon *mp,GeometryGraph *graph)
{
	for(int i=0;i<mp->getNumGeometries();i++) {
		Polygon *p=(Polygon*)mp->getGeometryN(i);
		LinearRing *shell=(LinearRing*) p->getExteriorRing();
		for(int j=0;j<mp->getNumGeometries();j++) {
			if (i==j) continue;
			Polygon *p2=(Polygon*) mp->getGeometryN(j);
			checkShellNotNested(shell,p2,graph);
			if (validErr!=NULL) return;
		}
	}
}

/**
* Check if a shell is incorrectly nested within a polygon.  This is the case
* if the shell is inside the polygon shell, but not inside a polygon hole.
* (If the shell is inside a polygon hole, the nesting is valid.)
* <p>
* The algorithm used relies on the fact that the rings must be properly contained.
* E.g. they cannot partially overlap (this has been previously checked by
* <code>checkRelateConsistency</code>
*/
void IsValidOp::checkShellNotNested(const LinearRing *shell, const Polygon *p,GeometryGraph *graph) {
	const CoordinateSequence *shellPts=shell->getCoordinatesRO();
	// test if shell is inside polygon shell
	LinearRing *polyShell=(LinearRing*) p->getExteriorRing();
	const CoordinateSequence *polyPts=polyShell->getCoordinatesRO();
	const Coordinate& shellPt=findPtNotNode(shellPts,polyShell,graph);
	// if no point could be found, we can assume that the shell is outside the polygon
	if (shellPt==Coordinate::getNull())
		return;
	bool insidePolyShell=CGAlgorithms::isPointInRing(shellPt,polyPts);
	if (!insidePolyShell) return;
	// if no holes, this is an error!
	if (p->getNumInteriorRing()<=0) {
		validErr=new TopologyValidationError(
			TopologyValidationError::NESTED_SHELLS,
			shellPt);
		return;
	}
	
	/**
	 * Check if the shell is inside one of the holes.
	 * This is the case if one of the calls to checkShellInsideHole
	 * returns a null coordinate.
	 * Otherwise, the shell is not properly contained in a hole, which is
	 * an error.
	 */
	Coordinate& badNestedPt=Coordinate::getNull();
	for(int i=0; i<p->getNumInteriorRing(); i++) {
		LinearRing *hole=(LinearRing*) p->getInteriorRingN(i);
		badNestedPt = checkShellInsideHole(shell, hole, graph);
		if (badNestedPt==Coordinate::getNull()) return;
	}
	validErr=new TopologyValidationError(
		TopologyValidationError::NESTED_SHELLS, badNestedPt
	);
}

/*
 * This routine checks to see if a shell is properly contained in a hole.
 * It assumes that the edges of the shell and hole do not
 * properly intersect.
 *
 * @return <code>null</code> if the shell is properly contained, or
 *   a Coordinate which is not inside the hole if it is not
 *
 */
const Coordinate&
IsValidOp::checkShellInsideHole(const LinearRing *shell, const LinearRing *hole, GeometryGraph *graph)
{
	const CoordinateSequence *shellPts=shell->getCoordinatesRO();
	const CoordinateSequence *holePts=hole->getCoordinatesRO();
	// TODO: improve performance of this - by sorting pointlists for instance?
	const Coordinate& shellPt=findPtNotNode(shellPts,hole,graph);
	// if point is on shell but not hole, check that the shell is inside the hole
	if (!(shellPt==Coordinate::getNull())) {
		bool insideHole=CGAlgorithms::isPointInRing(shellPt,holePts);
		if (!insideHole)
			return shellPt;
	}
	const Coordinate& holePt=findPtNotNode(holePts,shell,graph);
	// if point is on hole but not shell, check that the hole is outside the shell
	if(!(holePt==Coordinate::getNull())) {
		bool insideShell=CGAlgorithms::isPointInRing(holePt,shellPts);
		if (insideShell) {
			return holePt;
		}
		return Coordinate::getNull();
	}
	Assert::shouldNeverReachHere("points in shell and hole appear to be equal");
	return Coordinate::getNull();
}

void IsValidOp::checkConnectedInteriors(GeometryGraph *graph) {
	auto_ptr<ConnectedInteriorTester> cit(new ConnectedInteriorTester(graph));
	if (!cit->isInteriorsConnected())
		validErr=new TopologyValidationError(
		TopologyValidationError::DISCONNECTED_INTERIOR,
		cit->getCoordinate());
}


void
IsValidOp::checkInvalidCoordinates(const CoordinateSequence *cs)
{
	for (int i = 0; i<cs->getSize(); i++)
	{
		if (! isValid(cs->getAt(i)) )
		{
			validErr = new TopologyValidationError(
				TopologyValidationError::INVALID_COORDINATE,
				cs->getAt(i));
			return;

		}
	}
}

void
IsValidOp::checkInvalidCoordinates(const Polygon *poly)
{
	checkInvalidCoordinates(poly->getExteriorRing()->getCoordinatesRO());
	if (validErr != NULL) return;
	for (int i=0; i<poly->getNumInteriorRing(); i++)
	{
		checkInvalidCoordinates(
			poly->getInteriorRingN(i)->getCoordinatesRO()
		);
		if (validErr != NULL) return;
	}
}


}

