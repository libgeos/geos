#include "opValid.h"
#include "stdio.h"
#include <typeinfo>
#include <set>
#include "util.h"

Coordinate& IsValidOp::findPtNotNode(CoordinateList *testCoords,LinearRing *searchRing, GeometryGraph *graph) {
	// find edge corresponding to searchRing.
	Edge *searchEdge=graph->findEdge(searchRing);
	// find a point in the testCoords which is not a node of the searchRing
	EdgeIntersectionList *eiList=searchEdge->getEdgeIntersectionList();
	// somewhat inefficient - is there a better way? (Use a node map, for instance?)
	for(int i=0;i<testCoords->getSize(); i++) {
		Coordinate& pt=testCoords->getAt(i);
		if (!eiList->isIntersection(pt)) {
			return pt;
		}
	}
	return Coordinate::getNull();
}

IsValidOp::IsValidOp(Geometry *g): GeometryGraphOperation(g) {
	isChecked=false;
	validErr=NULL;
}

bool IsValidOp::isValid() {
	checkValid(getArgGeometry(0));
	return validErr==NULL;
}

TopologyValidationError* IsValidOp::getValidationError() {
	checkValid(getArgGeometry(0));
	return validErr;
}

void IsValidOp::checkValid(Geometry *g) {
    if (isChecked) return;
    validErr=NULL;
    if (g->isEmpty()) return;
    if (typeid(*g)==typeid(Point)) return;
    else if (typeid(*g)==typeid(MultiPoint)) return;
	// LineString also handles LinearRings
    else if (typeid(*g)==typeid(LineString)) checkValid((LineString*)g);
    else if (typeid(*g)==typeid(Polygon)) checkValid((Polygon*)g);
    else if (typeid(*g)==typeid(MultiPolygon)) checkValid((MultiPolygon*)g);
    else if (typeid(*g)==typeid(GeometryCollection)) checkValid((GeometryCollection*)g);
	else throw "UnsupportedOperationException";
}

void IsValidOp::checkValid(LineString *g){
	checkNoRepeatedPoint(g);
}

void IsValidOp::checkValid(Polygon *g){
	checkNoRepeatedPoint(g);
	if (validErr!=NULL) return;
	checkConsistentArea();
	if (validErr!=NULL) return;
	checkNoSelfIntersectingRings((*arg)[0]);
	if (validErr!=NULL) return;
	checkHolesInShell(g);
	if (validErr!=NULL) return;
	//SLOWcheckHolesNotNested(g);
	checkHolesNotNested(g);
	if (validErr!=NULL) return;
	checkConnectedInteriors();
}

void IsValidOp::checkValid(MultiPolygon *g){
	checkNoRepeatedPoint(g);
	if (validErr!=NULL) return;
	checkConsistentArea();
	if (validErr!=NULL) return;
	checkNoSelfIntersectingRings((*arg)[0]);
	if (validErr!=NULL) return;

	for(int i=0;i<g->getNumGeometries();i++) {
		Polygon *p=(Polygon*)g->getGeometryN(i);
		checkHolesInShell(p);
		if (validErr!=NULL) return;
	}
	for(int i=0;i<g->getNumGeometries();i++) {
		Polygon *p=(Polygon*)g->getGeometryN(i);
		//checkDisjointHolesNotNested(p);
		checkHolesNotNested(p);
		if (validErr!=NULL) return;
	}
	checkShellsNotNested(g);
	if (validErr!=NULL) return;
	checkConnectedInteriors();
}

void IsValidOp::checkValid(GeometryCollection *gc) {
	for(int i=0;i<gc->getNumGeometries();i++) {
		Geometry *g=gc->getGeometryN(i);
		checkValid(g);
		if (validErr!=NULL) return;
	}
}

void IsValidOp::checkConsistentArea() {
	ConsistentAreaTester *cat=new ConsistentAreaTester((*arg)[0]);
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
			return;
		} else {
			nodeSet->insert(ei->coord);
		}
	}
}

void IsValidOp::checkNoRepeatedPoint(Geometry *g) {
	RepeatedPointTester *rpt=new RepeatedPointTester();
	if (rpt->hasRepeatedPoint(g)) {
		validErr=new TopologyValidationError(
			TopologyValidationError::REPEATED_POINT,
			rpt->getCoordinate());
	}
}

/**
* Test that each hole is inside the polygon shell.
* This routine assumes that the holes have previously been tested
* to ensure that all vertices lie on the shell or inside it.
* A simple test of a single point in the hole can be used,
* provide the point is chosen such that it does not lie on the
* boundary of the shell.
*/
void IsValidOp::checkHolesInShell(Polygon *p) {
	LinearRing *shell=(LinearRing*) p->getExteriorRing();
	CoordinateList *shellPts=shell->getCoordinates();
	//PointInRing pir=new SimplePointInRing(shell);
	//PointInRing pir=new IntTreePointInRing(shell);
	PointInRing *pir=new MCPointInRing(shell);
	for(int i=0;i<p->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*) p->getInteriorRingN(i);
		Coordinate& holePt=findPtNotNode(hole->getCoordinates(),shell,(*arg)[0]);
//		Assert::isTrue(!(holePt==Coordinate::getNull()), "Unable to find a hole point not a vertex of the shell");
		//OLD Coordinate holePt=hole.getCoordinateN(0);
		bool outside=!pir->isInside(holePt);
		if (outside) {
			validErr=new TopologyValidationError(
				TopologyValidationError::HOLE_OUTSIDE_SHELL,
				holePt);
			return;
		}
	}
}

void IsValidOp::OLDcheckHolesInShell(Polygon *p) {
	LinearRing *shell=(LinearRing*) p->getExteriorRing();
	CoordinateList *shellPts=shell->getCoordinates();
	for(int i=0;i<p->getNumInteriorRing();i++) {
		Coordinate& holePt=findPtNotNode(p->getInteriorRingN(i)->getCoordinates(),shell,(*arg)[0]);
		Assert::isTrue(!(holePt==Coordinate::getNull()),"Unable to find a hole point not a vertex of the shell");
		bool onBdy=cga->isOnLine(holePt,shellPts);
		bool inside=cga->isPointInRing(holePt,shellPts);
		bool outside=!(onBdy||inside);
		if(outside) {
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
* <li>they do not touch at a vertex
* (checked by <code>????</code>)
* </ul>
*/
void IsValidOp::checkHolesNotNested(Polygon *p) {
	QuadtreeNestedRingTester *nestedTester=new QuadtreeNestedRingTester((*arg)[0]);
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

void IsValidOp::SLOWcheckHolesNotNested(Polygon *p) {
	for(int i=0;i<p->getNumInteriorRing();i++) {
		LinearRing *innerHole=(LinearRing*) p->getInteriorRingN(i);
		CoordinateList *innerHolePts=innerHole->getCoordinates();
		for(int j=0;j<p->getNumInteriorRing();j++) {
			// don't test hole against itself!
			if (i==j) continue;
			LinearRing *searchHole=(LinearRing*) p->getInteriorRingN(j);
			// if envelopes don't overlap, holes are not nested
			if (!innerHole->getEnvelopeInternal()->overlaps(searchHole->getEnvelopeInternal()))
				continue;
			CoordinateList *searchHolePts=searchHole->getCoordinates();
			Coordinate& innerholePt=findPtNotNode(innerHolePts,searchHole,(*arg)[0]);
			Assert::isTrue(!(innerholePt==Coordinate::getNull()),"Unable to find a hole point not a node of the search hole");
			bool inside=cga->isPointInRing(innerholePt,searchHolePts);
			if (inside) {
				validErr=new TopologyValidationError(
					TopologyValidationError::NESTED_HOLES,
					innerholePt);
				return;
			}
		}
	}
}
/**
* Test that no element polygon is wholly in the interior of another element polygon.
* TODO: It handles the case that one polygon is nested inside a hole of another.
* <p>
* Preconditions:
* <ul>
* <li>shells do not partially overlap
* <li>shells do not touch along an edge
* <li>no duplicate rings exist
* </ul>
* This routine relies on the fact that while polygon shells may touch at one or
* more vertices, they cannot touch at ALL vertices.
*/
void IsValidOp::checkShellsNotNested(MultiPolygon *mp) {
	for(int i=0;i<mp->getNumGeometries();i++) {
		Polygon *p=(Polygon*)mp->getGeometryN(i);
		LinearRing *shell=(LinearRing*) p->getExteriorRing();
		for(int j=0;j<mp->getNumGeometries();j++) {
			if (i==j) continue;
			Polygon *p2=(Polygon*) mp->getGeometryN(j);
			checkShellNotNested(shell,p2);
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
void IsValidOp::checkShellNotNested(LinearRing *shell,Polygon *p) {
	CoordinateList *shellPts=shell->getCoordinates();
	// test if shell is inside polygon shell
	LinearRing *polyShell=(LinearRing*) p->getExteriorRing();
	CoordinateList *polyPts=polyShell->getCoordinates();
	Coordinate& shellPt=findPtNotNode(shellPts,polyShell,(*arg)[0]);
	// if no point could be found, we can assume that the shell is outside the polygon
	if (shellPt==Coordinate::getNull())
		return;
	bool insidePolyShell=cga->isPointInRing(shellPt,polyPts);
	if (!insidePolyShell) return;
	// if no holes, this is an error!
	if (p->getNumInteriorRing()<=0) {
		validErr=new TopologyValidationError(
			TopologyValidationError::NESTED_SHELLS,
			shellPt);
		return;
	}
	for(int i=0;i<p->getNumInteriorRing();i++) {
		LinearRing *hole=(LinearRing*) p->getInteriorRingN(i);
		checkShellInsideHole(shell,hole);
		if (validErr!=NULL) return;
	}
}

/**
* This routine checks to see if a shell is properly contained in a hole.
*/
void IsValidOp::checkShellInsideHole(LinearRing *shell,LinearRing *hole) {
	CoordinateList *shellPts=shell->getCoordinates();
	CoordinateList *holePts=hole->getCoordinates();
	// TODO: improve performance of this - by sorting pointlists for instance?
	Coordinate& shellPt=findPtNotNode(shellPts,hole,(*arg)[0]);
	// if point is on shell but not hole, check that the shell is inside the hole
	if (!(shellPt==Coordinate::getNull())) {
		bool insideHole=cga->isPointInRing(shellPt,holePts);
		if (!insideHole)
			validErr=new TopologyValidationError(
				TopologyValidationError::NESTED_SHELLS,
				shellPt);
		return;
	}
	Coordinate& holePt=findPtNotNode(holePts,shell,(*arg)[0]);
	// if point is on hole but not shell, check that the hole is outside the shell
	if(!(holePt==Coordinate::getNull())) {
		bool insideShell=cga->isPointInRing(holePt,shellPts);
		if (insideShell) {
			validErr=new TopologyValidationError(
				TopologyValidationError::NESTED_SHELLS,
				holePt);
		}
		return;
	}
	Assert::shouldNeverReachHere("points in shell and hole appear to be equal");
}

void IsValidOp::checkConnectedInteriors() {
	ConnectedInteriorTester *cit=new ConnectedInteriorTester((*arg)[0]);
	if (!cit->isInteriorsConnected())
		validErr=new TopologyValidationError(
		TopologyValidationError::DISCONNECTED_INTERIOR,
		cit->getCoordinate());
}
